# Using Commands

The Command Button allows you to create and run shell commands (other programs) from Tiled.

You may setup as many commands as you like. This is useful if you edit maps for multiple games and you want to set up a command for each game. Or you could setup multiple commands for the same game that load different checkpoints or configurations.

## The Command Button

It is located on the main toolbar to the right of the redo button. Clicking on it will run the default command (the first command in the command list). Clicking the arrow next to it will bring down a menu that allows you to run any command you have set up, as well as an option to open the Edit Commands dialog.

You can press F5 as a shortcut to clicking the button to run the default command.

## Editing Commands

The 'Edit Commands' dialog contains a list of commands. Each command has several properties:

* **Name**: The name of the command as it will be shown in the drop down list, so you can easily identify it.
* **Command**: The actual shell command to execute. This usually starts with an executable program followed by arguments. You can use the following variables:
    + `%mapfile` is replaced with the current maps full path.
    + `%objecttype` is replaced with the type of the currently selected object, if any. (since Tiled 0.12)
    + `%objectid` is replaced with the ID of the currently selected object, if any. (since Tiled 0.17)
    + `%layername` is replaced with the name of the currently selected layer. (since Tiled 0.17)
* **Enabled**: A quick way to disable commands and remove them from the drop down list.
    + The default command is the first enabled command.

You can also change whether or not it should save the current map before running commands.

Note that if the program or any of its arguments contain spaces, these parts need to be quoted.

## Example Commands

Launching a custom game called "mygame" with a -loadmap parameter and the mapfile:

    mygame -loadmap %mapfile

On Mac, remember that Apps are folders, so you need to run the actual executable from within the `Contents/MacOS` folder:

    /Applications/TextEdit.app/Contents/MacOS/TextEdit %mapfile

Or use `open` (and note the quotes since one of the arguments contains spaces):

    open -a "/Applications/CoronaSDK/Corona Simulator.app" /Users/user/Desktop/project/main.lua

Some systems also have a command to open files in the appropriate program:

* OSX: `open %mapfile`
* GNOME systems like Ubuntu: `gnome-open %mapfile`
* FreeDesktop.org standard: `xdg-open %mapfile`
