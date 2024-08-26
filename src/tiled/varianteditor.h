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
#include <QStringListModel>
#include <QVariant>
#include <QWidget>

#include <memory>
#include <unordered_map>

class QGridLayout;

namespace Tiled {

class Property : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)

public:
    Property(const QString &name, QObject *parent = nullptr)
        : QObject(parent)
        , m_name(name)
    {}

    QString name() const { return m_name; }

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled)
    {
        if (m_enabled != enabled) {
            m_enabled = enabled;
            emit enabledChanged(enabled);
        }
    }

    virtual QVariant value() const = 0;
    virtual void setValue(const QVariant &value) = 0;

    virtual QWidget *createEditor(QWidget *parent) = 0;

signals:
    void valueChanged();
    void enabledChanged(bool enabled);

private:
    QString m_name;
    bool m_enabled = true;
};

class VariantProperty : public Property
{
    Q_OBJECT

public:
    VariantProperty(const QString &name, const QVariant &value, QObject *parent = nullptr)
        : Property(name, parent)
        , m_value(value)
    {
    }

    QVariant value() const override { return m_value; }
    void setValue(const QVariant &value) override;

    QWidget *createEditor(QWidget *parent) override;

private:
    QVariant m_value;
};

class EnumProperty : public Property
{
    Q_OBJECT

public:
    EnumProperty(const QString &name, QObject *parent = nullptr)
        : Property(name, parent)
    {}

    QWidget *createEditor(QWidget *parent) override;

    void setEnumNames(const QStringList &enumNames)
    {
        m_enumNamesModel.setStringList(enumNames);
    }

    void setEnumValues(const QList<int> &enumValues)
    {
        m_enumValues = enumValues;
    }

private:
    QStringListModel m_enumNamesModel;
    QList<int> m_enumValues;
};

class EditorFactory
{
    Q_DECLARE_TR_FUNCTIONS(EditorFactory)

public:
    virtual QWidget *createEditor(const QVariant &value,
                                  QWidget *parent) = 0;
};

class VariantEditor : public QScrollArea
{
    Q_OBJECT

public:
    VariantEditor(QWidget *parent = nullptr);

    void registerEditorFactory(int type, std::unique_ptr<EditorFactory> factory);

    void clear();
    void addHeader(const QString &text);
    void addSeparator();
    void addProperty(Property *property);
    // void addValue(const QVariant &value);

private:
    QWidget *createEditor(Property *property);

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
