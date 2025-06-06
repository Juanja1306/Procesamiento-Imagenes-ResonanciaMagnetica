// Filtros.cpp
#include "Filtros.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <itkImageRegionConstIterator.h>
#include <iostream>
#include <cstdio>

// ----------------------------------------------------------
// Funciones Auxiliares: cada una aplica el filtro correspondiente
// ----------------------------------------------------------

// 1) Thresholding truncado
cv::Mat aplicarThresholding(const cv::Mat& src)
{
    cv::Mat gray, dst;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = src;
    }
    double umbral = 80; // umbral fijo; podrías pedir input dinámico
    cv::threshold(gray, dst, umbral, 255, cv::THRESH_BINARY_INV);
    return dst;
}

// 2) Contrast Stretching (estiramiento lineal de contrastes)
cv::Mat aplicarContrastStretching(const cv::Mat& src)
{
    cv::Mat gray, dst;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = src;
    }
    double minVal, maxVal;
    cv::minMaxLoc(gray, &minVal, &maxVal);
    if (maxVal > minVal) {
        gray.convertTo(dst, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
    } else {
        dst = cv::Mat::zeros(gray.size(), CV_8U);
    }
    return dst;
}

// 3) Binarización por umbral de color o, si es imagen de 1 canal, umbral de intensidad
cv::Mat aplicarBinarizacionColor(const cv::Mat& src)
{
    // Si la imagen viene en escala de grises (1 canal), aplicamos threshold de intensidad
    if (src.channels() == 1)
    {
        cv::Mat dst;
        double umbral = 80; // puedes ajustar este valor o parametrizarlo
        cv::threshold(src, dst, umbral, 255, cv::THRESH_BINARY);
        return dst;
    }

    // Si la imagen es de 3 canales (BGR), convertimos a HSV y binarizamos según rango de color
    cv::Mat hsv, mask;
    cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);

    // Ejemplo: binarizar tonos de rojo 
    cv::Scalar lower_red1(0, 120, 70);
    cv::Scalar upper_red1(10, 255, 255);
    cv::Scalar lower_red2(170, 120, 70);
    cv::Scalar upper_red2(180, 255, 255);

    cv::Mat mask1, mask2;
    cv::inRange(hsv, lower_red1, upper_red1, mask1);
    cv::inRange(hsv, lower_red2, upper_red2, mask2);
    mask = mask1 | mask2;
    return mask;
}


// 4) Operaciones lógicas (NOT, AND, OR, XOR). 
//    typeOp: 0=NOT, 1=AND, 2=OR, 3=XOR.
//    Si es NOT, ignoramos mask y solo invertimos src. Para los demás, src & mask, etc.
cv::Mat aplicarOperacionLogica(const cv::Mat& src, const cv::Mat& mask, int tipoOp)
{
    cv::Mat graySrc;
    if (src.channels() == 3) {
        cv::cvtColor(src, graySrc, cv::COLOR_BGR2GRAY);
    } else {
        graySrc = src;
    }
    cv::Mat dst;
    switch (tipoOp) {
        case 0: // NOT
            cv::bitwise_not(graySrc, dst);
            break;
        case 1: // AND
            cv::bitwise_and(graySrc, mask, dst);
            break;
        case 2: // OR
            cv::bitwise_or(graySrc, mask, dst);
            break;
        case 3: // XOR
            cv::bitwise_xor(graySrc, mask, dst);
            break;
        default:
            dst = graySrc.clone();
    }
    return dst;
}

// 5) Detección de bordes (Canny)
cv::Mat aplicarDeteccionBordes(const cv::Mat& src)
{
    cv::Mat gray, edges;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = src;
    }
    double umbral1 = 50, umbral2 = 150;
    cv::Canny(gray, edges, umbral1, umbral2);
    return edges;
}

