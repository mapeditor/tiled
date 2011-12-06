#include "oslion.h"

void OSLion::addFullscreen(Tiled::Internal::MainWindow *window)
{
    NSString *string = [NSString string];

    if ([string respondsToSelector:@selector(linguisticTagsInRange:scheme:options:orthography:tokenRanges:)]) // checks if lion is running
    {
        NSView *nsview = (NSView *) window->winId();
        NSWindow *nswindow = [nsview window];
        [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    }
}
