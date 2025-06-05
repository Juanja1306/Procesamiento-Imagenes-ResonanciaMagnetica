// Principal.cpp
#include <iostream>
#include <string>
#include <filesystem>     // Para std::filesystem
#include "Utils.h"

// Prototipo del menú
int mostrarMenu();

int main()
{
    using namespace std;
    namespace fs = std::filesystem;

    cout << "=== Aplicación de procesamiento de RM (NIfTI) ===\n\n";

    // 1) Mostramos menú al usuario y leemos la opción
    int opcion = mostrarMenu();
    if (opcion < 1 || opcion > 11) {
        cerr << "[ERROR] Opción inválida.\n";
        return EXIT_FAILURE;
    }

    // 2) Rutas hardcodeadas (puedes cambiar según tus carpetas)
    const string rutaNifti         = "/home/juanja/Desktop/vision/Task06_Lung/imagesTr/lung_001.nii.gz";
    const string rutaMask          = "/home/juanja/Desktop/vision/Task06_Lung/labelsTr/lung_001.nii.gz";
    const string carpetaSalidaBase = "Output/";

    // 3) Si el usuario eligió la opción 11, generamos el video y salimos
    if (opcion == 11)
    {
        const string carpetaHighlighted = carpetaSalidaBase + "highlighted/";
        const string carpetaVideo       = carpetaSalidaBase + "video/";

        // ——— Contar cuántas imágenes hay en Output/highlighted/ ———
        int N = 0;
        if (fs::exists(carpetaHighlighted) && fs::is_directory(carpetaHighlighted)) {
            for (auto const& entry : fs::directory_iterator(carpetaHighlighted)) {
                if (!entry.is_regular_file()) continue;
                string ext = entry.path().extension().string();
                // Convertir ext a minúsculas
                for (auto& c : ext) c = static_cast<char>(tolower(c));
                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                    ext == ".bmp" || ext == ".tif" || ext == ".tiff")
                {
                    ++N;
                }
            }
        }

        if (N == 0) {
            cerr << "[ERROR] No se encontraron imágenes en '" << carpetaHighlighted << "'.\n";
            return EXIT_FAILURE;
        }

        // ——— Pedir al usuario los índices de inicio y fin ———
        int inicio, fin;
        cout << "Ingrese índice inicial (1–" << N << "): ";
        cin >> inicio;
        if (cin.fail() || inicio < 1 || inicio > N) {
            cerr << "[ERROR] Índice inicial inválido. Debe estar entre 1 y " << N << ".\n";
            return EXIT_FAILURE;
        }

        cout << "Ingrese índice final (" << inicio << "–" << N << "): ";
        cin >> fin;
        if (cin.fail() || fin < inicio || fin > N) {
            cerr << "[ERROR] Índice final inválido. Debe estar entre " << inicio << " y " << N << ".\n";
            return EXIT_FAILURE;
        }

        // ——— Llamar a la función que genera el video con el rango [inicio, fin] ———
        bool ok = GenerarVideoHighlighted(carpetaHighlighted, carpetaVideo, inicio, fin);
        if (!ok) {
            cerr << "[ERROR] No se pudo generar el video. Verifica que existan archivos válidos en '"
                 << carpetaHighlighted << "'.\n";
            return EXIT_FAILURE;
        }

        cout << "Video generado correctamente en '" << carpetaVideo << "'.\n";
        return EXIT_SUCCESS;
    }

    // 4) Para opciones 1–10: procesamos todos los slices usando el filtro elegido
    cout << "Leyendo volúmenes y procesando todos los slices...\n";
    bool ok = ProcesarTodosSlices(rutaNifti, rutaMask, carpetaSalidaBase, opcion);
    if (!ok) {
        cerr << "[ERROR] Falló el procesamiento de slices.\n";
        return EXIT_FAILURE;
    }

    cout << "Proceso completado correctamente.\n";
    return EXIT_SUCCESS;
}

// Muestra el menú y devuelve la opción elegida (1–11)
int mostrarMenu()
{
    using namespace std;
    cout << "Selecciona UN solo filtro a aplicar:\n";
    cout << " 1) Thresholding\n";
    cout << " 2) Contrast Stretching\n";
    cout << " 3) Binarización por umbral de color\n";
    cout << " 4) Operaciones lógicas (NOT, AND, OR, XOR)\n";
    cout << " 5) Detección de Bordes\n";
    cout << " 6) Manipulación de píxeles\n";
    cout << " 7) Filtros de suavizado\n";
    cout << " 8) Operaciones morfológicas\n";
    cout << " 9) Segmentación Watershed\n";
    cout << "10) Aplicar TODOS los filtros en secuencia\n";
    cout << "11) Generar video con las imágenes en Output/highlighted/\n";
    cout << "Opción (1–11): ";

    int opc;
    cin >> opc;
    return opc;
}
