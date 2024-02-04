.. raw:: html

   <div class="new new-prev">Since Tiled 1.1</div>

Using Templates
===============

Any created object can be saved as a template. These templates can then be
instantiated elsewhere as objects that inherit the template's properties. This
can save a lot of tedious work of setting up the object class and properties,
or even just finding the right tile in the tileset.

Each template is stored in its own file, where they can be organized in
directories. You can save templates in either XML or JSON format, just like
map and tileset files.

.. figure:: images/templates/templates-overview.png
   :alt: Templates Overview

Creating Templates
------------------

A template can be created by right clicking on any object in the map and
selecting "Save As Template". You will be asked to choose the file name
and the format to save the template in. If the object already has a name
the suggested file name will be based on that.

.. figure:: images/templates/creating-templates.gif
   :alt: New Template Dialog

.. raw:: html

   <div class="new new-prev">Since Tiled 1.4</div>

To be able to select your templates for editing or instantiating you'll
generally want to use the :doc:`Project view <projects>`, so make sure to save
your templates in a folder that is part of your project. Dragging in a
template from a file manager is also possible.

.. note:: You can't create a template from a tile object that uses a
   tile from an embedded tileset, because
   :ref:`template files <tmx-template-files>` do not support
   referring to such tilesets.

.. _creating-template-instances:

Creating Template Instances
---------------------------

Shortcut: ``V``

Template instantiation works by either dragging and dropping the template from
the Project view to the map, or by using the "Insert Template" tool by
selecting a template and clicking on the map. The latter is more convenient
when you want to create many instances.

.. figure:: images/templates/creating-instances.gif
   :alt: Creating Instances

Editing Templates
-----------------

Editing templates is done using the *Template Editor* view. A template can be
opened for editing by selecting it in the Project view or by dragging the
template file on the *Template Editor* view. The template can also be selected
using the *Open File in Project* action.

When selecting the template in the *Template Editor* view, the *Properties*
view will show the template's properties, where they can be edited.

Any changes to the template are saved automatically and are immediately
reflected on all template instances.

.. figure:: images/templates/editing-templates.gif
   :alt: Editing Templates

If a property of a template instance is changed, it will be internally marked
as an overridden property and won't be changed when the template changes.

.. raw:: html

   <div class="new new-prev">Since Tiled 1.4</div>

If a template file changes on disk, it is automatically reloaded and any
changes will be reflected in the *Template Editor* as well as on any template
instances.

Detaching Template Instances
----------------------------

Detaching a template instance will disconnect it from its template, so any
further edits to the template will not affect the detached instance.

To detach an instance, right click on it and select *Detach*.

.. raw:: html

   <div class="new new-prev">Since Tiled 1.2</div>

If your map loader does not support object templates, but you'd still like to
use them, you can enable the *Detach templates* :ref:`export option
<export-options>`.

.. topic:: Future Extensions
   :class: future

    - Resetting overridden properties individually (`#1725 <https://github.com/bjorn/tiled/issues/1725>`__).
    - Locking template properties (`#1726 <https://github.com/bjorn/tiled/issues/1726>`__).
    - Handling wrong file paths (`#1732 <https://github.com/bjorn/tiled/issues/1732>`__).
    - Managing the templates folder, e.g. moving, renaming or deleting a template or a sub-folder
      (`#1723 <https://github.com/bjorn/tiled/issues/1723>`__).
