/*
 * scriptdialog.h
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

#pragma once

#include "mainwindow.h"
#include "scriptimage.h"
#include "fileedit.h"
#include "colorbutton.h"
#include <QString>
#include <QObject>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QString>
#include <QLineEdit>
#include <QTextEdit>
#include <QList>
#include <QSize>
#include <QPixmap>
#include <QBitmap>
class QJSEngine;

namespace Tiled {

/**
 * A widget which allows the user to display a ScriptImage
 */
class ScriptImageWidget: public QLabel{
    Q_OBJECT
    Q_PROPERTY(Tiled::ScriptImage *image READ image WRITE setImage)

public:
    ScriptImageWidget(Tiled::ScriptImage *image, QWidget *parent);
    ScriptImage *image() const;
    void setImage(Tiled::ScriptImage *image);
private:
    ScriptImage *m_image;
};
class ScriptDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(Tiled::ScriptDialog::NewRowMode newRowMode READ newRowMode WRITE setNewRowMode)

public:

    enum NewRowMode {
        SameWidgetRows =0,
        ManualRows = 1,
        SingleWidgetRows = 2
    };
    Q_ENUM(NewRowMode)
    Q_INVOKABLE ScriptDialog(const QString &title = QString());

    Q_INVOKABLE QWidget *addHeading(const QString &text, bool fillRow = false);
    Q_INVOKABLE QWidget *addLabel(const QString &text);
    Q_INVOKABLE QWidget *addSeparator(const QString &labelText = QString());
    Q_INVOKABLE QWidget *addTextInput(const QString &labelText= QString(), const QString &defaultValue= QString());
    Q_INVOKABLE QWidget *addTextEdit(const QString &labelText, const QString &defaultValue= QString());
    Q_INVOKABLE QWidget *addNumberInput(const QString &labelText);
    Q_INVOKABLE QWidget *addSlider(const QString &labelText);
    Q_INVOKABLE QWidget *addComboBox(const QString &labelText, const QStringList &values);
    Q_INVOKABLE QWidget *addCheckBox(const QString &labelText =QString(), bool defaultValue = false);
    Q_INVOKABLE QWidget *addButton(const QString &labelText = QString());
    Q_INVOKABLE QWidget *addFilePicker(const QString &labelText = QString());
    Q_INVOKABLE QWidget *addColorButton(const QString &labelText = QString());
    Q_INVOKABLE QWidget *addImage(const QString &labelText, Tiled::ScriptImage * image);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void addNewRow();

    NewRowMode newRowMode() const;
    void setNewRowMode(NewRowMode mode);
private:
    int m_rowIndex = 0;
    int m_widgetsInRow = 0;
    QGridLayout *m_gridLayout;
    QLabel *newLabel(const QString& labelText);
    void initializeLayout();
    QHBoxLayout* m_rowLayout;
    QString m_lastWidgetTypeName;
    NewRowMode m_newRowMode = SameWidgetRows;
    void determineWidgetGrouping(QWidget *widget);
    QWidget *addDialogWidget(QWidget * widget, QLabel *widgetLabel = nullptr);
};


void registerDialog(QJSEngine *jsEngine);

} // namespace Tiled
Q_DECLARE_METATYPE(Tiled::ScriptDialog*);
Q_DECLARE_METATYPE(QCheckBox*)
Q_DECLARE_METATYPE(QPushButton*)
Q_DECLARE_METATYPE(QSlider*)
Q_DECLARE_METATYPE(QLabel*)
Q_DECLARE_METATYPE(QLineEdit*)
Q_DECLARE_METATYPE(QTextEdit*)
Q_DECLARE_METATYPE(Tiled::ScriptImageWidget*)