// 6) Manipulación de píxeles: ImagenOriginal + (TopHat – BlackHat)
cv::Mat aplicarManipulacionPixeles(const cv::Mat& src)
{
    // 1) Convertir a escala de grises
    cv::Mat gray;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = src.clone();
    }

    // 2) Crear elemento estructurante (ej. disco de radio 3)
    cv::Mat element = cv::getStructuringElement(
        cv::MORPH_ELLIPSE,
        cv::Size(30, 30)  // diámetro = 2*radio+1, aquí radio=3
    );

    // 3) Calcular TopHat y BlackHat
    cv::Mat opening, closing;
    cv::morphologyEx(gray, opening,  cv::MORPH_OPEN,  element);
    cv::morphologyEx(gray, closing,  cv::MORPH_CLOSE, element);

    cv::Mat tophat, blackhat;
    cv::subtract(gray, opening, tophat);    // TopHat = src – opening
    cv::subtract(closing, gray, blackhat);  // BlackHat = closing – src

    // 4) Diferencia = TopHat – BlackHat
    cv::Mat diff;
    cv::subtract(tophat, blackhat, diff, cv::noArray(), CV_16S);

    // 5) Convertir gray a 16 bits para sumar sin saturar
    cv::Mat gray16;
    gray.convertTo(gray16, CV_16S);

    // 6) Resultado16 = gray16 + diff
    cv::Mat resultado16;
    cv::add(gray16, diff, resultado16, cv::noArray(), CV_16S);

    // 7) Reconvertir a 8 bits (se saturará en 0–255 si sale fuera de rango)
    cv::Mat resultado;
    resultado16.convertTo(resultado, CV_8U);

    return resultado;
}

// 7) Filtros de suavizado (GaussianBlur)
cv::Mat aplicarFiltroSuavizado(const cv::Mat& src)
{
    cv::Mat gray, dst;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = src;
    }
    cv::GaussianBlur(gray, dst, cv::Size(5, 5), 0);
    return dst;
}

// 8) Operaciones morfológicas (apertura + cierre)
cv::Mat aplicarOperacionesMorfo(const cv::Mat& src)
{
    cv::Mat gray, dst;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = src;
    }
    cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::morphologyEx(gray, dst, cv::MORPH_OPEN, element);
    cv::morphologyEx(dst, dst, cv::MORPH_CLOSE, element);
    return dst;
}

// 9) Otra técnica: Segmentación Watershed
cv::Mat aplicarOtraTecnica(const cv::Mat& src)
{
    // --- 1) Convertir a escala de grises ---
    cv::Mat gray;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = src.clone();
    }

    // --- 2) Ruido y suavizado ligero ---
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);

    // --- 3) Binarizar con umbral (para objetos brillantes sobre fondo oscuro) ---
    cv::Mat binary;
    cv::threshold(blurred, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // --- 4) Eliminar ruido aplicando morfología de apertura ---
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::Mat opening;
    cv::morphologyEx(binary, opening, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 2);

    // --- 5) Determinar fondo seguro (erosión) ---
    cv::Mat sureBg;
    cv::dilate(opening, sureBg, kernel, cv::Point(-1, -1), 3);

    // --- 6) Determinar primer plano seguro (distancia + umbral) ---
    cv::Mat dist;
    cv::distanceTransform(opening, dist, cv::DIST_L2, 5);
    // Normalizar para visualizar (opcional)
    // cv::normalize(dist, dist, 0, 1.0, cv::NORM_MINMAX);
    cv::Mat sureFg;
    double maxVal;
    cv::minMaxLoc(dist, nullptr, &maxVal);
    cv::threshold(dist, sureFg, 0.5 * maxVal, 255, cv::THRESH_BINARY);
    sureFg.convertTo(sureFg, CV_8U);

    // --- 7) Determinar regiones desconocidas ---
    cv::Mat unknown;
    cv::subtract(sureBg, sureFg, unknown);

    // --- 8) Etiquetar marcadores para Watershed ---
    cv::Mat markers;
    cv::connectedComponents(sureFg, markers);
    // Incrementar todos los marcadores en 1, para que el fondo sea 1 en lugar de 0
    markers += 1;
    // Los píxeles desconocidos se marcarán con 0
    for (int y = 0; y < unknown.rows; ++y) {
        for (int x = 0; x < unknown.cols; ++x) {
            if (unknown.at<uchar>(y, x) == 255) {
                markers.at<int>(y, x) = 0;
            }
        }
    }

    // --- 9) Convertir src a color para visualizar el resultado ---
    cv::Mat srcColor;
    if (src.channels() == 1) {
        cv::cvtColor(src, srcColor, cv::COLOR_GRAY2BGR);
    } else {
        srcColor = src.clone();
    }

    // --- 10) Aplicar Watershed ---
    cv::watershed(srcColor, markers);

    // --- 11) Crear imagen de salida coloreando cada región con un color distinto ---
    std::vector<cv::Vec3b> colors;
    int nMarkers = *std::max_element(markers.begin<int>(), markers.end<int>()) + 1;
    colors.resize(nMarkers);
    cv::RNG rng(12345);
    for (int i = 0; i < nMarkers; ++i) {
        colors[i] = cv::Vec3b(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
    }

    cv::Mat salida(markers.size(), CV_8UC3);
    for (int y = 0; y < markers.rows; ++y) {
        for (int x = 0; x < markers.cols; ++x) {
            int idx = markers.at<int>(y, x);
            if (idx <= 0 || idx > nMarkers - 1) {
                salida.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0); // bordes u “desconocidos”
            } else {
                salida.at<cv::Vec3b>(y, x) = colors[idx];
            }
        }
    }

    return salida;
}



