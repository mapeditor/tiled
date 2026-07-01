## Changelog

### 1.12.0

- Updated to the new API from Tiled 1.11.1 and 1.12
  - Added Layer.blendMode (#4140)
  - Added API for accessing and changing custom property types (#3971)
  - Added Tileset.transformationFlags (#4100)
  - Added TileMap.chunkSize and TileMap.compressionLevel (#4242)
  - Added MapEditor.selectedTool and MapEditor.tool (#4330)
  - Added Dialog.addRadioButtonGroup (#4107)
  - Added optional defaultValue and toolTip params to Dialog add widget methods (#4358)
  - Expose session as tiled.session (#4345)
  - Made currentWangSet and currentWangColorIndex writeable (#4105)
- Added link from World to tiled.worlds (#4092)
- Fixed link to deprecated property Tileset.image (#4033)
- Fixed errors in API type definitions (#4056)
- Fixed strict mode error (#4104)
- Clarified how to distinguish an invalid color from black (#3154)
- Automatic updates to some dependencies

### 1.11.0

- Added the new API from Tiled 1.10.2 and 1.11.0
- Documented FileEdit.isDirectory / filter and fix @link tags (#3777)
- Split ToolDefinition from Tool for registerTool function
- Clarify how to erase tiles and highlight it in the Tool.preview
- Extended docs for TileLayerEdit.apply and MapObject.textAlignment
- Fixed missing "| null" in a few more places
- Fixed hidden doc for Dialog.exec() (#3837)
- Fixed the type of FilePath.url and ImageLayer.imageSource
- Link Tile.frames from Tile.animated

### 1.10.1

- Added the new API from Tiled 1.10.1
- Added missing documentation for MapObject(shape, name?) constructor
- Fixed link in docs for Image
- Improved docs for QCheckBox and QPushButton

### 1.10.0

- Added the new API from Tiled 1.10.0
- Added missing documentation for QComboBox.clear, QTextEdit.html,
  QTextEdit.markdown, QLineEdit.text, QWidget.styleSheet, Dialog.exec,
  Tileset.transparentColor and Tile.imageRect
- Fixed documentation for Dialog.addImage
- Fixed QDoubleSpinBox step value property name
- Fixed type of ImageLayer.transparentColor
* Fixed link in docs for Dialog.addTextEdit
- Clarified the return value of TileLayer.flagsAt
- Clarified documentation for TileLayerEdit.setTile
- Updated links to Qt documentation to Qt 6

### 1.9.2

> 22 Sep 2022

- Added the new API from Tiled 1.8.5, 1.9.0, 1.9.1 and 1.9.2
- Changed FileInfo API to a namespace so it can be accessed in TypeScript (#3346)
- Clarified the format of integer color values
- Fixed a few members to be marked readonly

### 1.8.2

> 3 May 2022

- Changed File API to a namespace so it can be accessed in TypeScript (#3346)
- Strict mode corrections (adding "| null" or "| undefined")
- Added missing declarations for Layer.parallaxFactor and Layer.parentLayer
- Added funding options
- Fixed custom tool example

### 1.8.1

> 8 February 2022

- Enabled strict mode
- Fixed strict mode error

### 1.8.0

> 7 February 2022

- Added the new API in Tiled 1.8
- Improved documentation in many places (#3171, #3233, #3239, #3240, #3246)
- Updated typescript to 4.4
- Updated typedoc to 0.21

### 1.6.0

> 3 May 2021

- First stable release, based on the API provided by Tiled 1.6

### 1.4.3-alpha

> 15 December 2020

- First release, with only part of the API provided by Tiled 1.4 included
