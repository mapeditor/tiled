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
#include "compression.h"
#include "map.h"
#include "utils.h"
#include "propertyeditorwidgets.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QResizeEvent>
#include <QSpacerItem>
#include <QSpinBox>
#include <QStringListModel>

namespace Tiled {

AbstractProperty::AbstractProperty(const QString &name,
                                   EditorFactory *editorFactory,
                                   QObject *parent)
    : Property(name, parent)
    , m_editorFactory(editorFactory)
{}

QWidget *AbstractProperty::createEditor(QWidget *parent)
{
    return m_editorFactory->createEditor(this, parent);
}


GetSetProperty::GetSetProperty(const QString &name,
                               std::function<QVariant ()> get,
                               std::function<void (const QVariant &)> set,
                               EditorFactory *editorFactory,
                               QObject *parent)
    : AbstractProperty(name, editorFactory, parent)
    , m_get(std::move(get))
    , m_set(std::move(set))
{}



class StringEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(Property *property, QWidget *parent) override
    {
        auto value = property->value();
        auto editor = new QLineEdit(parent);
        editor->setText(value.toString());
        return editor;
    }
};

class IntEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(Property *property, QWidget *parent) override
    {
        auto value = property->value();
        auto editor = new SpinBox(parent);
        editor->setValue(value.toInt());
        return editor;
    }
};

class FloatEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(Property *property, QWidget *parent) override
    {
        auto value = property->value();
        auto editor = new DoubleSpinBox(parent);
        editor->setValue(value.toDouble());
        return editor;
    }
};

class BoolEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(Property *property, QWidget *parent) override
    {
        auto editor = new QCheckBox(parent);
        auto syncEditor = [=]() {
            const QSignalBlocker blocker(editor);
            bool checked = property->value().toBool();
            editor->setChecked(checked);
            editor->setText(checked ? tr("On") : tr("Off"));
        };
        syncEditor();

        QObject::connect(property, &Property::valueChanged, editor, syncEditor);
        QObject::connect(editor, &QCheckBox::toggled, property, [=](bool checked) {
            editor->setText(checked ? QObject::tr("On") : QObject::tr("Off"));
            property->setValue(checked);
        });

        return editor;
    }
};

class PointEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(Property *property, QWidget *parent) override
    {
        auto value = property->value();
        auto editor = new QWidget(parent);
        auto horizontalLayout = new QHBoxLayout(editor);
        horizontalLayout->setContentsMargins(QMargins());

        auto xLabel = new QLabel(QStringLiteral("X"), editor);
        horizontalLayout->addWidget(xLabel, 0, Qt::AlignRight);

        auto xSpinBox = new SpinBox(editor);
        xLabel->setBuddy(xSpinBox);
        horizontalLayout->addWidget(xSpinBox, 1);

        auto yLabel = new QLabel(QStringLiteral("Y"), editor);
        horizontalLayout->addWidget(yLabel, 0, Qt::AlignRight);

        auto ySpinBox = new SpinBox(editor);
        yLabel->setBuddy(ySpinBox);
        horizontalLayout->addWidget(ySpinBox, 1);

        xSpinBox->setValue(value.toPoint().x());
        ySpinBox->setValue(value.toPoint().y());

        return editor;
    }
};

class PointFEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(Property *property, QWidget *parent) override
    {
        auto value = property->value();
        auto editor = new QWidget(parent);
        auto horizontalLayout = new QHBoxLayout(editor);
        horizontalLayout->setContentsMargins(QMargins());

        auto xLabel = new QLabel(QStringLiteral("X"), editor);
        horizontalLayout->addWidget(xLabel, 0, Qt::AlignRight);

        auto xSpinBox = new DoubleSpinBox(editor);
        xLabel->setBuddy(xSpinBox);
        horizontalLayout->addWidget(xSpinBox, 1);

        auto yLabel = new QLabel(QStringLiteral("Y"), editor);
        horizontalLayout->addWidget(yLabel, 0, Qt::AlignRight);

        auto ySpinBox = new DoubleSpinBox(editor);
        yLabel->setBuddy(ySpinBox);
        horizontalLayout->addWidget(ySpinBox, 1);

        xSpinBox->setValue(value.toPointF().x());
        ySpinBox->setValue(value.toPointF().y());

        return editor;
    }
};


class SizeEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(Property *property, QWidget *parent) override
    {
        auto editor = new SizeEdit(parent);
        auto syncEditor = [property, editor]() {
            const QSignalBlocker blocker(editor);
            editor->setValue(property->value().toSize());
        };
        syncEditor();

        QObject::connect(property, &Property::valueChanged, editor, syncEditor);
        QObject::connect(editor, &SizeEdit::valueChanged, property,
                         [property, editor]() {
            property->setValue(editor->value());
        });

        return editor;
    }
};

class RectFEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(Property *property, QWidget *parent) override
    {
        auto value = property->value();
        auto editor = new QWidget(parent);
        auto gridLayout = new QGridLayout(editor);
        gridLayout->setContentsMargins(QMargins());
        gridLayout->setColumnStretch(4, 1);

        auto xLabel = new QLabel(QStringLiteral("X"), editor);
        gridLayout->addWidget(xLabel, 0, 0, Qt::AlignRight);

        auto xSpinBox = new DoubleSpinBox(editor);
        xLabel->setBuddy(xSpinBox);
        gridLayout->addWidget(xSpinBox, 0, 1);

        auto yLabel = new QLabel(QStringLiteral("Y"), editor);
        gridLayout->addWidget(yLabel, 0, 2, Qt::AlignRight);

        auto ySpinBox = new DoubleSpinBox(editor);
        yLabel->setBuddy(ySpinBox);
        gridLayout->addWidget(ySpinBox, 0, 3);

        auto widthLabel = new QLabel(QStringLiteral("W"), editor);
        widthLabel->setToolTip(tr("Width"));
        gridLayout->addWidget(widthLabel, 1, 0, Qt::AlignRight);

        auto widthSpinBox = new DoubleSpinBox(editor);
        widthLabel->setBuddy(widthSpinBox);
        gridLayout->addWidget(widthSpinBox, 1, 1);

        auto heightLabel = new QLabel(QStringLiteral("H"), editor);
        heightLabel->setToolTip(tr("Height"));
        gridLayout->addWidget(heightLabel, 1, 2, Qt::AlignRight);

        auto heightSpinBox = new DoubleSpinBox(editor);
        heightLabel->setBuddy(heightSpinBox);
        gridLayout->addWidget(heightSpinBox, 1, 3);

        const auto rect = value.toRectF();
        xSpinBox->setValue(rect.x());
        ySpinBox->setValue(rect.y());
        widthSpinBox->setValue(rect.width());
        heightSpinBox->setValue(rect.height());

        return editor;
    }
};

class ColorEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(Property *property, QWidget *parent) override
    {
        auto value = property->value();
        auto editor = new ColorButton(parent);
        editor->setColor(value.value<QColor>());
        return editor;
    }
};


ValueProperty::ValueProperty(const QString &name,
                             const QVariant &value,
                             EditorFactory *editorFactory,
                             QObject *parent)
    : AbstractProperty(name, editorFactory, parent)
    , m_value(value)
{}

void ValueProperty::setValue(const QVariant &value)
{
    if (m_value != value) {
        m_value = value;
        emit valueChanged();
    }
}


EnumProperty::EnumProperty(const QString &name,
                           QObject *parent)
    : AbstractProperty(name, &m_editorFactory, parent)
{}

void EnumProperty::setEnumNames(const QStringList &enumNames)
{
    m_editorFactory.setEnumNames(enumNames);
}

void EnumProperty::setEnumValues(const QList<int> &enumValues)
{
    m_editorFactory.setEnumValues(enumValues);
}


