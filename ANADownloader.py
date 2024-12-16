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
from typing import Tuple, List, Dict
import csv
from pathlib import Path

# Configurar logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('download_ana.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

class ConfigANA:
    def __init__(self, config_file: str = None):
        self.config = self._load_config()
        
    def _load_config(self) -> dict:
        return {
            'max_workers': 5,  # Reduzido de 10 para 5
            'max_retries': 5,  # Aumentado de 3 para 5
            'backoff_factor': 2,  # Aumentado de 1 para 2
            'timeout': 45,  # Aumentado de 30 para 45
            'rate_limit_pause': 3,  # Aumentado de 1 para 3
            'output_dir': 'dados_estacoes',
            'data_tipos': {
                'chuvas': '2',
                'cotas': '1',
                'vazoes': '3'
            }
        }

class ANADownloader:
    def __init__(self, config: ConfigANA):
        self.config = config
        self.session = self._create_session()
        self.estacoes_falha = []
        self.checkpoint_file = Path(self.config.config['output_dir']) / 'checkpoint.csv'
        self.falhas_file = Path(self.config.config['output_dir']) / 'estacoes_falha.csv'
        self.processed_stations = self._load_checkpoint()
        
    def _create_session(self) -> requests.Session:
        session = requests.Session()
        retry_strategy = Retry(
            total=self.config.config['max_retries'],
            backoff_factor=self.config.config['backoff_factor'],
            status_forcelist=[429, 500, 502, 503, 504],
            respect_retry_after_header=True
        )
        adapter = HTTPAdapter(max_retries=retry_strategy)
        session.mount("http://", adapter)
        session.mount("https://", adapter)
        return session
    
    def fetch_data(self, codigo: str, data_inicio: str, data_fim: str) -> Tuple[bool, str]:
        base_url = "http://telemetriaws1.ana.gov.br/ServiceANA.asmx/HidroSerieHistorica"
        params = {
            'codEstacao': codigo,
            'dataInicio': data_inicio,
            'dataFim': data_fim,
            'tipoDados': '2',
            'nivelConsistencia': ''
        }
        
        try:
            logger.info(f"Baixando dados para estação {codigo} - período: {data_inicio} a {data_fim}")
            response = self.session.get(
                base_url, 
                params=params, 
                timeout=self.config.config['timeout']
            )
            response.raise_for_status()
            
            # Validar se o XML está bem formado
            try:
                ET.fromstring(response.content)
            except ET.ParseError:
                raise ValueError("XML malformado recebido da API")
                
            output_file = self._save_response(response, codigo, data_inicio, data_fim)
            return True, ""
            
        except requests.exceptions.RequestException as e:
            erro_msg = f"Erro na requisição: {str(e)}"
            logger.error(erro_msg)
            return False, erro_msg
        except Exception as e:
            erro_msg = f"Erro inesperado: {str(e)}"
            logger.error(erro_msg)
            return False, erro_msg
            
    def _save_response(self, response: requests.Response, codigo: str, data_inicio: str, data_fim: str) -> str:
        # Cria prefixo com período (data_inicio - data_fim)
        output_file = os.path.join(
            self.config.config['output_dir'], 
            f"{codigo}_{data_inicio.replace('-', '')}_{data_fim.replace('-', '')}.xml"
        )
        
        os.makedirs(self.config.config['output_dir'], exist_ok=True)
        
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
            
    def process_stations(self, codigos: List[str], data_inicio: str, data_fim: str):
        # Filtra estações já processadas
        codigos_pendentes = [cod for cod in codigos if cod not in self.processed_stations]
        
        if not codigos_pendentes:
            logger.info("Todas as estações já foram processadas!")
            return
            
        logger.info(f"Processando {len(codigos_pendentes)} estações pendentes de {len(codigos)} total")
        
        with ThreadPoolExecutor(max_workers=self.config.config['max_workers']) as executor:
            # Adicionar delay entre lotes de requisições
            for i in range(0, len(codigos_pendentes), self.config.config['max_workers']):
                batch = codigos_pendentes[i:i + self.config.config['max_workers']]
                futures = {
                    executor.submit(
                        self.fetch_data, codigo, data_inicio, data_fim
                    ): codigo for codigo in batch
                }
                
                for future in as_completed(futures):
                    codigo = futures[future]
                    success, error = future.result()
                    
                    if not success:
                        self._save_failure(codigo, error)
                        self.estacoes_falha.append({
                            'codigo': codigo,
                            'erro': error
                        })
                    
                    # Salva progresso independentemente do resultado
                    self._save_checkpoint(codigo)
                    self.processed_stations.add(codigo)
                    
                    # Rate limiting
                    time.sleep(self.config.config['rate_limit_pause'])
                
                # Pausa entre lotes
                time.sleep(self.config.config['rate_limit_pause'] * 2)

def main():
    # Carregar configurações
    config = ConfigANA()
    
    # Carregar lista de estações
    try:
        df = pd.read_csv("codigos_estacoes.csv", header=None)
        codigos = df.iloc[:, 0].astype(str).tolist()
    except Exception as e:
        logger.error(f"Erro ao ler arquivo de estações: {e}")
        return
    
    try:
        downloader = ANADownloader(config)
        downloader.process_stations(
            codigos=codigos,
            data_inicio="1970-01-01",
            data_fim="2024-12-31"
        )
        
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
