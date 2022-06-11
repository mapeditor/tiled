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

#include "scriptdialog.h"
#include "scriptmanager.h"


#include <QProcessEnvironment>
#include <QJSEngine>

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif
#include <string.h>
#include <typeinfo>
#include <memory>

namespace Tiled {

    NumberInputArgs::NumberInputArgs(): QObject() {

    }
    NumberInputArgs::NumberInputArgs(const NumberInputArgs& args): QObject() {
        maximum = args.maximum;
        minimum = args.minimum;
        setPrefix(args.prefix());
        setSuffix(args.suffix());
        setLabel(args.label());
    }
    inline QString NumberInputArgs::prefix() const{
        return m_prefix;
    }
    inline QString NumberInputArgs::suffix() const{
        return m_suffix;
    }
    inline QString NumberInputArgs::label() const{
        return m_label;
    }
    void NumberInputArgs::setPrefix(const QString &prefix){
        m_prefix = prefix;
    }
    void NumberInputArgs::setSuffix(const QString &suffix){
        m_suffix = suffix;
    }
    void NumberInputArgs::setLabel(const QString &label){
        m_label = label;
    }


ScriptDialog::ScriptDialog(const QString &title, const int width=400, const int height=400): QDialog(MainWindow::maybeInstance())
{
    if (title.isEmpty()){
        setTitle(QString::fromUtf8("Script"));
    } else {
      setTitle(title);
    }
    setAttribute(Qt::WA_DeleteOnClose);
    m_widgetNumber = 0;
    m_lastWidgetTypeName.clear();

    setMinimumSize(width, height);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    initializeLayout();
    QMetaObject::connectSlotsByName(this);
}
void ScriptDialog::initializeLayout(){
    m_gridLayout = new QGridLayout(this);
    m_gridLayout->setMargin(0);
    m_gridLayout->setObjectName(QString::fromUtf8("m_gridLayout"));
    m_verticalLayoutWidget = new QWidget(this);
    m_verticalLayoutWidget->setGeometry(QRect(15,15, minimumWidth()-30, minimumHeight()-30));
    m_verticalLayoutWidget->setObjectName(QString::fromUtf8("m_verticalLayoutWidget"));
    m_verticalLayoutWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_verticalLayout = new QVBoxLayout(m_verticalLayoutWidget);
    m_verticalLayout->setMargin(0);
    m_verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    m_gridLayout->addLayout(m_verticalLayout, 0, 0, 1, 1);
}
void ScriptDialog::clear(){
    m_widgetNumber = 0;
    QLayoutItem * item;
    QLayout * sublayout;
    QWidget * widget;
    while ((item = m_gridLayout->takeAt(0))) {
        if ((sublayout = item->layout()) != 0) {/* do the same for sublayout*/}
        else if ((widget = item->widget()) != 0) {widget->hide(); delete widget;}
        else {delete item;}
    }
    while ((item = m_verticalLayout->takeAt(0))) {
        if ((sublayout = item->layout()) != 0) {/* do the same for sublayout*/}
        else if ((widget = item->widget()) != 0) {widget->hide(); delete widget;}
        else {delete item;}
    }
    delete m_gridLayout;
    delete m_verticalLayout;
    delete m_verticalLayoutWidget;
    initializeLayout();
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
    checkIfSameType(typeid(label).name());
    label = new QLabel(m_verticalLayoutWidget);
    label->setObjectName(QString::fromUtf8("label%1").arg(m_widgetNumber));
    m_widgetNumber++;
    label->setContentsMargins(0,0,0,0);
    label->setText(text);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setWordWrap(true);
    m_rowLayout->addWidget(label);
    return label;
}

QFrame* ScriptDialog::addSeparator(){
    QFrame *line;
    newRow();
    line = new QFrame(m_verticalLayoutWidget);
    line->setObjectName(QString::fromUtf8("line%1").arg(m_widgetNumber));
    m_widgetNumber++;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setContentsMargins(10, 0, 10, 0);
    m_rowLayout->addWidget(line);
    return line;
}

QDoubleSpinBox* ScriptDialog::addNumberInput(const NumberInputArgs inputArgs){
    QDoubleSpinBox *doubleSpinBox;
    checkIfSameType(typeid(doubleSpinBox).name());
    if (!inputArgs.label().isEmpty()){
        QLabel * numberLabel= new QLabel(m_verticalLayoutWidget);
        numberLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        numberLabel->setObjectName(QString::fromUtf8("numberLabel%1").arg(m_widgetNumber));
        numberLabel->setText(inputArgs.label());
        m_widgetNumber++;
        m_rowLayout->addWidget(numberLabel);
    }
    doubleSpinBox = new QDoubleSpinBox(m_verticalLayoutWidget);
    m_rowLayout->addWidget(doubleSpinBox);
    doubleSpinBox->setObjectName(QString::fromUtf8("doubleSpinBox%1").arg(m_widgetNumber));
    m_widgetNumber++;

    return doubleSpinBox;
}

QDoubleSpinBox* ScriptDialog::addNumberInput(const QString &labelText){
    QDoubleSpinBox *doubleSpinBox;
    checkIfSameType(typeid(doubleSpinBox).name());
    if (!labelText.isEmpty()){
        m_widgetNumber++;
        QLabel * numberLabel= new QLabel(m_verticalLayoutWidget);
        numberLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        numberLabel->setObjectName(QString::fromUtf8("numberLabel%1").arg(m_widgetNumber));
        numberLabel->setText(labelText);
        m_widgetNumber++;
        m_rowLayout->addWidget(numberLabel);
    }

    doubleSpinBox = new QDoubleSpinBox(m_verticalLayoutWidget);
    m_verticalLayout->addWidget(doubleSpinBox);
    m_rowLayout->addWidget(doubleSpinBox);
    doubleSpinBox->setObjectName(QString::fromUtf8("doubleSpinBox%1").arg(m_widgetNumber));
    m_widgetNumber++;

    return doubleSpinBox;
}
QSlider *ScriptDialog::addSlider(const QString &labelText){
    QSlider *horizontalSlider;
    checkIfSameType(typeid(horizontalSlider).name());
    if (!labelText.isEmpty()){
        QLabel * sliderLabel= new QLabel(m_verticalLayoutWidget);
        sliderLabel->setObjectName(QString::fromUtf8("sliderLabel%1").arg(m_widgetNumber));
        sliderLabel->setText(labelText);
        sliderLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_widgetNumber++;
        m_rowLayout->addWidget(sliderLabel);
    }
    horizontalSlider = new QSlider(m_verticalLayoutWidget);
    m_rowLayout->addWidget(horizontalSlider);
    horizontalSlider->setOrientation(Qt::Horizontal);
    horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider%1").arg(m_widgetNumber));
    m_widgetNumber++;

    return horizontalSlider;
}

QCheckBox * ScriptDialog::addCheckbox(const QString &labelText, bool defaultValue){
    QCheckBox * checkBox;
    checkIfSameType(typeid(checkBox).name());
    checkBox = new QCheckBox(labelText, m_verticalLayoutWidget);
    checkBox->setObjectName(QString::fromUtf8("checkbox").arg(m_widgetNumber));
    m_widgetNumber++;
    checkBox->setCheckState(defaultValue ? Qt::Checked: Qt::Unchecked);
    m_verticalLayout->addWidget(checkBox);
    return checkBox;
}

QPushButton * ScriptDialog::addButton(const QString &labelText){
    QPushButton *pushButton;
    checkIfSameType(typeid(pushButton).name());
    pushButton = new QPushButton(m_verticalLayoutWidget);
    pushButton->setObjectName(QString::fromUtf8("pushButton").arg(m_widgetNumber));
    m_widgetNumber++;
    m_rowLayout->addWidget(pushButton);
    pushButton->setText(labelText);
    return pushButton;
}

Tiled::ColorButton* ScriptDialog::addColorButton (const QString &labelText){
    ColorButton *colorButton;
    checkIfSameType(typeid(colorButton).name());
    if (!labelText.isEmpty()){

        QLabel * colorLabel= new QLabel(m_verticalLayoutWidget);
        colorLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        colorLabel->setObjectName(QString::fromUtf8("colorLabel%1").arg(m_widgetNumber));
        colorLabel->setText(labelText);
        m_widgetNumber++;
        m_rowLayout->addWidget(colorLabel);
    }
    colorButton = new ColorButton(m_verticalLayoutWidget);
    m_rowLayout->addWidget(colorButton);
    colorButton->setObjectName(QString::fromUtf8("colorButton%1").arg(m_widgetNumber));
    m_widgetNumber++;
    return colorButton;
}


ScriptDialog::~ScriptDialog()
{
    close();
}

void ScriptDialog::newRow(){
    m_rowLayout= new QHBoxLayout();
    m_rowLayout->setObjectName(QString::fromUtf8("rowLayout%1").arg(m_widgetNumber));
    m_rowLayout->setContentsMargins(0,0,0,0);
    m_widgetNumber++;
    m_verticalLayout->addLayout(m_rowLayout);
    m_lastWidgetTypeName.clear();
}

void ScriptDialog::checkIfSameType(const char * widgetTypeName){
    if (m_lastWidgetTypeName.compare(widgetTypeName) != 0){
       // if the widget type is not the same as the last
       newRow();
       m_lastWidgetTypeName = widgetTypeName;
    }
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
    jsEngine->globalObject().setProperty(QStringLiteral("NumberInputArgs"),
                                         jsEngine->newQMetaObject<NumberInputArgs>());
    jsEngine->globalObject().setProperty(QStringLiteral("Dialog"),
                                         jsEngine->newQMetaObject<ScriptDialog>());
}

} // namespace Tiled
Q_DECLARE_METATYPE(QSlider*)
#include "moc_scriptdialog.cpp"
