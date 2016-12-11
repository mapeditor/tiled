/*
 * sparkleautoupdater.mm
 *
 * Copyright (C) 2008 Remko Troncon.
 *   See https://github.com/remko/mixing-cocoa-and-qt
 *       https://el-tramo.be/blog/mixing-cocoa-and-qt
 *   License: BSD. Assuming 2-clause BSD reference by Sparke based on context:
 *       https://github.com/sparkle-project/Sparkle/blob/master/LICENSE
 * Copyright (C) 2013 Marius Kintel.
 *   License: MIT License (http://opensource.org/licenses/MIT)
 * Copyright (C) 2016 Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 */

#include "sparkleautoupdater.h"

#include <Cocoa/Cocoa.h>
#include <Sparkle/Sparkle.h>

#ifdef TILED_SNAPSHOT
static const char appcastUrl[] = "http://update.mapeditor.org/appcast-osx-snapshots.xml";
#else
static const char appcastUrl[] = "http://update.mapeditor.org/appcast-osx.xml";
#endif


class SparkleAutoUpdater::Private
{
public:
    NSAutoreleasePool *autoReleasePool;
    SUUpdater *updater;
};


SparkleAutoUpdater::SparkleAutoUpdater()
    : d(new Private)
{
    NSApplicationLoad();
    d->autoReleasePool = [[NSAutoreleasePool alloc] init];

    d->updater = [SUUpdater sharedUpdater];
    [d->updater retain];

    NSURL* url = [NSURL URLWithString:
            [NSString stringWithUTF8String: appcastUrl]];
    [d->updater setFeedURL: url];
}

SparkleAutoUpdater::~SparkleAutoUpdater()
{
    [d->updater release];
    [d->autoReleasePool release];
    delete d;
}

/**
 * Checks for updates using Sparkle.
 */
void SparkleAutoUpdater::checkForUpdates()
{
    [d->updater checkForUpdates:nil];
}

void SparkleAutoUpdater::setAutomaticallyChecksForUpdates(bool on)
{
    [d->updater setAutomaticallyChecksForUpdates:on];
}

bool SparkleAutoUpdater::automaticallyChecksForUpdates()
{
    return [d->updater automaticallyChecksForUpdates];
}

void SparkleAutoUpdater::setAutomaticallyDownloadsUpdates(bool on)
{
    [d->updater setAutomaticallyDownloadsUpdates:on];
}

bool SparkleAutoUpdater::automaticallyDownloadsUpdates()
{
    return [d->updater automaticallyDownloadsUpdates];
}

QDateTime SparkleAutoUpdater::lastUpdateCheckDate()
{
    NSDate *date = [d->updater lastUpdateCheckDate];
    NSTimeInterval timeInterval = [date timeIntervalSince1970];
    return QDateTime::fromMSecsSinceEpoch(timeInterval * 1000);
}
