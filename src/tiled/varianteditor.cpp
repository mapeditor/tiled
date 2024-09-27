/*
 * varianteditor.cpp
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

#include "varianteditor.h"

#include "colorbutton.h"
#include "fileedit.h"
#include "textpropertyedit.h"
#include "utils.h"
#include "propertyeditorwidgets.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QFontComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QResizeEvent>
#include <QSpacerItem>
#include <QSpinBox>
#include <QToolButton>

namespace Tiled {

void Property::setToolTip(const QString &toolTip)
{
    if (m_toolTip != toolTip) {
        m_toolTip = toolTip;
        emit toolTipChanged(toolTip);
    }
}

void Property::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit enabledChanged(enabled);
    }
}

void Property::setModified(bool modified)
{
    if (m_modified != modified) {
        m_modified = modified;
        emit modifiedChanged(modified);
    }
}

void StringProperty::setPlaceholderText(const QString &placeholderText)
{
    if (m_placeholderText != placeholderText) {
        m_placeholderText = placeholderText;
        emit placeholderTextChanged(placeholderText);
    }
}

QWidget *StringProperty::createEditor(QWidget *parent)
{
    auto editor = new QLineEdit(parent);
    editor->setPlaceholderText(m_placeholderText);

    auto syncEditor = [=] {
        editor->setText(value());
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(this, &StringProperty::placeholderTextChanged, editor, &QLineEdit::setPlaceholderText);
    connect(editor, &QLineEdit::textEdited, this, &StringProperty::setValue);

    return editor;
}

QWidget *MultilineStringProperty::createEditor(QWidget *parent)
{
    auto editor = new TextPropertyEdit(parent);
    editor->lineEdit()->setPlaceholderText(placeholderText());

    auto syncEditor = [=] {
        const QSignalBlocker blocker(editor);
        editor->setText(value());
    };
    syncEditor();

    connect(this, &StringProperty::valueChanged, editor, syncEditor);
    connect(this, &StringProperty::placeholderTextChanged,
            editor->lineEdit(), &QLineEdit::setPlaceholderText);
    connect(editor, &TextPropertyEdit::textChanged, this, &StringProperty::setValue);

    return editor;
}

QWidget *UrlProperty::createEditor(QWidget *parent)
{
    auto editor = new FileEdit(parent);
    editor->setFilter(m_filter);

    auto syncEditor = [=] {
        editor->setFileUrl(value());
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(editor, &FileEdit::fileUrlChanged, this, &UrlProperty::setValue);

    return editor;
}

QWidget *IntProperty::createEditor(QWidget *parent)
{
    auto widget = new QWidget(parent);
    auto layout = new QHBoxLayout(widget);
    layout->setContentsMargins(QMargins());

    if (m_sliderEnabled) {
        QSlider *slider = new QSlider(Qt::Horizontal, widget);
        slider->setRange(m_minimum, m_maximum);
        slider->setSingleStep(m_singleStep);
        slider->setValue(value());

        layout->addWidget(slider);

        connect(this, &Property::valueChanged, slider, [this, slider] {
            const QSignalBlocker blocker(slider);
            slider->setValue(value());
        });
        connect(slider, &QSlider::valueChanged, this, &IntProperty::setValue);
    }

    auto spinBox = new SpinBox(parent);
    spinBox->setRange(m_minimum, m_maximum);
    spinBox->setSingleStep(m_singleStep);
    spinBox->setSuffix(m_suffix);
    spinBox->setValue(value());

    layout->addWidget(spinBox);

    connect(this, &Property::valueChanged, spinBox, [this, spinBox] {
        const QSignalBlocker blocker(spinBox);
        spinBox->setValue(value());
    });
    connect(spinBox, qOverload<int>(&SpinBox::valueChanged),
            this, &IntProperty::setValue);

    return widget;
}

QWidget *FloatProperty::createEditor(QWidget *parent)
{
    auto editor = new DoubleSpinBox(parent);
    editor->setRange(m_minimum, m_maximum);
    editor->setSingleStep(m_singleStep);
    editor->setSuffix(m_suffix);

    auto syncEditor = [=] {
        const QSignalBlocker blocker(editor);
        editor->setValue(value());
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(editor, qOverload<double>(&DoubleSpinBox::valueChanged),
            this, &FloatProperty::setValue);

    return editor;
}

QWidget *BoolProperty::createEditor(QWidget *parent)
{
    auto editor = new QCheckBox(name(), parent);
    auto syncEditor = [=] {
        const QSignalBlocker blocker(editor);
        bool checked = value();
        editor->setChecked(checked);

        // Reflect modified state on the checkbox, since we're not showing the
        // property label.
        auto font = editor->font();
        font.setBold(isModified());
        editor->setFont(font);
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(this, &Property::modifiedChanged, editor, syncEditor);
    connect(editor, &QCheckBox::toggled, this, &BoolProperty::setValue);

    return editor;
}

QWidget *PointProperty::createEditor(QWidget *parent)
{
    auto editor = new PointEdit(parent);
    editor->setSuffix(m_suffix);

    auto syncEditor = [this, editor] {
        const QSignalBlocker blocker(editor);
        editor->setValue(value());
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(editor, &PointEdit::valueChanged, this, [this, editor] {
        setValue(editor->value());
    });

    return editor;
}

QWidget *PointFProperty::createEditor(QWidget *parent)
{
    auto editor = new PointFEdit(parent);
    editor->setSingleStep(m_singleStep);

    auto syncEditor = [this, editor] {
        const QSignalBlocker blocker(editor);
        editor->setValue(value());
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(editor, &PointFEdit::valueChanged, this, [this, editor] {
        this->setValue(editor->value());
    });

    return editor;
}

QWidget *SizeProperty::createEditor(QWidget *parent)
{
    auto editor = new SizeEdit(parent);
    editor->setMinimum(m_minimum);
    editor->setSuffix(m_suffix);

    auto syncEditor = [this, editor] {
        const QSignalBlocker blocker(editor);
        editor->setValue(value());
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(editor, &SizeEdit::valueChanged, this, [this, editor] {
        setValue(editor->value());
    });

    return editor;
}

QWidget *SizeFProperty::createEditor(QWidget *parent)
{
    auto editor = new SizeFEdit(parent);
    auto syncEditor = [this, editor] {
        const QSignalBlocker blocker(editor);
        editor->setValue(value());
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(editor, &SizeFEdit::valueChanged, this, [this, editor] {
        setValue(editor->value());
    });

    return editor;
}

QWidget *RectProperty::createEditor(QWidget *parent)
{
    auto editor = new RectEdit(parent);
    editor->setConstraint(m_constraint);

    auto syncEditor = [this, editor] {
        const QSignalBlocker blocker(editor);
        editor->setValue(value());
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(editor, &RectEdit::valueChanged, this, [this, editor] {
        setValue(editor->value());
    });
    connect(this, &RectProperty::constraintChanged,
            editor, &RectEdit::setConstraint);

    return editor;
}

void RectProperty::setConstraint(const QRect &constraint)
{
    if (m_constraint != constraint) {
        m_constraint = constraint;
        emit constraintChanged(m_constraint);
    }
}

QWidget *RectFProperty::createEditor(QWidget *parent)
{
    auto editor = new RectFEdit(parent);
    auto syncEditor = [this, editor] {
        const QSignalBlocker blocker(editor);
        editor->setValue(value());
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(editor, &RectFEdit::valueChanged, this, [this, editor] {
        setValue(editor->value());
    });

    return editor;
}

// todo: needs to handle invalid color (unset value)
QWidget *ColorProperty::createEditor(QWidget *parent)
{
    auto editor = new ColorButton(parent);
    auto syncEditor = [=] {
        const QSignalBlocker blocker(editor);
        editor->setColor(value());
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(editor, &ColorButton::colorChanged, this, [this, editor] {
        setValue(editor->color());
    });

    return editor;
}

QWidget *FontProperty::createEditor(QWidget *parent)
{
    auto editor = new QWidget(parent);
    auto fontComboBox = new QFontComboBox(editor);

    auto sizeSpinBox = new QSpinBox(editor);
    sizeSpinBox->setRange(1, 999);
    sizeSpinBox->setSuffix(tr(" px"));
    sizeSpinBox->setKeyboardTracking(false);

    auto bold = new QToolButton(editor);
    bold->setIcon(QIcon(QStringLiteral("://images/scalable/text-bold-symbolic.svg")));
    bold->setToolTip(tr("Bold"));
    bold->setCheckable(true);

    auto italic = new QToolButton(editor);
    italic->setIcon(QIcon(QStringLiteral("://images/scalable/text-italic-symbolic.svg")));
    italic->setToolTip(tr("Italic"));
    italic->setCheckable(true);

    auto underline = new QToolButton(editor);
    underline->setIcon(QIcon(QStringLiteral("://images/scalable/text-underline-symbolic.svg")));
    underline->setToolTip(tr("Underline"));
    underline->setCheckable(true);

    auto strikeout = new QToolButton(editor);
    strikeout->setIcon(QIcon(QStringLiteral("://images/scalable/text-strikethrough-symbolic.svg")));
    strikeout->setToolTip(tr("Strikethrough"));
    strikeout->setCheckable(true);

    auto kerning = new QCheckBox(tr("Kerning"), editor);

    auto layout = new QVBoxLayout(editor);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(Utils::dpiScaled(4));
    layout->addWidget(fontComboBox);
    layout->addWidget(sizeSpinBox);

    auto buttonsLayout = new QHBoxLayout;
    buttonsLayout->setContentsMargins(QMargins());
    buttonsLayout->addWidget(bold);
    buttonsLayout->addWidget(italic);
    buttonsLayout->addWidget(underline);
    buttonsLayout->addWidget(strikeout);
    buttonsLayout->addStretch();

    layout->addLayout(buttonsLayout);
    layout->addWidget(kerning);

    auto syncEditor = [=] {
        const auto font = value();
        const QSignalBlocker fontBlocker(fontComboBox);
        const QSignalBlocker sizeBlocker(sizeSpinBox);
        const QSignalBlocker boldBlocker(bold);
        const QSignalBlocker italicBlocker(italic);
        const QSignalBlocker underlineBlocker(underline);
        const QSignalBlocker strikeoutBlocker(strikeout);
        const QSignalBlocker kerningBlocker(kerning);
        fontComboBox->setCurrentFont(font);
        sizeSpinBox->setValue(font.pixelSize());
        bold->setChecked(font.bold());
        italic->setChecked(font.italic());
        underline->setChecked(font.underline());
        strikeout->setChecked(font.strikeOut());
        kerning->setChecked(font.kerning());
    };

    auto syncProperty = [=] {
        auto font = fontComboBox->currentFont();
        font.setPixelSize(sizeSpinBox->value());
        font.setBold(bold->isChecked());
        font.setItalic(italic->isChecked());
        font.setUnderline(underline->isChecked());
        font.setStrikeOut(strikeout->isChecked());
        font.setKerning(kerning->isChecked());
        setValue(font);
    };

    syncEditor();

    connect(this, &Property::valueChanged, fontComboBox, syncEditor);
    connect(fontComboBox, &QFontComboBox::currentFontChanged, this, syncProperty);
    connect(sizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, syncProperty);
    connect(bold, &QAbstractButton::toggled, this, syncProperty);
    connect(italic, &QAbstractButton::toggled, this, syncProperty);
    connect(underline, &QAbstractButton::toggled, this, syncProperty);
    connect(strikeout, &QAbstractButton::toggled, this, syncProperty);
    connect(kerning, &QAbstractButton::toggled, this, syncProperty);

    return editor;
}

QWidget *QtAlignmentProperty::createEditor(QWidget *parent)
{
    auto editor = new QWidget(parent);
    auto layout = new QGridLayout(editor);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(Utils::dpiScaled(4));

    auto horizontalLabel = new ElidingLabel(tr("Horizontal"), editor);
    layout->addWidget(horizontalLabel, 0, 0);

    auto verticalLabel = new ElidingLabel(tr("Vertical"), editor);
    layout->addWidget(verticalLabel, 1, 0);

    auto horizontalComboBox = new QComboBox(editor);
    horizontalComboBox->addItem(tr("Left"), Qt::AlignLeft);
    horizontalComboBox->addItem(tr("Center"), Qt::AlignHCenter);
    horizontalComboBox->addItem(tr("Right"), Qt::AlignRight);
    horizontalComboBox->addItem(tr("Justify"), Qt::AlignJustify);
    layout->addWidget(horizontalComboBox, 0, 1);

    auto verticalComboBox = new QComboBox(editor);
    verticalComboBox->addItem(tr("Top"), Qt::AlignTop);
    verticalComboBox->addItem(tr("Center"), Qt::AlignVCenter);
    verticalComboBox->addItem(tr("Bottom"), Qt::AlignBottom);
    layout->addWidget(verticalComboBox, 1, 1);

    layout->setColumnStretch(1, 1);

    auto syncEditor = [=] {
        const QSignalBlocker horizontalBlocker(horizontalComboBox);
        const QSignalBlocker verticalBlocker(verticalComboBox);
        const auto alignment = value();
        horizontalComboBox->setCurrentIndex(horizontalComboBox->findData(static_cast<int>(alignment & Qt::AlignHorizontal_Mask)));
        verticalComboBox->setCurrentIndex(verticalComboBox->findData(static_cast<int>(alignment & Qt::AlignVertical_Mask)));
    };

    auto syncProperty = [=] {
        setValue(Qt::Alignment(horizontalComboBox->currentData().toInt() |
                               verticalComboBox->currentData().toInt()));
    };

    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(horizontalComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, syncProperty);
    connect(verticalComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, syncProperty);

    return editor;
}


VariantEditor::VariantEditor(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);

    m_layout->setContentsMargins(QMargins());
    m_layout->setSpacing(0);
}

/**
 * Removes all properties from the editor. The properties are not deleted.
 */
