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
### Estrutura dos Arquivos CSV de saída do programa 'ana2csv.py'

```plaintext
48002;1;01/12/1982;1;59,4;74,7;25;3;1;1;1;2060,9;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;59,4;8,9;6,4;0;0;0;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;;
48002;1;01/11/1982;1;12,5;14,5;17;2;1;1;1;2060,9;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;12,5;0;0;0;0;0;0;2;0;0;0;0;0;0;;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;0;;
48002;1;01/10/1982;1;9;44,4;22;9;1;1;1;2060,9;1;0;0;0;0;6;0;0;6;0;0;2,2;6;0;0;0;0;0;0;6;2,2;5;9;0;0;0;2;0;0;0;0;0;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;;
```

onde a header é:
```plaintext
EstacaoCodigo;NivelConsistencia;DataHora;TipoMedicaoChuvas;Maxima;Total;DiaMaxima;NumDiasDeChuva;MaximaStatus;TotalStatus;NumDiasDeChuvaStatus;TotalAnual;TotalAnualStatus;Chuva01;Chuva02;Chuva03;Chuva04;Chuva05;Chuva06;Chuva07;Chuva08;Chuva09;Chuva10;Chuva11;Chuva12;Chuva13;Chuva14;Chuva15;Chuva16;Chuva17;Chuva18;Chuva19;Chuva20;Chuva21;Chuva22;Chuva23;Chuva24;Chuva25;Chuva26;Chuva27;Chuva28;Chuva29;Chuva30;Chuva31;Chuva01Status;Chuva02Status;Chuva03Status;Chuva04Status;Chuva05Status;Chuva06Status;Chuva07Status;Chuva08Status;Chuva09Status;Chuva10Status;Chuva11Status;Chuva12Status;Chuva13Status;Chuva14Status;Chuva15Status;Chuva16Status;Chuva17Status;Chuva18Status;Chuva19Status;Chuva20Status;Chuva21Status;Chuva22Status;Chuva23Status;Chuva24Status;Chuva25Status;Chuva26Status;Chuva27Status;Chuva28Status;Chuva29Status;Chuva30Status;Chuva31Status;DataIns;
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
