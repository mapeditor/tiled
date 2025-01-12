/*
 * propertyeditorwidgets.cpp
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

#include "propertyeditorwidgets.h"

#include "propertiesview.h"
#include "utils.h"

#include <QAction>
#include <QColorDialog>
#include <QFontDatabase>
#include <QGridLayout>
#include <QMenu>
#include <QPainter>
#include <QResizeEvent>
#include <QScopedValueRollback>
#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>
#include <QValidator>

namespace Tiled {

/**
 * A layout for label/widget pairs, wrapping them either two per row or each on
 * their own row, depending on the available space.
 *
 * The labels are forced to width of the widest label, while the widgets share
 * the remaining space.
 */
class PairwiseWrappingLayout : public QLayout
{
    Q_OBJECT

public:
    struct WidgetPair {
        QLabel *label;
        QWidget *widget;
    };

    PairwiseWrappingLayout(const QVector<WidgetPair> &widgetPairs,
                           QWidget *parent);
    ~PairwiseWrappingLayout();

    void addItem(QLayoutItem *item) override
    { m_items.append(item); }

    Qt::Orientations expandingDirections() const override
    { return {}; }

    bool hasHeightForWidth() const override
    { return true; }

    int heightForWidth(int width) const override
    { return doLayout(QRect(0, 0, width, 0), true); }

    int count() const override
    { return m_items.size(); }

    QLayoutItem *itemAt(int index) const override
    { return m_items.value(index); }

    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;

    QSize sizeHint() const override
    { return minimumSize(); }

    QLayoutItem *takeAt(int index) override;

private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int minimumTwoColumnWidth() const;

    QList<QLayoutItem *> m_items;
};

PairwiseWrappingLayout::PairwiseWrappingLayout(const QVector<WidgetPair> &widgetPairs,
                                               QWidget *parent)
    : QLayout(parent)
{
    setContentsMargins(QMargins());
    setSpacing(Utils::dpiScaled(2) * 2);

    const int horizontalMargin = Utils::dpiScaled(3);

    for (auto &pair : widgetPairs) {
        pair.label->setAlignment(Qt::AlignCenter);
        pair.label->setContentsMargins(horizontalMargin, 0, horizontalMargin, 0);
        addWidget(pair.label);
        addWidget(pair.widget);
    }
}

PairwiseWrappingLayout::~PairwiseWrappingLayout()
{
    while (QLayoutItem *item = takeAt(0))
        delete item;
}

QLayoutItem *PairwiseWrappingLayout::takeAt(int index)
{
    if (index >= 0 && index < m_items.size())
        return m_items.takeAt(index);
    return nullptr;
}

void PairwiseWrappingLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize PairwiseWrappingLayout::minimumSize() const
{
    QSize size;
    size.setWidth(minimumTwoColumnWidth());
    size.setHeight(doLayout(QRect(0, 0, size.width(), 0), true));
    return size.grownBy(contentsMargins());
}

int PairwiseWrappingLayout::doLayout(const QRect &rect, bool testOnly) const
{
    const auto margins = contentsMargins();
    const QRect effectiveRect = rect.marginsRemoved(margins);
    const int spacing = QLayout::spacing();
    const int columns = effectiveRect.width() < minimumTwoColumnWidth() ? 1 : 2;

    // For simplicity, all lines will have the same height. Even columns will
    // get their width from the widest label, whereas odd columns will share
    // the remaining space.
    int lineHeight = 0;
    int maxLabelWidth = 0;
    for (qsizetype i = 0; i < m_items.size(); ++i) {
        const auto sizeHint = m_items.at(i)->minimumSize();
        lineHeight = qMax(lineHeight, sizeHint.height());
        if (i % 2 == 0)
            maxLabelWidth = qMax(maxLabelWidth, sizeHint.width());
    }

    if (testOnly) {
        const int lines = (m_items.size() / 2 + columns - 1) / columns;
        return margins.top() + margins.bottom()
                + lines * (lineHeight + spacing) - spacing;
    }

    int totalWidgetWidth = (effectiveRect.width() -
                            maxLabelWidth * columns -
                            spacing * (columns * 2 - 1));
    QList<int> widgetWidths;
    for (int i = columns; i > 0; --i) {
        widgetWidths.append(totalWidgetWidth / i);
        totalWidgetWidth -= widgetWidths.last();
    }

    int x = effectiveRect.x();
    int y = effectiveRect.y();
    for (qsizetype i = 0; i < m_items.size(); ++i) {
        const int column = i / 2 % columns;
        const QSize size((i % 2 == 0) ? maxLabelWidth
                                      : widgetWidths.at(column),
                         lineHeight);

        m_items.at(i)->setGeometry(QRect(QPoint(x, y), size));
        x += size.width() + spacing;

        if (column == columns - 1 && (i % 2 == 1)) {
            x = effectiveRect.x();
            y += lineHeight + spacing;
        }
    }

    return 0;
}

