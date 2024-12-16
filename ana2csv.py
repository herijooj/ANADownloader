import csv
import xml.etree.ElementTree as ET
import os
from datetime import datetime

# Input and output directories
input_dir = "dados_estacoes"
output_dir = "new_output_folder"

# Ensure output directory exists
os.makedirs(output_dir, exist_ok=True)

namespaces = {
    'diffgr': 'urn:schemas-microsoft-com:xml-diffgram-v1',
    'msdata': 'urn:schemas-microsoft-com:xml-msdata'
}

def strip_namespace(tag):
    # Remove namespace da tag
    if '}' in tag:
        return tag.split('}', 1)[1]
    return tag

def format_date(date_str):
    if not date_str:
        return date_str
    try:
        # Considerar apenas a parte da data (antes do espaço)
        date_str = date_str.split()[0]
        
        # Converter para objeto datetime, assumindo formato YYYY-MM-DD ou YYYY/MM/DD
        if '-' in date_str:
            date_obj = datetime.strptime(date_str, '%Y-%m-%d')
        elif '/' in date_str:
            date_obj = datetime.strptime(date_str, '%Y/%m/%d')
        else:
            return date_str
        
        return date_obj.strftime('%d/%m/%Y')
    except:
        return date_str

def format_number(value):
    if not value:
        return value
    # Apenas substitui ponto por vírgula
    return value.replace('.', ',')

def format_value(value, col_tag):
    if value is None:
        return ''
    
    # Se for um campo com "Data" no nome, formata como data
    if 'Data' in col_tag:
        return format_date(value)
    
    # Caso contrário, tenta formatar número
    return format_number(value)

# Process each XML file in the input directory
for xml_filename in os.listdir(input_dir):
    if xml_filename.endswith(".xml"):
        xml_file = os.path.join(input_dir, xml_filename)
        
        # Parse the XML file
        tree = ET.parse(xml_file)
        root = tree.getroot()

        # Find all SerieHistorica elements
        document_element = root.find('.//diffgr:diffgram/DocumentElement', namespaces)
        if document_element is None:
            print(f"Could not find DocumentElement in the XML file: {xml_file}")
            continue

        series = document_element.findall('SerieHistorica')

        if not series:
            print(f"No SerieHistorica elements found in the XML file: {xml_file}")
            continue

        # Get EstacaoCodigo from first series to use as filename
        first_serie = series[0]
        estacao_elem = first_serie.find('EstacaoCodigo')
        if estacao_elem is None or not estacao_elem.text:
            continue
            
        output_filename = f"{estacao_elem.text.zfill(8)}.txt"
        output_path = os.path.join(output_dir, output_filename)

        # Extract column names from the first SerieHistorica element
        columns = [strip_namespace(elem.tag) for elem in first_serie]

        # Write data to TXT
        with open(output_path, mode='w', newline='', encoding='utf-8') as f:
            for serie in series:
                row = []
                for col_tag in columns:
                    element = serie.find(col_tag)
                    value = element.text if element is not None else ''
                    formatted_value = format_value(value, col_tag)
                    row.append(formatted_value)
                # Escreve linha separada por ';' e adiciona ';' no final, conforme exemplo
                f.write(';'.join(row) + ';\n')

        print(f"Arquivo '{output_filename}' foi criado com sucesso.")
