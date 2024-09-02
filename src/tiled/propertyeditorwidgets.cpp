#include "propertyeditorwidgets.h"

#include "utils.h"

#include <QGridLayout>
#include <QPainter>
#include <QResizeEvent>
#include <QStyle>
#include <QStyleOption>

namespace Tiled {

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


SpinBox::SpinBox(QWidget *parent)
    : QSpinBox(parent)
{
    // Allow the full range by default.
    setRange(std::numeric_limits<int>::lowest(),
             std::numeric_limits<int>::max());

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


DoubleSpinBox::DoubleSpinBox(QWidget *parent)
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

QSize DoubleSpinBox::minimumSizeHint() const
{
    // Don't adjust the horizontal size hint based on the maximum value.
    auto hint = QDoubleSpinBox::minimumSizeHint();
    hint.setWidth(Utils::dpiScaled(50));
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


SizeEdit::SizeEdit(QWidget *parent)
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
    return QSize(m_widthSpinBox->value(), m_heightSpinBox->value());
}

void SizeEdit::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    const auto orientation = event->size().width() < minimumHorizontalWidth()
            ? Qt::Vertical : Qt::Horizontal;

    if (m_orientation != orientation) {
        m_orientation = orientation;

        auto layout = qobject_cast<QGridLayout *>(this->layout());

        // Remove all widgets from layout, without deleting them
        layout->removeWidget(m_widthLabel);
        layout->removeWidget(m_widthSpinBox);
        layout->removeWidget(m_heightLabel);
        layout->removeWidget(m_heightSpinBox);

        if (orientation == Qt::Horizontal) {
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

int SizeEdit::minimumHorizontalWidth() const
{
    return m_widthLabel->minimumSizeHint().width() +
            m_widthSpinBox->minimumSizeHint().width() +
            m_heightLabel->minimumSizeHint().width() +
            m_heightSpinBox->minimumSizeHint().width() +
            layout()->spacing() * 3;
}


ElidingLabel::ElidingLabel(QWidget *parent)
    : ElidingLabel(QString(), parent)
{}

ElidingLabel::ElidingLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
}

QSize ElidingLabel::minimumSizeHint() const
{
    auto hint = QLabel::minimumSizeHint();
    hint.setWidth(std::min(hint.width(), Utils::dpiScaled(30)));
    return hint;
}

void ElidingLabel::paintEvent(QPaintEvent *)
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


QSize LineEditLabel::sizeHint() const
{
    auto hint = ElidingLabel::sizeHint();
    hint.setHeight(m_lineEdit.sizeHint().height());
    return hint;
}

} // namespace Tiled

#include "moc_propertyeditorwidgets.cpp"