int PairwiseWrappingLayout::minimumTwoColumnWidth() const
{
    const int spacing = QLayout::spacing();
    int sum = 0;
    int minimum = 0;
    int index = 0;

    for (qsizetype i = 0; i < m_items.size() - 1; i += 2) {
        sum += (m_items.at(i)->minimumSize().width() +
                m_items.at(i + 1)->minimumSize().width() +
                spacing * 2);

        if (++index % 2 == 0) {
            minimum = std::max(sum - spacing, minimum);
            sum = 0;
        }
    }

    return minimum;
}


/**
 * Returns whether the given event is a shortcut override event for the undo or
 * redo shortcuts. We generally want to use the global undo and redo shortcuts
 * instead.
 */
static bool isUndoRedoShortcutOverride(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        auto ke = static_cast<QKeyEvent *>(event);
        return (ke == QKeySequence::Redo || ke == QKeySequence::Undo);
    }
    return false;
}

/**
 * Strips a floating point number representation of redundant trailing zeros.
 * Examples:
 *
 *  0.01000 -> 0.01
 *  3.000   -> 3.0
 */
static QString removeRedundantTrialingZeros(const QString &text)
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


void Slider::wheelEvent(QWheelEvent *event)
{
    if (!hasFocus())
        event->ignore();
    else
        QSlider::wheelEvent(event);
}


bool LineEdit::event(QEvent *event)
{
    if (isUndoRedoShortcutOverride(event))
        return false;

    return QLineEdit::event(event);
}


ComboBox::ComboBox(QWidget *parent)
    : QComboBox(parent)
{
    // Combo boxes in properties view don't adjust to their contents
    setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

    // Don't take focus by mouse wheel
    if (focusPolicy() == Qt::WheelFocus)
        setFocusPolicy(Qt::StrongFocus);
}

bool ComboBox::event(QEvent *event)
{
    if (isUndoRedoShortcutOverride(event))  // relevant when editable
        return false;

    return QComboBox::event(event);
}

void ComboBox::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        emit returnPressed();
        return;
    }

    QComboBox::keyPressEvent(event);
}

void ComboBox::wheelEvent(QWheelEvent *event)
{
    if (!hasFocus())
        event->ignore();
    else
        QComboBox::wheelEvent(event);
}


