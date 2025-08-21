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
#include <qqmlengine.h>

namespace Tiled {

ExpressionEvaluator *ExpressionEvaluator::mInstance;

ExpressionEvaluator &ExpressionEvaluator::instance()
{
    if (!mInstance)
    {
        mInstance = new ExpressionEvaluator;
        mInstance->mEngine = new QQmlEngine(mInstance);
    }
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

// ExpressionSpinBox

ExpressionSpinBox::ExpressionSpinBox(QWidget *parent)
    : QSpinBox(parent)
{

}

QJSValue ExpressionSpinBox::evaluate(const QString &text) const
{
    QString parseText = text;
    if (!prefix().isEmpty())
        parseText = parseText.replace(QRegularExpression(
                                          QString(QLatin1String("^") + QRegularExpression::escape(prefix()))),
                                      QString());
    if (!suffix().isEmpty())
        parseText = parseText.replace(QRegularExpression(
                                          QString(QRegularExpression::escape(suffix())) + QLatin1String("$")),
                                      QString());

    return ExpressionEvaluator::instance().evaluate(parseText);
}

int ExpressionSpinBox::valueFromText(const QString &text) const
{
    int originalValue = value();
    QJSValue result = this->evaluate(text);
    if (result.isNumber())
        return result.toNumber();

    return originalValue;
}

QValidator::State ExpressionSpinBox::validate(QString &text, int &pos) const
{
    return QValidator::Acceptable;
}

// ExpressionDoubleSpinBox
ExpressionDoubleSpinBox::ExpressionDoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent)
{

}

QJSValue ExpressionDoubleSpinBox::evaluate(const QString &text) const
{
    QString parseText = text;
    if (!prefix().isEmpty())
        parseText = parseText.replace(QRegularExpression(
                                          QString(QLatin1String("^") + QRegularExpression::escape(prefix()))),
                                      QString());
    if (!suffix().isEmpty())
        parseText = parseText.replace(QRegularExpression(
                                          QString(QRegularExpression::escape(suffix())) + QLatin1String("$")),
                                      QString());

    return ExpressionEvaluator::instance().evaluate(parseText);
}

double ExpressionDoubleSpinBox::valueFromText(const QString &text) const
{
    double originalValue = value();
    QJSValue result = this->evaluate(text);
    if (result.isNumber())
        return result.toNumber();

    return originalValue;
}

QValidator::State ExpressionDoubleSpinBox::validate(QString &text, int &pos) const
{
    return QValidator::Acceptable;
}

} // namespace Tiled

#include "moc_expressionspinbox.cpp"