VariantEditor::VariantEditor(QWidget *parent)
    : QScrollArea(parent)
{
    m_widget = new QWidget;
    m_widget->setBackgroundRole(QPalette::Base);
    auto verticalLayout = new QVBoxLayout(m_widget);
    m_gridLayout = new QGridLayout;
    verticalLayout->addLayout(m_gridLayout);
    verticalLayout->addStretch();
    verticalLayout->setContentsMargins(QMargins());

    setWidget(m_widget);
    setWidgetResizable(true);

    m_gridLayout->setContentsMargins(QMargins());
    m_gridLayout->setSpacing(Utils::dpiScaled(3));

    m_gridLayout->setColumnStretch(LabelColumn, 2);
    m_gridLayout->setColumnStretch(WidgetColumn, 3);
    m_gridLayout->setColumnMinimumWidth(LeftSpacing, Utils::dpiScaled(3));
    m_gridLayout->setColumnMinimumWidth(MiddleSpacing, Utils::dpiScaled(2));
    m_gridLayout->setColumnMinimumWidth(RightSpacing, Utils::dpiScaled(3));

    // auto alignmentEditorFactory = std::make_unique<EnumEditorFactory>();
    // alignmentEditorFactory->setEnumNames({
    //                                          tr("Unspecified"),
    //                                          tr("Top Left"),
    //                                          tr("Top"),
    //                                          tr("Top Right"),
    //                                          tr("Left"),
    //                                          tr("Center"),
    //                                          tr("Right"),
    //                                          tr("Bottom Left"),
    //                                          tr("Bottom"),
    //                                          tr("Bottom Right"),
    //                                      });
    // registerEditorFactory(qMetaTypeId<Alignment>(), std::move(alignmentEditorFactory));


    // auto staggerAxisEditorFactory = std::make_unique<EnumEditorFactory>();
    // staggerAxisEditorFactory->setEnumNames({
    //                                            tr("X"),
    //                                            tr("Y"),
    //                                        });
    // registerEditorFactory(qMetaTypeId<Map::StaggerAxis>(), std::move(staggerAxisEditorFactory));

    // auto staggerIndexEditorFactory = std::make_unique<EnumEditorFactory>();
    // staggerIndexEditorFactory->setEnumNames({
    //                                             tr("Odd"),
    //                                             tr("Even"),
    //                                         });
    // registerEditorFactory(qMetaTypeId<Map::StaggerIndex>(), std::move(staggerIndexEditorFactory));

    QStringList layerFormatNames = {
        QCoreApplication::translate("PreferencesDialog", "XML (deprecated)"),
        QCoreApplication::translate("PreferencesDialog", "Base64 (uncompressed)"),
        QCoreApplication::translate("PreferencesDialog", "Base64 (gzip compressed)"),
        QCoreApplication::translate("PreferencesDialog", "Base64 (zlib compressed)"),
    };
    QList<int> layerFormatValues = {
        Map::XML,
        Map::Base64,
        Map::Base64Gzip,
        Map::Base64Zlib,
    };

    if (compressionSupported(Zstandard)) {
        layerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "Base64 (Zstandard compressed)"));
        layerFormatValues.append(Map::Base64Zstandard);
    }

    layerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "CSV"));
    layerFormatValues.append(Map::CSV);

    // auto layerFormatEditorFactory = std::make_unique<EnumEditorFactory>();
    // layerFormatEditorFactory->setEnumNames(layerFormatNames);
    // layerFormatEditorFactory->setEnumValues(layerFormatValues);
    // registerEditorFactory(qMetaTypeId<Map::LayerDataFormat>(), std::move(layerFormatEditorFactory));

    // auto renderOrderEditorFactory = std::make_unique<EnumEditorFactory>();
    // renderOrderEditorFactory->setEnumNames({
    //                                            tr("Right Down"),
    //                                            tr("Right Up"),
    //                                            tr("Left Down"),
    //                                            tr("Left Up"),
    //                                        });
    // registerEditorFactory(qMetaTypeId<Map::RenderOrder>(), std::move(renderOrderEditorFactory));

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

    // addHeader(tr("Map"));
    // addProperty(new VariantProperty(tr("Class"), QString()));
    // addProperty(new VariantProperty(tr("Orientation"), QVariant::fromValue(Map::Hexagonal)));
    // addValue(tr("Class"), QString());
    // addSeparator();
    // addValue(tr("Orientation"), QVariant::fromValue(Map::Hexagonal));
    // addValue(tr("Infinite"), false);
    // addValue(tr("Map Size"), QSize(20, 20));
    // addValue(tr("Tile Size"), QSize(14, 12));
    // addValue(tr("Tile Side Length (Hex)"), 6);
    // addValue(tr("Stagger Axis"), QVariant::fromValue(Map::StaggerY));
    // addValue(tr("Stagger Index"), QVariant::fromValue(Map::StaggerEven));
    // addSeparator();
    // addValue(tr("Parallax Origin"), QPointF());
    // addSeparator();
    // addValue(tr("Tile Layer Format"), QVariant::fromValue(Map::Base64Zlib));
    // addValue(tr("Output Chunk Size"), QSize(16, 16));
    // addValue(tr("Compression Level"), -1);
    // addSeparator();
    // addValue(tr("Tile Render Order"), QVariant::fromValue(Map::RightDown));
    // addValue(tr("Background Color"), QColor());
    // addHeader(tr("Custom Properties"));
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

