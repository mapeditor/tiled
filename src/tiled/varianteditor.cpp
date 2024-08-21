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
#include "tiled.h"
#include "utils.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QSpacerItem>
#include <QSpinBox>
#include <QStringListModel>

namespace Tiled {

class SpinBox : public QSpinBox
{
    Q_OBJECT

public:
    SpinBox(QWidget *parent = nullptr)
        : QSpinBox(parent)
    {
        // Allow the full range by default.
        setRange(std::numeric_limits<int>::lowest(),
                 std::numeric_limits<int>::max());

        // Allow the widget to shrink horizontally.
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }

    QSize minimumSizeHint() const override
    {
        // Don't adjust the horizontal size hint based on the maximum value.
        auto hint = QSpinBox::minimumSizeHint();
        hint.setWidth(Utils::dpiScaled(50));
        return hint;
    }
};

/**
 * Strips a floating point number representation of redundant trailing zeros.
 * Examples:
 *
 *  0.01000 -> 0.01
 *  3.000   -> 3.0
 */
QString removeRedundantTrialingZeros(const QString &text)
{
    const QString decimalPoint = QLocale::system().decimalPoint();
    const auto decimalPointIndex = text.lastIndexOf(decimalPoint);
    if (decimalPointIndex < 0) // return if there is no decimal point
        return text;

    const auto afterDecimalPoint = decimalPointIndex + decimalPoint.length();
    int redundantZeros = 0;

    for (int i = text.length() - 1; i > afterDecimalPoint && text.at(i) == QLatin1Char('0'); --i)
        ++redundantZeros;

    return text.left(text.length() - redundantZeros);
}

class DoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    DoubleSpinBox(QWidget *parent = nullptr)
        : QDoubleSpinBox(parent)
    {
        // Allow the full range by default.
        setRange(std::numeric_limits<double>::lowest(),
                 std::numeric_limits<double>::max());

        // Increase possible precision.
        setDecimals(9);

        // Allow the widget to shrink horizontally.
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }

    QSize minimumSizeHint() const override
    {
        // Don't adjust the horizontal size hint based on the maximum value.
        auto hint = QDoubleSpinBox::minimumSizeHint();
        hint.setWidth(Utils::dpiScaled(50));
        return hint;
    }

    // QDoubleSpinBox interface
    QString textFromValue(double val) const override
    {
        auto text = QDoubleSpinBox::textFromValue(val);

        // remove redundant trailing 0's in case of high precision
        if (decimals() > 3)
            return removeRedundantTrialingZeros(text);

        return text;
    }
};


// A label that elides its text if there is not enough space
class ElidingLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ElidingLabel(QWidget *parent = nullptr)
        : ElidingLabel(QString(), parent)
    {}

    ElidingLabel(const QString &text, QWidget *parent = nullptr)
        : QLabel(text, parent)
    {
        setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    }

    QSize minimumSizeHint() const override
    {
        auto hint = QLabel::minimumSizeHint();
        hint.setWidth(std::min(hint.width(), Utils::dpiScaled(30)));
        return hint;
    }

    void paintEvent(QPaintEvent *) override
    {
        const int m = margin();
        const QRect cr = contentsRect().adjusted(m, m, -m, -m);
        const Qt::LayoutDirection dir = text().isRightToLeft() ? Qt::RightToLeft : Qt::LeftToRight;
        const int align = QStyle::visualAlignment(dir, alignment());
        const int flags = align | (dir == Qt::LeftToRight ? Qt::TextForceLeftToRight
                                                          : Qt::TextForceRightToLeft);

        QStyleOption opt;
        opt.initFrom(this);

        const auto elidedText = opt.fontMetrics.elidedText(text(), Qt::ElideRight, cr.width());

        const bool isElided = elidedText != text();
        if (isElided != m_isElided) {
            m_isElided = isElided;
            setToolTip(isElided ? text() : QString());
        }

        QPainter painter(this);
        QWidget::style()->drawItemText(&painter, cr, flags, opt.palette, isEnabled(), elidedText, foregroundRole());
    }

private:
    bool m_isElided = false;
};

// A label that matches its preferred height with that of a line edit
class LineEditLabel : public ElidingLabel
{
    Q_OBJECT

public:
    using ElidingLabel::ElidingLabel;

    QSize sizeHint() const override
    {
        auto hint = ElidingLabel::sizeHint();
        hint.setHeight(lineEdit.sizeHint().height());
        return hint;
    }

private:
    QLineEdit lineEdit;
};

class StringEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new QLineEdit(parent);
        editor->setText(value.toString());
        return editor;
    }

    QVariant value(QWidget *editor) const override
    {
        return static_cast<QLineEdit *>(editor)->text();
    }
};

class IntEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new SpinBox(parent);
        editor->setValue(value.toInt());
        return editor;
    }
};

class FloatEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new DoubleSpinBox(parent);
        editor->setValue(value.toDouble());
        return editor;
    }
};

class BoolEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new QCheckBox(parent);
        bool checked = value.toBool();
        editor->setChecked(checked);
        editor->setText(checked ? tr("On") : tr("Off"));

        QObject::connect(editor, &QCheckBox::toggled, [editor](bool checked) {
            editor->setText(checked ? QObject::tr("On") : QObject::tr("Off"));
        });

        return editor;
    }
};

class PointEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
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

        // horizontalLayout->addStretch();

        xSpinBox->setValue(value.toPoint().x());
        ySpinBox->setValue(value.toPoint().y());

        return editor;
    }
};

class PointFEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
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

        // horizontalLayout->addStretch();

        xSpinBox->setValue(value.toPointF().x());
        ySpinBox->setValue(value.toPointF().y());

        return editor;
    }
};

class SizeEditor : public QWidget
{
public:
    SizeEditor(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_widthLabel(new QLabel(QStringLiteral("W"), this))
        , m_heightLabel(new QLabel(QStringLiteral("H"), this))
        , m_widthSpinBox(new SpinBox(this))
        , m_heightSpinBox(new SpinBox(this))
    {
        m_widthLabel->setAlignment(Qt::AlignCenter);
        m_heightLabel->setAlignment(Qt::AlignCenter);

        auto layout = new QGridLayout(this);
        layout->setContentsMargins(QMargins());
        layout->setColumnStretch(1, 1);
        layout->setColumnStretch(3, 1);
        layout->setSpacing(Utils::dpiScaled(3));

        const int horizontalMargin = Utils::dpiScaled(3);
        m_widthLabel->setContentsMargins(horizontalMargin, 0, horizontalMargin, 0);
        m_heightLabel->setContentsMargins(horizontalMargin, 0, horizontalMargin, 0);

        layout->addWidget(m_widthLabel, 0, 0);
        layout->addWidget(m_widthSpinBox, 0, 1);
        layout->addWidget(m_heightLabel, 0, 2);
        layout->addWidget(m_heightSpinBox, 0, 3);
    }

    void setValue(const QSize &size)
    {
        m_widthSpinBox->setValue(size.width());
        m_heightSpinBox->setValue(size.height());
    }

    QSize value() const
    {
        return QSize(m_widthSpinBox->value(), m_heightSpinBox->value());
    }

private:
    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);

        const auto direction = event->size().width() < minimumHorizontalWidth()
                ? Qt::Vertical : Qt::Horizontal;

        if (m_direction != direction) {
            m_direction = direction;

            auto layout = qobject_cast<QGridLayout *>(this->layout());

            // Remove all widgets from layout, without deleting them
            layout->removeWidget(m_widthLabel);
            layout->removeWidget(m_widthSpinBox);
            layout->removeWidget(m_heightLabel);
            layout->removeWidget(m_heightSpinBox);

            if (direction == Qt::Horizontal) {
                layout->addWidget(m_widthLabel, 0, 0);
                layout->addWidget(m_widthSpinBox, 0, 1);
                layout->addWidget(m_heightLabel, 0, 2);
                layout->addWidget(m_heightSpinBox, 0, 3);
                layout->setColumnStretch(3, 1);
            } else {
                layout->addWidget(m_widthLabel, 0, 0);
                layout->addWidget(m_widthSpinBox, 0, 1);
                layout->addWidget(m_heightLabel, 1, 0);
                layout->addWidget(m_heightSpinBox, 1, 1);
                layout->setColumnStretch(3, 0);
            }

            // this avoids flickering when the layout changes
            layout->activate();
        }
    }

    int minimumHorizontalWidth() const
    {
        return m_widthLabel->minimumSizeHint().width() +
                m_widthSpinBox->minimumSizeHint().width() +
                m_heightLabel->minimumSizeHint().width() +
                m_heightSpinBox->minimumSizeHint().width() +
                layout()->spacing() * 3;
    }

    Qt::Orientation m_direction = Qt::Horizontal;
    QLabel *m_widthLabel;
    QLabel *m_heightLabel;
    SpinBox *m_widthSpinBox;
    SpinBox *m_heightSpinBox;
};

class SizeEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new SizeEditor(parent);
        editor->setValue(value.toSize());
        return editor;
    }
};

class RectFEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
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
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new ColorButton(parent);
        editor->setColor(value.value<QColor>());
        return editor;
    }
};

class EnumEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new QComboBox(parent);
        // This allows the combo box to shrink horizontally.
        editor->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
        editor->setModel(&m_enumNamesModel);

        if (m_enumValues.isEmpty())
            editor->setCurrentIndex(value.toInt());
        else
            editor->setCurrentIndex(m_enumValues.indexOf(value.toInt()));

        return editor;
    }

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

    registerEditorFactory(QMetaType::QString, std::make_unique<StringEditorFactory>());
    registerEditorFactory(QMetaType::Int, std::make_unique<IntEditorFactory>());
    registerEditorFactory(QMetaType::Double, std::make_unique<FloatEditorFactory>());
    registerEditorFactory(QMetaType::Bool, std::make_unique<BoolEditorFactory>());
    registerEditorFactory(QMetaType::QPoint, std::make_unique<PointEditorFactory>());
    registerEditorFactory(QMetaType::QPointF, std::make_unique<PointFEditorFactory>());
    registerEditorFactory(QMetaType::QSize, std::make_unique<SizeEditorFactory>());
    registerEditorFactory(QMetaType::QRectF, std::make_unique<RectFEditorFactory>());
    registerEditorFactory(QMetaType::QColor, std::make_unique<ColorEditorFactory>());

    auto alignmentEditorFactory = std::make_unique<EnumEditorFactory>();
    alignmentEditorFactory->setEnumNames({
                                             tr("Unspecified"),
                                             tr("Top Left"),
                                             tr("Top"),
                                             tr("Top Right"),
                                             tr("Left"),
                                             tr("Center"),
                                             tr("Right"),
                                             tr("Bottom Left"),
                                             tr("Bottom"),
                                             tr("Bottom Right"),
                                         });
    registerEditorFactory(qMetaTypeId<Alignment>(), std::move(alignmentEditorFactory));

    auto orientationEditorFactory = std::make_unique<EnumEditorFactory>();
    orientationEditorFactory->setEnumNames({
                                               tr("Orthogonal"),
                                               tr("Isometric"),
                                               tr("Isometric (Staggered)"),
                                               tr("Hexagonal (Staggered)"),
                                           });
    orientationEditorFactory->setEnumValues({
                                                Map::Orthogonal,
                                                Map::Isometric,
                                                Map::Staggered,
                                                Map::Hexagonal,
                                            });
    registerEditorFactory(qMetaTypeId<Map::Orientation>(), std::move(orientationEditorFactory));

    auto staggerAxisEditorFactory = std::make_unique<EnumEditorFactory>();
    staggerAxisEditorFactory->setEnumNames({
                                               tr("X"),
                                               tr("Y"),
                                           });
    registerEditorFactory(qMetaTypeId<Map::StaggerAxis>(), std::move(staggerAxisEditorFactory));

    auto staggerIndexEditorFactory = std::make_unique<EnumEditorFactory>();
    staggerIndexEditorFactory->setEnumNames({
                                                tr("Odd"),
                                                tr("Even"),
                                            });
    registerEditorFactory(qMetaTypeId<Map::StaggerIndex>(), std::move(staggerIndexEditorFactory));

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

    auto layerFormatEditorFactory = std::make_unique<EnumEditorFactory>();
    layerFormatEditorFactory->setEnumNames(layerFormatNames);
    layerFormatEditorFactory->setEnumValues(layerFormatValues);
    registerEditorFactory(qMetaTypeId<Map::LayerDataFormat>(), std::move(layerFormatEditorFactory));

    auto renderOrderEditorFactory = std::make_unique<EnumEditorFactory>();
    renderOrderEditorFactory->setEnumNames({
                                               tr("Right Down"),
                                               tr("Right Up"),
                                               tr("Left Down"),
                                               tr("Left Up"),
                                           });
    registerEditorFactory(qMetaTypeId<Map::RenderOrder>(), std::move(renderOrderEditorFactory));

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

    addHeader(tr("Map"));
    addValue(tr("Class"), QString());
    addSeparator();
    addValue(tr("Orientation"), QVariant::fromValue(Map::Hexagonal));
    addValue(tr("Infinite"), false);
    addValue(tr("Map Size"), QSize(20, 20));
    addValue(tr("Tile Size"), QSize(14, 12));
    addValue(tr("Tile Side Length (Hex)"), 6);
    addValue(tr("Stagger Axis"), QVariant::fromValue(Map::StaggerY));
    addValue(tr("Stagger Index"), QVariant::fromValue(Map::StaggerEven));
    addSeparator();
    addValue(tr("Parallax Origin"), QPointF());
    addSeparator();
    addValue(tr("Tile Layer Format"), QVariant::fromValue(Map::Base64Zlib));
    addValue(tr("Output Chunk Size"), QSize(16, 16));
    addValue(tr("Compression Level"), -1);
    addSeparator();
    addValue(tr("Tile Render Order"), QVariant::fromValue(Map::RightDown));
    addValue(tr("Background Color"), QColor());
    addHeader(tr("Custom Properties"));
}

void VariantEditor::registerEditorFactory(int type, std::unique_ptr<EditorFactory> factory)
{
    m_factories[type] = std::move(factory);
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

void VariantEditor::addValue(const QString &name, const QVariant &value)
{
    auto label = new LineEditLabel(name, m_widget);
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    m_gridLayout->addWidget(label, m_rowIndex, LabelColumn, Qt::AlignTop/* | Qt::AlignRight*/);
    if (auto editor = createEditor(value)) {
        m_gridLayout->addWidget(editor, m_rowIndex, WidgetColumn);
    }
    ++m_rowIndex;
}

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

QWidget *VariantEditor::createEditor(const QVariant &value)
{
    const int type = value.userType();
    auto factory = m_factories.find(type);
    if (factory != m_factories.end()) {
        const auto editor = factory->second->createEditor(value, m_widget);
        editor->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        return editor;
    } else {
        qDebug() << "No editor factory for type" << type;
    }
    return nullptr;
}

} // namespace Tiled

#include "moc_varianteditor.cpp"
#include "varianteditor.moc"
