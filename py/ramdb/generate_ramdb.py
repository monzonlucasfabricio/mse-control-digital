# Script en Python para generar el archivo de encabezado con el enum

# Define las listas con los nombres de RAM para cada tipo de dato
ram_u8_names = ["RAM_1","RAM_2","RAM_3"]
ram_u16_names = ["RAM_KP", "RAM_KI", "RAM_KD"]
ram_u32_names = []

# Define el tamaño en bytes de cada tipo de dato
size_u8 = 1
size_u16 = 2
size_u32 = 4

# Dirección base en RAM
base_address = 0x2002F800

# Tamaño de la sección de RAM a utilizar (2K)
ram_size = 2048

# Función para generar las entradas del enum
def generate_enum_entries(names, size, base_address):
    current_address = base_address
    entries = []
    for name in names:
        entries.append(f"    {name} = 0x{current_address:X}")
        current_address += size
    return entries

# Generar las entradas del enum para cada lista
entries_u8 = generate_enum_entries(ram_u8_names, size_u8, base_address)
entries_u16 = generate_enum_entries(ram_u16_names, size_u16, base_address + len(ram_u8_names) * size_u8)
entries_u32 = generate_enum_entries(ram_u32_names, size_u32, base_address + (len(ram_u8_names) + len(ram_u16_names)) * size_u16)

# Combinar todas las entradas del enum
all_entries = entries_u8 + entries_u16 + entries_u32

# Generar el contenido del archivo de encabezado sin f-string
header_content = "#ifndef RAM_ENUMS_H\n#define RAM_ENUMS_H\n\n"
header_content += "typedef enum {\n"
header_content += ",\n".join(all_entries)
header_content += "\n} RamAddress;\n\n#endif // RAM_ENUMS_H\n"

# Escribir el contenido en el archivo de encabezado
with open("ram_enums.h", "w") as file:
    file.write(header_content)

print("Archivo de encabezado 'ram_enums.h' generado con éxito.")