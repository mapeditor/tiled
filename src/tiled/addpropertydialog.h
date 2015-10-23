#ifndef ADDPROPERTYDIALOG_H
#define ADDPROPERTYDIALOG_H

#include <QDialog>

namespace Ui {
class AddPropertyDialog;
}

class AddPropertyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddPropertyDialog(QWidget *parent = 0);
    ~AddPropertyDialog();


    QString getPropertyName();
    QVariant::Type getPropertyType();




private slots:
    void on_name_textChanged(const QString &arg1);

private:
    Ui::AddPropertyDialog *ui;
};

#endif // ADDPROPERTYDIALOG_H
