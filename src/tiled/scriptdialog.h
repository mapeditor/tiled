/*
 * scriptdialog.h
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

#pragma once

#include <QLabel>
#include <QDialog>

class QGridLayout;
class QHBoxLayout;

class QJSEngine;

namespace Tiled {

class ScriptImage;

/**
 * A widget which allows the user to display a ScriptImage
 */
class ScriptImageWidget : public QLabel
{
    Q_OBJECT

    Q_PROPERTY(Tiled::ScriptImage *image READ image WRITE setImage)

public:
    ScriptImageWidget(Tiled::ScriptImage *image, QWidget *parent);

    ScriptImage *image() const;
    void setImage(ScriptImage *image);
};


class ScriptDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(Tiled::ScriptDialog::NewRowMode newRowMode READ newRowMode WRITE setNewRowMode)

public:
    enum NewRowMode {
        SameWidgetRows = 0,
        ManualRows = 1,
        SingleWidgetRows = 2
    };
    Q_ENUM(NewRowMode)

    // Duplicate from QDialog, because the enum on QDialog isn't marked Q_ENUM
    enum DialogCode { Rejected, Accepted };
    Q_ENUM(DialogCode)

    Q_INVOKABLE ScriptDialog(const QString &title = QString());
    ~ScriptDialog() override;

    Q_INVOKABLE QWidget *addHeading(const QString &text, bool fillRow = false);
    Q_INVOKABLE QWidget *addLabel(const QString &text);
    Q_INVOKABLE QWidget *addSeparator(const QString &labelText = QString());
    Q_INVOKABLE QWidget *addTextInput(const QString &labelText = QString(), const QString &defaultValue = QString());
    Q_INVOKABLE QWidget *addTextEdit(const QString &labelText, const QString &defaultValue= QString());
    Q_INVOKABLE QWidget *addNumberInput(const QString &labelText);
    Q_INVOKABLE QWidget *addSlider(const QString &labelText);
    Q_INVOKABLE QWidget *addComboBox(const QString &labelText, const QStringList &values);
    Q_INVOKABLE QWidget *addCheckBox(const QString &labelText = QString(), bool defaultValue = false);
    Q_INVOKABLE QWidget *addButton(const QString &labelText = QString());
    Q_INVOKABLE QWidget *addFilePicker(const QString &labelText = QString());
    Q_INVOKABLE QWidget *addColorButton(const QString &labelText = QString());
    Q_INVOKABLE QWidget *addImage(const QString &labelText, Tiled::ScriptImage *image);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void addNewRow();

    NewRowMode newRowMode() const;
    void setNewRowMode(NewRowMode mode);

    int exec() override;

    static void deleteAllDialogs();

private:
    QLabel *newLabel(const QString &labelText);
    void initializeLayout();
    void determineWidgetGrouping(QWidget *widget);
    QWidget *addDialogWidget(QWidget * widget, const QString &label = QString());

    int m_rowIndex = 0;
    int m_widgetsInRow = 0;
    QGridLayout *m_gridLayout;
    QHBoxLayout *m_rowLayout;
    const QMetaObject *m_lastWidgetType;
    NewRowMode m_newRowMode = SameWidgetRows;

    static QSet<ScriptDialog*> sDialogInstances;
};


void registerDialog(QJSEngine *jsEngine);

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::ScriptDialog*);
Q_DECLARE_METATYPE(Tiled::ScriptImageWidget*)
