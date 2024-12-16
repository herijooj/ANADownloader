#!/bin/bash

# Nome do arquivo de entrada
input_file="./Inventario/Estacao.csv"

# Nome do arquivo filtrado
filtered_file="planilha_filtrada.csv"

# Nome do arquivo com os códigos das estações
stations_file="codigos_estacoes.csv"

# Limites de latitude e longitude (com base no mapa fornecido)
lat_min=-35.0
lat_max=-26.0
lon_min=-58.0
lon_max=-49.0

# Passo 1: Filtrar os dados com base nas coordenadas
awk -F',' '
BEGIN {
    # Define os limites das coordenadas
    lat_min = '"$lat_min"'
    lat_max = '"$lat_max"'
    lon_min = '"$lon_min"'
    lon_max = '"$lon_max"'

    # Imprime o cabeçalho
    print "RegistroID,Importado,Temporario,Removido,ImportadoRepetido,BaciaCodigo,SubBaciaCodigo,RioCodigo,EstadoCodigo,MunicipioCodigo,ResponsavelCodigo,ResponsavelUnidade,ResponsavelJurisdicao,OperadoraCodigo,OperadoraUnidade,OperadoraSubUnidade,TipoEstacao,Codigo,Nome,CodigoAdicional,Latitude,Longitude,Altitude,AreaDrenagem,TipoEstacaoEscala,TipoEstacaoRegistradorNivel,TipoEstacaoDescLiquida,TipoEstacaoSedimentos,TipoEstacaoQualAgua,TipoEstacaoPluviometro,TipoEstacaoRegistradorChuva,TipoEstacaoTanqueEvapo,TipoEstacaoClimatologica,TipoEstacaoPiezometria,TipoEstacaoTelemetrica,PeriodoEscalaInicio,PeriodoEscalaFim,PeriodoRegistradorNivelInicio,PeriodoRegistradorNivelFim,PeriodoDescLiquidaInicio,PeriodoDescLiquidaFim,PeriodoSedimentosInicio,PeriodoSedimentosFim,PeriodoQualAguaInicio,PeriodoQualAguaFim,PeriodoPluviometroInicio,PeriodoPluviometroFim,PeriodoRegistradorChuvaInicio,PeriodoRegistradorChuvaFim,PeriodoTanqueEvapoInicio,PeriodoTanqueEvapoFim,PeriodoClimatologicaInicio,PeriodoClimatologicaFim,PeriodoPiezometriaInicio,PeriodoPiezometriaFim,PeriodoTelemetricaInicio,PeriodoTelemetricaFim,TipoRedeBasica,TipoRedeEnergetica,TipoRedeNavegacao,TipoRedeCursoDagua,TipoRedeEstrategica,TipoRedeCaptacao,TipoRedeSedimentos,TipoRedeQualAgua,TipoRedeClasseVazao,UltimaAtualizacao,Operando,Descricao,Historico,NumImagens,DataIns,DataAlt,RespAlt"
}

# Ignora o cabeçalho da entrada
NR > 1 {
    # Pega os valores de Latitude e Longitude (colunas 21 e 22)
    latitude = $21
    longitude = $22

    # Verifica se os valores estão dentro do intervalo
    if (latitude >= lat_min && latitude <= lat_max && longitude >= lon_min && longitude <= lon_max) {
        print $0
    }
}
' "$input_file" > "$filtered_file"

# Passo 2: Extrair apenas os códigos das estações (coluna 18 - "Codigo")
awk -F',' '
NR == 1 {
    # Imprime o cabeçalho no novo arquivo
    print "Codigo"
}

NR > 1 {
    # Extrai a coluna "Codigo" (coluna 18)
    print $18
}
' "$filtered_file" > "$stations_file"

# Passo 3: Criar o arquivo hidroweb.coords
awk -F',' '
NR > 1 {
    printf "%08d#%.6f#%.6f\n", $18, $21, $22
}
' "$filtered_file" | sort -t'#' -k1,1 > "hidroweb.coords"

echo "Arquivo filtrado foi salvo como $filtered_file."
echo "Os códigos das estações foram salvos em $stations_file."
echo "O arquivo hidroweb.coords foi criado."
