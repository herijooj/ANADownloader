# Processo de Download e Conversão dos Dados da Bacia da Lagoa dos Patos

Este documento descreve o processo de download e conversão dos dados da Bacia da Lagoa dos Patos, destacando as etapas realizadas e as ferramentas utilizadas.

## Download dos Dados

Os dados foram obtidos a partir da seguinte fonte:
- **[ANA (Agência Nacional de Águas)](https://dadosabertos.ana.gov.br/documents/fb3426be2d4a4f9abfa90fb87b30bd4f/explore)**

### Etapas do Processo

#### 1. Filtrar Estações
- Foi baixado um arquivo `.mdb` contendo informações sobre as estações de monitoramento da ANA.
- O arquivo foi convertido para `.csv`.
- As estações relevantes para a Bacia da Lagoa dos Patos foram filtradas utilizando o script `filtraPlanilha.py`, que gera um arquivo contendo as estações e suas informações.

Também, foi usado o arquivo 'ListaEstacoesTelemetricas.xml' baixado do hidroweb. O script 'filtra_estacoes_hidroweb.py' gera o arquivo de coordenadas das estações. (hidroweb.coords) necessário para o programa 'cx_chuva'

#### 2. Download de Dados da ANA
- O acesso à API do Hidro Webservice da ANA foi solicitado, mas devido às limitações de requisições diárias, os dados foram baixados diretamente do site da ANA.
- O script `ANADownloader.py` realiza requisições POST e salva os dados no formato XML.

##### Exemplo de Requisição:
```python
def fetch_data(self, codigo: str, data_inicio: str, data_fim: str) -> Tuple[bool, str]:
    base_url = "http://telemetriaws1.ana.gov.br/ServiceANA.asmx/HidroSerieHistorica"
    params = {
        'codEstacao': codigo,
        'dataInicio': data_inicio,
        'dataFim': data_fim,
        'tipoDados': '2',
        'nivelConsistencia': ''
    }
```

##### Técnicas Implementadas:
- Controle de taxa (`rate limiting`) para evitar bloqueios:
```python
def _load_config(self) -> dict:
    return {
        'max_workers': 5,
        'max_retries': 5,
        'backoff_factor': 2,
        'timeout': 45,
        'rate_limit_pause': 3,
        'output_dir': 'dados_estacoes',
        'data_tipos': {
            'chuvas': '2',
            'cotas': '1',
            'vazoes': '3'
        }
    }
```

## Conversão dos Dados

Os dados obtidos estavam no formato **XML** e precisaram ser convertidos para o formato aceito pelo programa `cx_chuva`.

### Requisitos do Programa `cx_chuva`

#### Formato de Entrada
O programa `cx_chuva` espera os dados no seguinte formato:
```plaintext
[código da estação][ano][mês] [dia 1] [dia 2] [dia 3] ... [dia 31]
```

#### Exemplo:
```plaintext
00004700219770001  777.7  777.7 0000000000000000  777.7 777.7 777.7 777.7 777.7 777.7 777.7 777.7 777.7
```

### Estrutura dos Arquivos XML

Os arquivos XML possuem a seguinte estrutura:
```xml
<?xml version="1.0" encoding="utf-8"?>
<DataTable xmlns="http://MRCS/">
  <xs:schema id="NewDataSet" xmlns:xs="http://www.w3.org/2001/XMLSchema">
    <xs:element name="SerieHistorica">
      <xs:complexType>
        <xs:sequence>
          <xs:element name="EstacaoCodigo" type="xs:int" />
          <xs:element name="DataHora" type="xs:string" />
          <xs:element name="Chuva01" type="xs:string" />
          <xs:element name="Chuva02" type="xs:string" />
          <!-- Outros elementos -->
        </xs:sequence>
      </xs:complexType>
    </xs:element>
  </xs:schema>
</DataTable>
```

### Processo de Conversão

#### 1. Converter XML para CSV
O script `anatocsv.py` realiza a conversão dos arquivos XML para CSV, atendendo aos seguintes requisitos:
- **Códigos das estações**: Devem ter 8 caracteres.
- **Datas**: Formato `YYYY/MM/DD`.
- **Valores de chuva**: Devem estar no formato `0.0`.
- **Separador**: Substituir `","` por `";"`.

#### 2. Converter CSV para o Formato do `cx_chuva`
Após a conversão para CSV, os dados foram processados pelo programa de conversão existente (`csv2cxchuva`), gerando dois arquivos de saída:
- **`.DIA`**: Dados de chuva diária.
- **`.MES`**: Dados de chuva mensal.

### Estrutura de Saída

Os arquivos de saída apresentam os seguintes formatos:
- **Arquivo `.DIA`**: Dados organizados por dias.
- **Arquivo `.MES`**: Dados organizados por meses.

## Ferramentas Utilizadas

- **Scripts**: 
  - `filtraPlanilha.py`
  - `ANADownloader.py`
  - `anatocsv.py`
- **Ferramenta de Conversão**: `csv2cxchuva`

## Diretórios e Arquivos

- **dados_estacoes**: Diretório onde os dados brutos foram armazenados.
- **csv2cxchuva**: Programa de conversão para o formato aceito pelo `cx_chuva`.
