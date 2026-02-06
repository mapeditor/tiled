/*
 * expressionspinbox.cpp
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

#include "expressionspinbox.h"

#include <QJSEngine>

namespace Tiled {

QJSEngine *ExpressionEvaluator::mEngine;

void ExpressionEvaluator::deleteInstance()
{
    delete mEngine;
    mEngine = nullptr;
}

QJSValue ExpressionEvaluator::evaluate(const QString &program)
{
    if (!mEngine)
        mEngine = new QJSEngine;
    return mEngine->evaluate(program);
}


// ExpressionSpinBox

ExpressionSpinBox::ExpressionSpinBox(QWidget *parent)
    : QSpinBox(parent)
{}

int ExpressionSpinBox::valueFromText(const QString &text) const
{
    const QJSValue result = ExpressionEvaluator::evaluate(text);
    if (result.isNumber())
        return result.toNumber();

    return value();
}

QValidator::State ExpressionSpinBox::validate(QString &/*text*/, int &/*pos*/) const
{
    return QValidator::Acceptable;
}

void ExpressionSpinBox::focusInEvent(QFocusEvent *event)
{
    // Remember current prefix/suffix and remove them
    mPrefix = prefix();
    mSuffix = suffix();
    if (!mPrefix.isEmpty())
        setPrefix(QString());
    if (!mSuffix.isEmpty())
        setSuffix(QString());

    QSpinBox::focusInEvent(event);
}

void ExpressionSpinBox::focusOutEvent(QFocusEvent *event)
{
    QSpinBox::focusOutEvent(event);

    // Restore any previously removed prefix/suffix
    if (!mPrefix.isEmpty())
        setPrefix(mPrefix);
    if (!mSuffix.isEmpty())
        setSuffix(mSuffix);
    mPrefix.clear();
    mSuffix.clear();
}

// ExpressionDoubleSpinBox

ExpressionDoubleSpinBox::ExpressionDoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent)
{}

double ExpressionDoubleSpinBox::valueFromText(const QString &text) const
{
    const QJSValue result = ExpressionEvaluator::evaluate(text);
    if (result.isNumber())
        return result.toNumber();

    return value();
}

QValidator::State ExpressionDoubleSpinBox::validate(QString &/*text*/, int &/*pos*/) const
{
    return QValidator::Acceptable;
}

void ExpressionDoubleSpinBox::focusInEvent(QFocusEvent *event)
{
    // Remember current prefix/suffix and remove them
    mPrefix = prefix();
    mSuffix = suffix();
    if (!mPrefix.isEmpty())
        setPrefix(QString());
    if (!mSuffix.isEmpty())
        setSuffix(QString());

    QDoubleSpinBox::focusInEvent(event);
}

void ExpressionDoubleSpinBox::focusOutEvent(QFocusEvent *event)
{
    QDoubleSpinBox::focusOutEvent(event);

    // Restore any previously removed prefix/suffix
    if (!mPrefix.isEmpty())
        setPrefix(mPrefix);
    if (!mSuffix.isEmpty())
        setSuffix(mSuffix);
    mPrefix.clear();
    mSuffix.clear();
}

} // namespace Tiled

#include "moc_expressionspinbox.cpp"
