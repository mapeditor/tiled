/*
 * propertiesview.h
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

#include <QHash>
#include <QIcon>
#include <QPointer>
#include <QScrollArea>
#include <QString>
#include <QVariant>
#include <QWidget>

class QHBoxLayout;
class QMenu;
class QToolButton;
class QVBoxLayout;

namespace Tiled {

class GroupProperty;
class PropertyLabel;

/**
 * A property represents a named value that can create its own edit widget.
 */
class Property : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip NOTIFY toolTipChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool modified READ isModified WRITE setModified NOTIFY modifiedChanged)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY(Actions actions READ actions WRITE setActions NOTIFY actionsChanged)

public:
    enum class DisplayMode {
        Default,
        NoLabel,
        Header,
        Separator,
        ChildrenOnly
    };

    enum class Action {
        Reset = 0x01,
        Remove = 0x02,
        Add = 0x04,
        AddDisabled = Add | 0x08,
        Select = 0x10,
    };
    Q_DECLARE_FLAGS(Actions, Action)

    Property(const QString &name, QObject *parent = nullptr)
        : QObject(parent)
        , m_name(name)
    {}

    const QString &name() const { return m_name; }
    void setName(const QString &name);

    const QString &toolTip() const { return m_toolTip; }
    void setToolTip(const QString &toolTip);

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    bool isDimmed() const { return m_dimmed; }
    void setDimmed(bool dimmed);

    bool isModified() const { return m_modified; }
    void setModified(bool modified);

    bool isSelected() const { return m_selected; }
    void setSelected(bool selected);

    Actions actions() const { return m_actions; }
    void setActions(Actions actions);

    GroupProperty *parentProperty() const { return m_parent; }

    virtual DisplayMode displayMode() const { return DisplayMode::Default; }

    virtual QWidget *createLabel(int level, QWidget *parent);
    virtual QWidget *createEditor(QWidget *parent) = 0;
    virtual void addContextMenuActions(QMenu *) {}

signals:
    void nameChanged(const QString &name);
    void toolTipChanged(const QString &toolTip);
    void valueChanged();
    void enabledChanged(bool enabled);
    void dimmedChanged(bool dimmed);
    void modifiedChanged(bool modified);
    void selectedChanged(bool selected);
    void actionsChanged(Actions actions);

    void resetRequested();
    void removeRequested();
    void addRequested();

    void contextMenuRequested(const QPoint &globalPos);

private:
    friend class GroupProperty;

    QString m_name;
    QString m_toolTip;
    bool m_enabled = true;
    bool m_dimmed = false;
    bool m_modified = false;
    bool m_selected = false;
    Actions m_actions;
    GroupProperty *m_parent = nullptr;
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
    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded NOTIFY expandedChanged)

public:
    /**
     * Creates an unnamed group, which will only display its children.
     */
    explicit GroupProperty(QObject *parent = nullptr)
        : Property(QString(), parent)
    {}

    explicit GroupProperty(const QString &name, QObject *parent = nullptr)
        : Property(name, parent)
    {}

    ~GroupProperty() override { clear(); }

    DisplayMode displayMode() const override;

    QWidget *createLabel(int level, QWidget *parent) override;
    QWidget *createEditor(QWidget */* parent */) override { return nullptr; }
    void addContextMenuActions(QMenu *) override;

    void setHeader(bool header) { m_header = header; }

    bool isExpanded() const { return m_expanded; }
    void setExpanded(bool expanded);
    void expandAll();
    void collapseAll();

    void clear()
    {
        qDeleteAll(m_subProperties);
        m_subProperties.clear();
    }

    void addProperty(Property *property)
    {
        insertProperty(m_subProperties.size(), property);
    }

    void insertProperty(int index, Property *property)
    {
        property->m_parent = this;

        m_subProperties.insert(index, property);
        emit propertyAdded(index, property);
    }

    void deleteProperty(Property *property)
    {
        removeProperty(property);
        delete property;
    }

    /**
     * Removes the given property from this group. Ownership of the property
     * is transferred to the caller.
     */
    void removeProperty(Property *property)
    {
        if (!m_subProperties.removeOne(property))
            return;

        property->m_parent = nullptr;
        emit propertyRemoved(property);
    }

    int indexOfProperty(Property *property) const
    {
        return m_subProperties.indexOf(property);
    }

    void addSeparator() { addProperty(new Separator(this)); }

    const QList<Property*> &subProperties() const { return m_subProperties; }

