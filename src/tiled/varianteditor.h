/*
 * varianteditor.h
 * Copyright 2024, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QCoreApplication>
#include <QScrollArea>
#include <QString>
#include <QVariant>
#include <QWidget>

#include <memory>
#include <unordered_map>

class QGridLayout;

namespace Tiled {

class Property : public QObject
{
    Q_OBJECT

public:
    Property(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    virtual QWidget *createEditor(QWidget *parent) = 0;

    virtual QVariant value() const = 0;
    virtual void setValue(const QVariant &value) = 0;

signals:
    void valueChanged(const QVariant &value);
};

class EditorFactory
{
    Q_DECLARE_TR_FUNCTIONS(EditorFactory)

public:
    virtual QWidget *createEditor(const QVariant &value,
                                  QWidget *parent) = 0;

    virtual QVariant value(QWidget *editor) const { return {}; }
    virtual void setValue(QWidget *editor, const QVariant &value) {};
};

class VariantEditor : public QScrollArea
{
    Q_OBJECT

public:
    VariantEditor(QWidget *parent = nullptr);

    void registerEditorFactory(int type, std::unique_ptr<EditorFactory> factory);

    void addHeader(const QString &text);
    void addSeparator();
    void addValue(const QString &name, const QVariant &value);
    void addValue(const QVariant &value);
    QWidget *createEditor(const QVariant &value);

private:
    enum Column {
        LeftSpacing,
        LabelColumn,
        MiddleSpacing,
        WidgetColumn,
        RightSpacing,
        ColumnCount,
    };

    QWidget *m_widget;
    QGridLayout *m_gridLayout;
    int m_rowIndex = 0;
    std::unordered_map<int, std::unique_ptr<EditorFactory>> m_factories;
};

} // namespace Tiled
