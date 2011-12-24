#ifndef MACSUPPORT_H
#define MACSUPPORT_H

#include "mainwindow.h"

class MacSupport
{
public:
    static bool isLion(); // checks if tiled is running on Lion
    static void addFullscreen(Tiled::Internal::MainWindow *window); // adds fullscreen button to window for lion
};

#endif // MACSUPPORT_H
