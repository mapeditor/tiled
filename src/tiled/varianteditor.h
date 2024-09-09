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
#include <QMetaProperty>
#include <QScrollArea>
#include <QString>
#include <QStringListModel>
#include <QVariant>
#include <QWidget>

class QGridLayout;

namespace Tiled {

/**
 * A property represents a named value that can create its own edit widget.
 */
class Property : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip NOTIFY toolTipChanged)
    Q_PROPERTY(QVariant value READ variantValue WRITE setVariantValue NOTIFY valueChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)

public:
    Property(const QString &name, QObject *parent = nullptr)
        : QObject(parent)
        , m_name(name)
    {}

    const QString &name() const { return m_name; }

    const QString &toolTip() const { return m_toolTip; }
    void setToolTip(const QString &toolTip)
    {
        if (m_toolTip != toolTip) {
            m_toolTip = toolTip;
            emit toolTipChanged(toolTip);
        }
    }

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled)
    {
        if (m_enabled != enabled) {
            m_enabled = enabled;
            emit enabledChanged(enabled);
        }
    }

    virtual QVariant variantValue() const = 0;
    virtual void setVariantValue(const QVariant &value) = 0;

    virtual QWidget *createEditor(QWidget *parent) = 0;

signals:
    void toolTipChanged(const QString &toolTip);
    void valueChanged();
    void enabledChanged(bool enabled);

private:
    QString m_name;
    QString m_toolTip;
    bool m_enabled = true;
};

/**
 * A helper class for creating a property that wraps a value of a given type.
 */
template <typename Type>
class PropertyTemplate : public Property
{
public:
    using ValueType = Type;

    PropertyTemplate(const QString &name,
                     std::function<Type()> get,
                     std::function<void(const Type&)> set = {},
                     QObject *parent = nullptr)
        : Property(name, parent)
        , m_get(std::move(get))
        , m_set(std::move(set))
    {}

    Type value() const { return m_get(); }
    void setValue(const Type &value) { m_set(value); }

    QVariant variantValue() const override
    {
        return QVariant::fromValue(m_get());
    }

    void setVariantValue(const QVariant &value) override
    {
        if (m_set)
            m_set(value.value<Type>());
    }

private:
    std::function<Type()> m_get;
    std::function<void(const Type&)> m_set;
};

struct StringProperty : PropertyTemplate<QString>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
};

struct UrlProperty : PropertyTemplate<QUrl>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
    void setFilter(const QString &filter) { m_filter = filter; }
private:
    QString m_filter;
};

struct IntProperty : PropertyTemplate<int>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
};

struct FloatProperty : PropertyTemplate<double>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
    void setSuffix(const QString &suffix) { m_suffix = suffix; }
private:
    QString m_suffix;
};

struct BoolProperty : PropertyTemplate<bool>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
};

struct PointProperty : PropertyTemplate<QPoint>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
};

struct PointFProperty : PropertyTemplate<QPointF>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
};

struct SizeProperty : PropertyTemplate<QSize>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
};

struct SizeFProperty : PropertyTemplate<QSizeF>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
};

struct RectProperty : PropertyTemplate<QRect>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
};

struct RectFProperty : PropertyTemplate<QRectF>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
};

// todo: needs to handle invalid color (unset value)
struct ColorProperty : PropertyTemplate<QColor>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
};

struct FontProperty : PropertyTemplate<QFont>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
};

struct QtAlignmentProperty : PropertyTemplate<Qt::Alignment>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
};


/**
 * A property factory that instantiates the appropriate property type based on
 * the type of the property value.
 */
class PropertyFactory
{
public:
    PropertyFactory() = default;

    /**
     * Creates a property that wraps a QObject property.
     */
    Property *createQObjectProperty(QObject *qObject,
                                    const char *propertyName,
                                    const QString &displayName = {});

    /**
     * Creates a property with the given name and get/set functions. The
     * value type determines the kind of property that will be created.
     */
    Property *createProperty(const QString &name,
                             std::function<QVariant()> get,
                             std::function<void(const QVariant&)> set);
};

struct EnumData
{
    EnumData(const QStringList &names,
             const QList<int> &values = {},
             const QMap<int, QIcon> &icons = {})
        : names(names)
        , values(values)
        , icons(icons)
    {}

    QStringList names;
    QList<int> values;          // optional
    QMap<int, QIcon> icons;     // optional
};

template<typename>
EnumData enumData()
{
    return {{}};
}

QWidget *createEnumEditor(IntProperty *property,
                          const EnumData &enumData,
                          QWidget *parent);

/**
 * A property that wraps an enum value and creates a combo box based on the
 * given EnumData.
 */
template <typename Enum>
class EnumProperty : public IntProperty
{
public:
    EnumProperty(const QString &name,
                 std::function<Enum()> get,
                 std::function<void(Enum)> set,
                 QObject *parent = nullptr)
        : IntProperty(name,
                      [get] {
                          return static_cast<int>(get());
                      },
                      set ? [set](const int &value){ set(static_cast<Enum>(value)); }
                          : std::function<void(const int&)>(),
                      parent)
        , m_enumData(enumData<Enum>())
    {}

    void setEnumData(const EnumData &enumData)
    {
        m_enumData = enumData;
    }

    QWidget *createEditor(QWidget *parent) override
    {
        return createEnumEditor(this, m_enumData, parent);
    }

private:
    EnumData m_enumData;
};


class VariantEditor : public QScrollArea
{
    Q_OBJECT

public:
    VariantEditor(QWidget *parent = nullptr);

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
};

} // namespace Tiled
