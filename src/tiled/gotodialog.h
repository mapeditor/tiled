#ifndef GOTODIALOG_H
#define GOTODIALOG_H

#include <QApplication>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>

namespace Tiled {

namespace Internal {

class GotoDialog : public QDialog
{
    Q_OBJECT

public:
    GotoDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    static GotoDialog *instance() { return mInstance; }
    static GotoDialog *showDialog();

signals:
    void changeCoordinates(int x, int y);

private:
    void goToCoordinates();
    QLineEdit *lineEditX;
    QLineEdit *lineEditY;

    static GotoDialog *mInstance;
};

}

}

#endif // GOTODIALOG_H
