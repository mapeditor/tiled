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

void writeMap(Map *map, QString const &fileName) {
  MapFormat *format = findFormat<MapFormat>(fileName);
  if (format) {
    qDebug() << "Writing " << format->nameFilter();
    format->write(map, fileName);
  } else {
    qDebug() << "Writing TMX format";
    MapWriter writer;
    writer.writeMap(map, fileName);
  }
}

void writeTileset(SharedTileset tileset, QString const &fileName) {
  TilesetFormat *format = findFormat<TilesetFormat>(fileName);
  if (format) {
    qDebug() << "Writing " << format->nameFilter();
    format->write(*tileset, fileName);
  } else {
    qDebug() << "Writing TSX format";
    MapWriter writer;
    writer.writeTileset(*tileset, fileName);
  }
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

  PluginManager::instance()->loadPlugins();
  foreach (LoadedPlugin plugin, PluginManager::instance()->plugins()) {
    qDebug() << "Loaded plugin " << plugin.fileName;
  }

  if (parser.isSet(tilesetOption)) {
    SharedTileset tileset = readTileset(args.at(0));
    tileset->setFileName("");
    writeTileset(tileset, args.at(1));
  } else {
    Map *map = readMap(args.at(0));
    writeMap(map, args.at(1));
  }

  return 0;
}
