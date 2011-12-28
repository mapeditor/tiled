#include "macsupport.h"

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
#warning no fullscreen support will be included in this build
#endif
}
