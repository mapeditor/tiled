/*
 * sentryhelper.h
 * Copyright 2021, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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
#ifdef TILED_SENTRY

#include "tilededitor_global.h"

#include <QObject>

namespace Tiled {

class TILED_EDITOR_EXPORT Sentry : public QObject
{
    Q_OBJECT

public:
    Sentry();
    ~Sentry();

    static Sentry *instance() { return sInstance; }

    // Matches sentry_user_consent_t in sentry.h
    enum UserConsent {
        ConsentUnknown = -1,
        ConsentGiven = 1,
        ConsentRevoked = 0,
    };

    UserConsent userConsent() const;
    void setUserConsent(UserConsent consent);

signals:
    void userConsentChanged(UserConsent consent);

private:
    static Sentry *sInstance;
};

} // namespace Tiled

#endif // TILED_SENTRY
