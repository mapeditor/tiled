/*
 * commandlineparser.h
 * Copyright 2011, Ben Longbons <b.r.longbons@gmail.com>
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QStringList>
#include <QVector>

namespace Tiled {
namespace Internal {

/**
 * C-style callback function taking an arbitrary data pointer.
 */
typedef void (*Callback)(void *data);

/**
 * A template function that will static-cast the given \a object to a type T
 * and call the member function of T given in the second template argument.
 */
template<typename T, void (T::*memberFunction)()>
void MemberFunctionCall(void *object)
{
    T *t = static_cast<T*>(object);
    (t->*memberFunction)();
}


/**
 * A simple command line parser. Options should be registered through
 * registerOption().
 *
 * The help option (-h/--help) is provided by the parser based on the
 * registered options.
 */
class CommandLineParser
{

public:
    CommandLineParser();

    /**
     * Registers an option with the parser. When an option with the given
     * \a shortName or \a longName is encountered, \a callback is called with
     * \a data as its only parameter.
     */
    void registerOption(Callback callback,
                        void *data,
                        QChar shortName,
                        const QString &longName,
                        const QString &help);

    /**
     * Convenience overload that allows registering an option with a callback
     * as a member function of a class. The class type and the member function
     * are given as template parameters, while the instance is passed in as
     * \a handler.
     *
     * \overload
     */
    template <typename T, void (T::*memberFunction)()>
    void registerOption(T *handler,
                        QChar shortName,
                        const QString &longName,
                        const QString &help)
    {
        registerOption(&MemberFunctionCall<T, memberFunction>,
                       handler,
                       shortName,
                       longName,
                       help);
    }

    /**
     * Parses the given \a arguments. Returns false when the application is not
     * expected to run (either there was a parsing error, or the help was
     * requested).
     */
    bool parse(const QStringList &arguments);

    /**
     * Returns the files to open that were found among the arguments.
     */
    const QStringList &filesToOpen() const { return mFilesToOpen; }

private:
    void showHelp();

    bool handleLongOption(const QString &longName);
    bool handleShortOption(QChar c);

    /**
     * Internal definition of a command line option.
     */
    struct Option
    {
        Option()
            : callback(0)
            , data(0)
        {}

        Option(Callback callback,
               void *data,
               QChar shortName,
               const QString &longName,
               const QString &help)
            : callback(callback)
            , data(data)
            , shortName(shortName)
            , longName(longName)
            , help(help)
        {}

        Callback callback;
        void *data;
        QChar shortName;
        QString longName;
        QString help;
    };

    QVector<Option> mOptions;
    int mLongestArgument;
    QString mCurrentProgramName;
    QStringList mFilesToOpen;
    bool mShowHelp;
};

} // namespace Internal
} // namespace Tiled

#endif // COMMANDLINEPARSER_H
