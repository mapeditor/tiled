.. raw:: html

   <div class="new">

New in Tiled 1.1

.. raw:: html

   </div>

.. _templates-introduction:

Using Templates
===============

Any created object can be saved with all of its properties as a template, this
template can be used to create copies of the object on object layers. This can
save a lot of tedious work of setting up the object type and properties, or
even just finding the right tile in the tileset.

Templates are organized into template groups. Template groups can be part of
your project and are referred to by the map. You can save them in either XML
or JSON format, just like map and tileset files.

.. figure:: images/templates/overview.png
   :alt: Templates Overview

Creating Templates
------------------

A template can be create by right clicking on any object in the map and
selecting "Save as Template". You will be asked to choose the name and the
template group to save the template to. If the object already has a name the
name field will be auto-filled with it.

Template groups can be created from the new template group button in the
templates view or from the new template dialog.

.. figure:: images/templates/creating-templates.gif
   :alt: New Template Dialog

The templates View
------------------

Working with templates is done through the templates view. The templates view
is divided into two parts, the left part is a tree view that shows the loaded
template groups and their templates, the right part shows a preview of the
selected template.

Creating Template Instances
---------------------------

Shortcut: ``V``

Template instantiation works by either dragging and dropping the template from
the templates tree view into the map, or by using the "Insert Template" tool
by selecting a template and clicking on the map which is more convenient when
you want to create many instances.

.. figure:: images/templates/creating-instances.gif
   :alt: Creating Instances


Editing Templates
-----------------

Selecting a template will load an editable preview in the templates view and
will load the template's properties into the properties view where they can be
edited.

All template instances are linked to the base template, so all edits will be
immediately reflected upon all the template instances on the map.

.. figure:: images/templates/editing-templates.gif
   :alt: Editing Templates

If a property of a template instance is changed, it will be internally marked
as an overridden property and won't be changed when the template changes.

Detaching template instances
----------------------------

Detaching template instance will disconnect it from the base template, so any
further edits to the template will not affect the detached instance.

To detach an instance, right click on it and select detach.

Future Extensions
-----------------

- Reseting overridden properties.
- Locking template properties.
- Handle wrong file paths.
- Managing template groups, e.g. removing a template or a template group.
