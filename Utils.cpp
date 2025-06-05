// Utils.cpp
#include "Utils.h"
#include <iostream>               // para std::cerr y std::cout
#include <opencv2/core.hpp>       // para cv::Mat
#include <opencv2/imgcodecs.hpp>  // para cv::imwrite
#include "Filtros.h"              // para ITKImage2DtoCVMat, ITKMask2BinCVMat, ProcesarYGuardarSlice
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <vector>
#include <algorithm>

bool GenerarVideoHighlighted(
    const std::string& carpetaHighlighted,
    const std::string& carpetaVideo,
    int inicio,
    int fin
)
{
    namespace fs = std::filesystem;

    // 1) Verificar que carpetaHighlighted exista y sea directorio
    fs::path pathH = carpetaHighlighted;
    if (!fs::exists(pathH) || !fs::is_directory(pathH)) {
        std::cerr << "[ERROR] La carpeta '" << carpetaHighlighted << "' no existe o no es un directorio.\n";
        return false;
    }

    // 2) Recorrer todos los archivos dentro de carpetaHighlighted y guardar rutas de imágenes
    std::vector<fs::path> listaImagenes;
    for (auto const& entry : fs::directory_iterator(pathH)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        // Convertimos la extensión a minúsculas
        for (auto& c : ext) c = static_cast<char>(tolower(c));
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
            ext == ".bmp" || ext == ".tif" || ext == ".tiff")
        {
            listaImagenes.push_back(entry.path());
        }
    }

    if (listaImagenes.empty()) {
        std::cerr << "[ERROR] No se encontraron imágenes en '" << carpetaHighlighted << "'.\n";
        return false;
    }

    // 3) Ordenar alfabéticamente (para asegurar la secuencia correcta de slices)
    std::sort(listaImagenes.begin(), listaImagenes.end());

    // 4) Validar rangos (revisados ya en main, pero por seguridad):
    int total = static_cast<int>(listaImagenes.size());
    if (inicio < 1 || fin < inicio || fin > total) {
        std::cerr << "[ERROR] Rangos inválidos: inicio=" << inicio << ", fin=" << fin
                  << ". Debe ser 1 ≤ inicio ≤ fin ≤ " << total << ".\n";
        return false;
    }

    // 5) Leer la imagen en la posición 'inicio' para obtener dimensiones (asumimos todas iguales)
    cv::Mat primera = cv::imread(listaImagenes[inicio - 1].string());
    if (primera.empty()) {
        std::cerr << "[ERROR] No se pudo leer la imagen: "
                  << listaImagenes[inicio - 1] << "\n";
        return false;
    }
    int altura = primera.rows;
    int ancho  = primera.cols;

    // 6) Crear carpetaVideo si no existe
    fs::path pathV = carpetaVideo;
    try {
        fs::create_directories(pathV);
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] No se pudo crear carpeta '" << carpetaVideo
                  << "': " << e.what() << "\n";
        return false;
    }

    // 7) Configurar VideoWriter
    std::string salidaVideo = (pathV / "highlighted_video.avi").string();
    int fps = 10;
    cv::VideoWriter writer;
    writer.open(salidaVideo,
                cv::VideoWriter::fourcc('M','J','P','G'),
                fps,
                cv::Size(ancho, altura),
                true  // true = color
    );
    if (!writer.isOpened()) {
        std::cerr << "[ERROR] No se pudo abrir VideoWriter en: " << salidaVideo << "\n";
        return false;
    }

    // 8) Recorrer las imágenes desde (inicio-1) hasta (fin-1) y escribirlas como fotogramas
    for (int idx = inicio - 1; idx <= fin - 1; ++idx) {
        cv::Mat frame = cv::imread(listaImagenes[idx].string());
        if (frame.empty()) {
            std::cerr << "[WARNING] Saltando imagen no leída: " << listaImagenes[idx] << "\n";
            continue;
        }
        // Si difiere de tamaño, redimensionar
        if (frame.rows != altura || frame.cols != ancho) {
            cv::resize(frame, frame, cv::Size(ancho, altura));
        }
        writer.write(frame);
    }

    // 9) Liberar recursos (cierra el archivo AVI)
    writer.release();

    std::cout << "[INFO] Video guardado en: " << salidaVideo << "\n";
    return true;
}


