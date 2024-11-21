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
#include <QFontComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMetaProperty>
#include <QPainter>
#include <QPointer>
#include <QResizeEvent>
#include <QSpacerItem>
#include <QSpinBox>
#include <QToolButton>

namespace Tiled {

void Property::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged(name);
    }
}

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

void Property::setActions(Actions actions)
{
    if (m_actions != actions) {
        m_actions = actions;
        emit actionsChanged(actions);
    }
}

QWidget *Property::createLabel(int level, QWidget *parent)
{
    auto label = new PropertyLabel(parent);
    label->setLevel(level);

    connect(label, &PropertyLabel::contextMenuRequested,
            this, &Property::contextMenuRequested);

    if (displayMode() != Property::DisplayMode::NoLabel) {
        label->setText(name());
        label->setModified(isModified());
        label->setToolTip(toolTip());
        connect(this, &Property::nameChanged, label, &PropertyLabel::setText);
        connect(this, &Property::toolTipChanged, label, &PropertyLabel::setToolTip);
        connect(this, &Property::modifiedChanged, label, &PropertyLabel::setModified);
    }

    if (displayMode() == Property::DisplayMode::Header)
        label->setHeader(true);

    return label;
}

Property::DisplayMode GroupProperty::displayMode() const
{
    if (name().isEmpty())
        return DisplayMode::ChildrenOnly;

    if (m_header)
        return DisplayMode::Header;

    return DisplayMode::Default;
}

QWidget *GroupProperty::createLabel(int level, QWidget *parent)
{
    auto label = static_cast<PropertyLabel*>(Property::createLabel(level, parent));
    label->setExpandable(true);
    label->setExpanded(isExpanded());

    connect(this, &GroupProperty::expandedChanged, label, &PropertyLabel::setExpanded);
    connect(label, &PropertyLabel::toggled, this, &GroupProperty::setExpanded);

    return label;
}

void GroupProperty::setExpanded(bool expanded)
{
    if (m_expanded != expanded) {
        m_expanded = expanded;
        emit expandedChanged(expanded);
    }
}

void GroupProperty::expandAll()
{
    setExpanded(true);
    for (auto property : std::as_const(m_subProperties))
        if (auto groupProperty = qobject_cast<GroupProperty *>(property))
            groupProperty->expandAll();
}

