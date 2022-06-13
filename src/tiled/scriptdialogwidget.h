/*
 * scriptdialogwidget.h
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
#include <QObject>
#include <QString>
#include <QtWidgets/QLabel>
#include <QWidget>
namespace Tiled {
/**
 * Class wrapping a widget created by calls to ScriptDialog's methods.
 * Pairs the widget with its optional widget label so that both can be
 * updated at once.
 */
class ScriptDialogWidget : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString toolTip READ getToolTip WRITE setToolTip)
    Q_PROPERTY(QWidget *mainWidget READ mainWidget)
public:
    Q_INVOKABLE void setLabelText(const QString &labelText);
    Q_INVOKABLE void setToolTip(const QString &labelText);
    Q_INVOKABLE ScriptDialogWidget(QLabel *label, QWidget *mainWidget);

    QLabel *label;
    QWidget *mainWidget() const;
    QString getToolTip();
private:
    ScriptDialogWidget();
    QWidget *m_mainWidget;
};
} //namespace Tiled
Q_DECLARE_METATYPE(Tiled::ScriptDialogWidget*);
Q_DECLARE_METATYPE(QWidget*);


