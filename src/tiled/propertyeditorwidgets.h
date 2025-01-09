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

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QSpinBox>

class QAction;
class QLabel;

namespace Tiled {

class Property;

/**
 * A slider that doesn't respond to wheel events when not focused.
 */
class Slider : public QSlider
{
    Q_OBJECT

public:
    using QSlider::QSlider;

protected:
    void wheelEvent(QWheelEvent *event) override;
};

/**
 * A line edit that doesn't override global undo/redo shortcuts.
 */
class LineEdit : public QLineEdit
{
    Q_OBJECT

public:
    using QLineEdit::QLineEdit;

protected:
    bool event(QEvent *event) override;
};

/**
 * A combo box that doesn't respond to wheel events when not focused and
 * doesn't override global undo/redo shortcuts.
 */
class ComboBox : public QComboBox
{
    Q_OBJECT

public:
    ComboBox(QWidget *parent = nullptr);

signals:
    /** This signal is emitted when the Return or Enter key is pressed. */
    void returnPressed();

protected:
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
};

/**
 * A spin box that allows the full range by default and shrinks horizontally.
 * It also doesn't adjust the horizontal size hint based on the maximum value
 * and doesn't respond to wheel events when not focused.
 */
class SpinBox : public QSpinBox
{
    Q_OBJECT

public:
    SpinBox(QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;

protected:
    bool event(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
};

/**
 * A double spin box that allows the full range by default and shrinks
 * horizontally. It also doesn't adjust the horizontal size hint based on the
 * maximum value and doesn't respond to wheel events when not focused.
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

protected:
    bool event(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
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
class SizeFEdit : public QWidget
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
class PointEdit : public QWidget
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
class PointFEdit : public QWidget
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
class RectEdit : public QWidget
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
class RectFEdit : public QWidget
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
 * A widget for editing a QColor value.
 */
class ColorEdit : public LineEdit
{
    Q_OBJECT
    Q_PROPERTY(QColor value READ value WRITE setValue NOTIFY valueEdited FINAL)

public:
    ColorEdit(QWidget *parent = nullptr);

    void setValue(const QColor &color);
    QColor value() const { return m_color; }

    void setShowAlpha(bool enabled);

signals:
    void valueEdited();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void onTextEdited();
    void pickColor();

    QColor m_color;
    bool m_showAlpha = false;
    bool m_editingText = false;
    QAction *m_colorIconAction;
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

    void setToolTip(const QString &toolTip);

    void setSelected(bool selected);
    bool isSelected() const { return m_selected; }

    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QString m_toolTip;
    bool m_isElided = false;
    bool m_selected = false;
};

/**
 * A property label widget, which can be a header or just be expandable.
 */
class PropertyLabel : public ElidingLabel
{
    Q_OBJECT

public:
    PropertyLabel(QWidget *parent = nullptr);

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
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *) override;

private:
    void updateContentMargins();
    QRect branchIndicatorRect() const;

    int m_level = 0;
    bool m_header = false;
    bool m_expandable = false;
    bool m_expanded = false;
};

/**
 * A widget that represents a single property.
 */
class PropertyWidget : public QWidget
{
    Q_OBJECT

public:
    PropertyWidget(Property *property, QWidget *parent = nullptr);

    Property *property() const { return m_property; }

    void setSelectable(bool selectable);
    bool isSelectable() const { return m_selectable; }

    void setSelected(bool selected);
    bool isSelected() const { return m_selected; }

signals:
    void mousePressed(Qt::MouseButton button, Qt::KeyboardModifiers modifiers);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    // Protected by QPointer because PropertyWidget might outlive the Property
    // for a short moment, due to delayed widget deletion.
    QPointer<Property> m_property;
    bool m_selectable = false;
    bool m_selected = false;
};

} // namespace Tiled
