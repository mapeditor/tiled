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
static QString labelType = QString::fromUtf8("QLabel");

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
ScriptHeadingWidget::ScriptHeadingWidget(const QString &text, QWidget *parent):
    QLabel(text, parent)
{
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
    QLabel *label = newLabel(text, true);
    // if fillRow, specify column span = -1, meaning fill all remaining columns
    m_gridLayout->addWidget(label, m_rowIndex, 0, leftColumnStretch, fillRow? -1 : 1);
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
    addNewRow();
    m_rowLayout = new QHBoxLayout();
    // we don't use addDialogWidget() here so that we can make the size of the separator
    // level independent of the size of the left column.
    m_gridLayout->addLayout(m_rowLayout, m_rowIndex, 0, 1, -1); // span entire row
    if (!labelText.isEmpty()) {
        QLabel *separatorLabel = newLabel(labelText, true);
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
    if (!labelText.isEmpty()) {
        QLabel *lineEditLabel = newLabel(labelText);
        addDialogWidget(lineEditLabel);
    }
    return addDialogWidget(new QLineEdit(defaultValue, this));
}
QWidget *ScriptDialog::addTextEdit(const QString &defaultValue)
{
    QTextEdit *textEdit = new QTextEdit(defaultValue, this);
    addDialogWidget(textEdit);
    textEdit->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextEditorInteraction);
    textEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    return textEdit;
}
QWidget *ScriptDialog::addImage(Tiled::ScriptImage *image){

    ScriptImageWidget *imageWidget = new ScriptImageWidget(image, this);
    return addDialogWidget(imageWidget);
}
QWidget *ScriptDialog::addNumberInput(const QString &labelText)
{
    if (!labelText.isEmpty()) {
        QLabel *numberLabel = newLabel(labelText);
        addDialogWidget(numberLabel);
    }
    return addDialogWidget(new QDoubleSpinBox(this));
}
QWidget *ScriptDialog::addSlider(const QString &labelText)
{
    if (!labelText.isEmpty()) {
        QLabel *sliderLabel= newLabel(labelText);
        addDialogWidget(sliderLabel);
    }
    QSlider *horizontalSlider = new QSlider(this);
    horizontalSlider->setOrientation(Qt::Horizontal);
    return addDialogWidget(horizontalSlider);
}

QWidget *ScriptDialog::addCheckBox(const QString &labelText, bool defaultValue)
{
    QCheckBox *checkBox = new QCheckBox(labelText, this);
    checkBox->setCheckState(defaultValue ? Qt::Checked: Qt::Unchecked);
    return addDialogWidget(checkBox);
}
QWidget *ScriptDialog::addComboBox(const QString &labelText, const QStringList &values)
{
    if (!labelText.isEmpty()) {
        QLabel *comboBoxLabel = newLabel(labelText);
        addDialogWidget(comboBoxLabel);
    }
    QComboBox *comboBox = new QComboBox(this);
    comboBox->addItems(values);
    return addDialogWidget(comboBox);
}

QWidget *ScriptDialog::addButton(const QString &labelText)
{
    return addDialogWidget(new QPushButton(labelText, this));
}
QWidget *ScriptDialog::addFilePicker(const QString &labelText)
{
    if (!labelText.isEmpty()) {
        QLabel *filePickerLabel = newLabel(labelText);
        addDialogWidget(filePickerLabel);
    }
    return addDialogWidget(new FileEdit(this));
}
QWidget *ScriptDialog::addColorButton(const QString &labelText)
{
    if (!labelText.isEmpty()) {
        QLabel *colorLabel = newLabel(labelText);
        addDialogWidget(colorLabel);
    }
    ColorButton *colorButton = (ColorButton *)addDialogWidget(new ColorButton(this));
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
QWidget *ScriptDialog::addDialogWidget(QWidget *widget)
{
    determineWidgetGrouping(widget);
    if (m_widgetsInRow == 0)
        m_widgetsInRow = 1;
    // right-hand side elements, add to the layout inside the second column
    if (m_widgetsInRow == 1) {
        m_rowLayout = new QHBoxLayout();
        m_gridLayout->addLayout(m_rowLayout, m_rowIndex, 1, 1, 1);
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

QLabel *ScriptDialog::newLabel(const QString& labelText, bool isHeading)
{
    QLabel *label;
    if (isHeading)
        label = new ScriptHeadingWidget(labelText, this);
    else
        label = new QLabel(labelText, this);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
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
