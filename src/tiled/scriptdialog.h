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
#include <QCoreApplication>
#include <QDialog>
#include "colorbutton.h"
#include <QString>
#include <QList>

class QJSEngine;

namespace Tiled {

class NumberInputArgs : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int numberOfDecimals  MEMBER numberOfDecimals)
    Q_PROPERTY(double maximum MEMBER maximum)
    Q_PROPERTY(double minimum MEMBER minimum)
    Q_PROPERTY(double defaultValue MEMBER defaultValue)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
    Q_PROPERTY(QString label  READ label WRITE setLabel)

public:
    Q_INVOKABLE NumberInputArgs();
    Q_INVOKABLE NumberInputArgs(const NumberInputArgs& args);
    // number of decimal places. 0 for integers.
    int numberOfDecimals;
    double maximum;
    double minimum;
    double defaultValue =0;

    QString prefix() const;
    QString suffix() const;
    QString label() const;
    void setPrefix(const QString &prefix);
    void setSuffix(const QString &suffix);
    void setLabel(const QString &label);
private:
    QString m_prefix;
    QString m_suffix;
    QString m_label;
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
    Q_INVOKABLE QDoubleSpinBox *addNumberInput(const NumberInputArgs &inputArgs);
    Q_INVOKABLE QSlider * addSlider(const QString &labelText);
    Q_INVOKABLE QCheckBox * addCheckbox(const QString &labelText, bool defaultValue);
    Q_INVOKABLE QPushButton * addButton(const QString &labelText);
    Q_INVOKABLE Tiled::ColorButton * addColorButton(const QString &labelText);
    Q_INVOKABLE void setMinimumWidth(const int width);
    Q_INVOKABLE void close();
    Q_INVOKABLE void clear();
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    // used to generate unique IDs for all components
    int m_widgetNumber;
    QList<QWidget*> m_ScriptWidgets;
    QGridLayout *m_gridLayout;
    QWidget *m_verticalLayoutWidget;
    QVBoxLayout *m_verticalLayout;
    void initializeLayout();

};

void registerDialog(QJSEngine *jsEngine);

} // namespace Tiled
Q_DECLARE_METATYPE(Tiled::NumberInputArgs);
Q_DECLARE_METATYPE(Tiled::NumberInputArgs*);
Q_DECLARE_METATYPE(Tiled::ScriptDialog*);