SpinBox::SpinBox(QWidget *parent)
    : QSpinBox(parent)
{
    // Allow the full range by default.
    setRange(std::numeric_limits<int>::lowest(),
             std::numeric_limits<int>::max());

    // Don't take focus by mouse wheel
    setFocusPolicy(Qt::StrongFocus);

    // Allow the widget to shrink horizontally.
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

QSize SpinBox::minimumSizeHint() const
{
    // Don't adjust the horizontal size hint based on the maximum value.
    auto hint = QSpinBox::minimumSizeHint();
    hint.setWidth(Utils::dpiScaled(50));
    return hint;
}

bool SpinBox::event(QEvent *event)
{
    if (isUndoRedoShortcutOverride(event))  // relevant when editable
        return false;

    return QSpinBox::event(event);
}

void SpinBox::wheelEvent(QWheelEvent *event)
{
    if (!hasFocus())
        event->ignore();
    else
        QSpinBox::wheelEvent(event);
}


DoubleSpinBox::DoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent)
{
    // Allow the full range by default.
    setRange(std::numeric_limits<double>::lowest(),
             std::numeric_limits<double>::max());

    // Increase possible precision.
    setDecimals(9);

    // Don't take focus by mouse wheel
    setFocusPolicy(Qt::StrongFocus);

    // Allow the widget to shrink horizontally.
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

QSize DoubleSpinBox::minimumSizeHint() const
{
    // Don't adjust the horizontal size hint based on the maximum value.
    auto hint = QDoubleSpinBox::minimumSizeHint();
    hint.setWidth(Utils::dpiScaled(65));
    return hint;
}

QString DoubleSpinBox::textFromValue(double val) const
{
    auto text = QDoubleSpinBox::textFromValue(val);

    // remove redundant trailing 0's in case of high precision
    if (decimals() > 3)
        return removeRedundantTrialingZeros(text);

    return text;
}

bool DoubleSpinBox::event(QEvent *event)
{
    if (isUndoRedoShortcutOverride(event))  // relevant when editable
        return false;

    return QDoubleSpinBox::event(event);
}

void DoubleSpinBox::wheelEvent(QWheelEvent *event)
{
    if (!hasFocus())
        event->ignore();
    else
        QDoubleSpinBox::wheelEvent(event);
}


SizeEdit::SizeEdit(QWidget *parent)
    : QWidget(parent)
    , m_widthLabel(new QLabel(QStringLiteral("W"), this))
    , m_heightLabel(new QLabel(QStringLiteral("H"), this))
    , m_widthSpinBox(new SpinBox(this))
    , m_heightSpinBox(new SpinBox(this))
{
    new PairwiseWrappingLayout({
                                   { m_widthLabel, m_widthSpinBox },
                                   { m_heightLabel, m_heightSpinBox },
                               }, this);

    connect(m_widthSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SizeEdit::valueChanged);
    connect(m_heightSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SizeEdit::valueChanged);
}

void SizeEdit::setValue(const QSize &size)
{
    m_widthSpinBox->setValue(size.width());
    m_heightSpinBox->setValue(size.height());
}

QSize SizeEdit::value() const
{
    return QSize(m_widthSpinBox->value(),
                 m_heightSpinBox->value());
}

void SizeEdit::setMinimum(int minimum)
{
    m_widthSpinBox->setMinimum(minimum);
    m_heightSpinBox->setMinimum(minimum);
}

void SizeEdit::setSuffix(const QString &suffix)
{
    m_widthSpinBox->setSuffix(suffix);
    m_heightSpinBox->setSuffix(suffix);
}


SizeFEdit::SizeFEdit(QWidget *parent)
    : QWidget(parent)
    , m_widthLabel(new QLabel(QStringLiteral("W"), this))
    , m_heightLabel(new QLabel(QStringLiteral("H"), this))
    , m_widthSpinBox(new DoubleSpinBox(this))
    , m_heightSpinBox(new DoubleSpinBox(this))
{
    new PairwiseWrappingLayout({
                                   { m_widthLabel, m_widthSpinBox },
                                   { m_heightLabel, m_heightSpinBox },
                               }, this);

    connect(m_widthSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SizeFEdit::valueChanged);
    connect(m_heightSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SizeFEdit::valueChanged);
}

void SizeFEdit::setValue(const QSizeF &size)
{
    m_widthSpinBox->setValue(size.width());
    m_heightSpinBox->setValue(size.height());
}

QSizeF SizeFEdit::value() const
{
    return QSizeF(m_widthSpinBox->value(),
                  m_heightSpinBox->value());
}


PointEdit::PointEdit(QWidget *parent)
    : QWidget(parent)
    , m_xLabel(new QLabel(QStringLiteral("X"), this))
    , m_yLabel(new QLabel(QStringLiteral("Y"), this))
    , m_xSpinBox(new SpinBox(this))
    , m_ySpinBox(new SpinBox(this))
{
    new PairwiseWrappingLayout({
                                   { m_xLabel, m_xSpinBox },
                                   { m_yLabel, m_ySpinBox },
                               }, this);

    connect(m_xSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &PointEdit::valueChanged);
    connect(m_ySpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &PointEdit::valueChanged);
}

void PointEdit::setValue(const QPoint &point)
{
    m_xSpinBox->setValue(point.x());
    m_ySpinBox->setValue(point.y());
}

QPoint PointEdit::value() const
{
    return QPoint(m_xSpinBox->value(),
                  m_ySpinBox->value());
}

void PointEdit::setSuffix(const QString &suffix)
{
    m_xSpinBox->setSuffix(suffix);
    m_ySpinBox->setSuffix(suffix);
}


PointFEdit::PointFEdit(QWidget *parent)
    : QWidget(parent)
    , m_xLabel(new QLabel(QStringLiteral("X"), this))
    , m_yLabel(new QLabel(QStringLiteral("Y"), this))
    , m_xSpinBox(new DoubleSpinBox(this))
    , m_ySpinBox(new DoubleSpinBox(this))
{
    new PairwiseWrappingLayout({
                                   { m_xLabel, m_xSpinBox },
                                   { m_yLabel, m_ySpinBox },
                               }, this);

    connect(m_xSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &PointFEdit::valueChanged);
    connect(m_ySpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &PointFEdit::valueChanged);
}

void PointFEdit::setValue(const QPointF &point)
{
    m_xSpinBox->setValue(point.x());
    m_ySpinBox->setValue(point.y());
}

QPointF PointFEdit::value() const
{
    return QPointF(m_xSpinBox->value(),
                   m_ySpinBox->value());
}

void PointFEdit::setSingleStep(double step)
{
    m_xSpinBox->setSingleStep(step);
    m_ySpinBox->setSingleStep(step);
}


RectEdit::RectEdit(QWidget *parent)
    : QWidget(parent)
    , m_xLabel(new QLabel(QStringLiteral("X"), this))
    , m_yLabel(new QLabel(QStringLiteral("Y"), this))
    , m_widthLabel(new QLabel(QStringLiteral("W"), this))
    , m_heightLabel(new QLabel(QStringLiteral("H"), this))
    , m_xSpinBox(new SpinBox(this))
    , m_ySpinBox(new SpinBox(this))
    , m_widthSpinBox(new SpinBox(this))
    , m_heightSpinBox(new SpinBox(this))
{
    new PairwiseWrappingLayout({
                                   { m_xLabel, m_xSpinBox },
                                   { m_yLabel, m_ySpinBox },
                                   { m_widthLabel, m_widthSpinBox },
                                   { m_heightLabel, m_heightSpinBox },
                               }, this);

    m_widthSpinBox->setMinimum(0);
    m_heightSpinBox->setMinimum(0);

    connect(m_xSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &RectEdit::valueChanged);
    connect(m_ySpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &RectEdit::valueChanged);
    connect(m_widthSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &RectEdit::valueChanged);
    connect(m_heightSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &RectEdit::valueChanged);
}

void RectEdit::setValue(const QRect &rect)
{
    m_xSpinBox->setValue(rect.x());
    m_ySpinBox->setValue(rect.y());
    m_widthSpinBox->setValue(rect.width());
    m_heightSpinBox->setValue(rect.height());
}

QRect RectEdit::value() const
{
    return QRect(m_xSpinBox->value(),
                 m_ySpinBox->value(),
                 m_widthSpinBox->value(),
                 m_heightSpinBox->value());
}

void RectEdit::setConstraint(const QRect &constraint)
{
    if (constraint.isNull()) {
        m_xSpinBox->setRange(std::numeric_limits<int>::lowest(),
                             std::numeric_limits<int>::max());
        m_ySpinBox->setRange(std::numeric_limits<int>::lowest(),
                             std::numeric_limits<int>::max());
        m_widthSpinBox->setRange(0, std::numeric_limits<int>::max());
        m_heightSpinBox->setRange(0, std::numeric_limits<int>::max());
    } else {
        m_xSpinBox->setRange(constraint.left(), constraint.right() + 1);
        m_ySpinBox->setRange(constraint.top(), constraint.bottom() + 1);
        m_widthSpinBox->setRange(0, constraint.width());
        m_heightSpinBox->setRange(0, constraint.height());
    }
}


RectFEdit::RectFEdit(QWidget *parent)
    : QWidget(parent)
    , m_xLabel(new QLabel(QStringLiteral("X"), this))
    , m_yLabel(new QLabel(QStringLiteral("Y"), this))
    , m_widthLabel(new QLabel(QStringLiteral("W"), this))
    , m_heightLabel(new QLabel(QStringLiteral("H"), this))
    , m_xSpinBox(new DoubleSpinBox(this))
    , m_ySpinBox(new DoubleSpinBox(this))
    , m_widthSpinBox(new DoubleSpinBox(this))
    , m_heightSpinBox(new DoubleSpinBox(this))
{
    new PairwiseWrappingLayout({
                                   { m_xLabel, m_xSpinBox },
                                   { m_yLabel, m_ySpinBox },
                                   { m_widthLabel, m_widthSpinBox },
                                   { m_heightLabel, m_heightSpinBox },
                               }, this);

    connect(m_xSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &RectFEdit::valueChanged);
    connect(m_ySpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &RectFEdit::valueChanged);
    connect(m_widthSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &RectFEdit::valueChanged);
    connect(m_heightSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &RectFEdit::valueChanged);
}

void RectFEdit::setValue(const QRectF &rect)
{
    m_xSpinBox->setValue(rect.x());
    m_ySpinBox->setValue(rect.y());
    m_widthSpinBox->setValue(rect.width());
    m_heightSpinBox->setValue(rect.height());
}

QRectF RectFEdit::value() const
{
    return QRectF(m_xSpinBox->value(),
                  m_ySpinBox->value(),
                  m_widthSpinBox->value(),
                  m_heightSpinBox->value());
}


class ColorValidator : public QValidator
{
    Q_OBJECT

public:
    using QValidator::QValidator;

    State validate(QString &input, int &) const override
    {
        if (isValidName(input))
            return State::Acceptable;

        return State::Intermediate;
    }

    void fixup(QString &input) const override
    {
        if (!isValidName(input) && isValidName(QLatin1Char('#') + input))
            input.prepend(QLatin1Char('#'));
    }

    static bool isValidName(const QString &name)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
        return QColor::isValidColorName(name);
#else
        return QColor::isValidColor(name);
#endif
    }
};


