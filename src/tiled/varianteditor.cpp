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

class SizeEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new QWidget(parent);
        auto horizontalLayout = new QHBoxLayout(editor);
        horizontalLayout->setContentsMargins(QMargins());

        auto widthLabel = new QLabel(QStringLiteral("W"), editor);
        widthLabel->setToolTip(tr("Width"));
        horizontalLayout->addWidget(widthLabel, 0, Qt::AlignRight);

        auto widthSpinBox = new SpinBox(editor);
        widthLabel->setBuddy(widthSpinBox);
        horizontalLayout->addWidget(widthSpinBox, 1);

        auto heightLabel = new QLabel(QStringLiteral("H"), editor);
        heightLabel->setToolTip(tr("Height"));
        horizontalLayout->addWidget(heightLabel, 0, Qt::AlignRight);

        auto heightSpinBox = new SpinBox(editor);
        heightLabel->setBuddy(heightSpinBox);
        horizontalLayout->addWidget(heightSpinBox, 1);

        // horizontalLayout->addStretch();

        widthSpinBox->setValue(value.toSize().width());
        heightSpinBox->setValue(value.toSize().height());

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
        editor->setCurrentIndex(value.toInt());
        return editor;
    }

    void setEnumNames(const QStringList &enumNames)
    {
        m_enumNamesModel.setStringList(enumNames);
    }

private:
    QStringListModel m_enumNamesModel;
};


VariantEditor::VariantEditor(QWidget *parent)
    : QScrollArea(parent)
{
    m_widget = new QWidget;
    auto verticalLayout = new QVBoxLayout(m_widget);
    m_gridLayout = new QGridLayout;
    verticalLayout->addLayout(m_gridLayout);
    verticalLayout->addStretch();

    setWidget(m_widget);
    setWidgetResizable(true);

    m_gridLayout->setSpacing(Utils::dpiScaled(2));

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
                                               tr("Unknown"),       // todo: hide this one
                                               tr("Orthogonal"),
                                               tr("Isometric"),
                                               tr("Staggered"),
                                               tr("Hexagonal (Staggered)"),
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

    auto layerFormatEditorFactory = std::make_unique<EnumEditorFactory>();
    layerFormatEditorFactory->setEnumNames({
                                               tr("XML (deprecated)"),
                                               tr("Base64"),
                                               tr("Base64Gzip"),
                                               tr("Base64Zlib"),
                                               tr("Base64Zstandard"),
                                               tr("CSV"),
                                           });
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

    // m_gridLayout->setColumnStretch(0, 0);
    // m_gridLayout->setColumnStretch(1, 0);
    m_gridLayout->setColumnStretch(2, 1);
    m_gridLayout->setColumnMinimumWidth(1, Utils::dpiScaled(10));

    // setValue(QVariantList {
    //              QVariant(QLatin1String("Hello")),
    //              QVariant(10),
    //              QVariant(3.14)
    //          });

    addValue(tr("Class"), QString());
    addValue(tr("Orientation"), QVariant::fromValue(Map::Hexagonal));
    addValue(tr("Size"), QSize(20, 20));
    addValue(tr("Tile Size"), QSize(14, 12));
    addValue(tr("Infinite"), false);
    addValue(tr("Tile Side Length (Hex)"), 6);
    addValue(tr("Stagger Axis"), QVariant::fromValue(Map::StaggerY));
    addValue(tr("Stagger Index"), QVariant::fromValue(Map::StaggerEven));
    addValue(tr("Parallax Origin"), QPointF());
    addValue(tr("Tile Layer Format"), QVariant::fromValue(Map::Base64Zlib));
    addValue(tr("Output Chunk Size"), QSize(16, 16));
    addValue(tr("Tile Render Order"), QVariant::fromValue(Map::RightDown));
    addValue(tr("Compression Level"), -1);
    addValue(tr("Background Color"), QColor());
}

void VariantEditor::registerEditorFactory(int type, std::unique_ptr<EditorFactory> factory)
{
    m_factories[type] = std::move(factory);
}

void VariantEditor::addValue(const QString &name, const QVariant &value)
{
    auto label = new LineEditLabel(name, m_widget);
    label->setBuddy(new QLabel(m_widget));
    m_gridLayout->addWidget(label, m_rowIndex, 0, Qt::AlignTop | Qt::AlignRight);
    setValue(value);
}

void VariantEditor::setValue(const QVariant &value)
{
    const int type = value.userType();
    switch (type) {
    case QMetaType::QVariantList: {
        const auto list = value.toList();
        for (const auto &item : list)
            setValue(item);
        break;
    }
    case QMetaType::QVariantMap: {
        const auto map = value.toMap();
        for (auto it = map.constBegin(); it != map.constEnd(); ++it)
            addValue(it.key(), it.value());
        break;
    }
    default: {
        auto factory = m_factories.find(type);
        if (factory != m_factories.end()) {
            const auto editor = factory->second->createEditor(value, m_widget);
            m_gridLayout->addWidget(editor, m_rowIndex, 2);
        } else {
            qDebug() << "No editor factory for type" << type;
        }
        ++m_rowIndex;
    }
    }
}

QSize VariantEditor::viewportSizeHint() const
{
    return m_widget->minimumSizeHint();
}

} // namespace Tiled

#include "moc_varianteditor.cpp"
#include "varianteditor.moc"