void VariantEditor::clear()
{
    QHashIterator<Property *, PropertyWidgets> it(m_propertyWidgets);
    while (it.hasNext()) {
        it.next();
        auto &widgets = it.value();
        Utils::deleteAllFromLayout(widgets.layout);
        delete widgets.layout;
        delete widgets.children;

        it.key()->disconnect(this);
    }
    m_propertyWidgets.clear();
}

/**
 * Adds the given property to the editor. Does not take ownership of the
 * property.
 */
void VariantEditor::addProperty(Property *property)
{
    auto &widgets = m_propertyWidgets[property];
    const auto displayMode = property->displayMode();

    connect(property, &Property::destroyed, this, [this](QObject *object) {
        removeProperty(static_cast<Property *>(object));
    });

    const auto halfSpacing = Utils::dpiScaled(2);

    widgets.layout = new QHBoxLayout;
    widgets.layout->setSpacing(halfSpacing * 2);

    if (displayMode == Property::DisplayMode::Separator) {
        auto separator = new QFrame(this);
        widgets.layout->setContentsMargins(0, halfSpacing, 0, halfSpacing);
        separator->setFrameShape(QFrame::HLine);
        separator->setFrameShadow(QFrame::Plain);
        separator->setForegroundRole(QPalette::Mid);
        widgets.layout->addWidget(separator);
        m_layout->addLayout(widgets.layout);
        return;
    }

    auto label = new PropertyLabel(m_level, this);

    if (displayMode != Property::DisplayMode::NoLabel) {
        label->setText(property->name());
        label->setToolTip(property->toolTip());
        label->setEnabled(property->isEnabled());
        label->setModified(property->isModified());
        connect(property, &Property::toolTipChanged, label, &QWidget::setToolTip);
        connect(property, &Property::enabledChanged, label, &QWidget::setEnabled);
        connect(property, &Property::modifiedChanged, label, &PropertyLabel::setModified);
    }

    if (displayMode == Property::DisplayMode::Header)
        label->setHeader(true);
    else
        widgets.layout->setContentsMargins(0, halfSpacing, halfSpacing * 2, halfSpacing);

    widgets.layout->addWidget(label, LabelStretch, Qt::AlignTop);

    QHBoxLayout *editorLayout = widgets.layout;
    const auto editor = property->createEditor(this);

    if (editor && property->actions()) {
        editorLayout = new QHBoxLayout;
        widgets.layout->addLayout(editorLayout, WidgetStretch);
    }

    if (editor) {
        editor->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        editor->setToolTip(property->toolTip());
        editor->setEnabled(property->isEnabled());
        connect(property, &Property::toolTipChanged, editor, &QWidget::setToolTip);
        connect(property, &Property::enabledChanged, editor, &QWidget::setEnabled);

        editorLayout->addWidget(editor, WidgetStretch, Qt::AlignTop);
    }

    if (property->actions()) {
        if (property->actions() & Property::Reset) {
            auto resetButton = new QToolButton;
            resetButton->setToolTip(tr("Reset"));
            resetButton->setAutoRaise(true);
            resetButton->setEnabled(property->isModified());
            Utils::setThemeIcon(resetButton, "edit-clear");
            editorLayout->addWidget(resetButton, 0, Qt::AlignTop);
            connect(resetButton, &QAbstractButton::clicked, property, &Property::resetRequested);
            connect(property, &Property::modifiedChanged, resetButton, &QWidget::setEnabled);
        }

        if (property->actions() & Property::Remove) {
            auto removeButton = new QToolButton;
            removeButton->setToolTip(tr("Remove"));
            removeButton->setAutoRaise(true);
            Utils::setThemeIcon(removeButton, "remove");
            editorLayout->addWidget(removeButton, 0, Qt::AlignTop);
            connect(removeButton, &QAbstractButton::clicked, property, &Property::removeRequested);
        }
    }

    m_layout->addLayout(widgets.layout);

    if (auto groupProperty = dynamic_cast<GroupProperty *>(property)) {
        label->setExpandable(true);
        label->setExpanded(label->isHeader());

        auto children = new VariantEditor(this);
        if (label->isHeader())
            children->setContentsMargins(0, halfSpacing, 0, halfSpacing);
        children->setLevel(m_level + 1);
        children->setVisible(label->isExpanded());
        for (auto property : groupProperty->subProperties())
            children->addProperty(property);

        connect(groupProperty, &GroupProperty::propertyAdded,
                children, &VariantEditor::addProperty);

        connect(label, &PropertyLabel::toggled, children, [=](bool expanded) {
            children->setVisible(expanded);

            // needed to avoid flickering when hiding the editor
            QWidget *widget = this;
            while (widget && widget->layout()) {
                widget->layout()->activate();
                widget = widget->parentWidget();
            }
        });

        widgets.children = children;
        m_layout->addWidget(widgets.children);
    }
}

