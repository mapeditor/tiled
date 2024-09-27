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
#include <QHash>
#include <QMetaProperty>
#include <QScrollArea>
#include <QString>
#include <QStringListModel>
#include <QVariant>
#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;

namespace Tiled {

class PropertyLabel;

/**
 * A property represents a named value that can create its own edit widget.
 */
class Property : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip NOTIFY toolTipChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool modified READ isModified WRITE setModified NOTIFY modifiedChanged)

public:
    enum class DisplayMode {
        Default,
        NoLabel,
        Header,
        Separator
    };

    enum Action {
        Reset = 0x01,
        Remove = 0x02,
    };
    Q_DECLARE_FLAGS(Actions, Action)

    Property(const QString &name, QObject *parent = nullptr)
        : QObject(parent)
        , m_name(name)
    {}

    const QString &name() const { return m_name; }

    const QString &toolTip() const { return m_toolTip; }
    void setToolTip(const QString &toolTip);

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    bool isModified() const { return m_modified; }
    void setModified(bool modified);

    Actions actions() const { return m_actions; }
    void setActions(Actions actions) { m_actions = actions; }

    virtual DisplayMode displayMode() const { return DisplayMode::Default; }

    virtual QWidget *createEditor(QWidget *parent) = 0;

signals:
    void toolTipChanged(const QString &toolTip);
    void valueChanged();
    void enabledChanged(bool enabled);
    void modifiedChanged(bool modified);

    void resetRequested();
    void removeRequested();

private:
    friend class GroupProperty;

    QString m_name;
    QString m_toolTip;
    bool m_enabled = true;
    bool m_modified = false;
    Actions m_actions;
};

class Separator final : public Property
{
    Q_OBJECT

public:
    Separator(QObject *parent = nullptr)
        : Property(QString(), parent)
    {}

    DisplayMode displayMode() const override { return DisplayMode::Separator; }
    QWidget *createEditor(QWidget */*parent*/) override { return nullptr; }
};

/**
 * A property that can have sub-properties. The GroupProperty owns the sub-
 * properties and will delete them when it is deleted.
 */
class GroupProperty : public Property
{
    Q_OBJECT

public:
    GroupProperty(const QString &name, QObject *parent = nullptr)
        : Property(name, parent)
    {}

    ~GroupProperty() override { clear(); }

    DisplayMode displayMode() const override
    { return m_header ? DisplayMode::Header : DisplayMode::Default; }

    QWidget *createEditor(QWidget */* parent */) override { return nullptr; }

    void setHeader(bool header) { m_header = header; }

    void clear()
    {
        qDeleteAll(m_subProperties);
        m_subProperties.clear();
    }

    void addProperty(Property *property)
    {
        m_subProperties.append(property);
        emit propertyAdded(property);
    }

    void deleteProperty(Property *property)
    {
        m_subProperties.removeOne(property);
        delete property;
    }

    void addSeparator() { addProperty(new Separator(this)); }

    const QList<Property*> &subProperties() const { return m_subProperties; }

signals:
    void propertyAdded(Property *property);

private:
    bool m_header = true;
    QList<Property*> m_subProperties;
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
    void setValue(const Type &value) { if (m_set) m_set(value); }

private:
    std::function<Type()> m_get;
    std::function<void(const Type&)> m_set;
};

struct StringProperty : PropertyTemplate<QString>
{
    Q_OBJECT

public:
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;

    void setPlaceholderText(const QString &placeholderText);
    const QString &placeholderText() const { return m_placeholderText; }

signals:
    void placeholderTextChanged(const QString &placeholderText);

private:
    QString m_placeholderText;
};

struct MultilineStringProperty : StringProperty
{
    using StringProperty::StringProperty;
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

    void setMinimum(int minimum) { m_minimum = minimum; }
    void setMaximum(int maximum) { m_maximum = maximum; }
    void setSingleStep(int singleStep) { m_singleStep = singleStep; }
    void setSuffix(const QString &suffix) { m_suffix = suffix; }
    void setRange(int minimum, int maximum)
    {
        setMinimum(minimum);
        setMaximum(maximum);
    }
    void setSliderEnabled(bool enabled) { m_sliderEnabled = enabled; }

protected:
    int m_minimum = std::numeric_limits<int>::min();
    int m_maximum = std::numeric_limits<int>::max();
    int m_singleStep = 1;
    QString m_suffix;
    bool m_sliderEnabled = false;
};

