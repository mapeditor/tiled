#include <QDebug>
#include <QGuiApplication>
#include <QCommandLineParser>

#include "mapformat.h"
#include "tilesetformat.h"
#include "mapreader.h"
#include "mapwriter.h"

using namespace Tiled;

template <typename Format>
Format *findFormat(QString const &fileName) {
  FormatHelper<Format> formatHelper(FileFormat::ReadWrite, "");
  foreach (Format *format, formatHelper.formats()) {
    if (format->supportsFile(fileName))
      return format;
  }
  return nullptr;
}

Map *readMap(QString const &fileName) {
  MapFormat *format = findFormat<MapFormat>(fileName);
  if (format) {
    qDebug() << "Reading " << format->nameFilter();
    return format->read(fileName);
  } else {
    qDebug() << "Reading TMX format";
    MapReader reader;
    return reader.readMap(fileName);
  }
}

bool writeMap(Map *map, QString const &fileName) {
  MapFormat *format = findFormat<MapFormat>(fileName);
  if (format) {
    qDebug() << "Writing " << format->nameFilter();
    if (!format->write(map, fileName)) {
      qFatal("%s", qPrintable(format->errorString()));
      return false;
    }
  } else {
    qDebug() << "Writing TMX format";
    MapWriter writer;
    if (!writer.writeMap(map, fileName)) {
      qFatal("%s", qPrintable(writer.errorString()));
      return false;
    }
  }
  return true;
}

bool writeTileset(SharedTileset tileset, QString const &fileName) {
  TilesetFormat *format = findFormat<TilesetFormat>(fileName);
  if (format) {
    qDebug() << "Writing " << format->nameFilter();
    if (!format->write(*tileset, fileName)) {
      qFatal("%s", qPrintable(format->errorString()));
      return false;
    }
  } else {
    qDebug() << "Writing TSX format";
    MapWriter writer;
    if (!writer.writeTileset(*tileset, fileName)) {
      qFatal("%s", qPrintable(writer.errorString()));
      return false;
    }
  }
  return true;
}

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  QGuiApplication::setApplicationName("formatconvert");
  QGuiApplication::setApplicationVersion("0.1");

  QCommandLineParser parser;
  parser.setApplicationDescription("Convert Tiled map files to other formats. Formats are detected from the file extensions.");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("source", QCoreApplication::translate("formatconvert", "input file to read"));
  parser.addPositionalArgument("destination", QCoreApplication::translate("formatconvert", "output file to write"));

  QCommandLineOption tilesetOption(QStringList() << "t" << "tileset",
      QCoreApplication::translate("formatconvert", "Convert a tileset instead of a map."));
  parser.addOption(tilesetOption);

  parser.process(app);

  QStringList const args = parser.positionalArguments();

  if (args.size() != 2) {
    parser.showHelp(1);
  }

  PluginManager::instance()->loadPlugins();
  foreach (PluginFile plugin, PluginManager::instance()->plugins()) {
    qDebug() << "Loaded plugin " << plugin.fileName();
  }

  if (parser.isSet(tilesetOption)) {
    SharedTileset tileset = readTileset(args.at(0));
    tileset->setFileName("");
    if (!writeTileset(tileset, args.at(1)))
      return 2;
  } else {
    Map *map = readMap(args.at(0));
    if (!writeMap(map, args.at(1)))
      return 2;
  }

  return 0;
}
