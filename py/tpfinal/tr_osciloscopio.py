import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Ruta del archivo CSV
archivo_csv = '1v-2v-1hz.csv'
archivo_modificado_csv = 'modif.csv'

# Archivos de salida para las señales suavizadas
archivo_y1_suavizado_csv = 'y1.csv'
archivo_y2_suavizado_csv = 'y2.csv'

# # Leer y modificar el archivo CSV
# with open(archivo_csv, 'r') as file:
#     lines = file.readlines()

# with open(archivo_modificado_csv, 'w') as file:
#     for i, line in enumerate(lines):
#         if i >= 19:  # A partir de la fila 20 (índice 19)
#             # Contador de comas encontradas
#             coma_count = 0
#             new_line = ''
#             for char in line:
#                 if char == ',':
#                     coma_count += 1
#                     if coma_count in [1, 3, 5, 7]:
#                         new_line += '.'
#                     else:
#                         new_line += ','
#                 else:
#                     new_line += char
#             file.write(new_line)
#         else:
#             file.write(line)

# Leer el archivo CSV modificado omitiendo las primeras 18 filas y especifica la coma como separador decimal
df = pd.read_csv(archivo_modificado_csv, skiprows=18, decimal=',')

# Asigna la fila 19 como los encabezados
df.columns = df.iloc[0]
df = df[1:]

# Convierte los datos a números (ya que por defecto son strings)
df = df.apply(pd.to_numeric)

# Separa los datos para los dos gráficos
x1, y1 = df.iloc[:, 0], df.iloc[:, 1]
x2, y2 = df.iloc[:, 2], df.iloc[:, 3]

x2_min = np.min(x2)
x2 += -x2_min

x1_min = np.min(x1)
x1 += -x1_min

# Aplicar filtro de media móvil a las señales y1 y y2
window_size = 5
y1_smooth_ma = y1.rolling(window=window_size).mean()
y2_smooth_ma = y2.rolling(window=window_size).mean()

# Aplicar suavizado exponencial a las señales y1 y y2
alpha = 0.05
y1_smooth_exp = y1.ewm(alpha=alpha).mean()
y2_smooth_exp = y2.ewm(alpha=alpha).mean()

# Crear DataFrames con las señales suavizadas y los ejes x correspondientes
df_y1_suavizado = pd.DataFrame({'X1': x1, 'Y1_suavizado': y1_smooth_exp})
df_y2_suavizado = pd.DataFrame({'X2': x2, 'Y2_suavizado': y2_smooth_exp})

# Guardar las señales suavizadas en archivos CSV
df_y1_suavizado.to_csv(archivo_y1_suavizado_csv, index=False)
df_y2_suavizado.to_csv(archivo_y2_suavizado_csv, index=False)
# Crear los gráficos
plt.figure(figsize=(15, 10))

# # Grafica el primer gráfico con media móvil
# plt.subplot(2, 2, 1)
# plt.plot(x1, y1, marker='o', linestyle='-', color='b', alpha=0.5, label='Original')
# plt.plot(x1, y1_smooth_ma, marker='o', linestyle='-', color='g', label='Media Móvil')
# plt.xlabel('X1')
# plt.ylabel('Y1')
# plt.title('Gráfico 1 - Media Móvil')
# plt.legend()

START_SAMPLES = 1000
SAMPLES = 50000

y1_smooth_exp = y1_smooth_exp[START_SAMPLES:SAMPLES]
x1 = x1[START_SAMPLES:SAMPLES]

# Grafica el primer gráfico con suavizado exponencial
plt.subplot(2, 1, 1)
# plt.plot(x1, y1, marker='o', linestyle='-', color='b', alpha=0.5, label='Original')
plt.plot(x1, y1_smooth_exp, marker='o', linestyle='-', color='g', label='Entrada')
plt.xlabel('Tiempo(S)')
plt.ylabel('Amplitud(V)')
plt.title('Entrada')
plt.legend()

# # Grafica el segundo gráfico con media móvil
# plt.subplot(2, 2, 3)
# plt.plot(x2, y2, marker='o', linestyle='-', color='r', alpha=0.5, label='Original')
# plt.plot(x2, y2_smooth_ma, marker='o', linestyle='-', color='g', label='Media Móvil')
# plt.xlabel('X2')
# plt.ylabel('Y2')
# plt.title('Gráfico 2 - Media Móvil')
# plt.legend()

# Grafica el segundo gráfico con suavizado exponencial

y2_smooth_exp = y2_smooth_exp[START_SAMPLES:SAMPLES]
x2 = x2[START_SAMPLES:SAMPLES]

y2_smooth_exp = np.array(y2_smooth_exp)
x2 = np.array(x2)

# Calcular el tiempo de subida
minlevel = np.min(y2_smooth_exp)
print("minlevel",minlevel)

maxlevel = np.max(y2_smooth_exp)
print("maxlevel",maxlevel)

out = maxlevel - minlevel
print('p-p',out)

out_10 = out * 0.1 + minlevel
out_90 = out * 0.9 + minlevel

print("10%", out_10)
print("90%", out_90)

y2_10 = 11193 - START_SAMPLES
y2_90 = 14891 - START_SAMPLES

#check the nearest value
# for index,val in enumerate(y2_smooth_exp):
#     if val > out_10 * 0.9999 and val < out_10 * 1.0001:
#         print(index,val)

# for index,val in enumerate(y2_smooth_exp):
#     if val > out_90 * 0.99999 and val < out_90 * 1.0001:
#         print(index,val)

t10 = x2[y2_10]
t90 = x2[y2_90]
print("Tiempo de subida : {}ms".format(round((t90 - t10)*1000,2)))

plt.subplot(2, 1, 2)
# plt.plot(x2, y2, marker='o', linestyle='-', color='r', alpha=0.5, label='Original')
plt.plot(x2, y2_smooth_exp, marker='o', linestyle='-', color='g', label='Salida')
plt.hlines(out_10,0, x2[-1])
plt.hlines(out_90,0, x2[-1])
plt.vlines(x2[y2_10],minlevel,maxlevel, linestyles='-', color='red')
plt.vlines(x2[y2_90],minlevel,maxlevel, linestyles='-', color='red')
plt.xlabel('Tiempo(S)')
plt.ylabel('Amplitud(V)')
plt.title('Salida')
plt.legend()

# Muestra los gráficos
plt.tight_layout()
plt.show()