/**
 * Removes the given property from the editor. The property is not deleted.
 */
void VariantEditor::removeProperty(Property *property)
{
    auto it = m_propertyWidgets.constFind(property);
    Q_ASSERT(it != m_propertyWidgets.constEnd());

    if (it != m_propertyWidgets.end()) {
        auto &widgets = it.value();
        Utils::deleteAllFromLayout(widgets.layout);
        delete widgets.layout;
        delete widgets.children;

        m_propertyWidgets.erase(it);
    }

    property->disconnect(this);
}

void VariantEditor::setLevel(int level)
{
    m_level = level;

    setBackgroundRole((m_level % 2) ? QPalette::AlternateBase
                                    : QPalette::Base);
    setAutoFillBackground(m_level > 1);
}


QWidget *BaseEnumProperty::createEnumEditor(QWidget *parent)
{
    auto editor = new QComboBox(parent);
    // This allows the combo box to shrink horizontally.
    editor->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

    for (qsizetype i = 0; i < m_enumData.names.size(); ++i) {
        auto value = m_enumData.values.value(i, i);
        editor->addItem(m_enumData.icons[value],
                        m_enumData.names[i],
                        value);
    }

    auto syncEditor = [this, editor] {
        const QSignalBlocker blocker(editor);
        editor->setCurrentIndex(editor->findData(value()));
    };
    syncEditor();

    QObject::connect(this, &Property::valueChanged, editor, syncEditor);
    QObject::connect(editor, qOverload<int>(&QComboBox::currentIndexChanged), this,
                     [editor, this] {
        setValue(editor->currentData().toInt());
    });

    return editor;
}