ColorEdit::ColorEdit(QWidget *parent)
    : LineEdit(parent)
{
    setValidator(new ColorValidator(this));
    setClearButtonEnabled(true);
    setPlaceholderText(tr("Not set"));
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    m_colorIconAction = addAction(Utils::colorIcon(QColor(), Utils::smallIconSize()), QLineEdit::LeadingPosition);
    m_colorIconAction->setText(tr("Pick Color"));
    connect(m_colorIconAction, &QAction::triggered, this, &ColorEdit::pickColor);

    connect(this, &QLineEdit::textEdited, this, &ColorEdit::onTextEdited);
}

void ColorEdit::setValue(const QColor &color)
{
    if (m_color == color)
        return;

    m_color = color;

    if (!m_editingText) {
        if (!color.isValid())
            setText(QString());
        else if (m_color.alpha() == 255)
            setText(color.name(QColor::HexRgb));
        else
            setText(color.name(QColor::HexArgb));
    }

    m_colorIconAction->setIcon(Utils::colorIcon(color, Utils::smallIconSize()));

    if (m_editingText)
        emit valueEdited();
}

void ColorEdit::setShowAlpha(bool enabled)
{
    if (m_showAlpha == enabled)
        return;

    m_showAlpha = enabled;
}

void ColorEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QPointer<QMenu> menu = createStandardContextMenu();
    if (!menu)
        return;

    menu->addSeparator();
    menu->addAction(tr("Pick Color"), this, &ColorEdit::pickColor);
    menu->exec(event->globalPos());
    delete static_cast<QMenu *>(menu);

    event->accept();
}