void VariantEditor::addHeader(const QString &text)
{
    auto label = new ElidingLabel(text, m_widget);
    label->setBackgroundRole(QPalette::Dark);
    const int verticalMargin = Utils::dpiScaled(3);
    const int horizontalMargin = Utils::dpiScaled(6);
    label->setContentsMargins(horizontalMargin, verticalMargin,
                              horizontalMargin, verticalMargin);

    label->setAutoFillBackground(true);

    m_gridLayout->addWidget(label, m_rowIndex, 0, 1, ColumnCount);
    ++m_rowIndex;
}

void VariantEditor::addSeparator()
{
    auto separator = new QFrame(m_widget);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setForegroundRole(QPalette::Mid);
    m_gridLayout->addWidget(separator, m_rowIndex, 0, 1, ColumnCount);
    ++m_rowIndex;
}

void VariantEditor::addProperty(Property *property)
{
    auto label = new LineEditLabel(property->name(), m_widget);
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    label->setEnabled(property->isEnabled());
    connect(property, &Property::enabledChanged, label, &QLabel::setEnabled);
    m_gridLayout->addWidget(label, m_rowIndex, LabelColumn, Qt::AlignTop/* | Qt::AlignRight*/);

    if (auto editor = createEditor(property)) {
        editor->setEnabled(property->isEnabled());
        connect(property, &Property::enabledChanged, editor, &QWidget::setEnabled);
        m_gridLayout->addWidget(editor, m_rowIndex, WidgetColumn);
    }
    ++m_rowIndex;
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
    if (const auto editor = property->createEditor(m_widget)) {
        editor->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        return editor;
    } else {
        qDebug() << "No editor for property" << property->name();
    }
    return nullptr;
}



EnumEditorFactory::EnumEditorFactory(const QStringList &enumNames,
                                     const QList<int> &enumValues)
    : m_enumNamesModel(enumNames)
    , m_enumValues(enumValues)
{}

void EnumEditorFactory::setEnumNames(const QStringList &enumNames)
{
    m_enumNamesModel.setStringList(enumNames);
}

void EnumEditorFactory::setEnumValues(const QList<int> &enumValues)
{
    m_enumValues = enumValues;
}

QWidget *EnumEditorFactory::createEditor(Property *property, QWidget *parent)
{
    auto editor = new QComboBox(parent);
    // This allows the combo box to shrink horizontally.
    editor->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    editor->setModel(&m_enumNamesModel);

    auto syncEditor = [property, editor, this]() {
        const QSignalBlocker blocker(editor);
        if (m_enumValues.isEmpty())
            editor->setCurrentIndex(property->value().toInt());
        else
            editor->setCurrentIndex(m_enumValues.indexOf(property->value().toInt()));
    };
    syncEditor();

    QObject::connect(property, &Property::valueChanged, editor, syncEditor);
    QObject::connect(editor, qOverload<int>(&QComboBox::currentIndexChanged), property,
                     [property, this](int index) {
        property->setValue(m_enumValues.isEmpty() ? index : m_enumValues.at(index));
    });

    return editor;
}


