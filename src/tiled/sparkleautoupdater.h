/*
 * sparkleautoupdater.h
 *
 * License: MIT License (http://opensource.org/licenses/MIT)
 *   See sparkleautoupdater.mm
 */

#pragma once

#include "autoupdater.h"

class SparkleAutoUpdater : public AutoUpdater
{
public:
    SparkleAutoUpdater();
    ~SparkleAutoUpdater();

    void checkForUpdates() override;

    void setAutomaticallyChecksForUpdates(bool on) override;
    bool automaticallyChecksForUpdates() override;

    void setAutomaticallyDownloadsUpdates(bool on);
    bool automaticallyDownloadsUpdates();

    QDateTime lastUpdateCheckDate() override;

private:
    class Private;
    Private *d;
};