// ----------------------------------------------------------
// 1) Convierte un ImageType2D a cv::Mat de 8 bits
//    (mantener igual que antes)
// ----------------------------------------------------------
cv::Mat ITKImage2DtoCVMat(const ImageType2D::Pointer& image2D)
{
    auto region2D = image2D->GetLargestPossibleRegion();
    auto size2D   = region2D.GetSize(); // size2D[0]=ancho, size2D[1]=alto

    cv::Mat mat16s(size2D[1], size2D[0], CV_16S);
    itk::ImageRegionConstIterator<ImageType2D> it(image2D, region2D);
    for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
        auto idx = it.GetIndex();
        short val = it.Get();
        mat16s.at<short>(static_cast<int>(idx[1]), static_cast<int>(idx[0])) = val;
    }

    double minVal, maxVal;
    cv::minMaxLoc(mat16s, &minVal, &maxVal);
    cv::Mat mat8u;
    if (maxVal > minVal) {
        mat16s.convertTo(
            mat8u,
            CV_8U,
            255.0 / (maxVal - minVal),
            -minVal * 255.0 / (maxVal - minVal)
        );
    } else {
        mat8u = cv::Mat::zeros(mat16s.size(), CV_8U);
    }
    return mat8u;
}

// ----------------------------------------------------------
// 2) Convierte un ImageType2D (máscara) a cv::Mat binaria
//    (mantener igual que antes)
// ----------------------------------------------------------
cv::Mat ITKMask2BinCVMat(const ImageType2D::Pointer& mask2D)
{
    auto region2D = mask2D->GetLargestPossibleRegion();
    auto size2D   = region2D.GetSize();

    cv::Mat matBin(size2D[1], size2D[0], CV_8U, cv::Scalar(0));
    itk::ImageRegionConstIterator<ImageType2D> it(mask2D, region2D);
    for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
        auto idx = it.GetIndex();
        short val = it.Get();
        if (val > 0) {
            matBin.at<uchar>(
                static_cast<int>(idx[1]),
                static_cast<int>(idx[0])
            ) = 255;
        }
    }
    return matBin;
}

