/*
 * propertyeditorwidgets.h
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

    void setMinimum(int minimum);
    void setSuffix(const QString &suffix);

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

    void setSuffix(const QString &suffix);

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

    void setSingleStep(double step);

signals:
    void valueChanged();

private:
    QLabel *m_xLabel;
    QLabel *m_yLabel;
    DoubleSpinBox *m_xSpinBox;
    DoubleSpinBox *m_ySpinBox;
};

/**
 * A widget for editing a QRect value.
 */
class RectEdit : public ResponsivePairswiseWidget
{
    Q_OBJECT
    Q_PROPERTY(QRect value READ value WRITE setValue NOTIFY valueChanged FINAL)

public:
    RectEdit(QWidget *parent = nullptr);

    void setValue(const QRect &size);
    QRect value() const;

    void setConstraint(const QRect &constraint);

signals:
    void valueChanged();

private:
    QLabel *m_xLabel;
    QLabel *m_yLabel;
    QLabel *m_widthLabel;
    QLabel *m_heightLabel;
    SpinBox *m_xSpinBox;
    SpinBox *m_ySpinBox;
    SpinBox *m_widthSpinBox;
    SpinBox *m_heightSpinBox;
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

protected:
    void paintEvent(QPaintEvent *) override;

private:
    bool m_isElided = false;
};

/**
 * A property label widget, which can be a header or just be expandable.
 */
class PropertyLabel : public ElidingLabel
{
    Q_OBJECT

public:
    PropertyLabel(int level, QWidget *parent = nullptr);

    void setLevel(int level);

    void setHeader(bool header);
    bool isHeader() const { return m_header; }

    void setExpandable(bool expandable);
    bool isExpandable() const { return m_expandable; }

    void setExpanded(bool expanded);
    bool isExpanded() const { return m_expanded; }

    void setModified(bool modified);

    QSize sizeHint() const override;

signals:
    void toggled(bool expanded);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *) override;

private:
    QLineEdit m_lineEdit;
    int m_level = 0;
    bool m_header = false;
    bool m_expandable = false;
    bool m_expanded = false;
};

} // namespace Tiled
