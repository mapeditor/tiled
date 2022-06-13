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
#include <string.h>
#include <typeinfo>
#include <memory>

namespace Tiled {

ScriptDialog::ScriptDialog(): ScriptDialog(QString(), 450, 450)
{
    
}
ScriptDialog::ScriptDialog(const QString &title, const int width=450, const int height=450)
 : QDialog(MainWindow::maybeInstance())
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
void ScriptDialog::initializeLayout()
{
    m_gridLayoutWidget = new QWidget(this);
    m_gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
    m_gridLayoutWidget->setGeometry(QRect(15, 15, minimumWidth()-30, minimumHeight()-30));
    m_gridLayout = new QGridLayout(m_gridLayoutWidget);
    m_gridLayout->setContentsMargins(15, 15, 15, 15);
    m_gridLayout->setObjectName(QString::fromUtf8("m_gridLayout"));
    // make the right-hand column more likely to stretch
    m_gridLayout->setColumnStretch(0, m_leftColumnStretch);
    m_gridLayout->setColumnStretch(1, m_rightColumnStretch);
    m_rowIndex = 0;
    addNewRow();
}
void ScriptDialog::clear()
{
    m_widgetNumber = 0;
    QLayoutItem * item;
    QLayout * sublayout;
    QWidget * widget;
    while ((item = m_gridLayout->takeAt(0))) {
        if ((sublayout = item->layout()) != 0) {/* do the same for sublayout*/}
        else if ((widget = item->widget()) != 0) {widget->hide(); delete widget;}
        else {delete item;}
    }
    delete m_gridLayoutWidget;
    initializeLayout();
}
void ScriptDialog::resize(const int width, const int height)
{
    QSize oldSize = size();
    QDialog::resize(width, height);
    QSize newSize = QSize(width, height);
    m_gridLayout->setGeometry(QRect(15,15, width-30, height-30));
    m_gridLayoutWidget->setGeometry(QRect(15,15, width-30, height-30));
    adjustSize();
    QResizeEvent event = QResizeEvent(newSize, oldSize);
    resizeEvent(&event);
}
void ScriptDialog::resizeEvent(QResizeEvent* event)
{
    m_gridLayout->invalidate();
    m_gridLayoutWidget->setGeometry(QRect(15,15, event->size().width()-30, event->size().height()-30));
}
QLabel * ScriptDialog::addLabel(const QString &text)
{
    addLabel(text, false);
}
QLabel * ScriptDialog::addLabel(const QString &text, bool maxWidth)
{
    QLabel * label;
    addNewRow();
    checkIfSameType(typeid(label).name());
    label = newLabel(text);
    if (m_widgetsInRow == 0) {
        m_gridLayout->addWidget(label, m_rowIndex, m_widgetsInRow, 1, maxWidth ? -1 : 1); // max width
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        label->setSizePolicy(sizePolicy);
        m_widgetsInRow++;
        if (maxWidth){
            label->setWordWrap(true);
            addNewRow();
        }
    }
    else
        addDialogWidget(label);
    return label;
}
QFrame* ScriptDialog::addSeparator()
{
    addSeparator(QString());
}
QFrame* ScriptDialog::addSeparator(const QString &labelText)
{
    QFrame *line;
    addNewRow();
    m_gridLayout->addLayout(m_rowLayout, m_rowIndex,0, 1, -1); // span entire
    if(!labelText.isEmpty()){
        QLabel * separatorLabel = newLabel(labelText);
        separatorLabel->setWordWrap(false);
        m_rowLayout->addWidget(separatorLabel, 1);
    }
    line = new QFrame(m_gridLayoutWidget);
    line->setObjectName(QString::fromUtf8("line%1").arg(m_widgetNumber));
    m_widgetNumber++;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setContentsMargins(10, 0, 10, 0);
    m_rowLayout->addWidget(line, m_rightColumnStretch); // higher stretch
    addNewRow();
    return line;
}

QDoubleSpinBox* ScriptDialog::addNumberInput(const QString &labelText)
{
    QDoubleSpinBox *doubleSpinBox;
    checkIfSameType(typeid(doubleSpinBox).name());
    moveToColumn2();
    QLabel * numberLabel = newLabel(labelText);
    addDialogWidget(numberLabel);
    doubleSpinBox = new QDoubleSpinBox(m_gridLayoutWidget);
    doubleSpinBox->setObjectName(QString::fromUtf8("doubleSpinBox%1").arg(m_widgetNumber));
    addDialogWidget(doubleSpinBox);
    return doubleSpinBox;
}
Tiled::ScriptDialogWidget *ScriptDialog::addSlider(const QString &labelText)
{
    QSlider *horizontalSlider;
    checkIfSameType(typeid(horizontalSlider).name());
    moveToColumn2();
    QLabel * sliderLabel= newLabel(labelText);
    addDialogWidget(sliderLabel);
    horizontalSlider = new QSlider(m_gridLayoutWidget);
    horizontalSlider->setOrientation(Qt::Horizontal);
    horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider%1").arg(m_widgetNumber));
    horizontalSlider->setContentsMargins(0,0,0,0);
    addDialogWidget(horizontalSlider);
    return new ScriptDialogWidget(sliderLabel, horizontalSlider);
}

QCheckBox * ScriptDialog::addCheckbox(const QString &labelText, bool defaultValue)
{
    QCheckBox * checkBox;
    checkIfSameType(typeid(checkBox).name());
    moveToColumn2();
    checkBox = new QCheckBox(labelText, m_gridLayoutWidget);
    checkBox->setObjectName(QString::fromUtf8("checkbox").arg(m_widgetNumber));
    checkBox->setCheckState(defaultValue ? Qt::Checked: Qt::Unchecked);
    m_gridLayout->addWidget(checkBox, m_rowIndex, m_widgetsInRow);
    addDialogWidget(checkBox);
    return checkBox;
}
QComboBox * ScriptDialog::addComboBox(const QString &labelText, const QStringList &values)
{
    QComboBox * comboBox;
    checkIfSameType(typeid(comboBox).name());
    moveToColumn2();
    QLabel *comboBoxLabel = newLabel(labelText);
    addDialogWidget(comboBoxLabel);
    comboBox = new QComboBox(m_gridLayoutWidget);
    comboBox->setObjectName(QString::fromUtf8("comboBox").arg(m_widgetNumber));
    comboBox->addItems(values);
    m_gridLayout->addWidget(comboBox, m_rowIndex, m_widgetsInRow);
    addDialogWidget(comboBox);
    return comboBox;
}

QPushButton * ScriptDialog::addButton(const QString &labelText)
{
    QPushButton *pushButton;
    checkIfSameType(typeid(pushButton).name());
    moveToColumn2();
    pushButton = new QPushButton(m_gridLayoutWidget);
    pushButton->setObjectName(QString::fromUtf8("pushButton").arg(m_widgetNumber));
    m_gridLayout->addWidget(pushButton, m_rowIndex, m_widgetsInRow);
    pushButton->setText(labelText);
    addDialogWidget(pushButton);
    return pushButton;
}

Tiled::ColorButton* ScriptDialog::addColorButton(const QString &labelText)
{
    ColorButton *colorButton;
    checkIfSameType(typeid(colorButton).name());
    moveToColumn2();
    QLabel *colorLabel = newLabel(labelText);
    addDialogWidget(colorLabel);
    colorButton = new ColorButton(m_gridLayoutWidget);
    colorButton->setObjectName(QString::fromUtf8("colorButton%1").arg(m_widgetNumber));
    addDialogWidget(colorButton);
    return colorButton;
}


ScriptDialog::~ScriptDialog()
{
    close();
}

void ScriptDialog::addDialogWidget(QWidget * widget)
{
    if (m_widgetsInRow == 0){
        // left-hand side element that occupies the first grid column
        m_gridLayout->addWidget(widget, m_rowIndex, 0);
    } else {
        // right-hand side elements, add to the layout inside the second column
        if (m_widgetsInRow == 1)
            m_gridLayout->addLayout(m_rowLayout, m_rowIndex, 1, 1, 1);
        m_rowLayout->addWidget(widget);
    }
    m_widgetsInRow++;
}

/**
 * Check if the current widget we are trying to add is of the same type
 * as the last we added. If so, we keep it on the same row
 *
 * @param widgetTypeName - the name of the new widget (typeinfo(widget).name())
 * @return true if the widget was the same as the last type, false if they were different and a new
 *         row was made
 */
bool ScriptDialog::checkIfSameType(const char *widgetTypeName)
{
    bool isSameType = true;
    QLabel exampleLabel;
    QString widgetTypeNameQt = QString::fromUtf8(widgetTypeName);
    if (m_widgetsInRow == 1 &&
        m_lastWidgetTypeName.compare(QString::fromUtf8(typeid(exampleLabel).name())) == 0){
        isSameType = true;
    }
    else if (m_widgetsInRow != 1 &&
             !m_lastWidgetTypeName.isEmpty() &&
             m_lastWidgetTypeName.compare(widgetTypeNameQt) != 0){
       // if the widget type is not the same as the last
       addNewRow();
       isSameType = false;
    }
    m_lastWidgetTypeName = QString::fromUtf8(widgetTypeName);
    return isSameType;
}
void ScriptDialog::addNewRow()
{
    m_rowIndex++;
    m_widgetsInRow = 0;
    m_lastWidgetTypeName.clear();
    m_rowLayout = new QHBoxLayout(m_gridLayoutWidget);
    m_rowLayout->setObjectName(QString::fromUtf8("horizontalLayout%1").arg(m_widgetNumber));
    m_widgetNumber++;
}

void ScriptDialog::moveToColumn2()
{
    if (m_widgetsInRow == 0){
        m_widgetsInRow = 1;
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

QLabel *ScriptDialog::newLabel(const QString& labelText)
{
    QLabel* label = new QLabel(m_gridLayoutWidget);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setObjectName(QString::fromUtf8("label%1").arg(m_widgetNumber));
    label->setText(labelText);
    label->setContentsMargins(0,0,0,0);
    label->setWordWrap(false);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->adjustSize();
    m_widgetNumber++;
    return label;
}

void registerDialog(QJSEngine *jsEngine)
{

    jsEngine->globalObject().setProperty(QStringLiteral("Dialog"),
                                         jsEngine->newQMetaObject<ScriptDialog>());
}

} // namespace Tiled
Q_DECLARE_METATYPE(QSlider*)
#include "moc_scriptdialog.cpp"
