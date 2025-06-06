# Filtros para Imagenes de Resonancia Magnetica ProcessorQt

## Descripción

RMProcessorQt es una aplicación de escritorio (Qt) para el procesamiento de volúmenes de Resonancia Magnética en formato NIfTI (.nii, .nii.gz). Permite:

- Cargar una imagen volumétrica original y su máscara.
- Aplicar diferentes filtros y técnicas a cada slice (umbralización, contraste, binarización por color, operaciones lógicas, detección de bordes, suavizado, operaciones morfológicas, watershed, entre otros).
- Visualizar slice a slice las imágenes original, máscara y resaltada.
- Generar videos AVI de los slices resaltados en un rango seleccionado.
- Mostrar estadísticas (media, mediana, moda, varianza, desviación estándar) de los píxeles de un slice, con un boxplot, mediante un script Python.

## Características

- Interfaz gráfica Qt5 (Widgets, QPushButton, QLabel, QComboBox, QSlider).
- Lectura de volúmenes 3D NIfTI con ITK y conversión a imágenes 2D OpenCV.
- Implementación de múltiples filtros y técnicas de procesamiento de imagen en C++/OpenCV.
- Generación de vídeos con OpenCV.
- Cálculo de estadísticas en Python (numpy, matplotlib, tkinter).

## Requisitos

- Linux (probado en WSL2 / Kali Linux).
- CMake >= 3.10.
- Compilador con soporte C++17.
- Qt5 Widgets.
- OpenCV.
- ITK.
- Python3 con los módulos: `numpy`, `matplotlib`, `tkinter`, `opencv-python`.

## Instalación de WSL

Para instalar y configurar WSL2 con Kali Linux y soporte de GUI, consultar la [Guía Completa en Notion](https://www.notion.so/WSL-con-Kali-y-GUI-203ae96cf28d80c9b2c4d497c846e439?source=copy_link).

## Instalación del proyecto

```bash
# En el directorio raíz del proyecto
git clone <repositorio> vision
cd vision
mkdir build && cd build
cmake ..
make
```

En caso de necesitar limpiar la caché de CMake:

```bash
rm -rf CMakeCache.txt CMakeFiles
cmake .
```

## Ejecución

```bash
./RMProcessorQt
```

### Uso paso a paso

1. Cargar la **imagen volumétrica** original (.nii / .nii.gz).
2. Cargar la **máscara** volumétrica (.nii / .nii.gz).
3. Seleccionar un filtro del menú desplegable.
4. Hacer clic en **Aplicar filtro** para procesar todos los slices.
5. Usar el slider para navegar por los slices generados.
6. (Opcional) Hacer clic en **Hacer video** para generar un video AVI de los slices resaltados en un rango específico.
7. Hacer clic en **Abrir video** para reproducir el video generado.
8. Hacer clic en **Sacar Estadísticas** para ver estadísticas de intensidad y un boxplot.

## Estructura del proyecto

```
vision/
├── CMakeLists.txt          # Configuración de CMake
├── main.cpp                # Punto de entrada de la aplicación Qt
├── MainWindow.h/cpp        # Lógica de interfaz y slots
├── VideoDialog.h/cpp       # Diálogo para selección de rango de video
├── Utils.h/cpp             # Funciones de procesamiento de slices y video
├── Filtros.h/cpp           # Implementación de filtros y conversión ITK/OpenCV
├── image_stats.py          # Script Python para estadísticas y boxplot
├── build/                  # Carpeta de compilación (generada)
└── Output/                 # Carpeta de resultados (original, mask, highlighted, video)
```

## image_stats.py

Script en Python que recibe una imagen en escala de grises y muestra:
- Media, mediana, moda, varianza y desviación estándar de la intensidad de píxeles.
- Boxplot de los valores.

Uso:

```bash
python3 image_stats.py <ruta_imagen_gris.png>
```

## WSL

Si se esta corriendo en wsl la forma de acceder a los archivos desde el buscador de archivos de windows es:

```bash
\\wsl.localhost\kali-linux\home\juanja\Desktop\vision 
```

## Notas
* Recordar cambiar la ruta en `MainWindow.cpp`:
```bash
QString scriptPath = "/home/juanja/Desktop/vision/image_stats.py";
```
* Si da error de persmisos, introducir el siguietne comando:
```bash
sudo chmod 0700 /run/user/1000
```