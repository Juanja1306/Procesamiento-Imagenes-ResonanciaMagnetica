// MainWindow.cpp
#include "MainWindow.h"
#include "VideoDialog.h"
#include "Utils.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QPixmap>
#include <QImage>
#include <QDesktopServices>
#include <QUrl>
#include <filesystem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      rutaImagenVolumetrica(""),
      rutaMascaraVolumetrica(""),
      carpetaSalidaBase("Output/"),
      numSlices(0)
{
    setWindowTitle("Procesamiento de Resonancia Magnética (NIfTI) - Qt");

    // ----- 1) Crear widgets principales -----
    btnLoadImage   = new QPushButton("Cargar imagen original");
    btnLoadMask    = new QPushButton("Cargar máscara");
    lblImagePath   = new QLabel("No se ha cargado imagen.");
    lblMaskPath    = new QLabel("No se ha cargado máscara.");

    comboFilter    = new QComboBox();
    comboFilter->addItem("1) Thresholding");
    comboFilter->addItem("2) Contrast Stretching");
    comboFilter->addItem("3) Binarización por umbral de color");
    comboFilter->addItem("4) Operaciones lógicas (NOT, AND, OR, XOR)");
    comboFilter->addItem("5) Detección de bordes");
    comboFilter->addItem("6) Manipulación de píxeles");
    comboFilter->addItem("7) Filtros de suavizado");
    comboFilter->addItem("8) Operaciones morfológicas");
    comboFilter->addItem("9) Segmentación Watershed");
    comboFilter->addItem("10) Aplicar TODOS los filtros en secuencia");

    btnApplyFilter = new QPushButton("Aplicar filtro");

    // Tres QLabel para mostrar original, máscara y filtrada
    lblOriginalView  = new QLabel();
    lblMaskView      = new QLabel();
    lblFilteredView  = new QLabel();
    QList<QLabel*> labelViews = {lblOriginalView, lblMaskView, lblFilteredView};
    for (auto *lbl : labelViews) {
        lbl->setFixedSize(512, 512);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("QLabel { background-color : #202020; }");
    }

    sliderSlice    = new QSlider(Qt::Horizontal);
    sliderSlice->setEnabled(false);

    btnMakeVideo   = new QPushButton("Hacer video");
    btnOpenVideo   = new QPushButton("Abrir video");
    btnOpenVideo->setEnabled(false);  // Desactivado hasta que se genere un video

    // ----- 2) Conectar señales y slots -----
    connect(btnLoadImage,   &QPushButton::clicked, this, &MainWindow::onLoadImage);
    connect(btnLoadMask,    &QPushButton::clicked, this, &MainWindow::onLoadMask);
    connect(btnApplyFilter, &QPushButton::clicked, this, &MainWindow::onApplyFilter);
    connect(sliderSlice,    &QSlider::valueChanged, this, &MainWindow::onSliderValueChanged);
    connect(btnMakeVideo,   &QPushButton::clicked, this, &MainWindow::onMakeVideo);
    connect(btnOpenVideo,   &QPushButton::clicked, this, &MainWindow::onOpenVideo);

    // ----- 3) Layout general -----
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // Línea carga original + ruta
    QHBoxLayout *h1 = new QHBoxLayout();
    h1->addWidget(btnLoadImage);
    h1->addWidget(lblImagePath);
    mainLayout->addLayout(h1);

    // Línea carga máscara + ruta
    QHBoxLayout *h2 = new QHBoxLayout();
    h2->addWidget(btnLoadMask);
    h2->addWidget(lblMaskPath);
    mainLayout->addLayout(h2);

    mainLayout->addSpacing(10);

    // Dropdown de filtros
    QHBoxLayout *h3 = new QHBoxLayout();
    QLabel *lblFilter = new QLabel("Filtro a aplicar:");
    h3->addWidget(lblFilter);
    h3->addWidget(comboFilter);
    mainLayout->addLayout(h3);

    mainLayout->addWidget(btnApplyFilter);
    mainLayout->addSpacing(10);

    // HBox con las tres vistas (original, máscara, filtrada)
    QHBoxLayout *hImages = new QHBoxLayout();
    hImages->addWidget(lblOriginalView);
    hImages->addWidget(lblMaskView);
    hImages->addWidget(lblFilteredView);
    mainLayout->addLayout(hImages);

    mainLayout->addWidget(sliderSlice);

    // HBox para los dos botones: “Hacer video” y “Abrir video”
    QHBoxLayout *hVideo = new QHBoxLayout();
    hVideo->addWidget(btnMakeVideo);
    hVideo->addWidget(btnOpenVideo);
    mainLayout->addLayout(hVideo);

    setCentralWidget(central);
}