void ColorEdit::onTextEdited()
{
    QScopedValueRollback<bool> editing(m_editingText, true);

    const QString text = this->text();
    QColor color;

    if (!text.isEmpty()) {
        if (ColorValidator::isValidName(text))
            color = QColor(text);
        else if (ColorValidator::isValidName(QLatin1Char('#') + text))
            color = QColor(QLatin1Char('#') + text);
        else
            return;
    }

    setValue(color);
}

void ColorEdit::pickColor()
{
    QColorDialog::ColorDialogOptions options;
    if (m_showAlpha)
        options |= QColorDialog::ShowAlphaChannel;

    const QColor newColor = QColorDialog::getColor(m_color, this, QString(),
                                                   options);

    if (newColor.isValid() && newColor != m_color) {
        setValue(newColor);
        emit valueEdited();
    }
}


ElidingLabel::ElidingLabel(QWidget *parent)
    : ElidingLabel(QString(), parent)
{}

ElidingLabel::ElidingLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
}

/**
 * Sets a tool tip on the label.
 *
 * When a tool tip is set, it will be shown instead of the on-demand tool tip
 * that shows the full text when the text is elided.
 */
void ElidingLabel::setToolTip(const QString &toolTip)
{
    if (m_toolTip == toolTip)
        return;

    m_toolTip = toolTip;

    if (m_toolTip.isEmpty())
        QLabel::setToolTip(m_isElided ? text() : QString());
    else
        QLabel::setToolTip(m_toolTip);
}