QWidget *BaseEnumProperty::createFlagsEditor(QWidget *parent)
{
    auto editor = new QWidget(parent);
    auto layout = new QVBoxLayout(editor);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    for (qsizetype i = 0; i < m_enumData.names.size(); ++i) {
        auto checkBox = new QCheckBox(m_enumData.names[i], editor);
        layout->addWidget(checkBox);

        QObject::connect(checkBox, &QCheckBox::toggled, this, [=](bool checked) {
            const auto enumItemValue = m_enumData.values.value(i, 1 << i);
            int flags = value();
            if (checked)
                flags |= enumItemValue;
            else
                flags &= ~enumItemValue;
            setValue(flags);
        });
    }

    auto syncEditor = [=] {
        for (int i = 0; i < layout->count(); ++i) {
            auto checkBox = qobject_cast<QCheckBox *>(layout->itemAt(i)->widget());
            if (checkBox) {
                const auto enumItemValue = m_enumData.values.value(i, 1 << i);

                QSignalBlocker blocker(checkBox);
                checkBox->setChecked((value() & enumItemValue) == enumItemValue);
            }
        }
    };

    syncEditor();

    QObject::connect(this, &Property::valueChanged, editor, syncEditor);

    return editor;
}

/**
 * Creates a property that wraps a QObject property.
 */
