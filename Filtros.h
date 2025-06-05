// Filtros.h
#ifndef FILTROS_H
#define FILTROS_H

#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <itkImage.h>
#include <itkImageRegionConstIterator.h>

namespace fs = std::filesystem;

// Definiciones de tipos (3D para leer, 2D para extraer)
constexpr unsigned int Dimension2D = 2;
using PixelType2D  = short;
using ImageType2D  = itk::Image<PixelType2D, Dimension2D>;

/**
 * Convierte un ImageType2D (original) a cv::Mat de 8 bits (escalado de 0 a 255).
 */
cv::Mat ITKImage2DtoCVMat(const ImageType2D::Pointer& image2D);

/**
 * Convierte un ImageType2D (máscara) a cv::Mat binaria (0 ó 255).
 */
cv::Mat ITKMask2BinCVMat(const ImageType2D::Pointer& mask2D);

/**
 * Procesa un único slice:
 *  - Aplica el filtro elegido (filterOption)
 *  - Guarda las imágenes resultantes (original ecualizada, máscara refinada, highlighted)
 *
 * @param slice8u      Imagen 8-bit (slice ecualizado o procesado previamente)
 * @param maskBin      Mascara binaria 8-bit
 * @param dirOrig      Carpeta donde se guardará la versión “original” (ecualizada)
 * @param dirMask      Carpeta donde se guardará la máscara refinada
 * @param dirHigh      Carpeta donde se guardará la imagen highlight (ROI + bordes)
 * @param indiceZ      Índice del slice para nombrar los archivos (slice_XXX.png)
 * @param filterOption Entero (1–10) que indica qué filtro/técnica aplicar.
 */
void ProcesarYGuardarSlice(
    const cv::Mat& slice8u,
    const cv::Mat& maskBin,
    const fs::path& dirOrig,
    const fs::path& dirMask,
    const fs::path& dirHigh,
    unsigned int indiceZ,
    int filterOption
);

// —————— Declaración de funciones para cada técnica ——————

// 1) Thresholding
cv::Mat aplicarThresholding(const cv::Mat& src);

// 2) Contrast Stretching
cv::Mat aplicarContrastStretching(const cv::Mat& src);

// 3) Binarización por umbral de color
cv::Mat aplicarBinarizacionColor(const cv::Mat& src);

// 4) Operaciones lógicas (NOT, AND, OR, XOR)
//    NOT toma solo A; AND/OR/XOR toman A y B (para B podemos usar la máscara u otra imagen)
cv::Mat aplicarOperacionLogica(const cv::Mat& src, const cv::Mat& mask, int tipoOp); 
// tipoOp: 0=NOT, 1=AND, 2=OR, 3=XOR

// 5) Detección de Bordes (p. ej. Canny)
cv::Mat aplicarDeteccionBordes(const cv::Mat& src);

// 6) Manipulación de píxeles (ej. negativo, invertir intensidades)
cv::Mat aplicarManipulacionPixeles(const cv::Mat& src);

// 7) Filtros de suavizado (e.g. GaussianBlur, Mediana, Bilateral)
cv::Mat aplicarFiltroSuavizado(const cv::Mat& src);

// 8) Operaciones morfológicas (apertura + cierre, dilatación, erosión, etc.)
cv::Mat aplicarOperacionesMorfo(const cv::Mat& src);

// 9) Otra técnica (por ejemplo: ecualización de histograma)
cv::Mat aplicarOtraTecnica(const cv::Mat& src);

#endif // FILTROS_H
