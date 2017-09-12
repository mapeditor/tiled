Using Commands
==============

The Command Button allows you to create and run shell commands (other
programs) from Tiled.

You may setup as many commands as you like. This is useful if you edit
maps for multiple games and you want to set up a command for each game.
Or you could setup multiple commands for the same game that load
different checkpoints or configurations.

The Command Button
------------------

It is located on the main toolbar to the right of the redo button.
Clicking on it will run the default command (the first command in the
command list). Clicking the arrow next to it will bring down a menu that
allows you to run any command you have set up, as well as an option to
open the Edit Commands dialog. You can also find all the commands in the
File menu.

Apart from this, you can set up custom keyboard shortcuts for each
command.

Editing Commands
----------------

The 'Edit Commands' dialog contains a list of commands. Each command has
several properties:

Name
    The name of the command as it will be shown in the drop
    down list, so you can easily identify it.

Executable
    The executable to run. It should either be a full
    path or the name of an executable in the system PATH.

Arguments
    The arguments for running the executable.

Working directory
    The path to the working directory.

Shortcut
    A custom key sequence to trigger the command. You can use 'Clear'
    to reset the shortcut.

Output in Debug Console
    If this is enabled, then the output (stdout and stderr) of this
    command will be displayed in the Debug Console. You can find the
    Debug Console in *View > Views and Toolbars > Debug Console*.

Save map before executing
    If this is enabled, then the current map will be saved before
    executing the command.

Enabled
    A quick way to disable commands and remove them from the drop down list.
    The default command is the first enabled command.

Note that if the executable or any of its arguments contain spaces,
these parts need to be quoted.

Substituted Variables
~~~~~~~~~~~~~~~~~~~~~

In the executable, arguments and working directory fields, you can use
the following variables:

``%mapfile``
    the current maps full path.

``%mappath``
    the full folder path in which the map is located. (since Tiled 0.18)

``%objecttype``
    the type of the currently selected object, if any. (since Tiled 0.12)

``%objectid``
    the ID of the currently selected object, if any. (since Tiled 0.17)

``%layername``
    the name of the currently selected layer. (since Tiled 0.17)

For the working directory field, you can additionally use the following
variable:

``%executablepath``
    the path to the executable.


Example Commands
----------------

Launching a custom game called "mygame" with a -loadmap parameter and
the mapfile:

::

    mygame -loadmap %mapfile

On Mac, remember that Apps are folders, so you need to run the actual
executable from within the ``Contents/MacOS`` folder:

::

    /Applications/TextEdit.app/Contents/MacOS/TextEdit %mapfile

Or use ``open`` (and note the quotes since one of the arguments contains
spaces):

::

    open -a "/Applications/CoronaSDK/Corona Simulator.app" /Users/user/Desktop/project/main.lua

Some systems also have a command to open files in the appropriate
program:

-  OSX: ``open %mapfile``
-  GNOME systems like Ubuntu: ``gnome-open %mapfile``
-  FreeDesktop.org standard: ``xdg-open %mapfile``