signals:
    void expandedChanged(bool expanded);
    void propertyAdded(int index, Property *property);
    void propertyRemoved(Property *property);

private:
    bool m_header = true;
    bool m_expanded = true;
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
    void addContextMenuActions(QMenu *) override;

    void setFilter(const QString &filter) { m_filter = filter; }
    void setIsDirectory(bool isDirectory) { m_isDirectory = isDirectory; }

private:
    QString m_filter;
    bool m_isDirectory = false;
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
    DisplayMode displayMode() const override;
    QWidget *createEditor(QWidget *parent) override;

    void setNameOnCheckBox(bool nameOnCheckBox) { m_nameOnCheckBox = nameOnCheckBox; }

private:
    bool m_nameOnCheckBox = false;
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
    int m_minimum = 0;
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

struct ColorProperty : PropertyTemplate<QColor>
{
    using PropertyTemplate::PropertyTemplate;
    QWidget *createEditor(QWidget *parent) override;

    void setAlpha(bool alpha) { m_alpha = alpha; }

private:
    bool m_alpha = true;
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
    return {};
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


/**
 * A scrollable view that displays a tree of properties.
 */
class PropertiesView : public QScrollArea
{
    Q_OBJECT

public:
    explicit PropertiesView(QWidget *parent = nullptr);

    void setRootProperty(GroupProperty *root);

    QList<Property*> selectedProperties() const;
    void setSelectedProperties(const QList<Property*> &properties);

    enum FocusTarget {
        FocusRow,
        FocusLabel,
        FocusEditor,
    };

    bool focusProperty(Property *property, FocusTarget target = FocusEditor);
    Property *focusedProperty() const;

signals:
    void selectedPropertiesChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    bool focusNextPrevProperty(Property *property, bool next, bool shiftPressed);

    void deletePropertyWidgets(Property *property);
    void forgetProperty(Property *property);

    QWidget *focusPropertyImpl(GroupProperty *group, Property *property, FocusTarget target);

    struct PropertyWidgets
    {
        int level = 0;
        QWidget *rowWidget = nullptr;               // The top-level widget for this property
        QWidget *label = nullptr;
        QWidget *editor = nullptr;
        QToolButton *resetButton = nullptr;
        QToolButton *removeButton = nullptr;
        QToolButton *addButton = nullptr;
        QWidget *children = nullptr;                // The widget that contains the children of a group property
    };

    void createPropertyWidgets(Property *property, QWidget *parent, int level,
                               QVBoxLayout *layout, int index);
    PropertyWidgets createPropertyWidgets(Property *property, QWidget *parent, int level);
    QWidget *createChildrenWidget(GroupProperty *groupProperty, QWidget *parent, int level);

    void setPropertyChildrenExpanded(PropertyWidgets &widgets,
                                     GroupProperty *groupProperty, QVBoxLayout *rowVerticalLayout,
                                     bool expanded);

    void updatePropertyEnabled(const PropertyWidgets &widgets, Property *property);
    void updatePropertyActions(const PropertyWidgets &widgets,
                               Property::Actions actions);

    void fixTabOrder();

    static constexpr int LabelStretch = 1;
    static constexpr int EditorStretch = 1;

    bool m_fixTabOrderScheduled = false;
    QIcon m_resetIcon;
    QIcon m_removeIcon;
    QIcon m_addIcon;
    QPointer<GroupProperty> m_root;
    QVBoxLayout *m_rootLayout;
    QPointer<Property> m_selectionStart;
    QHash<Property*, PropertyWidgets> m_propertyWidgets;
};

} // namespace Tiled

Q_DECLARE_OPERATORS_FOR_FLAGS(Tiled::Property::Actions)