void GroupProperty::collapseAll()
{
    setExpanded(false);
    for (auto property : std::as_const(m_subProperties))
        if (auto groupProperty = qobject_cast<GroupProperty *>(property))
            groupProperty->collapseAll();
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
    auto editor = new LineEdit(parent);
    editor->setPlaceholderText(m_placeholderText);

    auto syncEditor = [=] {
        // Avoid affecting cursor position when the text is the same
        const QString v = value();
        if (editor->text() != v)
            editor->setText(v);
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
    editor->setIsDirectory(m_isDirectory);

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
        auto slider = new Slider(Qt::Horizontal, widget);
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

Property::DisplayMode BoolProperty::displayMode() const
{
    return m_nameOnCheckBox ? DisplayMode::NoLabel : DisplayMode::Default;
}

QWidget *BoolProperty::createEditor(QWidget *parent)
{
    auto editor = new QCheckBox(name(), parent);
    auto syncEditor = [=] {
        const QSignalBlocker blocker(editor);
        bool checked = value();
        editor->setChecked(checked);
        if (!m_nameOnCheckBox)
            editor->setText(checked ? tr("On") : tr("Off"));

        // Reflect modified state on the checkbox, since we're not showing the
        // property label.
        auto font = editor->font();
        font.setBold(isModified());
        editor->setFont(font);
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(this, &Property::modifiedChanged, editor, syncEditor);
    connect(editor, &QCheckBox::toggled, this, [=](bool checked) {
        if (!m_nameOnCheckBox)
            editor->setText(checked ? QObject::tr("On") : QObject::tr("Off"));
        setValue(checked);
    });

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

QWidget *ColorProperty::createEditor(QWidget *parent)
{
    auto editor = new QWidget(parent);
    auto layout = new QHBoxLayout(editor);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);

    auto colorButton = new ColorButton(editor);
    colorButton->setShowAlphaChannel(m_alpha);
    colorButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QIcon resetIcon(QStringLiteral(":/images/16/edit-clear.png"));
    resetIcon.addFile(QStringLiteral(":/images/24/edit-clear.png"));

    auto resetButton = new QToolButton(editor);
    resetButton->setIcon(resetIcon);
    resetButton->setToolTip(tr("Clear Color"));
    Utils::setThemeIcon(resetButton, "edit-clear");

    layout->addWidget(colorButton);
    layout->addWidget(resetButton);

    auto syncEditor = [=] {
        const QSignalBlocker blocker(colorButton);
        auto v = value();
        colorButton->setColor(v);
        resetButton->setEnabled(v.isValid());
    };
    syncEditor();

    connect(resetButton, &QToolButton::clicked, colorButton, [colorButton] {
        colorButton->setColor(QColor());
    });
    connect(this, &Property::valueChanged, colorButton, syncEditor);
    connect(colorButton, &ColorButton::colorChanged, this, [=] {
        resetButton->setEnabled(colorButton->color().isValid());
        setValue(colorButton->color());
    });

    editor->setFocusProxy(colorButton);

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

    auto horizontalComboBox = new ComboBox(editor);
    horizontalComboBox->addItem(tr("Left"), Qt::AlignLeft);
    horizontalComboBox->addItem(tr("Center"), Qt::AlignHCenter);
    horizontalComboBox->addItem(tr("Right"), Qt::AlignRight);
    horizontalComboBox->addItem(tr("Justify"), Qt::AlignJustify);
    layout->addWidget(horizontalComboBox, 0, 1);

    auto verticalComboBox = new ComboBox(editor);
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
    , m_resetIcon(QIcon(QStringLiteral(":/images/16/edit-clear.png")))
    , m_removeIcon(QIcon(QStringLiteral(":/images/16/remove.png")))
    , m_addIcon(QIcon(QStringLiteral(":/images/16/add.png")))
{
    m_resetIcon.addFile(QStringLiteral(":/images/24/edit-clear.png"));
    m_removeIcon.addFile(QStringLiteral(":/images/22/remove.png"));
    m_addIcon.addFile(QStringLiteral(":/images/22/add.png"));

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
        delete it.value().rowWidget;

        it.key()->disconnect(this);
    }
    m_propertyWidgets.clear();
}

/**
 * Adds the given property.
 * Does not take ownership of the property.
 */
void VariantEditor::addProperty(Property *property)
{
    m_layout->addWidget(createPropertyWidget(property));
}

/**
 * Insert the given property at the given index.
 * Does not take ownership of the property.
 */
void VariantEditor::insertProperty(int index, Property *property)
{
    m_layout->insertWidget(index, createPropertyWidget(property));
}

/**
 * Removes the given property from the editor. The property is not deleted.
 */
void VariantEditor::removeProperty(Property *property)
{
    auto it = m_propertyWidgets.constFind(property);
    Q_ASSERT(it != m_propertyWidgets.constEnd());

    if (it == m_propertyWidgets.constEnd())
        return;

    // Immediately remove from layout, but delete later to avoid deleting
    // widgets while they are still handling events.
    auto rowWidget = it.value().rowWidget;
    m_layout->removeWidget(rowWidget);
    rowWidget->deleteLater();

    m_propertyWidgets.erase(it);

    // This appears to be necessary to avoid flickering due to relayouting
    // not being done before the next paint.
    QWidget *widget = this;
    while (widget && widget->layout()) {
        widget->layout()->activate();
        widget = widget->parentWidget();
    }

    property->disconnect(this);
}

/**
 * Focuses the editor or label for the given property. Makes sure any parent
 * group properties are expanded.
 *
 * When the given property is a group property, the group property is expanded
 * and the first child property is focused.
 *
 * Returns the focused widget or nullptr if the property was not found.
 */
QWidget *VariantEditor::focusProperty(Property *property, FocusTarget target)
{
    for (auto it = m_propertyWidgets.constBegin(); it != m_propertyWidgets.constEnd(); ++it) {
        auto &widgets = it.value();

        if (it.key() == property) {
            if (target == FocusEditor && widgets.editor) {
                widgets.editor->setFocus();
                return widgets.editor;
            }
            if (target == FocusLabel && widgets.label) {
                widgets.label->setFocus();
                return widgets.label;
            }
            if (auto groupProperty = qobject_cast<GroupProperty *>(it.key())) {
                groupProperty->setExpanded(true);
                if (widgets.children && !groupProperty->subProperties().isEmpty())
                    widgets.children->focusProperty(groupProperty->subProperties().first(), target);
                return widgets.children;
            }
            return nullptr;
        } else if (auto groupProperty = qobject_cast<GroupProperty *>(it.key())) {
            if (widgets.children) {
                if (auto w = widgets.children->focusProperty(property, target)) {
                    groupProperty->setExpanded(true);
                    return w;
                }
            }
        }
    }

    return nullptr;
}

void VariantEditor::setLevel(int level)
{
    m_level = level;

    setBackgroundRole((m_level % 2) ? QPalette::Base
                                    : QPalette::AlternateBase);
    setAutoFillBackground(m_level > 0);
}

QWidget *VariantEditor::createPropertyWidget(Property *property)
{
    Q_ASSERT(!m_propertyWidgets.contains(property));

    auto &widgets = m_propertyWidgets[property];
    const auto displayMode = property->displayMode();

    connect(property, &Property::destroyed, this, [this](QObject *object) {
        removeProperty(static_cast<Property *>(object));
    });

    if (displayMode == Property::DisplayMode::ChildrenOnly) {
        auto editor = new VariantEditor(this);
        editor->setEnabled(property->isEnabled());

        connect(property, &Property::enabledChanged,
                editor, &QWidget::setEnabled);

        if (auto groupProperty = qobject_cast<GroupProperty *>(property)) {
            for (auto property : groupProperty->subProperties())
                editor->addProperty(property);

            connect(groupProperty, &GroupProperty::propertyAdded,
                    editor, &VariantEditor::insertProperty);
        }

        widgets.rowWidget = editor;
        return widgets.rowWidget;
    }

    const auto halfSpacing = Utils::dpiScaled(2);

    auto rowWidget = new QWidget(this);
    auto rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setSpacing(halfSpacing * 2);
    rowLayout->setContentsMargins(0, 0, 0, 0);

    widgets.rowWidget = rowWidget;

    if (displayMode == Property::DisplayMode::Separator) {
        auto separator = new QFrame(rowWidget);
        rowLayout->setContentsMargins(0, halfSpacing, 0, halfSpacing);
        separator->setFrameShape(QFrame::HLine);
        separator->setFrameShadow(QFrame::Plain);
        separator->setForegroundRole(QPalette::Mid);
        rowLayout->addWidget(separator);
        return widgets.rowWidget;
    }

    widgets.label = property->createLabel(m_level, rowWidget);

    if (displayMode != Property::DisplayMode::Header) {
        if (isLeftToRight())
            rowLayout->setContentsMargins(0, halfSpacing, halfSpacing * 2, halfSpacing);
        else
            rowLayout->setContentsMargins(halfSpacing * 2, halfSpacing, 0, halfSpacing);
    }

    if (widgets.label)
        rowLayout->addWidget(widgets.label, LabelStretch, Qt::AlignTop);

    auto editorLayout = new QHBoxLayout;
    widgets.editor = property->createEditor(rowWidget);

    if (widgets.editor) {
        widgets.editor->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        editorLayout->addWidget(widgets.editor, EditorStretch, Qt::AlignTop);
        rowLayout->addLayout(editorLayout, EditorStretch);
    } else {
        rowLayout->addLayout(editorLayout, 0);
    }

    widgets.resetButton = new QToolButton(rowWidget);
    widgets.resetButton->setToolTip(tr("Reset"));
    widgets.resetButton->setIcon(m_resetIcon);
    widgets.resetButton->setAutoRaise(true);
    widgets.resetButton->setEnabled(property->isModified());
    Utils::setThemeIcon(widgets.resetButton, "edit-clear");
    editorLayout->addWidget(widgets.resetButton, 0, Qt::AlignTop);
    connect(widgets.resetButton, &QAbstractButton::clicked, property, &Property::resetRequested);
    connect(property, &Property::modifiedChanged, widgets.resetButton, &QWidget::setEnabled);

    widgets.removeButton = new QToolButton(rowWidget);
    widgets.removeButton->setToolTip(tr("Remove"));
    widgets.removeButton->setIcon(m_removeIcon);
    widgets.removeButton->setAutoRaise(true);
    Utils::setThemeIcon(widgets.removeButton, "remove");
    editorLayout->addWidget(widgets.removeButton, 0, Qt::AlignTop);
    connect(widgets.removeButton, &QAbstractButton::clicked, property, &Property::removeRequested);

    widgets.addButton = new QToolButton(rowWidget);
    widgets.addButton->setToolTip(tr("Add"));
    widgets.addButton->setIcon(m_addIcon);
    widgets.addButton->setAutoRaise(true);
    widgets.addButton->setFocusPolicy(Qt::StrongFocus); // needed for AddValueProperty
    Utils::setThemeIcon(widgets.addButton, "add");
    editorLayout->addWidget(widgets.addButton, 0, Qt::AlignTop);
    connect(widgets.addButton, &QAbstractButton::clicked, property, &Property::addRequested);

    if (auto groupProperty = qobject_cast<GroupProperty *>(property)) {
        auto containerWidget = new QWidget(this);
        widgets.childrenLayout = new QVBoxLayout(containerWidget);
        widgets.childrenLayout->setContentsMargins(0, 0, 0, 0);
        widgets.childrenLayout->setSpacing(0);
        widgets.childrenLayout->addWidget(rowWidget);

        connect(groupProperty, &GroupProperty::expandedChanged, this, [=](bool expanded) {
            setPropertyChildrenExpanded(groupProperty, expanded);
        });

        setPropertyChildrenExpanded(groupProperty, groupProperty->isExpanded());

        widgets.rowWidget = containerWidget;
    }

    updatePropertyEnabled(widgets, property->isEnabled());
    updatePropertyActions(widgets, property->actions());

    connect(property, &Property::enabledChanged, this, [=] (bool enabled) {
        updatePropertyEnabled(m_propertyWidgets[property], enabled);
    });
    connect(property, &Property::actionsChanged, this, [=] (Property::Actions actions) {
        updatePropertyActions(m_propertyWidgets[property], actions);
    });

    return widgets.rowWidget;
}

void VariantEditor::setPropertyChildrenExpanded(GroupProperty *groupProperty, bool expanded)
{
    auto &widgets = m_propertyWidgets[groupProperty];

    // Create the children editor on-demand
    if (expanded && !widgets.children) {
        const auto halfSpacing = Utils::dpiScaled(2);
        const auto displayMode = groupProperty->displayMode();

        widgets.children = new VariantEditor(widgets.childrenLayout->parentWidget());
        if (widgets.label && displayMode == Property::DisplayMode::Header)
            widgets.children->setContentsMargins(0, halfSpacing, 0, halfSpacing);
        if (displayMode == Property::DisplayMode::Default)
            widgets.children->setLevel(m_level + 1);
        widgets.children->setEnabled(groupProperty->isEnabled());
        for (auto property : groupProperty->subProperties())
            widgets.children->addProperty(property);

        connect(groupProperty, &GroupProperty::propertyAdded,
                widgets.children, &VariantEditor::insertProperty);

        widgets.childrenLayout->addWidget(widgets.children);
    }

    if (widgets.children) {
        widgets.children->setVisible(expanded);

        // needed to avoid flickering when hiding the editor
        if (!expanded) {
            QWidget *widget = widgets.childrenLayout->parentWidget();
            while (widget && widget->layout()) {
                widget->layout()->activate();
                widget = widget->parentWidget();
            }
        }
    }
}

void VariantEditor::updatePropertyEnabled(const PropertyWidgets &widgets, bool enabled)
{
    if (widgets.label)
        widgets.label->setEnabled(enabled);
    if (widgets.editor)
        widgets.editor->setEnabled(enabled);
    if (widgets.children)
        widgets.children->setEnabled(enabled);
}

void VariantEditor::updatePropertyActions(const PropertyWidgets &widgets,
                                          Property::Actions actions)
{
    widgets.resetButton->setVisible(actions.testFlag(Property::Action::Reset));
    widgets.removeButton->setVisible(actions.testFlag(Property::Action::Remove));
    widgets.addButton->setVisible(actions.testFlag(Property::Action::Add));

    widgets.addButton->setEnabled(!actions.testFlag(Property::Action::AddDisabled));
}


QWidget *BaseEnumProperty::createEnumEditor(QWidget *parent)
{
    auto editor = new ComboBox(parent);

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

VariantEditorView::VariantEditorView(QWidget *parent)
    : QScrollArea(parent)
{
    auto scrollWidget = new QWidget(this);
    scrollWidget->setBackgroundRole(QPalette::AlternateBase);
    scrollWidget->setMinimumWidth(Utils::dpiScaled(120));

    auto verticalLayout = new QVBoxLayout(scrollWidget);
    m_editor = new VariantEditor(scrollWidget);
    verticalLayout->addWidget(m_editor);
    verticalLayout->addStretch();
    verticalLayout->setContentsMargins(QMargins());

    setWidgetResizable(true);
    setWidget(scrollWidget);
}

void VariantEditorView::focusProperty(Property *property,
                                      VariantEditor::FocusTarget target)
{
    if (auto widget = m_editor->focusProperty(property, target)) {
        if (widget->isVisible()) {
            ensureWidgetVisible(widget);
        } else {
            // Install event filter to detect when widget becomes visible
            widget->installEventFilter(this);
        }
    }
}

bool VariantEditorView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Show) {
        if (QPointer<QWidget> widget = qobject_cast<QWidget*>(watched)) {
            // Schedule after all pending events including layout
            QMetaObject::invokeMethod(this, [=] {
                if (widget)
                    ensureWidgetVisible(widget);
            }, Qt::QueuedConnection);
            widget->removeEventFilter(this);
        }
    }
    return QScrollArea::eventFilter(watched, event);
}

} // namespace Tiled

#include "moc_varianteditor.cpp"