// ----------------------------------------------------------
// 3) Procesamiento de un único slice: preprocesamiento y resaltado
//    Ahora recibe también 'filterOption' para saber qué función aplicar.
// ----------------------------------------------------------
void ProcesarYGuardarSlice(
    const cv::Mat& slice8u,
    const cv::Mat& maskBin,
    const fs::path& dirOrig,
    const fs::path& dirMask,
    const fs::path& dirHigh,
    unsigned int indiceZ,
    int filterOption
)
{
    cv::Mat processed;         // contendrá la imagen luego de aplicar el filtro elegido
    cv::Mat morphMask;         // para operaciones lógicas/mascara refinada si se necesita

    // ———  Selección del filtro a aplicar  ———
    switch (filterOption)
    {
        case 1:
            // Thresholding
            processed = aplicarThresholding(slice8u);
            break;
        case 2:
            // Contrast Stretching
            processed = aplicarContrastStretching(slice8u);
            break;
        case 3:
            // Binarización por umbral de color
            processed = aplicarBinarizacionColor(slice8u);
            break;
        case 4:
            // Operaciones lógicas: supongamos que aplicamos AND entre slice y mascara
            //   - tipoOp = 1 para AND. En NOT, la máscara no se usa.
            processed = aplicarOperacionLogica(slice8u, maskBin, 0);
            break;
        case 5:
            // Detección de Bordes (Canny)
            processed = aplicarDeteccionBordes(slice8u);
            break;
        case 6:
            // Manipulación de píxeles (ej. negativo)
            processed = aplicarManipulacionPixeles(slice8u);
            break;
        case 7:
            // Filtro de suavizado (GaussianBlur)
            processed = aplicarFiltroSuavizado(slice8u);
            break;
        case 8:
            // Operaciones morfológicas
            processed = aplicarOperacionesMorfo(slice8u);
            break;
        case 9:
            // Otra técnica (ecualización de histograma)
            processed = aplicarOtraTecnica(slice8u);
            break;
        case 10:
            // Aplicar todos los filtros EN SECUENCIA sobre el mismo slice
            {
                cv::Mat tmp = slice8u.clone();
                tmp = aplicarThresholding(tmp);
                tmp = aplicarContrastStretching(tmp);
                tmp = aplicarBinarizacionColor(tmp);
                tmp = aplicarOperacionLogica(tmp, maskBin, 0);
                tmp = aplicarDeteccionBordes(tmp);
                tmp = aplicarManipulacionPixeles(tmp);
                tmp = aplicarFiltroSuavizado(tmp);
                tmp = aplicarOperacionesMorfo(tmp);
                tmp = aplicarOtraTecnica(tmp);
                processed = tmp;
            }
            break;
        default:
            // Opcional: si la opción no coincide, devolvemos simplemente el slice ecualizado
            processed = slice8u.clone();
            break;
    }

    // ———  Refinamiento de la máscara usando operaciones morfológicas  ———
    cv::Mat maskRefined;
    cv::Mat elemento = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::morphologyEx(maskBin, maskRefined, cv::MORPH_OPEN, elemento); //MORPH_OPEN (erosión seguida de dilatación) 
    cv::morphologyEx(maskRefined, maskRefined, cv::MORPH_CLOSE, elemento); //MORPH_CLOSE (dilatación seguida de erosión)

    // ——— Construir overlay: combinar processed, máscara coloreada y bordes (del processed) ———
    // Convertimos processed a color para overlay
    cv::Mat processedColor;
    if (processed.channels() == 1)
        cv::cvtColor(processed, processedColor, cv::COLOR_GRAY2BGR);
    else
        processedColor = processed.clone();

    // Mapa de bordes sobre el resultado de "processed" (opción 5 o filtrado)
    cv::Mat edges;
    cv::Canny(processed, edges, 50, 150);
    cv::Mat edgeColor;
    cv::cvtColor(edges, edgeColor, cv::COLOR_GRAY2BGR);
    for (int y = 0; y < edgeColor.rows; ++y)
    {
        for (int x = 0; x < edgeColor.cols; ++x)
        {
            if (edgeColor.at<cv::Vec3b>(y, x) != cv::Vec3b(0, 0, 0))
                edgeColor.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 255, 0); // verde
        }
    }

    // Aplicar máscara roja semitransparente (ROI)
    cv::Mat overlay = processedColor.clone();
    for (int y = 0; y < overlay.rows; ++y)
    {
        for (int x = 0; x < overlay.cols; ++x)
        {
            if (maskRefined.at<uchar>(y, x) > 0)
                overlay.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 255);
        }
    }

    // Combinar processedColor + overlay + bordes
    cv::Mat highlighted;
    cv::addWeighted(processedColor, 0.6, overlay, 0.3, 0, highlighted);
    cv::addWeighted(highlighted, 0.8, edgeColor, 0.2, 0, highlighted); // Comentar si no se requiere bordes verdes

    // ——— Preparar nombres de archivos de salida ———
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "slice_%03u.png", indiceZ);

    fs::path rutaOrig     = dirOrig / buffer;
    fs::path rutaMaskImg  = dirMask / buffer;
    fs::path rutaHigh     = dirHigh / buffer;

    // Guardar cada imagen
    cv::imwrite(rutaOrig.string(), slice8u);       // processed “original” del filtro
    cv::imwrite(rutaMaskImg.string(), maskRefined);  // máscara refinada
    cv::imwrite(rutaHigh.string(), highlighted);     // Highlighted con ROI y bordes

    // std::cout << "Guardado slice " << indiceZ << " -> OriginalFiltro, Mask, Highlighted\n";
}
