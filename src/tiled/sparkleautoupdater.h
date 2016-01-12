/*
 * sparkleautoupdater.h
 *
 * License: MIT License (http://opensource.org/licenses/MIT)
 *   See sparkleautoupdater.mm
 */

#ifndef SPARKLEAUTOUPDATER_H
#define SPARKLEAUTOUPDATER_H

#include <QString>

class SparkleAutoUpdater
{
public:
    SparkleAutoUpdater();
    ~SparkleAutoUpdater();

    void checkForUpdates();

    void setAutomaticallyChecksForUpdates(bool on);
    bool automaticallyChecksForUpdates();

    QString lastUpdateCheckDate();

private:
    class Private;
    Private *d;
};

#endif // SPARKLEAUTOUPDATER_H
