.. raw:: html

   <div class="new">

New in Tiled 1.1

.. raw:: html

   </div>

Using Templates
===============

Any created object can be saved as a template. These templates can then be
instantiated elsewhere as objects that inherit the template's properties. This
can save a lot of tedious work of setting up the object type and properties, or
even just finding the right tile in the tileset.

Templates are organized into template groups. Template groups can be part of
your project and are referred to by the map. You can save them in either XML
or JSON format, just like map and tileset files.

.. figure:: images/templates/overview.png
   :alt: Templates Overview

Creating Templates
------------------

A template can be created by right clicking on any object in the map and
selecting "Save as Template". You will be asked to choose the name and the
template group to save the template to. If the object already has a name the
name field will be auto-filled with it.

Template groups can be created from the New Template Group button in the
Templates View or from the New Template Dialog.

.. figure:: images/templates/creating-templates.gif
   :alt: New Template Dialog

.. note:: You can't create a template from a tile object that uses a
   tile from an embedded tileset, because the
   :ref:`template group format <templategroup-format>` does not support
   referring to such tilesets.

The Templates View
------------------

Working with templates is done through the Templates View. The Templates View
is divided into two parts, the left part is a tree view that shows the loaded
template groups and their templates, the right part shows a preview of the
selected template.

Creating Template Instances
---------------------------

Shortcut: ``V``

Template instantiation works by either dragging and dropping the template from
the Templates Tree View into the map, or by using the "Insert Template" tool
by selecting a template and clicking on the map which is more convenient when
you want to create many instances.

.. figure:: images/templates/creating-instances.gif
   :alt: Creating Instances


Editing Templates
-----------------

Selecting a template will show an editable preview in the Templates View and
will load the template's properties into the Properties View where they can be
edited.

All template instances are linked to their template, so all edits will be
immediately reflected upon all the template instances on the map.

.. figure:: images/templates/editing-templates.gif
   :alt: Editing Templates

If a property of a template instance is changed, it will be internally marked
as an overridden property and won't be changed when the template changes.

Detaching Template Instances
----------------------------

Detaching a template instance will disconnect it from its template, so any
further edits to the template will not affect the detached instance.

To detach an instance, right click on it and select *Detach*.

Future Extensions
-----------------

- Reseting overridden properties.
- Locking template properties.
- Handling wrong file paths.
- Managing template groups, e.g. removing a template or a template group.
