/*
 * scriptdialog.cpp
 * Copyright 2022, Chris Boehm AKA dogboydog
 * Copyright 2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "colorbutton.h"
#include "fileedit.h"
#include "mainwindow.h"
#include "scriptimage.h"
#include "scriptmanager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QJSEngine>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QSet>
#include <QSize>
#include <QSlider>
#include <QTextEdit>

#include <memory>

static const int leftColumnStretch = 0;
// stretch as much as we can so that the left column looks as close to zero width as possible when there is no content
static const int rightColumnStretch = 1;

namespace Tiled {

QSet<ScriptDialog*> ScriptDialog::sDialogInstances;

static void deleteAllFromLayout(QLayout *layout)
{
    while (QLayoutItem *item = layout->takeAt(0)) {
        delete item->widget();

        if (QLayout *layout = item->layout())
            deleteAllFromLayout(layout);

        delete item;
    }
}

ScriptImageWidget::ScriptImageWidget(Tiled::ScriptImage *image, QWidget *parent)
    : QLabel(parent)
{
    setImage(image);
}

ScriptImage *ScriptImageWidget::image() const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    if (auto p = pixmap())
        return new ScriptImage(p->toImage());
    else
        return nullptr;
#else
    return new ScriptImage(pixmap().toImage());
#endif
}

void ScriptImageWidget::setImage(ScriptImage *image)
{
    if (!image) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid argument"));
        return;
    }

    setMinimumSize(image->width(), image->height());
    setPixmap(QPixmap::fromImage(image->image()));
}

class ScriptComboBox : public QComboBox
{
    Q_OBJECT

public:
    ScriptComboBox(QWidget *parent)
        : QComboBox(parent)
    {}

    Q_INVOKABLE void addItems(const QStringList &texts)
    { QComboBox::addItems(texts); }
};


ScriptDialog::ScriptDialog(const QString &title)
    : QDialog(MainWindow::maybeInstance())
    , m_gridLayout(new QGridLayout(this))
{
    setWindowTitle(title.isEmpty() ? tr("Script") : title);
    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // make the right-hand column more likely to stretch
    m_gridLayout->setColumnStretch(0, leftColumnStretch);
    m_gridLayout->setColumnStretch(1, rightColumnStretch);

    initializeLayout();
    sDialogInstances.insert(this);
}

ScriptDialog::~ScriptDialog()
{
    sDialogInstances.remove(this);
}

void ScriptDialog::deleteAllDialogs()
{
    QSet<ScriptDialog*> dialogToDelete;
    dialogToDelete.swap(sDialogInstances);

    for (ScriptDialog *dialog : std::as_const(dialogToDelete))
        dialog->deleteLater();
}

void ScriptDialog::initializeLayout()
{
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
    m_gridLayout->addWidget(label, m_rowIndex, 0, 1, fillRow ? -1 : 1);
    m_widgetsInRow++;

    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

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

    m_rowLayout = new QHBoxLayout;
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
    return addDialogWidget(new QLineEdit(defaultValue, this), labelText);
}

QWidget *ScriptDialog::addTextEdit(const QString &labelText, const QString &defaultValue)
{
    QTextEdit *textEdit = new QTextEdit(defaultValue, this);
    addDialogWidget(textEdit, labelText);
    textEdit->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextEditorInteraction);
    textEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    return textEdit;
}

QWidget *ScriptDialog::addImage(const QString &labelText, Tiled::ScriptImage *image)
{
    return addDialogWidget(new ScriptImageWidget(image, this), labelText);
}

QWidget *ScriptDialog::addNumberInput(const QString &labelText)
{
    return addDialogWidget(new QDoubleSpinBox(this), labelText);
}

QWidget *ScriptDialog::addSlider(const QString &labelText)
{
    QSlider *horizontalSlider = new QSlider(this);
    horizontalSlider->setOrientation(Qt::Horizontal);
    return addDialogWidget(horizontalSlider, labelText);
}

QWidget *ScriptDialog::addCheckBox(const QString &labelText, bool defaultValue)
{
    QCheckBox *checkBox = new QCheckBox(labelText, this);
    checkBox->setCheckState(defaultValue ? Qt::Checked: Qt::Unchecked);
    return addDialogWidget(checkBox);
}
QWidget *ScriptDialog::addComboBox(const QString &labelText, const QStringList &values)
{
    QComboBox *comboBox = new ScriptComboBox(this);
    comboBox->addItems(values);
    return addDialogWidget(comboBox, labelText);
}

QWidget *ScriptDialog::addButton(const QString &labelText)
{
    return addDialogWidget(new QPushButton(labelText, this));
}

QWidget *ScriptDialog::addFilePicker(const QString &labelText)
{
    return addDialogWidget(new FileEdit(this), labelText);
}

QWidget *ScriptDialog::addColorButton(const QString &labelText)
{
    QWidget *colorButton = addDialogWidget(new ColorButton(this), labelText);
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

int ScriptDialog::exec()
{
    ScriptManager::ResetBlocker blocker;
    return QDialog::exec();
}

QWidget *ScriptDialog::addDialogWidget(QWidget *widget, const QString &label)
{
    determineWidgetGrouping(widget);
    if (m_widgetsInRow == 0)
        m_widgetsInRow = 1;

    // right-hand side elements, add to the layout inside the second column
    if (m_widgetsInRow == 1) {
        m_rowLayout = new QHBoxLayout;
        m_gridLayout->addLayout(m_rowLayout, m_rowIndex, 1, 1, 1);
    }

    if (!label.isEmpty()) {
        QLabel *widgetLabel = newLabel(label);
        widgetLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        widgetLabel->setBuddy(widget);
        m_rowLayout->addWidget(widgetLabel);
    }

    widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_rowLayout->addWidget(widget);
    m_lastWidgetType = widget->metaObject();
    m_widgetsInRow++;

    return widget;
}

/**
 * Based on the current NewRowMode, determine whether we should add a newline
 * before placing a widget
 *
 * @param widget - the widget that is about to be added to the layout
 */
void ScriptDialog::determineWidgetGrouping(QWidget *widget)
{
    switch (newRowMode()) {
    case SameWidgetRows: {
        const QMetaObject *widgetType = widget->metaObject();

        // labels can be mixed with any type of widget
        if ((m_lastWidgetType != &QLabel::staticMetaObject &&
             widgetType != &QLabel::staticMetaObject &&
             m_lastWidgetType != nullptr &&
             m_lastWidgetType != widgetType)) {
            // if the new widget type is not the same as the last
            addNewRow();
        }
        break;
    }
    case ManualRows:
        // any widgets that get checked via this method
        // are right column widgets.
        break;
    case SingleWidgetRows:
        // if there are any other widgets on this row so far in single widget
        // mode, wrap to the next row
        if (m_widgetsInRow > 1)
            addNewRow();
        break;
    }
}

void ScriptDialog::addNewRow()
{
    m_rowIndex++;
    m_widgetsInRow = 0;
    m_lastWidgetType = nullptr;
}

QLabel *ScriptDialog::newLabel(const QString &labelText)
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
#include "scriptdialog.moc"
