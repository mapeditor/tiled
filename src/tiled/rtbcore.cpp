/*
 * rtbcore.cpp
 * Copyright 2016, David Stammer
 *
 * This file is part of Road to Ballhalla Editor.
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

#include "rtbcore.h"

#include "preferences.h"
#include "mapdocument.h"
#include "mapobjectitem.h"

#include <QSettings>
#include <QDir>
#include <QProcess>
#include <Windows.h>
#include <Psapi.h>

using namespace Tiled;
using namespace Internal;

RTBCore *RTBCore::mInstance = 0;

const QString RTBCore::mGameExe = QLatin1String("RoadToBallhallaUE4.exe");
const QString RTBCore::mGameShippingExe = QLatin1String("RoadToBallhallaUE4-Win64-Shipping.exe");

const QString steamRegPath = QLatin1String("HKEY_CURRENT_USER\\Software\\Valve\\Steam");
const QString steamPathValue = QLatin1String("SteamPath");
const QString steamAppsPath = QLatin1String("SteamApps");
const QString steamRTBPath = QLatin1String("common/Road to Ballhalla");
const QString steamBibFile = QLatin1String("libraryfolders.vdf");

RTBCore::RTBCore()
{
}

RTBCore *RTBCore::instance()
{
    if (!mInstance)
        mInstance = new RTBCore;
    return mInstance;
}

void RTBCore::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

QString RTBCore::findGameDirectory()
{
    QDir dir(QCoreApplication::applicationDirPath());
    if(dir.cdUp())
    {
        if(dir.cd(QLatin1String("Game"))){
            if(dir.entryList().contains(mGameExe)){
                return dir.absoluteFilePath(mGameExe);
            }
        }

    }


    // Search in all Steam Bibs
    /*
    // search in the default location
    QDir dir(QDir::current());
    QString path;

    if(dir.cdUp())
    {
        path = createPath(dir);
        if(!path.isEmpty())
            return path;
    }

    // search for steam bibs and check if the game is in one of them
    QSettings regSettings(steamRegPath, QSettings::NativeFormat);
    QString steamPath = regSettings.value(steamPathValue).toString();
    QString steamAppPaths = steamPath + QLatin1String("/") + steamAppsPath;

    // default steam bib
    dir.setPath(steamAppPaths + QLatin1String("/") + steamRTBPath);

    path = createPath(dir);
    if(!path.isEmpty())
        return path;

    // search for other steam bibs
    dir.setPath(steamAppPaths);
    if(dir.exists(steamBibFile))
    {
        QFile file(dir.absoluteFilePath(steamBibFile));
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString contents = QString::fromUtf8(file.readAll());
        file.close();

        QStringList splitted = contents.split(QLatin1String("\n"));
        QStringList bibs;
        int i = 1;
        QString bibID = QLatin1String("\"") + QString::number(i) + QLatin1String("\"");

        for(QString line : splitted)
        {
            line = line.trimmed();
            if(line.startsWith(bibID))
            {
                line = line.remove(bibID);
                line = line.remove(QLatin1String("\""));
                line = line.trimmed();
                bibs.append(line);

                i++;
                bibID = bibID.replace(QString::number(i-1), QString::number(i));
            }
        }

        for(QString bib : bibs)
        {
            dir.setPath(bib + QLatin1String("/") + steamAppsPath + QLatin1String("/") + steamRTBPath);
            path = createPath(dir);
            if(!path.isEmpty())
                return path;
        }
    }*/

    return QLatin1String("");
}

QString RTBCore::createPath(QDir dir)
{
    if(dir.exists(mGameExe) && dir.entryList().contains(mGameExe))
    {
        // create the path to the exe - example:
        // D:/Development/RTBTest/RollPlayingGameUE4.exe
        return dir.path() + QLatin1String("/") + mGameExe;
    }

    return QLatin1String("");
}

bool RTBCore::isGameAlreadyRunning()
{
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if (EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
    {
        // Calculate how many process identifiers were returned.
        cProcesses = cbNeeded / sizeof(DWORD);
        // Print the name and process identifier for each process.
        for ( i = 0; i < cProcesses; i++ )
        {
            if( aProcesses[i] != 0 )
            {
                TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

                // Get a handle to the process.
                HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                               PROCESS_VM_READ,
                                               FALSE, aProcesses[i] );

                // Get the process name.
                if (NULL != hProcess )
                {
                    HMODULE hMod;
                    DWORD cbNeeded;

                    if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod),
                         &cbNeeded) )
                    {
                        GetModuleBaseName( hProcess, hMod, szProcessName,
                                           sizeof(szProcessName)/sizeof(TCHAR) );
                    }
                }

                // check if it is the game process
                if(gameExe() == QString::fromWCharArray(szProcessName)
                        || gameShippingExe() == QString::fromWCharArray(szProcessName))
                {
                    return true;
                }

                // Release the handle to the process.
                CloseHandle( hProcess );
            }
        }
    }

    return false;
}

void RTBCore::buildMap(MapDocument *mapDocument)
{
    QProcess *gameProcess = new QProcess(this);
    QString executionString = Preferences::instance()->gameDirectory();

    // create the string that should be executed via CMD - example:
    // D:/Development/RTBTest/RollPlayingGameUE4.exe -localMap="D:/LocalTest.json"
    executionString = QLatin1String("\"") + executionString + QLatin1String("\" -localMap=\"")
            + mapDocument->fileName().replace(QLatin1String(" "), QLatin1String("%20")) + QLatin1String("\"");
    gameProcess->start(executionString);
}

bool RTBCore::isHalfTileAllowed(MapObject *mapObject)
{
    int type = mapObject->rtbMapObject()->objectType();
    if(type == RTBMapObject::CustomFloorTrap || type == RTBMapObject::MovingFloorTrapSpawner
            || type == RTBMapObject::LaserBeam || type == RTBMapObject::FinishHole)
    {
        return false;
    }
    else
        return true;
}

bool RTBCore::isHalfTileAllowed(QSet<MapObjectItem *> mapObjectItems)
{
    foreach (MapObjectItem *item, mapObjectItems)
    {
        if(!isHalfTileAllowed(item->mapObject()))
            return false;
    }

    return true;
}