ValueTypeEditorFactory::ValueTypeEditorFactory()
{
    // Register some useful default editor factories
    registerEditorFactory(QMetaType::Bool, std::make_unique<BoolEditorFactory>());
    registerEditorFactory(QMetaType::Double, std::make_unique<FloatEditorFactory>());
    registerEditorFactory(QMetaType::Int, std::make_unique<IntEditorFactory>());
    registerEditorFactory(QMetaType::QColor, std::make_unique<ColorEditorFactory>());
    registerEditorFactory(QMetaType::QPoint, std::make_unique<PointEditorFactory>());
    registerEditorFactory(QMetaType::QPointF, std::make_unique<PointFEditorFactory>());
    registerEditorFactory(QMetaType::QRectF, std::make_unique<RectFEditorFactory>());
    registerEditorFactory(QMetaType::QSize, std::make_unique<SizeEditorFactory>());
    registerEditorFactory(QMetaType::QString, std::make_unique<StringEditorFactory>());
}

void ValueTypeEditorFactory::registerEditorFactory(int type, std::unique_ptr<EditorFactory> factory)
{
    m_factories[type] = std::move(factory);
}

QObjectProperty *ValueTypeEditorFactory::createQObjectProperty(QObject *qObject,
                                                               const char *name,
                                                               const QString &displayName)
{
    auto metaObject = qObject->metaObject();
    auto propertyIndex = metaObject->indexOfProperty(name);
    if (propertyIndex < 0)
        return nullptr;

    return new QObjectProperty(qObject,
                               metaObject->property(propertyIndex),
                               displayName.isEmpty() ? QString::fromUtf8(name)
                                                     : displayName,
                               this);
}

ValueProperty *ValueTypeEditorFactory::createProperty(const QString &name, const QVariant &value)
{
    const int type = value.userType();
    auto factory = m_factories.find(type);
    if (factory != m_factories.end())
        return new ValueProperty(name, value, factory->second.get());
    return nullptr;
}

AbstractProperty *ValueTypeEditorFactory::createProperty(const QString &name,
                                                         std::function<QVariant ()> get,
                                                         std::function<void (const QVariant &)> set)
{
    const int type = get().userType();
    auto factory = m_factories.find(type);
    if (factory != m_factories.end())
        return new GetSetProperty(name, get, set, factory->second.get());
    return nullptr;
}

QWidget *ValueTypeEditorFactory::createEditor(Property *property, QWidget *parent)
{
    const auto value = property->value();
    const int type = value.userType();
    auto factory = m_factories.find(type);
    if (factory != m_factories.end())
        return factory->second->createEditor(property, parent);
    return nullptr;
}


QObjectProperty::QObjectProperty(QObject *object,
                                 QMetaProperty property,
                                 const QString &displayName,
                                 EditorFactory *editorFactory,
                                 QObject *parent)
    : AbstractProperty(displayName, editorFactory, parent)
    , m_object(object)
    , m_property(property)
{
    // If the property has a notify signal, forward it to valueChanged
    auto notify = property.notifySignal();
    if (notify.isValid()) {
        auto valuePropertyIndex = metaObject()->indexOfProperty("value");
        auto valueProperty = metaObject()->property(valuePropertyIndex);
        auto valueChanged = valueProperty.notifySignal();

        connect(m_object, notify, this, valueChanged);
    }

    setEnabled(m_property.isWritable());
}

QVariant QObjectProperty::value() const
{
    return m_property.read(m_object);
}

void QObjectProperty::setValue(const QVariant &value)
{
    m_property.write(m_object, value);
}

} // namespace Tiled

#include "moc_varianteditor.cpp"