bool ProcesarTodosSlices(
    const std::string& rutaNifti,
    const std::string& rutaMask,
    const std::string& carpetaSalidaBase,
    int filterOption
)
{
    // Tipos de reader 3D de ITK
    using ReaderType3D = itk::ImageFileReader<ImageType3D>;

    // --- 1) Leer volumen de imagen ---
    ReaderType3D::Pointer readerImg = ReaderType3D::New();
    auto niftiIOImg = itk::NiftiImageIO::New();
    readerImg->SetImageIO(niftiIOImg);
    readerImg->SetFileName(rutaNifti);

    try
    {
        readerImg->Update();
    }
    catch (itk::ExceptionObject& err)
    {
        std::cerr << "[ERROR] Leyendo NIfTI imagen '" << rutaNifti << "': "
                  << err << "\n";
        return false;
    }
    auto image3D = readerImg->GetOutput();

    // --- 2) Leer volumen de máscara ---
    using ReaderTypeMask3D = itk::ImageFileReader<ImageType3D>;
    ReaderTypeMask3D::Pointer readerMask = ReaderTypeMask3D::New();
    auto niftiIOMask = itk::NiftiImageIO::New();
    readerMask->SetImageIO(niftiIOMask);
    readerMask->SetFileName(rutaMask);

    try
    {
        readerMask->Update();
    }
    catch (itk::ExceptionObject& err)
    {
        std::cerr << "[ERROR] Leyendo NIfTI máscara '" << rutaMask << "': "
                  << err << "\n";
        return false;
    }
    auto mask3D = readerMask->GetOutput();

    // --- 3) Obtener región y tamaño en Z ---
    auto region3D = image3D->GetLargestPossibleRegion();
    auto size3D   = region3D.GetSize(); // size3D[2] = número de slices en Z

    // --- 4) Crear carpetas de salida: original, mask, highlighted ---
    fs::path outDirBase{ carpetaSalidaBase };
    fs::path dirOrig    = outDirBase / "original";
    fs::path dirMaskOut = outDirBase / "mask";
    fs::path dirHigh    = outDirBase / "highlighted";

    try
    {
        fs::create_directories(dirOrig);
        fs::create_directories(dirMaskOut);
        fs::create_directories(dirHigh);
    }
    catch (std::exception& e)
    {
        std::cerr << "[ERROR] No se pudieron crear carpetas de salida: "
                  << e.what() << "\n";
        return false;
    }

    // --- 5) Recorrer cada slice en Z ---
    for (unsigned int z = 0; z < size3D[2]; ++z)
    {
        // ----- 5.1) Extraer slice de la imagen 3D -----
        using ExtractFilterTypeImg = itk::ExtractImageFilter<ImageType3D, ImageType2D>;
        ExtractFilterTypeImg::Pointer extractorImg = ExtractFilterTypeImg::New();
        extractorImg->SetInput(image3D);

        ImageType3D::RegionType desiredRegionImg = region3D;
        ImageType3D::IndexType startImg = desiredRegionImg.GetIndex();
        ImageType3D::SizeType sizeImg = desiredRegionImg.GetSize();

        startImg[2] = z;
        sizeImg[2]  = 0;
        desiredRegionImg.SetIndex(startImg);
        desiredRegionImg.SetSize(sizeImg);

        extractorImg->SetExtractionRegion(desiredRegionImg);
        extractorImg->SetDirectionCollapseToIdentity();

        try
        {
            extractorImg->Update();
        }
        catch (itk::ExceptionObject& err)
        {
            std::cerr << "[ERROR] Extrayendo slice Z=" << z << " de imagen: "
                      << err << "\n";
            continue; // pasa al siguiente slice
        }
        auto sliceImg2D = extractorImg->GetOutput();
        cv::Mat matSlice = ITKImage2DtoCVMat(sliceImg2D);

        // ----- 5.2) Extraer slice de la máscara 3D -----
        using ExtractFilterTypeMask = itk::ExtractImageFilter<ImageType3D, ImageType2D>;
        ExtractFilterTypeMask::Pointer extractorMask2D = ExtractFilterTypeMask::New();
        extractorMask2D->SetInput(mask3D);

        ImageType3D::RegionType desiredRegionMask = region3D;
        ImageType3D::IndexType startMask = desiredRegionMask.GetIndex();
        ImageType3D::SizeType sizeMask = desiredRegionMask.GetSize();

        startMask[2] = z;
        sizeMask[2]  = 0;
        desiredRegionMask.SetIndex(startMask);
        desiredRegionMask.SetSize(sizeMask);

        extractorMask2D->SetExtractionRegion(desiredRegionMask);
        extractorMask2D->SetDirectionCollapseToIdentity();

        try
        {
            extractorMask2D->Update();
        }
        catch (itk::ExceptionObject& err)
        {
            std::cerr << "[ERROR] Extrayendo slice Z=" << z << " de máscara: "
                      << err << "\n";
            continue;
        }
        auto sliceMask2D = extractorMask2D->GetOutput();
        cv::Mat matMask = ITKMask2BinCVMat(sliceMask2D);

        // ----- 5.3) Procesar Y GUARDAR, aplicando solo el filtro elegido (filterOption) -----
        ProcesarYGuardarSlice(matSlice, matMask, dirOrig, dirMaskOut, dirHigh, z, filterOption);
    }

    return true;
}
