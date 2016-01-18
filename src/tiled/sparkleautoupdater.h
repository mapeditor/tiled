/*
 * sparkleautoupdater.h
 *
 * License: MIT License (http://opensource.org/licenses/MIT)
 *   See sparkleautoupdater.mm
 */

#ifndef SPARKLEAUTOUPDATER_H
#define SPARKLEAUTOUPDATER_H

#include "autoupdater.h"

class SparkleAutoUpdater : public AutoUpdater
{
public:
    SparkleAutoUpdater();
    ~SparkleAutoUpdater();

    void checkForUpdates() override;

    void setAutomaticallyChecksForUpdates(bool on) override;
    bool automaticallyChecksForUpdates() override;

    QString lastUpdateCheckDate() override;

private:
    class Private;
    Private *d;
};

#endif // SPARKLEAUTOUPDATER_H
