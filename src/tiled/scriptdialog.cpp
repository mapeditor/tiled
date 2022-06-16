/*
 * scriptdialog.cpp
 * Copyright 2022, Chris Boehm AKA dogboydog
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
#include <QJSEngine>
#include <string.h>
#include <memory>

static int leftColumnStretch = 0;
// stretch as much as we can so that the left column looks as close to zero width as possible when there is no content
static int rightColumnStretch = 1;
namespace Tiled {

static void deleteAllFromLayout(QLayout *layout)
{
    while (QLayoutItem *item = layout->takeAt(0)) {
        delete item->widget();

        if (QLayout *layout = item->layout())
            deleteAllFromLayout(layout);

        delete item;
    }
}

ScriptDialog::ScriptDialog(const QString &title)
 : QDialog(MainWindow::maybeInstance())
{
    if (title.isEmpty())
        QDialog::setWindowTitle(tr("Script"));
    else
        QDialog::setWindowTitle(title);

    setAttribute(Qt::WA_DeleteOnClose);
    m_lastWidgetTypeName.clear();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_gridLayout = new QGridLayout(this);
    initializeLayout();
}
void ScriptDialog::initializeLayout()
{
    // make the right-hand column more likely to stretch
    m_gridLayout->setColumnStretch(0, leftColumnStretch);
    m_gridLayout->setColumnStretch(1, rightColumnStretch);
    m_rowIndex = 0;

    addNewRow();
}
void ScriptDialog::clear()
{
    deleteAllFromLayout(layout());
    initializeLayout();
}

QLabel *ScriptDialog::addHeading(const QString &text, bool fillRow)
{
    addNewRow();
    QLabel *label = newLabel(text);
    m_gridLayout->addWidget(label, m_rowIndex, m_widgetsInRow, 1, fillRow? -1 : 1); // max width
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_widgetsInRow++;
    if (fillRow) {
        label->setWordWrap(true);
        addNewRow();
    }
    return label;
}
QLabel *ScriptDialog::addLabel(const QString &text)
{
    moveToRightColumn();
    return (QLabel *)addDialogWidget(newLabel(text));
}

QFrame *ScriptDialog::addSeparator(const QString &labelText)
{
    addNewRow();
    m_rowLayout = new QHBoxLayout();
    // we don't use addDialogWidget() here so that we can make the size of the separator
    // level independent of the size of the left column.
    m_gridLayout->addLayout(m_rowLayout, m_rowIndex, 0, 1, -1); // span entire row
    if (!labelText.isEmpty()) {
        QLabel *separatorLabel;
        separatorLabel= newLabel(labelText);
        separatorLabel->setWordWrap(false);
        m_rowLayout->addWidget(separatorLabel, 1);
    }
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    m_rowLayout->addWidget(line, rightColumnStretch); // higher stretch
    addNewRow();
    return line;
}
QLineEdit *ScriptDialog::addTextInput(const QString &labelText, const QString &defaultValue)
{
    checkIfSameType("QLineEdit");
    moveToRightColumn();
    if (!labelText.isEmpty()) {
        QLabel *lineEditLabel = newLabel(labelText);
        addDialogWidget(lineEditLabel);
    }
    return (QLineEdit *)addDialogWidget(new QLineEdit(defaultValue, this));
}
QDoubleSpinBox *ScriptDialog::addNumberInput(const QString &labelText)
{
    checkIfSameType("QDoubleSpinBox");
    moveToRightColumn();
    if (!labelText.isEmpty()) {
        QLabel *numberLabel = newLabel(labelText);
        addDialogWidget(numberLabel);
    }
    return (QDoubleSpinBox *)addDialogWidget(new QDoubleSpinBox(this));
}
QSlider *ScriptDialog::addSlider(const QString &labelText)
{
    checkIfSameType("QSlider");
    moveToRightColumn();
    if (!labelText.isEmpty()) {
        QLabel *sliderLabel= newLabel(labelText);
        addDialogWidget(sliderLabel);
    }
    QSlider *horizontalSlider = new QSlider(this);
    horizontalSlider->setOrientation(Qt::Horizontal);
    return (QSlider *)addDialogWidget(horizontalSlider);
}

QCheckBox *ScriptDialog::addCheckBox(const QString &labelText, bool defaultValue)
{
    checkIfSameType("QCheckBox");
    moveToRightColumn();
    QCheckBox *checkBox = new QCheckBox(labelText, this);
    checkBox->setCheckState(defaultValue ? Qt::Checked: Qt::Unchecked);
    return (QCheckBox *)addDialogWidget(checkBox);
}
QComboBox *ScriptDialog::addComboBox(const QString &labelText, const QStringList &values)
{
    checkIfSameType("QComboBox");
    moveToRightColumn();
    if (!labelText.isEmpty()) {
        QLabel *comboBoxLabel = newLabel(labelText);
        addDialogWidget(comboBoxLabel);
    }
    QComboBox *comboBox = new QComboBox(this);
    comboBox->addItems(values);
    return (QComboBox *)addDialogWidget(comboBox);
}

QPushButton *ScriptDialog::addButton(const QString &labelText)
{
    checkIfSameType("QPushButton");
    moveToRightColumn();
    return (QPushButton *)addDialogWidget(new QPushButton(labelText, this));
}
Tiled::FileEdit *ScriptDialog::addFilePicker(const QString &labelText)
{
    checkIfSameType("FileEdit");
    moveToRightColumn();
    if (!labelText.isEmpty()) {
        QLabel *filePickerLabel = newLabel(labelText);
        addDialogWidget(filePickerLabel);
    }
    return (FileEdit *)addDialogWidget(new FileEdit(this));
}
Tiled::ColorButton *ScriptDialog::addColorButton(const QString &labelText)
{    
    checkIfSameType("ColorButton");
    moveToRightColumn();
    if (!labelText.isEmpty()) {
        QLabel *colorLabel = newLabel(labelText);
        addDialogWidget(colorLabel);
    }
    ColorButton *colorButton = (ColorButton *)addDialogWidget(new ColorButton(this));
    colorButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return colorButton;
}

QWidget *ScriptDialog::addDialogWidget(QWidget *widget)
{
    // left-hand side element that occupies the first grid column
    if (m_widgetsInRow == 0)
        m_gridLayout->addWidget(widget, m_rowIndex, 0);
    else {
        // right-hand side elements, add to the layout inside the second column
        if (m_widgetsInRow == 1) {
            m_rowLayout = new QHBoxLayout();
            m_gridLayout->addLayout(m_rowLayout, m_rowIndex, 1, 1, 1);
        }
        widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        m_rowLayout->addWidget(widget);
    }
    m_widgetsInRow++;
    return widget;
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
    QString widgetTypeNameQt = QString::fromUtf8(widgetTypeName);
    if (m_lastWidgetTypeName.compare(QString::fromUtf8("QLabel")) == 0)
        isSameType = true;
    else if (m_widgetsInRow != 1 &&
             !m_lastWidgetTypeName.isEmpty() &&
             m_lastWidgetTypeName.compare(widgetTypeNameQt) != 0) {
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
}

void ScriptDialog::moveToRightColumn()
{
    if (m_widgetsInRow == 0)
        m_widgetsInRow = 1;
}

QLabel *ScriptDialog::newLabel(const QString& labelText)
{
    QLabel *label = new QLabel(this);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setText(labelText);
    label->setWordWrap(false);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->adjustSize();
    return label;
}

void registerDialog(QJSEngine *jsEngine)
{
    jsEngine->globalObject().setProperty(QStringLiteral("Dialog"),
                                         jsEngine->newQMetaObject<ScriptDialog>());
}

} // namespace Tiled

#include "moc_scriptdialog.cpp"
