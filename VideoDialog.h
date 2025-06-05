#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include <QDialog>

class QSpinBox;
class QLabel;
class QPushButton;

class VideoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VideoDialog(int maxIndex, QWidget *parent = nullptr);
    int getStart() const;
    int getEnd() const;

private:
    QSpinBox *spinStart;
    QSpinBox *spinEnd;
    QLabel   *lblTotal;
    QPushButton *btnOk;
    QPushButton *btnCancel;

    int maxSlices;
};

#endif // VIDEODIALOG_H
