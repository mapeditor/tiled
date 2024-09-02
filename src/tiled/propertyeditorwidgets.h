#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

class QLabel;

namespace Tiled {

/**
 * A spin box that allows the full range by default and shrinks horizontally.
 * It also doesn't adjust the horizontal size hint based on the maximum value.
 */
class SpinBox : public QSpinBox
{
    Q_OBJECT

public:
    SpinBox(QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
};

/**
 * A double spin box that allows the full range by default and shrinks
 * horizontally. It also doesn't adjust the horizontal size hint based on the
 * maximum value.
 *
 * The precision is increased to 9 decimal places. Redundant trailing 0's are
 * removed.
 */
class DoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    DoubleSpinBox(QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    QString textFromValue(double val) const override;
};

/**
 * A widget for editing a QSize value.
 */
class SizeEdit : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QSize value READ value WRITE setValue NOTIFY valueChanged FINAL)

public:
    SizeEdit(QWidget *parent = nullptr);

    void setValue(const QSize &size);
    QSize value() const;

signals:
    void valueChanged();

private:
    void resizeEvent(QResizeEvent *event) override;

    int minimumHorizontalWidth() const;

    Qt::Orientation m_orientation = Qt::Horizontal;
    QLabel *m_widthLabel;
    QLabel *m_heightLabel;
    SpinBox *m_widthSpinBox;
    SpinBox *m_heightSpinBox;
};

/**
 * A label that elides its text if there is not enough space.
 *
 * The elided text is shown as a tooltip.
 */
class ElidingLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ElidingLabel(QWidget *parent = nullptr);
    ElidingLabel(const QString &text, QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    void paintEvent(QPaintEvent *) override;

private:
    bool m_isElided = false;
};

/**
 * A label that matches its preferred height with that of a line edit.
 */
class LineEditLabel : public ElidingLabel
{
    Q_OBJECT

public:
    using ElidingLabel::ElidingLabel;

    QSize sizeHint() const override;

private:
    QLineEdit m_lineEdit;
};

} // namespace Tiled
