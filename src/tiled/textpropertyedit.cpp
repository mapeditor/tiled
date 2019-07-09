/*
 * textpropertyedit.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * Based loosely on the TextPropertyEditor and TextEditor classes from
 * Qt Designer (Copyright (C) 2015 The Qt Company Ltd., LGPLv2.1).
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

#include "textpropertyedit.h"

#include "texteditordialog.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QResizeEvent>
#include <QToolButton>
#include <QValidator>

#include <QDebug>

namespace Tiled {

namespace {

// A validator that replaces offending strings
class ReplacementValidator : public QValidator
{
public:
    ReplacementValidator(QObject *parent,
                         const QString &offending,
                         const QString &replacement)
        : QValidator(parent)
        , mOffending(offending)
        , mReplacement(replacement)
    {
    }

    void fixup(QString &input) const override
    {
        input.replace(mOffending, mReplacement);
    }

    State validate(QString &input, int &pos) const override
    {
        Q_UNUSED(pos);
        fixup(input);
        return Acceptable;
    }

private:
    const QString mOffending;
    const QString mReplacement;
};

} // anonymous namespace


QString escapeNewlines(const QString &string)
{
    if (string.isEmpty())
        return string;

    QString result(string);
    result.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));  // protect backslashes
    result.replace(QLatin1Char('\n'), QStringLiteral("\\n"));   // escape newlines
    return result;
}

// Note: As the properties are updated while the user types, it is important
// that trailing slashes ('bla\') are not deleted nor ignored, else this will
// cause jumping of the cursor
static QString unescapeNewlines(const QString &string)
{
    if (string.isEmpty())
        return string;

    QString result(string);
    for (int pos = 0; (pos = result.indexOf(QLatin1Char('\\'), pos)) >= 0;) {
        // found an escaped character. If not a newline or at end of string,
        // leave as is, else insert '\n'
        const int nextpos = pos + 1;
        if (nextpos >= result.length())  // trailing '\'
             break;
        // escaped newline
        if (result.at(nextpos) == QChar(QLatin1Char('n')))
             result[nextpos] = QChar(QLatin1Char('\n'));
        // remove escape, go past escaped
        result.remove(pos, 1);
        pos++;
    }
    return result;
}


TextPropertyEdit::TextPropertyEdit(QWidget *parent)
    : QWidget(parent)
    , mLineEdit(new QLineEdit(this))
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    setFocusProxy(mLineEdit);

    QToolButton *button = new QToolButton(this);
    button->setText(tr("..."));
    button->setAutoRaise(true);

    // Set a validator that replaces newline characters by literal "\\n".
    // While it is not possible to actually type newline characters, they
    // can be pasted into the line edit.
    //
    // This is the approach taken by Qt Designer. Identified problems:
    //
    // * After pasting text with newlines, the cursor does not sit at the end
    //   of the pasted text.
    //
    // * After pasting text with newlines, undo is no longer available.
    //
    mLineEdit->setValidator(new ReplacementValidator(mLineEdit,
                                                     QStringLiteral("\n"),
                                                     QStringLiteral("\\n")));

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mLineEdit);
    layout->addWidget(button);

    connect(button, &QToolButton::clicked, this, &TextPropertyEdit::onButtonClicked);
    connect(mLineEdit, &QLineEdit::textChanged, this, &TextPropertyEdit::onTextChanged);
}

QString TextPropertyEdit::text() const
{
    return mCachedText;
}

void TextPropertyEdit::setText(const QString &text)
{
    if (mCachedText == text)
        return;

    mCachedText = text;
    mLineEdit->setText(escapeNewlines(text));
}

void TextPropertyEdit::onTextChanged(const QString &text)
{
    mCachedText = unescapeNewlines(text);
    emit textChanged(mCachedText);
}

void TextPropertyEdit::onButtonClicked()
{
    TextEditorDialog dialog(this);
    dialog.setText(mCachedText);

    if (dialog.exec() != QDialog::Accepted)
        return;

    QString newText = dialog.text();

    if (newText != mCachedText) {
        setText(newText);
        emit textChanged(mCachedText);
    }
}

} // namespace Tiled
