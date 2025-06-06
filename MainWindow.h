// MainWindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

class QPushButton;
class QLabel;
class QComboBox;
class QSlider;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadImage();
    void onLoadMask();
    void onApplyFilter();
    void onSliderValueChanged(int value);
    void onMakeVideo();
    void onOpenVideo();              // Slot para abrir el video
    void onStats();                  // Slot para mostrar estadísticas

private:
    // Rutas seleccionadas
    QString rutaImagenVolumetrica;
    QString rutaMascaraVolumetrica;

    // Carpeta base para salida (“Output/”)
    QString carpetaSalidaBase;

    // Widgets de la interfaz
    QPushButton *btnLoadImage;
    QPushButton *btnLoadMask;
    QLabel      *lblImagePath;
    QLabel      *lblMaskPath;

    QComboBox   *comboFilter;
    QPushButton *btnApplyFilter;

    // Tres QLabel para mostrar original, máscara y filtrada
    QLabel      *lblOriginalView;
    QLabel      *lblMaskView;
    QLabel      *lblFilteredView;
    QSlider     *sliderSlice;

    // Botones “Hacer video” y “Abrir video”
    QPushButton *btnMakeVideo;
    QPushButton *btnOpenVideo;
    QPushButton *btnStats;

    int numSlices;

    void updateSliderRange();
};

#endif // MAINWINDOW_H
