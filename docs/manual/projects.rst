.. raw:: html

   <div class="new new-prev">Since Tiled 1.4</div>

Projects
========

What's in a Project
-------------------

A Tiled project file primarily defines the list of folders containing the
assets belonging to that project. In addition, it provides an anchor for the
:ref:`session file <sessions>`.

Apart from the list of folders, a project currently has the following
properties, which can be changed through the *Project -> Project
Properties...* dialog.

Compatibility Version
   The Tiled version to target when saving or exporting files. Can be used to
   maintain compatibility with earlier versions of Tiled or with
   :doc:`/reference/support-for-tmx-maps` that do not yet support certain
   backwards-incompatible changes.

Extensions Directory
   A project-specific directory where you can put :doc:`Tiled extensions
   </manual/scripting>`. It defaults to simply ``extensions``, so when you
   have a directory called "extensions" alongside your project file it will be
   picked up automatically.

   The directory is loaded in addition to the global extensions.

Automapping Rules File
   Refers to an :doc:`automapping` rules file, or a single rule map, that
   should be used for all maps while this project is loaded. It is ignored for
   maps that have a ``rules.txt`` file saved alongside them.

Any types defined in the :ref:`Custom Types Editor <custom-property-types>`
are also saved in the project.

.. _sessions:

Sessions
--------

Each project file gets an associated *.tiled-session* file, stored alongside
it. The session file should generally not be shared with others and stores
your last opened files, part of their last editor state, last used parameters
in dialogs, etc.

When switching projects Tiled automatically switches to the associated
session, so you can easily resume where you left off. When no project is
loaded a global session file is used.

Opening a File in the Project
-----------------------------

Another advantage of setting up a project is that you can quickly open any
file with a recognized extension located in one of the folders of the project.
Use *File -> Open File in Project* (``Ctrl+P``) to open the file filter and
just type the name of the file you'd like to open.

.. figure:: images/open-file-in-project.png
   :alt: Open File in Project

   Open File in Project

.. topic:: Future Extensions
   :class: future

   There are many ways in which the projects could be made more powerful:

   -  Make the project accessible through the :doc:`scripting API
      </manual/scripting>`.

   -  Allow turning off features on a per-project basis, to simplify the UI
      and reduce the chance of accidentally doing something your project
      doesn't support.

   -  Recognizing the various assets in your project, so that selection of
      images, tilesets and templates can be made more efficient (potentially
      replacing the system file dialog).

   If you like any of these plans, please help me getting around to it
   faster by `sponsoring Tiled development <https://www.mapeditor.org/donate>`__. The
   more support I receive the more time I can afford to spend improving
   Tiled!
