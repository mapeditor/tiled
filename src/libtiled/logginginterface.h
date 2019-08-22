/*
 * logginginterface.h
 * Copyright 2013, Samuli Tuomola <samuli.tuomola@gmail.com>
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "tiled_global.h"

#include <QObject>

#include <functional>

class QString;

namespace Tiled {

struct TILEDSHARED_EXPORT Issue
{
    enum Severity {
        Error,
        Warning
    };

    Issue();
    Issue(Severity severity, const QString &text);

    Severity severity() const { return mSeverity; }
    QString text() const { return mText; }

    std::function<void()> callback() const { return mCallback; }
    void setCallback(std::function<void()> callback, void *context = nullptr);

    void *context() const { return mContext; }

    unsigned id() const { return mId; }

    void addOccurrence(const Issue &issue);
    int occurrences() const { return mOccurrences; }

    bool operator==(const Issue &o) const
    {
        return severity() == o.severity()
                && text() == o.text();
    }

private:
    Issue::Severity mSeverity = Issue::Error;
    QString mText;
    std::function<void()> mCallback;
    void *mContext = nullptr;

    int mOccurrences = 1;
    unsigned mId = 0;

    static unsigned mNextIssueId;
};

/**
 * An interface for reporting issues.
 *
 * Normally you'd use the convenience functions in the Tiled namespace.
 */
class TILEDSHARED_EXPORT LoggingInterface : public QObject
{
    Q_OBJECT

    explicit LoggingInterface(QObject *parent = nullptr);

public:
    static LoggingInterface &instance();

    enum OutputType {
        INFO,
        WARNING,
        ERROR
    };

    void report(const Issue &issue);
    void log(OutputType type, const QString &message);

signals:
    void issue(const Issue &issue);

    void info(const QString &message);
    void warning(const QString &message);
    void error(const QString &message);
};

inline void INFO(const QString &message) { LoggingInterface::instance().log(LoggingInterface::INFO, message); }
inline void WARNING(const QString &message) { LoggingInterface::instance().report(Issue { Issue::Warning, message }); }
inline void ERROR(const QString &message) { LoggingInterface::instance().report(Issue { Issue::Error, message }); }

inline void INFO(QLatin1String message) { LoggingInterface::instance().log(LoggingInterface::INFO, message); }
inline void WARNING(QLatin1String message) { LoggingInterface::instance().report(Issue { Issue::Warning, message }); }
inline void ERROR(QLatin1String message) { LoggingInterface::instance().report(Issue { Issue::Error, message }); }

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Issue)