struct FloatProperty : PropertyTemplate<double>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;

    void setMinimum(double minimum) { m_minimum = minimum; }
    void setMaximum(double maximum) { m_maximum = maximum; }
    void setSingleStep(double singleStep) { m_singleStep = singleStep; }
    void setSuffix(const QString &suffix) { m_suffix = suffix; }
    void setRange(double minimum, double maximum)
    {
        setMinimum(minimum);
        setMaximum(maximum);
    }

private:
    double m_minimum = -std::numeric_limits<double>::max();
    double m_maximum = std::numeric_limits<double>::max();
    double m_singleStep = 1.0;
    QString m_suffix;
};

struct BoolProperty : PropertyTemplate<bool>
{
    using PropertyTemplate::PropertyTemplate;
    DisplayMode displayMode() const override { return DisplayMode::NoLabel; }
    QWidget *createEditor(QWidget *parent) override;
};

struct PointProperty : PropertyTemplate<QPoint>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;

    void setSuffix(const QString &suffix) { m_suffix = suffix; }

private:
    QString m_suffix;
};

struct PointFProperty : PropertyTemplate<QPointF>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;

    void setSingleStep(double singleStep) { m_singleStep = singleStep; }

private:
    double m_singleStep = 1.0;
};

struct SizeProperty : PropertyTemplate<QSize>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;

    void setMinimum(int minimum) { m_minimum = minimum; }
    void setSuffix(const QString &suffix) { m_suffix = suffix; }

private:
    int m_minimum;
    QString m_suffix;
};

struct SizeFProperty : PropertyTemplate<QSizeF>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;
};

struct RectProperty : PropertyTemplate<QRect>
{
    Q_OBJECT

public:
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;

    void setConstraint(const QRect &constraint);

signals:
    void constraintChanged(const QRect &constraint);

private:
    QRect m_constraint;
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


Property *createQObjectProperty(QObject *qObject,
                                const char *propertyName,
                                const QString &displayName = {});

Property *createVariantProperty(const QString &name,
                                std::function<QVariant()> get,
                                std::function<void(const QVariant&)> set);

struct EnumData
{
    EnumData(const QStringList &names = {},
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

/**
 * A property that wraps an integer value and creates either a combo box or a
 * list of checkboxes based on the given EnumData.
 */
class BaseEnumProperty : public IntProperty
{
    Q_OBJECT

public:
    using IntProperty::IntProperty;

    void setEnumData(const EnumData &enumData) { m_enumData = enumData; }
    void setFlags(bool flags) { m_flags = flags; }

    QWidget *createEditor(QWidget *parent) override
    {
        return m_flags ? createFlagsEditor(parent)
                       : createEnumEditor(parent);
    }

protected:
    QWidget *createFlagsEditor(QWidget *parent);
    QWidget *createEnumEditor(QWidget *parent);

    EnumData m_enumData;
    bool m_flags = false;
};

/**
 * A property that wraps an enum value and automatically sets the EnumData
 * based on the given type.
 */
template <typename Enum>
class EnumProperty : public BaseEnumProperty
{
public:
    EnumProperty(const QString &name,
                 std::function<Enum()> get,
                 std::function<void(Enum)> set,
                 QObject *parent = nullptr)
        : BaseEnumProperty(name,
                           [get] {
                               return static_cast<int>(get());
                           },
                           set ? [set](const int &value){ set(static_cast<Enum>(value)); }
                               : std::function<void(const int&)>(),
                           parent)
    {
        setEnumData(enumData<Enum>());
    }
};


class VariantEditor : public QWidget
{
    Q_OBJECT

public:
    VariantEditor(QWidget *parent = nullptr);

    void clear();
    void addProperty(Property *property);
    void removeProperty(Property *property);

    void setLevel(int level);

private:
    static constexpr int LabelStretch = 4;
    static constexpr int WidgetStretch = 6;

    struct PropertyWidgets
    {
        QHBoxLayout *layout = nullptr;
        QWidget *children = nullptr;
    };

    QVBoxLayout *m_layout;
    QHash<Property*, PropertyWidgets> m_propertyWidgets;
    int m_level = 0;
};

} // namespace Tiled

Q_DECLARE_OPERATORS_FOR_FLAGS(Tiled::Property::Actions)
