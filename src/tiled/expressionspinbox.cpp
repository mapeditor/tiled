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

ExpressionEvaluator *ExpressionEvaluator::mInstance;

ExpressionEvaluator &ExpressionEvaluator::instance()
{
    if (!mInstance)
        mInstance = new ExpressionEvaluator;
    return *mInstance;
}

void ExpressionEvaluator::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

QJSValue ExpressionEvaluator::evaluate(const QString &program)
{
    return mEngine->evaluate(program);
}

ExpressionEvaluator::ExpressionEvaluator()
    : mEngine(new QJSEngine(this))
{}


template<typename SpinBox>
static QJSValue evaluate(SpinBox *spinBox, const QString &text)
{
    QString parseText = text;

    if (const QString p = spinBox->prefix(); !p.isEmpty() && parseText.startsWith(p))
        parseText.remove(0, p.length());

    if (const QString s = spinBox->suffix(); !s.isEmpty() && parseText.endsWith(s))
        parseText.chop(s.length());

    return ExpressionEvaluator::instance().evaluate(parseText);
}

template<typename SpinBox>
static QValidator::State validate(SpinBox *spinBox, const QString &text)
{
    if (evaluate(spinBox, text).isNumber())
        return QValidator::Acceptable;
    return QValidator::Intermediate;
}

// ExpressionSpinBox

ExpressionSpinBox::ExpressionSpinBox(QWidget *parent)
    : QSpinBox(parent)
{}

int ExpressionSpinBox::valueFromText(const QString &text) const
{
    const QJSValue result = evaluate(this, text);
    if (result.isNumber())
        return result.toNumber();

    return value();
}

QValidator::State ExpressionSpinBox::validate(QString &text, int &/*pos*/) const
{
    return Tiled::validate(this, text);
}


// ExpressionDoubleSpinBox

ExpressionDoubleSpinBox::ExpressionDoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent)
{}

double ExpressionDoubleSpinBox::valueFromText(const QString &text) const
{
    const QJSValue result = evaluate(this, text);
    if (result.isNumber())
        return result.toNumber();

    return value();
}

QValidator::State ExpressionDoubleSpinBox::validate(QString &text, int &/*pos*/) const
{
    return Tiled::validate(this, text);
}

} // namespace Tiled

#include "moc_expressionspinbox.cpp"
