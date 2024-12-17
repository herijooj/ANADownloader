import requests
import pandas as pd
import os
import json
import logging
import time
from datetime import datetime, timedelta
import xml.etree.ElementTree as ET
from concurrent.futures import ThreadPoolExecutor, as_completed
from requests.adapters import HTTPAdapter
from requests.packages.urllib3.util.retry import Retry
from typing import Tuple, List
import csv
from pathlib import Path

# =======================
# Configuration Section
# =======================
CONFIG = {
    'data_inicio': "1927-01-01",
    'data_fim': "2024-12-31",
    'output_dir': 'dados_estacoes_ALL',
    'max_workers': 50,
    'min_workers': 2,
    'initial_workers': 10,
    'workers_adjustment_interval': 100,  # Ajustar workers a cada 100 requisições
    'workers_success_threshold': 0.9,    # 90% de sucesso para aumentar
    'workers_failure_threshold': 0.2,    # 20% de falha para diminuir
    'max_retries': 8,                    # Aumentado número de tentativas
    'backoff_factor': 5,                 # Backoff mais agressivo
    'timeout': {
        'connect': 30,    # Timeout para conexão
        'read': 60,      # Timeout para leitura
        'total': 120     # Timeout total da operação
    },
    'retry_timeouts': {
        'connect': 45,   # Timeout para retry de conexão
        'read': 90,      # Timeout para retry de leitura
        'backoff_factor': 10,  # Fator de backoff mais agressivo para timeouts
        'max_retries': 8
    },
    'initial_rate_limit_pause': 5,       # Pausa inicial maior
    'rate_limit_min_pause': 3,           # Pausa mínima maior
    'rate_limit_max_pause': 30,          # Pausa máxima maior
    'adjustment_interval': 50,           # Adjust rate limiting every 50 requests
    'success_threshold_for_decrease': 0.9,  # If 90% or more are successful, try decreasing pause
    'failure_threshold_for_increase': 0.2,  # If 20% or more fail, increase pause
    'data_tipos': {
        'chuvas': '2',
        'cotas': '1',
        'vazoes': '3'
    },
    'estacoes_file': "all.csv",  # File containing the list of stations
    'log_file': 'downloadALL_ana.log',
    'base_url': "http://telemetriaws1.ana.gov.br/ServiceANA.asmx/HidroSerieHistorica",
    'pool_connections': 100,     # Tamanho do pool de conexões
    'pool_maxsize': 100,        # Máximo de conexões permitidas
    'circuit_breaker_threshold': 5,      # Número de 429s antes de pausar tudo
    'circuit_breaker_timeout': 60,       # Tempo de pausa quando circuito abrir
}

# =======================
# Logging Configuration
# =======================
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler(CONFIG['log_file']),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

class CircuitBreaker:
    def __init__(self, threshold: int, timeout: int):
        self.threshold = threshold
        self.timeout = timeout
        self.failures = 0
        self.last_failure_time = None
        self.is_open = False

    def record_failure(self):
        self.failures += 1
        self.last_failure_time = time.time()
        if self.failures >= self.threshold:
            self.is_open = True
            logger.warning(f"Circuit breaker ativado - pausando por {self.timeout} segundos")
            time.sleep(self.timeout)
            self.reset()

    def reset(self):
        self.failures = 0
        self.is_open = False
        self.last_failure_time = None

