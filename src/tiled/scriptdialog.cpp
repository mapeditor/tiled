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
#include <QSet>
#include <string.h>
#include <memory>
static int leftColumnStretch = 0;
// stretch as much as we can so that the left column looks as close to zero width as possible when there is no content
static int rightColumnStretch = 1;
static QString labelType = QString::fromUtf8("QLabel");

namespace Tiled {

static QSet<ScriptDialog*> sDialogInstances;

static void deleteAllFromLayout(QLayout *layout)
{
    while (QLayoutItem *item = layout->takeAt(0)) {
        delete item->widget();

        if (QLayout *layout = item->layout())
            deleteAllFromLayout(layout);

        delete item;
    }
}

ScriptImageWidget::ScriptImageWidget(Tiled::ScriptImage *image, QWidget *parent):
    QLabel(parent)
{
    setImage(image);
}

Tiled::ScriptImage *ScriptImageWidget::image() const{
    return m_image;
}
void ScriptImageWidget::setImage(Tiled::ScriptImage *image){
    m_image = image;
    setMinimumSize(m_image->width(), m_image->height());
    QPixmap newPixmap = QPixmap::fromImage(m_image->image());
    setPixmap(newPixmap);
    setMask(newPixmap.mask());
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
    sDialogInstances.insert(this);
}
ScriptDialog::~ScriptDialog()
{
    sDialogInstances.remove(this);
}

void ScriptDialog::deleteAllDialogs()
{
    for (ScriptDialog *dialog : sDialogInstances)
        dialog->deleteLater();
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

QWidget *ScriptDialog::addHeading(const QString &text, bool fillRow)
{
    // if anything has been placed in this row, go to the next row
    if (m_widgetsInRow != 0)
        addNewRow();
    QLabel *label = newLabel(text);
    // if fillRow, specify column span = -1, meaning fill all remaining columns
    m_gridLayout->addWidget(label, m_rowIndex, 0, 1, fillRow? -1 : 1);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_widgetsInRow++;
    if (fillRow) {
        label->setWordWrap(true);
        addNewRow();
    }
    return label;
}
QWidget *ScriptDialog::addLabel(const QString &text)
{
    return addDialogWidget(newLabel(text));
}

QWidget *ScriptDialog::addSeparator(const QString &labelText)
{
    if (m_widgetsInRow != 0)
        addNewRow();
    m_rowLayout = new QHBoxLayout();
    // we don't use addDialogWidget() here so that we can make the size of the separator
    // level independent of the size of the left column.
    m_gridLayout->addLayout(m_rowLayout, m_rowIndex, 0, 1, -1); // span entire row
    if (!labelText.isEmpty()) {
        QLabel *separatorLabel = newLabel(labelText);
        separatorLabel->setWordWrap(false);
        m_rowLayout->addWidget(separatorLabel, leftColumnStretch);
    }
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    m_rowLayout->addWidget(line, rightColumnStretch); // higher stretch
    addNewRow();
    return line;
}
QWidget *ScriptDialog::addTextInput(const QString &labelText, const QString &defaultValue)
{
    QLabel *lineEditLabel = nullptr;
    if (!labelText.isEmpty())
         lineEditLabel = newLabel(labelText);
    return addDialogWidget(new QLineEdit(defaultValue, this), lineEditLabel);
}
QWidget *ScriptDialog::addTextEdit(const QString &labelText, const QString &defaultValue)
{
    QLabel *textEditLabel = nullptr;
    if (!labelText.isEmpty())
        textEditLabel = newLabel(labelText);
    QTextEdit *textEdit = new QTextEdit(defaultValue, this);
    addDialogWidget(textEdit, textEditLabel);
    textEdit->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextEditorInteraction);
    textEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    return textEdit;
}
QWidget *ScriptDialog::addImage(const QString &labelText, Tiled::ScriptImage *image){
    QLabel *imageLabel = nullptr;
    if (!labelText.isEmpty())
        imageLabel = newLabel(labelText);
    ScriptImageWidget *imageWidget = new ScriptImageWidget(image, this);
    return addDialogWidget(imageWidget, imageLabel);
}
QWidget *ScriptDialog::addNumberInput(const QString &labelText)
{
    QLabel *numberLabel = nullptr;
    if (!labelText.isEmpty())
        numberLabel = newLabel(labelText);
    return addDialogWidget(new QDoubleSpinBox(this), numberLabel);
}
QWidget *ScriptDialog::addSlider(const QString &labelText)
{
    QLabel *sliderLabel = nullptr;
    if (!labelText.isEmpty())
        sliderLabel = newLabel(labelText);
    QSlider *horizontalSlider = new QSlider(this);
    horizontalSlider->setOrientation(Qt::Horizontal);
    return addDialogWidget(horizontalSlider, sliderLabel);
}

QWidget *ScriptDialog::addCheckBox(const QString &labelText, bool defaultValue)
{
    QCheckBox *checkBox = new QCheckBox(labelText, this);
    checkBox->setCheckState(defaultValue ? Qt::Checked: Qt::Unchecked);
    return addDialogWidget(checkBox);
}
QWidget *ScriptDialog::addComboBox(const QString &labelText, const QStringList &values)
{
    QLabel *comboBoxLabel = nullptr;
    if (!labelText.isEmpty())
        comboBoxLabel = newLabel(labelText);
    QComboBox *comboBox = new QComboBox(this);
    comboBox->addItems(values);
    return addDialogWidget(comboBox, comboBoxLabel);
}

QWidget *ScriptDialog::addButton(const QString &labelText)
{
    return addDialogWidget(new QPushButton(labelText, this));
}
QWidget *ScriptDialog::addFilePicker(const QString &labelText)
{
    QLabel *filePickerLabel = nullptr;
    if (!labelText.isEmpty())
        filePickerLabel = newLabel(labelText);
    return addDialogWidget(new FileEdit(this), filePickerLabel);
}
QWidget *ScriptDialog::addColorButton(const QString &labelText)
{
    QLabel *colorLabel = nullptr;
    if (!labelText.isEmpty())
         colorLabel = newLabel(labelText);
    QWidget *colorButton = addDialogWidget(new ColorButton(this), colorLabel);
    colorButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return colorButton;
}
ScriptDialog::NewRowMode ScriptDialog::newRowMode() const
{
    return m_newRowMode;
}
void ScriptDialog::setNewRowMode(NewRowMode mode)
{
    m_newRowMode = mode;
}
QWidget *ScriptDialog::addDialogWidget(QWidget *widget, QLabel *widgetLabel)
{
    determineWidgetGrouping(widget);
    if (m_widgetsInRow == 0)
        m_widgetsInRow = 1;
    // right-hand side elements, add to the layout inside the second column
    if (m_widgetsInRow == 1) {
        m_rowLayout = new QHBoxLayout();
        m_gridLayout->addLayout(m_rowLayout, m_rowIndex, 1, 1, 1);
    }
    if (widgetLabel != nullptr) {
        widgetLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        m_rowLayout->addWidget(widgetLabel);
    }
    widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_rowLayout->addWidget(widget);
    m_widgetsInRow++;
    return widget;
}

/**
 * Based on the current NewRoWmode, determine whether we should add a newline before
 * placing a widget
 *
 * @param widget - the widget that is about to be added to the layout
 */
void ScriptDialog::determineWidgetGrouping(QWidget *widget)
{
    const char *widgetTypeNameCStr = widget->metaObject()->className();
    QString widgetTypeName = QString::fromUtf8(widgetTypeNameCStr);

    // any widgets that get checked via this method
    // are right column widgets.
    if (newRowMode() == NewRowMode::ManualRows)
        return;
    if (newRowMode() == NewRowMode::SingleWidgetRows)
    {
        // if there are any other widgets on this row so far in single widget mode,
        // wrap to the next row
        if (m_widgetsInRow > 1)
            addNewRow();
        return;
    }
    // otherwise,  same-type widget grouping

    // labels can be mixed with any type of widget
    if ((m_lastWidgetTypeName.compare(labelType) == 0  ||
        m_lastWidgetTypeName.isEmpty()) &&
        widgetTypeName.compare(labelType) != 0)
            // store the widget type to help determine if the next widget will start a new row
            m_lastWidgetTypeName = widgetTypeName;
    // if the new widget type is not the same as the last
    else
        if (widgetTypeName.compare(labelType) != 0 &&
            widgetTypeName.compare(m_lastWidgetTypeName) != 0)
            addNewRow();
}
void ScriptDialog::addNewRow()
{
    m_rowIndex++;
    m_widgetsInRow = 0;
    m_lastWidgetTypeName.clear();
}

QLabel *ScriptDialog::newLabel(const QString& labelText)
{
    QLabel *label = new QLabel(this);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setWordWrap(false);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    label->setText(labelText);
    return label;
}

void registerDialog(QJSEngine *jsEngine)
{
    jsEngine->globalObject().setProperty(QStringLiteral("Dialog"),
                                         jsEngine->newQMetaObject<ScriptDialog>());
}

} // namespace Tiled

#include "moc_scriptdialog.cpp"