void ElidingLabel::setSelected(bool selected)
{
    if (m_selected == selected)
        return;

    m_selected = selected;
    update();
}

QSize ElidingLabel::minimumSizeHint() const
{
    auto hint = QLabel::minimumSizeHint();
    hint.setWidth(std::min(hint.width(), Utils::dpiScaled(30)));
    return hint;
}

void ElidingLabel::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.initFrom(this);

    const int m = margin();
    const QRect cr = contentsRect().adjusted(m, m, -m, -m);
    const int align = QStyle::visualAlignment(opt.direction, alignment());
    const int flags = align | (opt.direction == Qt::LeftToRight ? Qt::TextForceLeftToRight
                                                                : Qt::TextForceRightToLeft);

    const auto elidedText = opt.fontMetrics.elidedText(text(), Qt::ElideRight, cr.width());
    const bool isElided = elidedText != text();

    if (isElided != m_isElided) {
        m_isElided = isElided;

        if (m_toolTip.isEmpty())
            QLabel::setToolTip(m_isElided ? text() : QString());
    }

    QStylePainter p(this);
    QPalette::ColorRole role = m_selected ? QPalette::HighlightedText : foregroundRole();
    p.drawItemText(cr, flags, opt.palette, isEnabled(), elidedText, role);
}


PropertyLabel::PropertyLabel(QWidget *parent)
    : ElidingLabel(parent)
{
    setMinimumWidth(Utils::dpiScaled(50));
    updateContentMargins();
}

void PropertyLabel::setLevel(int level)
{
    if (m_level == level)
        return;

    m_level = level;
    updateContentMargins();
}

void PropertyLabel::setHeader(bool header)
{
    if (m_header == header)
        return;

    m_header = header;
    setBackgroundRole(header ? QPalette::Dark : QPalette::NoRole);
    setForegroundRole(header ? QPalette::BrightText : QPalette::NoRole);
    setAutoFillBackground(header);
    updateContentMargins();
}

void PropertyLabel::setExpandable(bool expandable)
{
    if (m_expandable == expandable)
        return;

    m_expandable = expandable;
    update();
}

void PropertyLabel::setExpanded(bool expanded)
{
    if (m_expanded == expanded)
        return;

    m_expanded = expanded;
    update();
    emit toggled(m_expanded);
}

void PropertyLabel::setModified(bool modified)
{
    auto f = font();
    f.setBold(modified);
    setFont(f);
}

bool PropertyLabel::event(QEvent *event)
{
    switch (event->type()) {
    // Handled here instead of in mousePressEvent because we want it to be
    // expandable also when the label is disabled.
    case QEvent::MouseButtonPress: {
        const auto pos = static_cast<QMouseEvent *>(event)->pos();
        if (!isHeader() && !branchIndicatorRect().contains(pos))
            break;
    }
        [[fallthrough]];
    case QEvent::MouseButtonDblClick:
        if (m_expandable) {
            if (static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton) {
                setExpanded(!m_expanded);
                return true;
            }
        }
        break;

    case QEvent::LayoutDirectionChange:
        updateContentMargins();
        break;

    default:
        break;
    }

    return ElidingLabel::event(event);
}

