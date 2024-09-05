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
 * A widget that shows label/widget pairs, wrapping them either two per row
 * or each on their own row, depending on the available space.
 */
class ResponsivePairswiseWidget : public QWidget
{
    Q_OBJECT

public:
    struct WidgetPair {
        QLabel *label;
        QWidget *widget;
    };

    ResponsivePairswiseWidget(QWidget *parent = nullptr);

    void setWidgetPairs(const QVector<WidgetPair> &widgetPairs);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void addWidgetsToLayout();
    int minimumHorizontalWidth() const;

    Qt::Orientation m_orientation = Qt::Horizontal;
    QVector<WidgetPair> m_widgetPairs;
};

/**
 * A widget for editing a QSize value.
 */
class SizeEdit : public ResponsivePairswiseWidget
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
    QLabel *m_widthLabel;
    QLabel *m_heightLabel;
    SpinBox *m_widthSpinBox;
    SpinBox *m_heightSpinBox;
};

/**
 * A widget for editing a QSizeF value.
 */
class SizeFEdit : public ResponsivePairswiseWidget
{
    Q_OBJECT
    Q_PROPERTY(QSizeF value READ value WRITE setValue NOTIFY valueChanged FINAL)

public:
    SizeFEdit(QWidget *parent = nullptr);

    void setValue(const QSizeF &size);
    QSizeF value() const;

signals:
    void valueChanged();

private:
    QLabel *m_widthLabel;
    QLabel *m_heightLabel;
    DoubleSpinBox *m_widthSpinBox;
    DoubleSpinBox *m_heightSpinBox;
};

/**
 * A widget for editing a QPoint value.
 */
class PointEdit : public ResponsivePairswiseWidget
{
    Q_OBJECT
    Q_PROPERTY(QPoint value READ value WRITE setValue NOTIFY valueChanged FINAL)

public:
    PointEdit(QWidget *parent = nullptr);

    void setValue(const QPoint &size);
    QPoint value() const;

signals:
    void valueChanged();

private:
    QLabel *m_xLabel;
    QLabel *m_yLabel;
    SpinBox *m_xSpinBox;
    SpinBox *m_ySpinBox;
};

/**
 * A widget for editing a QPointF value.
 */
class PointFEdit : public ResponsivePairswiseWidget
{
    Q_OBJECT
    Q_PROPERTY(QPointF value READ value WRITE setValue NOTIFY valueChanged FINAL)

public:
    PointFEdit(QWidget *parent = nullptr);

    void setValue(const QPointF &size);
    QPointF value() const;

signals:
    void valueChanged();

private:
    QLabel *m_xLabel;
    QLabel *m_yLabel;
    DoubleSpinBox *m_xSpinBox;
    DoubleSpinBox *m_ySpinBox;
};

/**
 * A widget for editing a QRectF value.
 */
class RectFEdit : public ResponsivePairswiseWidget
{
    Q_OBJECT
    Q_PROPERTY(QRectF value READ value WRITE setValue NOTIFY valueChanged FINAL)

public:
    RectFEdit(QWidget *parent = nullptr);

    void setValue(const QRectF &size);
    QRectF value() const;

signals:
    void valueChanged();

private:
    QLabel *m_xLabel;
    QLabel *m_yLabel;
    QLabel *m_widthLabel;
    QLabel *m_heightLabel;
    DoubleSpinBox *m_xSpinBox;
    DoubleSpinBox *m_ySpinBox;
    DoubleSpinBox *m_widthSpinBox;
    DoubleSpinBox *m_heightSpinBox;
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
