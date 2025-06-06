// Utils.h
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <filesystem>             // para std::filesystem::path
#include <itkImage.h>             // para definir ImageType3D
#include <itkImageFileReader.h>
#include <itkNiftiImageIO.h>
#include <itkExtractImageFilter.h>
#include "Filtros.h"              // para ITKImage2DtoCVMat, ITKMask2BinCVMat y ProcesarYGuardarSlice

namespace fs = std::filesystem;

// Tipos 3D de ITK (imagen y máscara)
constexpr unsigned int Dimension3D = 3;
using PixelType3D = short;
using ImageType3D = itk::Image<PixelType3D, Dimension3D>;

/**
 * Lee un volumen NIfTI (imagen y máscara), extrae cada slice en Z,
 * lo convierte a cv::Mat, aplica el filtro elegido y guarda resultados en carpetas.
 *
 * @param rutaNifti         Ruta al archivo NIfTI de la imagen 3D.
 * @param rutaMask          Ruta al archivo NIfTI de la máscara 3D.
 * @param carpetaSalidaBase Carpeta base donde se crearán subcarpetas:
 *                          "original", "mask" y "highlighted".
 * @param filterOption      Entero (1–10) que indica qué filtro aplicar.
 * @return true si todo salió bien; false en caso de error.
 */
bool ProcesarTodosSlices(
    const std::string& rutaNifti,
    const std::string& rutaMask,
    const std::string& carpetaSalidaBase,
    int filterOption
);

/**
 * Genera un video (AVI) usando sólo las imágenes cuyos índices estén
 * entre 'inicio' y 'fin' (1-based) encontradas en 'carpetaHighlighted'.
 *
 * @param carpetaHighlighted Carpeta donde están las imágenes “highlighted” (slices).
 * @param carpetaVideo       Carpeta destino donde guardaremos "highlighted_video.avi".
 * @param inicio             Índice (1-based) del primer slice a incluir.
 * @param fin                Índice (1-based) del último slice a incluir.
 * @return true si el video se generó y guardó correctamente; false en caso contrario.
 */
bool GenerarVideoHighlighted(
    const std::string& carpetaHighlighted,
    const std::string& carpetaVideo,
    int inicio,
    int fin
);

#endif // UTILS_H