void PropertyLabel::paintEvent(QPaintEvent *event)
{
    ElidingLabel::paintEvent(event);

    QStyleOption branchOption;
    branchOption.initFrom(this);
    branchOption.rect = branchIndicatorRect();

    if (isSelected())
        branchOption.state |= QStyle::State_Selected;
    if (m_expandable)
        branchOption.state |= QStyle::State_Children;
    if (m_expanded)
        branchOption.state |= QStyle::State_Open;

    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_IndicatorBranch, branchOption);

    if (m_header) {
        const QColor color = static_cast<QRgb>(p.style()->styleHint(QStyle::SH_Table_GridLineColor, &branchOption));
        p.save();
        p.setPen(QPen(color));
        p.drawLine(0, height() - 1, width(), height() - 1);
        p.restore();
    }
}

void PropertyLabel::updateContentMargins()
{
    const int spacing = Utils::dpiScaled(3);
    const int branchIndicatorWidth = Utils::dpiScaled(14);
    const int verticalSpacing = m_header ? spacing : 0;
    const int indent = branchIndicatorWidth * (m_level + 1);

    if (isLeftToRight())
        setContentsMargins(spacing + indent, verticalSpacing, spacing, verticalSpacing);
    else
        setContentsMargins(spacing, verticalSpacing, spacing + indent, verticalSpacing);
}

QRect PropertyLabel::branchIndicatorRect() const
{
    const int spacing = Utils::dpiScaled(3);
    const int branchIndicatorWidth = Utils::dpiScaled(14);
    const int indent = branchIndicatorWidth * m_level;

    if (layoutDirection() == Qt::LeftToRight) {
        return QRect(indent, 0,
                     branchIndicatorWidth + spacing, height());
    } else {
        return QRect(width() - indent - branchIndicatorWidth - spacing, 0,
                     branchIndicatorWidth + spacing, height());
    }
}

/**
 * To fit better alongside other widgets without vertical centering, the size
 * hint is adjusted to match that of a QLineEdit.
 */
QSize PropertyLabel::sizeHint() const
{
    constexpr int QLineEditPrivate_verticalMargin = 1;
    constexpr int QLineEditPrivate_horizontalMargin = 2;

    auto fm = fontMetrics();
    auto cm = contentsMargins();
    const int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this);
    int h = qMax(fm.height(), qMax(14, iconSize - 2)) + 2 * QLineEditPrivate_verticalMargin
            + cm.top() + cm.bottom();
    int w = fm.horizontalAdvance(u'x') * 17 + 2 * QLineEditPrivate_horizontalMargin
            + cm.left() + cm.right();
    QStyleOptionFrame opt;
    initStyleOption(&opt);
    return style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(w, h), this);
}


PropertyWidget::PropertyWidget(Property *property, QWidget *parent)
    : QWidget(parent)
    , m_property(property)
{
    setSelected(property->isSelected());

    connect(property, &Property::selectedChanged, this, &PropertyWidget::setSelected);
}

void PropertyWidget::setSelectable(bool selectable)
{
    if (m_selectable == selectable)
        return;

    m_selectable = selectable;

    if (!selectable)
        setSelected(false);
}

void PropertyWidget::setSelected(bool selected)
{
    if (m_selected == selected)
        return;

    m_selected = selected;
    update();
}

void PropertyWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    const auto halfSpacing = Utils::dpiScaled(2);
    const QRect r = rect().adjusted(halfSpacing, 0, -halfSpacing, 0);
    QStylePainter painter(this);

    if (isSelected())
        painter.fillRect(r, palette().highlight());

    if (hasFocus()) {
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.rect = r;
        option.backgroundColor = palette().color(backgroundRole());
        option.state |= QStyle::State_KeyboardFocusChange;

        painter.drawPrimitive(QStyle::PE_FrameFocusRect, option);
    }
}

void PropertyWidget::mousePressEvent(QMouseEvent *event)
{
    setFocus(Qt::MouseFocusReason);
    emit mousePressed(event->button(), event->modifiers());
}

} // namespace Tiled

#include "moc_propertyeditorwidgets.cpp"
#include "propertyeditorwidgets.moc"