class DynamicRateLimiter:
    """
    A simple dynamic rate limiter that adjusts the pause time based on request successes and failures.
    """
    def __init__(self, initial_pause: float, min_pause: float, max_pause: float, 
                 adjustment_interval: int, success_threshold: float, failure_threshold: float):
        self.pause = initial_pause
        self.min_pause = min_pause
        self.max_pause = max_pause
        self.adjustment_interval = adjustment_interval
        self.success_threshold = success_threshold
        self.failure_threshold = failure_threshold
        self.reset_metrics()
        self.consecutive_429s = 0

    def reset_metrics(self):
        self.request_count = 0
        self.success_count = 0
        self.failure_count = 0

    def record_result(self, success: bool):
        self.request_count += 1
        if success:
            self.success_count += 1
        else:
            self.failure_count += 1

        # Adjust parameters after a certain number of requests
        if self.request_count >= self.adjustment_interval:
            self.adjust_pause()

    def adjust_pause(self):
        if self.request_count == 0:
            return  # No data to base adjustments on

        success_rate = self.success_count / self.request_count
        failure_rate = self.failure_count / self.request_count

        # If too many failures, increase pause
        if failure_rate >= self.failure_threshold_for_increase:
            self.pause = min(self.pause * 2, self.max_pause)  # Multiplicar por 2 em vez de adicionar 1
            logger.info(f"Increasing pause exponentially due to high failure rate ({failure_rate*100:.1f}%): {self.pause}s")

        # If very high success, try decreasing pause
        elif success_rate >= self.success_threshold_for_decrease:
            self.pause = max(self.pause * 0.8, self.min_pause)  # Redução mais conservadora
            logger.info(f"Carefully decreasing pause ({success_rate*100:.1f}%): {self.pause}s")

        # Reset metrics after adjusting
        self.reset_metrics()

    @property
    def failure_threshold_for_increase(self):
        return self.failure_threshold

    @property
    def success_threshold_for_decrease(self):
        return self.success_threshold

    def wait(self):
        time.sleep(self.pause)


class DynamicWorkerManager:
    """Gerencia o número de workers de forma dinâmica baseado no desempenho"""
    def __init__(self, initial_workers: int, min_workers: int, max_workers: int,
                 adjustment_interval: int, success_threshold: float, failure_threshold: float):
        self.current_workers = initial_workers
        self.min_workers = min_workers
        self.max_workers = max_workers
        self.adjustment_interval = adjustment_interval
        self.success_threshold = success_threshold
        self.failure_threshold = failure_threshold
        self.reset_metrics()

    def reset_metrics(self):
        self.request_count = 0
        self.success_count = 0
        self.failure_count = 0

    def record_result(self, success: bool):
        self.request_count += 1
        if success:
            self.success_count += 1
        else:
            self.failure_count += 1

        if self.request_count >= self.adjustment_interval:
            self.adjust_workers()

    def adjust_workers(self):
        if self.request_count == 0:
            return

        success_rate = self.success_count / self.request_count
        failure_rate = self.failure_count / self.request_count

        if failure_rate >= self.failure_threshold:
            self.current_workers = max(self.current_workers - 1, self.min_workers)
            logger.info(f"Diminuindo workers devido à alta taxa de falha ({failure_rate*100:.1f}%): {self.current_workers}")
        elif success_rate >= self.success_threshold:
            self.current_workers = min(self.current_workers + 1, self.max_workers)
            logger.info(f"Aumentando workers devido à alta taxa de sucesso ({success_rate*100:.1f}%): {self.current_workers}")

        self.reset_metrics()

    @property
    def workers(self):
        return self.current_workers

