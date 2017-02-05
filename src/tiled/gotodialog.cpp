#include "gotodialog.h"

#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QIntValidator>
#include <QString>

GotoDialog *GotoDialog::mInstance;

GotoDialog::GotoDialog(QWidget *parent, Qt::WindowFlags f )
    : QDialog(parent,Qt::Tool)
{
    setWindowTitle(tr("Go to"));

    QGroupBox* horizontalGroupBox = new QGroupBox(tr("Go to tile:"));
    QHBoxLayout *layout = new QHBoxLayout;
    horizontalGroupBox->setLayout(layout);
    QLabel* labelX = new QLabel(tr("X:"));
    QLabel* labelY = new QLabel(tr("Y:"));
    lineEditX = new QLineEdit(tr("0"));
    lineEditY = new QLineEdit(tr("0"));
    QPushButton* goButton = new QPushButton(tr("Go"));

    lineEditX->setMaximumWidth(50);
    lineEditY->setMaximumWidth(50);
    lineEditX->setValidator(new QIntValidator);
    lineEditY->setValidator(new QIntValidator);

    layout->addWidget(labelX);
    layout->addWidget(lineEditX);
    layout->addWidget(labelY);
    layout->addWidget(lineEditY);
    layout->addWidget(goButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(horizontalGroupBox);

    connect(goButton, &QAbstractButton::clicked, this, &GotoDialog::goToCoordinates);

    setLayout(mainLayout);
}

GotoDialog* GotoDialog::showDialog()
{
    if (!mInstance) {
        QWidget* parentWidget = QApplication::activeWindow();
        mInstance = new GotoDialog(parentWidget);
        //connect(gotoDialog, &GotoDialog::changeCoordinates, this, &Dialog::updateCoordinates);
    }

    mInstance->show();
    mInstance->raise();
    mInstance->activateWindow();

    return mInstance;
}

void GotoDialog::goToCoordinates()
{
    /*QString textX = lineEditX->text();
    QString textY = lineEditY->text();

    emit changeCoordinates(textX.toInt(),textY.toInt());*/

    close();
}
