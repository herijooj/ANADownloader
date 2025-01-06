import xml.etree.ElementTree as ET
import csv
import os
import pandas as pd

class EstacoesFiltro:
    def __init__(self):
        # Configurações gerais
        self.FILTERED_FILE = "planilha_filtrada.csv"
        self.COORDS_FILE = "hidroweb.coords"
        self.STATIONS_FILE = "codigos_estacoes.csv"
        self.ALL_STATIONS_FILE = "codigo_estacoes_completo.csv"
        
        # Limites geográficos
        self.LAT_MIN = -35.0
        self.LAT_MAX = -26.0
        self.LON_MIN = -58.0
        self.LON_MAX = -49.0
        
        self.COMPLETO = 1

    def process_xml(self, input_file):
        tree = ET.parse(input_file)
        root = tree.getroot()
        
        all_data = []  # Armazena todas as estações
        filtered_data = []
        
        for table in root.findall('.//Table'):
            nome = table.find('NomeEstacao').text
            codigo = table.find('CodEstacao').text
            lat = float(table.find('Latitude').text)
            lon = float(table.find('Longitude').text)
            all_data.append([nome, codigo, lat, lon])
            if self.COMPLETO == 1:
                if self.LAT_MIN <= lat <= self.LAT_MAX and self.LON_MIN <= lon <= self.LON_MAX:
                    filtered_data.append([nome, codigo, lat, lon])
        
        # Salvar arquivos filtrados
        self.write_simple_files(filtered_data)
        
        # Gerar hidroweb.coords com todas as estações, ignorando o filtro
        self.write_coords_file([(item[1], item[2], item[3]) for item in all_data])
        
        # Salvar todas as estações no arquivo completo
        self.write_all_stations_file(all_data)

    def process_csv(self, input_file):
        df = pd.read_csv(input_file, low_memory=False)
        df['Codigo'] = df['Codigo'].astype(float).astype(int)
        
        # Guardar todas as estações
        all_coords_data = df[['Codigo', 'Latitude', 'Longitude']].values
        
        # Aplicar filtro apenas para CSV e códigos
        mask = (
            (df['Latitude'] >= self.LAT_MIN) & 
            (df['Latitude'] <= self.LAT_MAX) & 
            (df['Longitude'] >= self.LON_MIN) & 
            (df['Longitude'] <= self.LON_MAX)
        )
        filtered_df = df[mask]
        filtered_df.to_csv(self.FILTERED_FILE, index=False)
        filtered_df[['Codigo']].to_csv(self.STATIONS_FILE, index=False)
        
        # Gerar hidroweb.coords com todas as estações
        self.write_coords_file(all_coords_data)
        
        # Salvar todas as estações no arquivo completo
        self.write_all_stations_file(df[['Codigo', 'Latitude', 'Longitude']].values)

    def write_simple_files(self, data):
        # Escrever CSV filtrado
        with open(self.FILTERED_FILE, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['NomeEstacao', 'CodEstacao', 'Latitude', 'Longitude'])
            writer.writerows(data)
        
        self.write_coords_file([(row[1], row[2], row[3]) for row in data])

    def write_coords_file(self, coords_data):
        with open(self.COORDS_FILE, 'w') as f:
            for row in sorted(coords_data, key=lambda x: int(x[0])):
                # Garantir que o código seja tratado como inteiro
                codigo = int(float(row[0]))
                f.write(f"{codigo:08d}#{float(row[1]):.6f}#{float(row[2]):.6f}\n")

    def write_all_stations_file(self, all_data):
        with open(self.ALL_STATIONS_FILE, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['Codigo', 'Latitude', 'Longitude'])
            writer.writerows(all_data)

def main():
    filtro = EstacoesFiltro()
    
    # Inicializar arquivos necessários
    files_to_check = [
        "ListaEstacoesTelemetricas.xml",
        "./Inventario/Estacao.csv"
    ]
    
    for file in files_to_check:
        if not os.path.exists(file):
            print(f"Arquivo necessário não encontrado: {file}")
            return
    
    # Processar arquivo XML se existir
    if os.path.exists("ListaEstacoesTelemetricas.xml"):
        filtro.process_xml("ListaEstacoesTelemetricas.xml")
        print("Processamento XML concluído.")
    
    # Processar arquivo CSV se existir
    if os.path.exists("./Inventario/Estacao.csv"):
        filtro.process_csv("./Inventario/Estacao.csv")
        print("Processamento CSV concluído.")
    
    print(f"Os arquivos foram criados com sucesso.")

if __name__ == "__main__":
    main()