Property *createQObjectProperty(QObject *qObject,
                                const char *propertyName,
                                const QString &displayName)
{
    auto metaObject = qObject->metaObject();
    auto propertyIndex = metaObject->indexOfProperty(propertyName);
    if (propertyIndex < 0)
        return nullptr;

    auto metaProperty = metaObject->property(propertyIndex);
    auto property = createVariantProperty(
                displayName.isEmpty() ? QString::fromUtf8(propertyName)
                                      : displayName,
                [=] {
                    return metaProperty.read(qObject);
                },
                [=] (const QVariant &value) {
                    metaProperty.write(qObject, value);
                });

    // If the property has a notify signal, forward it to valueChanged
    auto notify = metaProperty.notifySignal();
    if (notify.isValid()) {
        auto propertyMetaObject = property->metaObject();
        auto valuePropertyIndex = propertyMetaObject->indexOfProperty("value");
        auto valueProperty = propertyMetaObject->property(valuePropertyIndex);
        auto valueChanged = valueProperty.notifySignal();

        QObject::connect(qObject, notify, property, valueChanged);
    }

    property->setEnabled(metaProperty.isWritable());

    return property;
}

template<typename PropertyClass>
Property *createTypedProperty(const QString &name,
                              std::function<QVariant ()> get,
                              std::function<void (const QVariant &)> set)
{
    return new PropertyClass(name,
                             [get = std::move(get)] { return get().value<typename PropertyClass::ValueType>(); },
                             [set = std::move(set)] (const typename PropertyClass::ValueType &v) { set(QVariant::fromValue(v)); });
}

