#ifndef GOTODIALOG_H
#define GOTODIALOG_H

#include <QDialog>
#include <QLineEdit>

class GotoDialog : public QDialog
{
    Q_OBJECT

signals:
        void changeCoordinates(int x, int y);

public:
        GotoDialog(QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
        static GotoDialog *instance() { return mInstance; }
        static GotoDialog *showDialog();

private:
        void goToCoordinates();
        QLineEdit* lineEditX;
        QLineEdit* lineEditY;

        static GotoDialog *mInstance;
};

#endif // GOTODIALOG_H