class ANADownloader:
    def __init__(self, config: dict):
        self.config = config
        self.session = self._create_session()
        self.rate_limiter = DynamicRateLimiter(
            initial_pause=self.config['initial_rate_limit_pause'],
            min_pause=self.config['rate_limit_min_pause'],
            max_pause=self.config['rate_limit_max_pause'],
            adjustment_interval=self.config['adjustment_interval'],
            success_threshold=self.config['success_threshold_for_decrease'],
            failure_threshold=self.config['failure_threshold_for_increase']
        )
        self.worker_manager = DynamicWorkerManager(
            initial_workers=self.config['initial_workers'],
            min_workers=self.config['min_workers'],
            max_workers=self.config['max_workers'],
            adjustment_interval=self.config['workers_adjustment_interval'],
            success_threshold=self.config['workers_success_threshold'],
            failure_threshold=self.config['workers_failure_threshold']
        )
        self.circuit_breaker = CircuitBreaker(
            threshold=config['circuit_breaker_threshold'],
            timeout=config['circuit_breaker_timeout']
        )
        self.estacoes_falha = []
        self.checkpoint_file = Path(self.config['output_dir']) / 'checkpoint.csv'
        self.falhas_file = Path(self.config['output_dir']) / 'estacoes_falha.csv'
        self.processed_stations = self._load_checkpoint()

    def _create_session(self) -> requests.Session:
        session = requests.Session()
        retry_strategy = Retry(
            total=self.config['retry_timeouts']['max_retries'],
            backoff_factor=self.config['retry_timeouts']['backoff_factor'],
            status_forcelist=[429, 500, 502, 503, 504],
            respect_retry_after_header=True,
            allowed_methods=["GET"],
            raise_on_status=True,
            # Força espera maior para 429
            backoff_jitter=1.0,
            connect=3,  # Tentativas específicas para erros de conexão
            read=3     # Tentativas específicas para erros de leitura
        )
        adapter = HTTPAdapter(
            max_retries=retry_strategy,
            pool_connections=self.config['pool_connections'],
            pool_maxsize=self.config['pool_maxsize']
        )
        session.mount("http://", adapter)
        session.mount("https://", adapter)
        return session

    def fetch_data(self, codigo: str) -> Tuple[bool, str]:
        if self.circuit_breaker.is_open:
            return False, "Circuit breaker ativo - muitas falhas 429"

        params = {
            'codEstacao': codigo,
            'dataInicio': self.config['data_inicio'],
            'dataFim': self.config['data_fim'],
            'tipoDados': '2',  # This could also be configurable if needed
            'nivelConsistencia': ''
        }

        try:
            logger.info(f"Baixando dados para estação {codigo} - período: {self.config['data_inicio']} a {self.config['data_fim']}")
            response = self.session.get(
                self.config['base_url'],
                params=params,
                timeout=(
                    self.config['timeout']['connect'],
                    self.config['timeout']['read']
                )
            )
            response.raise_for_status()

            # Validate XML
            try:
                ET.fromstring(response.content)
            except ET.ParseError:
                raise ValueError("XML malformado recebido da API")

            self._save_response(response, codigo)
            return True, ""

        except requests.exceptions.ReadTimeout:
            erro_msg = f"Timeout de leitura para estação {codigo}"
            logger.warning(erro_msg)
            # Aumentar o timeout de leitura para próximas tentativas
            self.config['timeout']['read'] = min(
                self.config['timeout']['read'] * 1.5,
                self.config['retry_timeouts']['read']
            )
            return False, erro_msg
        except requests.exceptions.ConnectTimeout:
            erro_msg = f"Timeout de conexão para estação {codigo}"
            logger.warning(erro_msg)
            # Aumentar o timeout de conexão para próximas tentativas
            self.config['timeout']['connect'] = min(
                self.config['timeout']['connect'] * 1.5,
                self.config['retry_timeouts']['connect']
            )
            return False, erro_msg
        except requests.exceptions.RequestException as e:
            erro_msg = f"Erro na requisição: {str(e)}"
            if "429" in str(e):
                self.circuit_breaker.record_failure()
                self.rate_limiter.pause = min(self.rate_limiter.pause * 2, self.rate_limiter.max_pause)
            logger.error(erro_msg)
            return False, erro_msg
        except Exception as e:
            erro_msg = f"Erro inesperado: {str(e)}"
            logger.error(erro_msg)
            return False, erro_msg

    def _save_response(self, response: requests.Response, codigo: str) -> str:
        # Create output filename
        output_file = os.path.join(
            self.config['output_dir'], 
            f"{codigo}_{self.config['data_inicio'].replace('-', '')}_{self.config['data_fim'].replace('-', '')}.xml"
        )

        os.makedirs(self.config['output_dir'], exist_ok=True)

        with open(output_file, 'wb') as f:
            f.write(response.content)

        logger.info(f"Arquivo salvo: {output_file}")
        return output_file

    def _load_checkpoint(self) -> set:
        """Carrega estações já processadas do checkpoint"""
        processed = set()
        if self.checkpoint_file.exists():
            with open(self.checkpoint_file, 'r') as f:
                processed = set(line.strip() for line in f)
        return processed

    def _save_checkpoint(self, codigo: str):
        """Salva estação processada no checkpoint"""
        with open(self.checkpoint_file, 'a') as f:
            f.write(f"{codigo}\n")

    def _save_failure(self, codigo: str, erro: str):
        """Salva falha incrementalmente"""
        mode = 'a' if self.falhas_file.exists() else 'w'
        with open(self.falhas_file, mode, newline='') as f:
            writer = csv.writer(f)
            if mode == 'w':
                writer.writerow(['codigo', 'erro'])
            writer.writerow([codigo, erro])

    def _format_duration(self, seconds: float) -> str:
        """Formata duração em segundos para formato legível"""
        duration = timedelta(seconds=seconds)
        hours = duration.seconds // 3600
        minutes = (duration.seconds % 3600) // 60
        seconds = duration.seconds % 60
        return f"{hours:02d}:{minutes:02d}:{seconds:02d}"

    def process_stations(self, codigos: List[str]):
        codigos_pendentes = [cod for cod in codigos if cod not in self.processed_stations]

        if not codigos_pendentes:
            logger.info("Todas as estações já foram processadas!")
            return

        logger.info(f"Processando {len(codigos_pendentes)} estações pendentes de {len(codigos)} total")
        
        total_start_time = time.time()
        batch_count = 0

        while codigos_pendentes:
            current_workers = self.worker_manager.workers
            batch = codigos_pendentes[:current_workers]
            batch_count += 1
            
            batch_start_time = time.time()
            logger.info(f"Iniciando lote {batch_count} com {len(batch)} estações...")

            with ThreadPoolExecutor(max_workers=current_workers) as executor:
                futures = {
                    executor.submit(self.fetch_data, codigo): codigo for codigo in batch
                }

                success_count = 0
                for future in as_completed(futures):
                    codigo = futures[future]
                    success, error = future.result()
                    
                    self.rate_limiter.record_result(success)
                    self.worker_manager.record_result(success)
                    
                    if success:
                        success_count += 1
                    else:
                        self._save_failure(codigo, error)
                        self.estacoes_falha.append({
                            'codigo': codigo,
                            'erro': error
                        })

                    self._save_checkpoint(codigo)
                    self.processed_stations.add(codigo)
                    self.rate_limiter.wait()

            batch_duration = time.time() - batch_start_time
            total_duration = time.time() - total_start_time
            
            logger.info(
                f"Lote {batch_count} concluído em {self._format_duration(batch_duration)} | "
                f"Sucesso: {success_count}/{len(batch)} | "
                f"Tempo total até agora: {self._format_duration(total_duration)} | "
                f"Restantes: {len(codigos_pendentes[current_workers:])} estações"
            )

            # Remover estações processadas da lista pendente
            codigos_pendentes = codigos_pendentes[current_workers:]
            # Pausa extra entre lotes
            self.rate_limiter.wait()

        total_duration = time.time() - total_start_time
        logger.info(f"Tempo total de processamento: {self._format_duration(total_duration)}")

def main():
    # Carrega lista de estações
    try:
        df = pd.read_csv(CONFIG['estacoes_file'], header=None)
        codigos = df.iloc[:, 0].astype(str).tolist()
    except Exception as e:
        logger.error(f"Erro ao ler arquivo de estações: {e}")
        return

    try:
        downloader = ANADownloader(CONFIG)
        downloader.process_stations(codigos=codigos)

        total_processado = len(downloader.processed_stations)
        total_falhas = len(downloader.estacoes_falha)
        logger.info(
            f"Download concluído. "
            f"Processadas: {total_processado}/{len(codigos)} estações. "
            f"Sucesso: {total_processado - total_falhas}, Falhas: {total_falhas}"
        )

    except KeyboardInterrupt:
        logger.info("Processo interrompido pelo usuário. Progresso foi salvo.")
    except Exception as e:
        logger.error(f"Erro durante execução: {e}")
        raise

if __name__ == "__main__":
    main()