/**
 * Creates a property with the given name and get/set functions. The
 * value type determines the kind of property that will be created.
 */
Property *createVariantProperty(const QString &name,
                                std::function<QVariant ()> get,
                                std::function<void (const QVariant &)> set)
{
    const auto type = get().userType();
    switch (type) {
    case QMetaType::QString:
        return createTypedProperty<MultilineStringProperty>(name, get, set);
    case QMetaType::QUrl:
        return createTypedProperty<UrlProperty>(name, get, set);
    case QMetaType::Int:
        return createTypedProperty<IntProperty>(name, get, set);
    case QMetaType::Double:
        return createTypedProperty<FloatProperty>(name, get, set);
    case QMetaType::Bool:
        return createTypedProperty<BoolProperty>(name, get, set);
    case QMetaType::QColor:
        return createTypedProperty<ColorProperty>(name, get, set);
    case QMetaType::QFont:
        return createTypedProperty<FontProperty>(name, get, set);
    case QMetaType::QPoint:
        return createTypedProperty<PointProperty>(name, get, set);
    case QMetaType::QPointF:
        return createTypedProperty<PointFProperty>(name, get, set);
    case QMetaType::QRect:
        return createTypedProperty<RectProperty>(name, get, set);
    case QMetaType::QRectF:
        return createTypedProperty<RectFProperty>(name, get, set);
    case QMetaType::QSize:
        return createTypedProperty<SizeProperty>(name, get, set);
    case QMetaType::QSizeF:
        return createTypedProperty<SizeFProperty>(name, get, set);
    default:
        if (type == qMetaTypeId<Qt::Alignment>())
            return createTypedProperty<QtAlignmentProperty>(name, get, set);
    }

    return nullptr;
}

} // namespace Tiled

#include "moc_varianteditor.cpp"
