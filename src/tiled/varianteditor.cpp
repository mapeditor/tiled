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

QWidget *GroupProperty::createEditor(QWidget *parent)
{
    auto widget = new VariantEditor(parent);
    for (auto property : std::as_const(m_subProperties))
        widget->addProperty(property);
    return widget;
}

QWidget *StringProperty::createEditor(QWidget *parent)
{
    auto editor = new QLineEdit(parent);
    auto syncEditor = [=] {
        editor->setText(value());
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(editor, &QLineEdit::textEdited, this, &StringProperty::setValue);

    return editor;
}

QWidget *MultilineStringProperty::createEditor(QWidget *parent)
{
    auto editor = new TextPropertyEdit(parent);
    auto syncEditor = [=] {
        const QSignalBlocker blocker(editor);
        editor->setText(value());
    };
    syncEditor();

    connect(this, &StringProperty::valueChanged, editor, syncEditor);
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
    auto editor = new SpinBox(parent);
    editor->setRange(m_minimum, m_maximum);
    editor->setSingleStep(m_singleStep);
    editor->setSuffix(m_suffix);

    auto syncEditor = [=] {
        const QSignalBlocker blocker(editor);
        editor->setValue(value());
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(editor, qOverload<int>(&SpinBox::valueChanged),
            this, &IntProperty::setValue);

    return editor;
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
    auto editor = new QCheckBox(parent);
    auto syncEditor = [=] {
        const QSignalBlocker blocker(editor);
        bool checked = value();
        editor->setChecked(checked);
        editor->setText(checked ? tr("On") : tr("Off"));
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(editor, &QCheckBox::toggled, this, [=](bool checked) {
        editor->setText(checked ? QObject::tr("On") : QObject::tr("Off"));
        setValue(checked);
    });

    return editor;
}

QWidget *PointProperty::createEditor(QWidget *parent)
{
    auto editor = new PointEdit(parent);
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
    layout->setSpacing(Utils::dpiScaled(3));
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
    layout->setSpacing(Utils::dpiScaled(3));

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
    m_gridLayout = new QGridLayout(this);

    m_gridLayout->setContentsMargins(0, 0, 0, Utils::dpiScaled(3));
    m_gridLayout->setSpacing(Utils::dpiScaled(3));

    m_gridLayout->setColumnStretch(LabelColumn, 2);
    m_gridLayout->setColumnStretch(WidgetColumn, 3);
    m_gridLayout->setColumnMinimumWidth(LeftSpacing, Utils::dpiScaled(3));
    m_gridLayout->setColumnMinimumWidth(MiddleSpacing, Utils::dpiScaled(2));
    m_gridLayout->setColumnMinimumWidth(RightSpacing, Utils::dpiScaled(3));

    // setValue(QVariantMap {
    //              { QStringLiteral("Name"), QVariant(QLatin1String("Hello")) },
    //              { QStringLiteral("Position"), QVariant(QPoint(15, 50)) },
    //              { QStringLiteral("Size"), QVariant(QSize(35, 400)) },
    //              { QStringLiteral("Rectangle"), QVariant(QRectF(15, 50, 35, 400)) },
    //              { QStringLiteral("Margin"), QVariant(10) },
    //              { QStringLiteral("Opacity"), QVariant(0.5) },
    //              { QStringLiteral("Visible"), true },
    //              { QStringLiteral("Object Alignment"), QVariant::fromValue(TopLeft) },
    //          });


    // setValue(QVariantList {
    //              QVariant(QLatin1String("Hello")),
    //              QVariant(10),
    //              QVariant(3.14)
    //          });
}

void VariantEditor::clear()
{
    QLayoutItem *item;
    while ((item = m_gridLayout->takeAt(0))) {
        delete item->widget();
        delete item;
    }
    m_rowIndex = 0;
}

HeaderWidget *VariantEditor::addHeader(const QString &text)
{
    auto headerWidget = new HeaderWidget(text, this);

    m_gridLayout->addWidget(headerWidget, m_rowIndex, 0, 1, ColumnCount);

    ++m_rowIndex;

    return headerWidget;
}

void VariantEditor::addSeparator()
{
    auto separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setForegroundRole(QPalette::Mid);
    m_gridLayout->addWidget(separator, m_rowIndex, 0, 1, ColumnCount);
    ++m_rowIndex;
}

void VariantEditor::addProperty(Property *property)
{
    switch (property->displayMode()) {
    case Property::DisplayMode::Default: {
        auto label = new LineEditLabel(property->name(), this);
        label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        label->setToolTip(property->toolTip());
        label->setEnabled(property->isEnabled());
        connect(property, &Property::toolTipChanged, label, &QWidget::setToolTip);
        connect(property, &Property::enabledChanged, label, &QLabel::setEnabled);
        m_gridLayout->addWidget(label, m_rowIndex, LabelColumn, Qt::AlignTop/* | Qt::AlignRight*/);

        if (auto editor = createEditor(property)) {
            editor->setToolTip(property->toolTip());
            editor->setEnabled(property->isEnabled());
            connect(property, &Property::toolTipChanged, editor, &QWidget::setToolTip);
            connect(property, &Property::enabledChanged, editor, &QWidget::setEnabled);
            m_gridLayout->addWidget(editor, m_rowIndex, WidgetColumn);
        }

        ++m_rowIndex;
        break;
    }
    case Property::DisplayMode::Header: {
        auto headerWidget = addHeader(property->name());

        if (auto editor = createEditor(property)) {
            connect(headerWidget, &HeaderWidget::toggled,
                    editor, [this, editor](bool checked) {
                editor->setVisible(checked);

                // needed to avoid flickering when hiding the editor
                layout()->activate();
            });

            m_gridLayout->addWidget(editor, m_rowIndex, 0, 1, ColumnCount);
            ++m_rowIndex;
        }

        break;
    }
    case Property::DisplayMode::Separator:
        addSeparator();
        break;
    }
}

#if 0
void VariantEditor::addValue(const QVariant &value)
{
    const int type = value.userType();
    switch (type) {
    case QMetaType::QVariantList: {
        const auto list = value.toList();
        for (const auto &item : list)
            addValue(item);
        break;
    }
    case QMetaType::QVariantMap: {
        const auto map = value.toMap();
        for (auto it = map.constBegin(); it != map.constEnd(); ++it)
            addValue(it.key(), it.value());
        break;
    }
    default: {
        if (auto editor = createEditor(value))
            m_gridLayout->addWidget(editor, m_rowIndex, LabelColumn, 1, 3);
        else
            qDebug() << "No editor factory for type" << type;

        ++m_rowIndex;
    }
    }
}
#endif

QWidget *VariantEditor::createEditor(Property *property)
{
    if (const auto editor = property->createEditor(this)) {
        editor->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        return editor;
    } else {
        qDebug() << "No editor for property" << property->name();
    }
    return nullptr;
}


QWidget *createEnumEditor(IntProperty *property, const EnumData &enumData, QWidget *parent)
{
    auto editor = new QComboBox(parent);
    // This allows the combo box to shrink horizontally.
    editor->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

    for (qsizetype i = 0; i < enumData.names.size(); ++i) {
        auto value = enumData.values.isEmpty() ? i : enumData.values.value(i);
        editor->addItem(enumData.icons[value],
                        enumData.names[i],
                        value);
    }

    auto syncEditor = [property, editor] {
        const QSignalBlocker blocker(editor);
        editor->setCurrentIndex(editor->findData(property->value()));
    };
    syncEditor();

    QObject::connect(property, &Property::valueChanged, editor, syncEditor);
    QObject::connect(editor, qOverload<int>(&QComboBox::currentIndexChanged), property,
                     [editor, property] {
        property->setValue(editor->currentData().toInt());
    });

    return editor;
}

Property *PropertyFactory::createQObjectProperty(QObject *qObject,
                                                 const char *propertyName,
                                                 const QString &displayName)
{
    auto metaObject = qObject->metaObject();
    auto propertyIndex = metaObject->indexOfProperty(propertyName);
    if (propertyIndex < 0)
        return nullptr;

    auto metaProperty = metaObject->property(propertyIndex);
    auto property = createProperty(displayName.isEmpty() ? QString::fromUtf8(propertyName)
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
                             [set = std::move(set)] (typename PropertyClass::ValueType v) { set(QVariant::fromValue(v)); });
}

Property *PropertyFactory::createProperty(const QString &name,
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
