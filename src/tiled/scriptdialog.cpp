/*
 * scriptdialog.cpp
 * Copyright 2020, David Konsumer <konsumer@jetboystudio.com>
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "scriptmanager.h"
#include "mainwindow.h"
#include "colorbutton.h"
#include <QString>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QProcessEnvironment>
#include <QJSEngine>
#include <QCoreApplication>
#include <QDialog>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif

#include <memory>

namespace Tiled {
    class DoubleInputArgs : QObject {
    public:
        int numberOfDecimals;
        double maximum;
        double minimum;
        QString &prefix;
    };

class ScriptDialog : public QDialog
{
    Q_OBJECT

public:
    Q_INVOKABLE ScriptDialog();
    ~ScriptDialog() override;

    bool checkForClosed() const;


    Q_INVOKABLE void setTitle(const QString &title);
    Q_INVOKABLE QLabel * addLabel(const QString &text);
    Q_INVOKABLE QFrame* addSeparator();
    Q_INVOKABLE QDoubleSpinBox *addDoubleInput(const QString &labelText);
    Q_INVOKABLE QSlider * addSlider(const QString &labelText);
    Q_INVOKABLE QCheckBox * addCheckbox(const QString &labelText, bool defaultValue);
    Q_INVOKABLE QPushButton * addButton(const QString &labelText);
    Q_INVOKABLE ColorButton * addColorButton(const QString &labelText);
    Q_INVOKABLE void setMinimumWidth(const int width);
    Q_INVOKABLE void close();
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    // used to generate unique IDs for all components
    int m_widgetNumber;
    QList<QWidget*> m_ScriptWidgets;
    QGridLayout *m_gridLayout;
    QWidget *m_verticalLayoutWidget;
    QVBoxLayout *m_verticalLayout;
};

ScriptDialog::ScriptDialog(): QDialog(MainWindow::maybeInstance())
{
    m_widgetNumber = 0;
    setMinimumSize(400, 400);
    m_gridLayout = new QGridLayout(this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_gridLayout->setMargin(0);
    m_gridLayout->setObjectName(QString::fromUtf8("m_gridLayout"));
    m_verticalLayoutWidget = new QWidget(this);
    m_verticalLayoutWidget->setGeometry(QRect(19, 19, 350, 350));
    m_verticalLayoutWidget->setObjectName(QString::fromUtf8("m_verticalLayoutWidget"));
    m_verticalLayoutWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_verticalLayout = new QVBoxLayout(m_verticalLayoutWidget);
    m_verticalLayout->setMargin(0);
    m_verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    m_gridLayout->addLayout(m_verticalLayout, 0, 0, 1, 1);
    QMetaObject::connectSlotsByName(this);
}

void ScriptDialog::setMinimumWidth(const int width) {
    QDialog::setMinimumWidth(width);
}
void ScriptDialog::resizeEvent(QResizeEvent* event)
{
    m_gridLayout->invalidate();
}
QLabel * ScriptDialog::addLabel(const QString &text){
    QLabel * label;
    label = new QLabel(m_verticalLayoutWidget);
    label->setObjectName(QString::fromUtf8("label%1").arg(m_widgetNumber));
    m_widgetNumber++;
    label->setContentsMargins(10, 0, 10, 0);
    label->setText(text);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setWordWrap(true);
    m_verticalLayout->addWidget(label);
    m_ScriptWidgets.append(label);
    return label;
}

QFrame* ScriptDialog::addSeparator(){
    QFrame *line;

    line = new QFrame(m_verticalLayoutWidget);
    line->setObjectName(QString::fromUtf8("line%1").arg(m_widgetNumber));
    m_widgetNumber++;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setContentsMargins(10, 0, 10, 0);
    m_verticalLayout->addWidget(line);
    m_ScriptWidgets.append(line);
    return line;
}

QDoubleSpinBox* ScriptDialog::addDoubleInput(const QString &labelText){
    QDoubleSpinBox *doubleSpinBox;
    if (!labelText.isEmpty()){
        QHBoxLayout* horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout").arg(m_widgetNumber));
        m_widgetNumber++;
        QLabel * doubleLabel= new QLabel(m_verticalLayoutWidget);
        doubleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        doubleLabel->setObjectName(QString::fromUtf8("doubleLabel%1").arg(m_widgetNumber));
        doubleLabel->setText(labelText);
        m_widgetNumber++;
        m_ScriptWidgets.append(doubleLabel);
        horizontalLayout->addWidget(doubleLabel);
        doubleSpinBox = new QDoubleSpinBox(m_verticalLayoutWidget);
        horizontalLayout->addWidget(doubleSpinBox);
        m_verticalLayout->addLayout(horizontalLayout);
    } else {
        doubleSpinBox = new QDoubleSpinBox(m_verticalLayoutWidget);
        m_verticalLayout->addWidget(doubleSpinBox);
    }
    doubleSpinBox->setObjectName(QString::fromUtf8("doubleSpinBox%1").arg(m_widgetNumber));
    m_widgetNumber++;

    m_ScriptWidgets.append(doubleSpinBox);
    return doubleSpinBox;
}

QSlider *ScriptDialog::addSlider(const QString &labelText){
    QSlider *horizontalSlider;
    if (!labelText.isEmpty()){
        QHBoxLayout* horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout").arg(m_widgetNumber));
        m_widgetNumber++;
        QLabel * sliderLabel= new QLabel(m_verticalLayoutWidget);
        sliderLabel->setObjectName(QString::fromUtf8("sliderLabel%1").arg(m_widgetNumber));
        sliderLabel->setText(labelText);
        sliderLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_widgetNumber++;
        m_ScriptWidgets.append(sliderLabel);
        horizontalLayout->addWidget(sliderLabel);
        horizontalSlider = new QSlider(m_verticalLayoutWidget);
        horizontalLayout->addWidget(horizontalSlider);
        m_verticalLayout->addLayout(horizontalLayout);
    } else {
        horizontalSlider = new QSlider(m_verticalLayoutWidget);
        m_verticalLayout->addWidget(horizontalSlider);
    }
    horizontalSlider->setOrientation(Qt::Horizontal);
    horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider%1").arg(m_widgetNumber));
    m_widgetNumber++;

    m_ScriptWidgets.append(horizontalSlider);
    return horizontalSlider;
}

QCheckBox * ScriptDialog::addCheckbox(const QString &labelText, bool defaultValue){
    QCheckBox * checkBox;
    checkBox = new QCheckBox(labelText, m_verticalLayoutWidget);
    checkBox->setObjectName(QString::fromUtf8("checkbox").arg(m_widgetNumber));
    m_widgetNumber++;
    checkBox->setCheckState(defaultValue ? Qt::Checked: Qt::Unchecked);
    m_verticalLayout->addWidget(checkBox);
    m_ScriptWidgets.append(checkBox);
    return checkBox;
}

QPushButton * ScriptDialog::addButton(const QString &labelText){
    QPushButton *pushButton;
    pushButton = new QPushButton(m_verticalLayoutWidget);
    pushButton->setObjectName(QString::fromUtf8("pushButton").arg(m_widgetNumber));
    m_widgetNumber++;
    m_verticalLayout->addWidget(pushButton);
    pushButton->setText(labelText);
    m_ScriptWidgets.append(pushButton);
    return pushButton;
}

ColorButton* ScriptDialog::addColorButton (const QString &labelText){
    ColorButton *colorButton;
    if (!labelText.isEmpty()){
        QHBoxLayout* horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout").arg(m_widgetNumber));
        m_widgetNumber++;
        QLabel * colorLabel= new QLabel(m_verticalLayoutWidget);
        colorLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        colorLabel->setObjectName(QString::fromUtf8("colorLabel%1").arg(m_widgetNumber));
        colorLabel->setText(labelText);
        m_widgetNumber++;
        m_ScriptWidgets.append(colorLabel);
        horizontalLayout->addWidget(colorLabel);
        colorButton = new ColorButton(m_verticalLayoutWidget);
        horizontalLayout->addWidget(colorButton);
        m_verticalLayout->addLayout(horizontalLayout);
    } else {
        colorButton = new ColorButton(m_verticalLayoutWidget);
        m_verticalLayout->addWidget(colorButton);
    }
    colorButton->setObjectName(QString::fromUtf8("colorButton%1").arg(m_widgetNumber));
    m_widgetNumber++;

    m_ScriptWidgets.append(colorButton);
    return colorButton;
}


ScriptDialog::~ScriptDialog()
{
    close();
}


void ScriptDialog::setTitle(const QString &title)
{
    setWindowTitle(title);
}

void ScriptDialog::close()
{
    QDialog::close();
}


void registerDialog(QJSEngine *jsEngine)
{
    jsEngine->globalObject().setProperty(QStringLiteral("Dialog"),
                                         jsEngine->newQMetaObject<ScriptDialog>());
}

} // namespace Tiled

#include "scriptdialog.moc"