MainWindow::~MainWindow()
{
    // Destructor vacío
}

void MainWindow::onLoadImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Seleccionar imagen volumétrica NIfTI",
        "",
        "NIfTI files (*.nii *.nii.gz)"
    );
    if (fileName.isEmpty()) return;

    rutaImagenVolumetrica = fileName;
    lblImagePath->setText(fileName);
}

void MainWindow::onLoadMask()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Seleccionar máscara NIfTI",
        "",
        "NIfTI files (*.nii *.nii.gz)"
    );
    if (fileName.isEmpty()) return;

    rutaMascaraVolumetrica = fileName;
    lblMaskPath->setText(fileName);
}

void MainWindow::onApplyFilter()
{
    if (rutaImagenVolumetrica.isEmpty() || rutaMascaraVolumetrica.isEmpty()) {
        QMessageBox::warning(this, "Error", "Debe cargar la imagen original y la máscara primero.");
        return;
    }

    // Limpiar carpetas original, mask y highlighted
    QDir dirOrig(carpetaSalidaBase + "original/");
    if (dirOrig.exists()) {
        dirOrig.removeRecursively();
    }
    QDir().mkpath(carpetaSalidaBase + "original/");

    QDir dirMask(carpetaSalidaBase + "mask/");
    if (dirMask.exists()) {
        dirMask.removeRecursively();
    }
    QDir().mkpath(carpetaSalidaBase + "mask/");

    QDir dirHigh(carpetaSalidaBase + "highlighted/");
    if (dirHigh.exists()) {
        dirHigh.removeRecursively();
    }
    QDir().mkpath(carpetaSalidaBase + "highlighted/");

    // Desactivar el botón “Abrir video” cada vez que se vuelva a aplicar un filtro
    btnOpenVideo->setEnabled(false);

    // Seleccionar filtro (1–10)
    int idx = comboFilter->currentIndex();
    int filtroSeleccionado = idx + 1;

    bool success = ProcesarTodosSlices(
        rutaImagenVolumetrica.toStdString(),
        rutaMascaraVolumetrica.toStdString(),
        carpetaSalidaBase.toStdString(),
        filtroSeleccionado
    );

    if (!success) {
        QMessageBox::critical(this, "Error", "Falló el procesamiento de slices.");
        return;
    }

    QMessageBox::information(this, "Éxito", "Procesamiento completado correctamente.");

    // Actualizar slider y cargar slice 0
    updateSliderRange();
}

void MainWindow::updateSliderRange()
{
    namespace fs = std::filesystem;
    QString dirPath = carpetaSalidaBase + "highlighted/";
    int countPNG = 0;

    if (fs::exists(dirPath.toStdString()) && fs::is_directory(dirPath.toStdString())) {
        for (auto const& entry : fs::directory_iterator(dirPath.toStdString())) {
            if (!entry.is_regular_file()) continue;
            std::string ext = entry.path().extension().string();
            for (auto &c: ext) c = static_cast<char>(tolower(c));
            if (ext == ".png") {
                ++countPNG;
            }
        }
    }

    numSlices = countPNG;
    if (numSlices > 0) {
        sliderSlice->setEnabled(true);
        sliderSlice->setMinimum(0);
        sliderSlice->setMaximum(numSlices - 1);
        sliderSlice->setValue(0);
        onSliderValueChanged(0);
    } else {
        sliderSlice->setEnabled(false);
        lblOriginalView->setText("Sin original");
        lblMaskView->setText("Sin máscara");
        lblFilteredView->setText("Sin filtrada");
    }
}

