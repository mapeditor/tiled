/*
 * sentryhelper.cpp
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

#include "sentryhelper.h"

#include <QDir>
#include <QStandardPaths>

#include <sentry.h>

#define STRINGIFY(x) #x
#define AS_STRING(x) STRINGIFY(x)

namespace Tiled {

Sentry *Sentry::sInstance;

Sentry::Sentry()
{
    sInstance = this;

    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, "https://6c72ea2c9d024333bae90e40bc1d41e0@o326665.ingest.sentry.io/1835065");
    sentry_options_set_require_user_consent(options, true);
    sentry_options_set_release(options, "tiled@" AS_STRING(TILED_VERSION));
#ifdef QT_DEBUG
    sentry_options_set_symbolize_stacktraces(options, true);
    sentry_options_set_debug(options, true);
#endif

    const QString cacheLocation { QStandardPaths::writableLocation(QStandardPaths::CacheLocation) };
    if (!cacheLocation.isEmpty()) {
        const QString databasePath = QDir{cacheLocation}.filePath(QStringLiteral("sentry"));
        sentry_options_set_database_path(options, databasePath.toLocal8Bit().constData());
    }

    sentry_init(options);
}

Sentry::~Sentry()
{
    sentry_shutdown();
    sInstance = nullptr;
}

Sentry::UserConsent Sentry::userConsent() const
{
    return static_cast<UserConsent>(sentry_user_consent_get());
}

void Sentry::setUserConsent(UserConsent consent)
{
    if (userConsent() == consent)
        return;

    switch (consent) {
    case ConsentUnknown:
        sentry_user_consent_reset();
        break;
    case ConsentGiven:
        sentry_user_consent_give();
        break;
    case ConsentRevoked:
        sentry_user_consent_revoke();
        break;
    }

    emit userConsentChanged(consent);
}

} // namespace Tiled
