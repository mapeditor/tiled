#include "addpropertydialog.h"
#include "ui_addpropertydialog.h"

AddPropertyDialog::AddPropertyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddPropertyDialog)
{
    ui->setupUi(this);

    // Add possible types from QVariant
    ui->typeBox->addItem(QLatin1String(QVariant::typeToName(QVariant::Bool)));

    ui->typeBox->addItem(QLatin1String(QVariant::typeToName(QVariant::Int)));

    ui->typeBox->addItem(QLatin1String("float")); /* Double */

    // Rename QString, QSize and QPoint to something else
    ui->typeBox->addItem(QLatin1String("String"));    /* QString */


    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    //Select String as Default
    ui->typeBox->setCurrentText(QLatin1String("String"));

}

AddPropertyDialog::~AddPropertyDialog()
{
    delete ui;
}

QString AddPropertyDialog::getPropertyName()
{
    return ui->name->text();
}

QVariant::Type AddPropertyDialog::getPropertyType()
{
    QString typeText = ui->typeBox->currentText();

    // Returned the correct type according to the name
    if(typeText == QLatin1String("float")){ return QVariant::Double;}
    if(typeText == QLatin1String("String")){ return QVariant::String;}

    if(typeText == QLatin1String("Size")){ return QVariant::Size;}
    if(typeText == QLatin1String("Size Float")){ return QVariant::SizeF;}
    if(typeText == QLatin1String("Point")){ return QVariant::Point;}
    if(typeText == QLatin1String("Point Float")){ return QVariant::PointF;}

    return QVariant::nameToType(typeText.toLatin1().constData());
}

void AddPropertyDialog::on_name_textChanged(const QString &arg1)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!arg1.isEmpty());
}
