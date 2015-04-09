/*
 * macsupport.mm
 * Copyright 2011, Vsevolod Klementjev <klemix@inbox.lv>
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

#include "macsupport.h"

#import <Foundation/NSString.h>
#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>

bool MacSupport::isLion()
{
    NSString *string = [NSString string];
    // this selector was added only in Lion. so we can check if it's responding, we are on Lion
    return [string respondsToSelector:@selector(linguisticTagsInRange:scheme:options:orthography:tokenRanges:)];
}

void MacSupport::addFullscreen(Tiled::Internal::MainWindow *window)
{
#if defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    if (isLion()) // checks if lion is running
    {
        NSView *nsview = (NSView *) window->winId();
        NSWindow *nswindow = [nsview window];
        [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    }
#else
#warning No fullscreen support will be included in this build
#endif
}
