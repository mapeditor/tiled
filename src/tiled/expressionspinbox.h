/*
 * expressionspinbox.h
 * Copyright 2025, dogboydog
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

#include <QJSValue>
#include <QSpinBox>

namespace Tiled {

/**
 * @brief The ExpressionEvaluator class can evaluate simple Javascript expressions
 *        that do not require access to the Tiled scripting API, nor the Tiled project.
 */
class ExpressionEvaluator
{
public:
    static void deleteInstance();
    static QJSValue evaluate(const QString &program);

private:
    static QJSEngine *mEngine;
};


class ExpressionSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    ExpressionSpinBox(QWidget *parent);

protected:
    int valueFromText(const QString &text) const override;
    QValidator::State validate(QString &text, int &pos) const override;

    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    QString mPrefix;
    QString mSuffix;
};


class ExpressionDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    ExpressionDoubleSpinBox(QWidget *parent);

protected:
    double valueFromText(const QString &text) const override;
    QValidator::State validate(QString &text, int &pos) const override;

    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    QString mPrefix;
    QString mSuffix;
};

} // namespace Tiled
