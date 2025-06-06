// VideoDialog.cpp
#include "VideoDialog.h"
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIntValidator>

VideoDialog::VideoDialog(int maxIndex, QWidget *parent)
    : QDialog(parent),
      maxSlices(maxIndex)
{
    setWindowTitle("Generar video (índices)");

    // Label que indica el total de imágenes
    lblTotal = new QLabel(tr("Total de imágenes en highlighted: %1").arg(maxSlices));

    // SpinBox para índice inicial
    spinStart = new QSpinBox();
    spinStart->setRange(1, maxSlices);
    spinStart->setValue(1);

    // SpinBox para índice final
    spinEnd = new QSpinBox();
    spinEnd->setRange(1, maxSlices);
    spinEnd->setValue(maxSlices);

    // Botones Aceptar/Cancelar
    btnOk     = new QPushButton("Aceptar");
    btnCancel = new QPushButton("Cancelar");

    connect(btnOk,     &QPushButton::clicked, this, &VideoDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &VideoDialog::reject);

    // Cuando cambie spinStart, forzar que spinEnd >= spinStart
    connect(spinStart, QOverload<int>::of(&QSpinBox::valueChanged), 
            [=](int v){ 
                spinEnd->setMinimum(v); 
            });

    // Layout del formulario
    QFormLayout *form = new QFormLayout();
    form->addRow(lblTotal);
    form->addRow("Índice inicial (1–" + QString::number(maxSlices) + "):", spinStart);
    form->addRow("Índice final (" + QString::number(spinStart->value()) + "–" 
                 + QString::number(maxSlices) + "):", spinEnd);

    // Botones abajo
    QHBoxLayout *hButtons = new QHBoxLayout();
    hButtons->addStretch();
    hButtons->addWidget(btnOk);
    hButtons->addWidget(btnCancel);

    // Layout final
    QVBoxLayout *vMain = new QVBoxLayout(this);
    vMain->addLayout(form);
    vMain->addStretch();
    vMain->addLayout(hButtons);

    setLayout(vMain);
    setFixedSize(350, 200);
}

int VideoDialog::getStart() const
{
    return spinStart->value();
}

int VideoDialog::getEnd() const
{
    return spinEnd->value();
}