void MainWindow::onSliderValueChanged(int value)
{
    if (numSlices <= 0) return;

    // Construir nombre de archivo slice_XXX.png
    QString nombreSlice = QString("slice_%1.png").arg(value, 3, 10, QChar('0'));

    // 1) Cargar original: Output/original/slice_XXX.png
    QString rutaOrig = carpetaSalidaBase + "original/" + nombreSlice;
    QImage imgOrig(rutaOrig);
    if (imgOrig.isNull()) {
        lblOriginalView->setText("No se pudo cargar:\n" + rutaOrig);
    } else {
        QPixmap pixOrig = QPixmap::fromImage(imgOrig).scaled(
            lblOriginalView->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        lblOriginalView->setPixmap(pixOrig);
    }

    // 2) Cargar máscara: Output/mask/slice_XXX.png
    QString rutaMaskImg = carpetaSalidaBase + "mask/" + nombreSlice;
    QImage imgMask(rutaMaskImg);
    if (imgMask.isNull()) {
        lblMaskView->setText("No se pudo cargar:\n" + rutaMaskImg);
    } else {
        QPixmap pixMask = QPixmap::fromImage(imgMask).scaled(
            lblMaskView->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        lblMaskView->setPixmap(pixMask);
    }

    // 3) Cargar filtrada: Output/highlighted/slice_XXX.png
    QString rutaFilt = carpetaSalidaBase + "highlighted/" + nombreSlice;
    QImage imgFilt(rutaFilt);
    if (imgFilt.isNull()) {
        lblFilteredView->setText("No se pudo cargar:\n" + rutaFilt);
    } else {
        QPixmap pixFilt = QPixmap::fromImage(imgFilt).scaled(
            lblFilteredView->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        lblFilteredView->setPixmap(pixFilt);
    }
}

void MainWindow::onMakeVideo()
{
    namespace fs = std::filesystem;
    QString carpetaHigh = carpetaSalidaBase + "highlighted/";
    if (!fs::exists(carpetaHigh.toStdString()) || !fs::is_directory(carpetaHigh.toStdString())) {
        QMessageBox::warning(this, "Error", "No existe la carpeta Output/highlighted/");
        return;
    }

    int N = 0;
    for (auto const& entry : fs::directory_iterator(carpetaHigh.toStdString())) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        for (auto &c: ext) c = static_cast<char>(tolower(c));
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tif" || ext == ".tiff") {
            ++N;
        }
    }
    if (N == 0) {
        QMessageBox::warning(this, "Error", "No se encontraron imágenes en Output/highlighted/");
        return;
    }

    VideoDialog dlg(N, this);
    if (dlg.exec() != QDialog::Accepted) return;

    int inicio = dlg.getStart();
    int fin    = dlg.getEnd();

    QString carpetaVideo = carpetaSalidaBase + "video/";
    QDir().mkpath(carpetaVideo);

    bool ok = GenerarVideoHighlighted(
        carpetaHigh.toStdString(),
        carpetaVideo.toStdString(),
        inicio,
        fin
    );
    if (!ok) {
        QMessageBox::critical(this, "Error", "Falló la generación del video.");
        return;
    }

    QMessageBox::information(
        this,
        "Video creado",
        "Video guardado en: " + carpetaVideo + "highlighted_video.avi"
    );

    // Habilitar el botón “Abrir video” pues el AVI ya está creado
    btnOpenVideo->setEnabled(true);
}

void MainWindow::onOpenVideo()
{
    // Construir ruta completa del AVI
    QString videoPath = carpetaSalidaBase + "video/highlighted_video.avi";

    // Verificar que exista
    if (!QFile::exists(videoPath)) {
        QMessageBox::warning(this, "Error", "No se encontró el archivo: " + videoPath);
        return;
    }

    // Abrir con la aplicación predeterminada
    QDesktopServices::openUrl(QUrl::fromLocalFile(videoPath));
}
