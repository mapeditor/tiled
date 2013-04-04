Qt Solutions Component: Property Browser

A property browser framework enabling the user to edit a set of
properties.

The framework provides a browser widget that displays the given
properties with labels and corresponding editing widgets (e.g.
line edits or comboboxes). The various types of editing widgets
are provided by the framework's editor factories: For each
property type, the framework provides a property manager (e.g.
QtIntPropertyManager and QtStringPropertyManager) which can be
associated with the preferred editor factory (e.g.
QtSpinBoxFactory and QtLineEditFactory). The framework also
provides a variant based property type with corresponding variant
manager and factory. Finally, the framework provides three
ready-made implementations of the browser widget:
QtTreePropertyBrowser, QtButtonPropertyBrowser and
QtGroupBoxPropertyBrowser.

Version history:

2.1: - QtTreePropertyBrowser - tooltip of property applied to
     first column, while second column shows the value text of property
     in its tooltip
     - QtAbstractPropertyManager - initializeProperty() and
     uninitializeProperty() without const modifier now
     - QtTreePropertyBrowser and QtGroupBoxPropertyBrowser - internal
     margin set to 0
     - QtProperty - setEnabled() and isEnabled() methods added
     - QtTreePropertyBrowser - "rootIsDecorated", "indentation" and
     "headerVisible" properties added
     - QtProperty - hasValue() method added, useful for group
     properties

2.2: - FocusOut event now filtered out in case of
     Qt::ActiveWindowFocusReason reason. In that case editor is not
     closed when its sub dialog is executed
     - Removed bug in color icon generation
     - Decimals attribute added to "double" property type
     - PointF, SizeF and RectF types supported
     - Proper translation calls for tree property browser
     - QtProperty - ensure inserted subproperty is different from
     "this" property
     - QtBrowserItem class introduced, useful for identifying browser's
     gui elements
     - Possibility to control expanded state of QtTreePropertyBrowser's
     items from code
     - QtTreePropertyBrowser - "resizeMode" and "splitterPosition"
     properties added
     - QtGroupBoxPropertyBrowser - fixed crash in case of deleting the
     editor factory and then deleting the manager
     - "Decoration" example added - it shows how to add new
     responsibilities to the existing managers and editor factories

2.3: - Various bugfixes and improvements
     - QtProperty - setModified() and isModified() methods added
     - QtTreePropertyBrowser - disabling an item closes its editor
     - KeySequence, Char, Locale and Cursor types supported
     - Support for icons in enum type added
     - Kerning subproperty exposed in Font type
     - New property browser class added - QtButtonPropertyBrowser with
     drop down button as a grouping element

2.4: - Fixed memory leak of QtProperty
     - QtTreePropertyBrowser - group items are rendered better
     - QtTreePropertyBrowser - propertiesWithoutValueMarked and
     alternatingRowColors features added
     - QtTreePropertyBrowser - possibility of coloring properties added
     - QtTreePropertyBrowser - keyboard navigation improved
     - New factories providing popup dialogs added:
     QtColorEditorFactory and QtFontEditorFactory
     - Single step attribute added to: QtIntPropertyManager and
     QtDoublePropertyManager

2.5: - "Object Controller" example added. It implements a similar
     widget to the property editor in QDesigner
     - Compile with QT_NO_CURSOR
     - Expand root item with single click on the '+' icon
     - QtRectPropertyManager and QtRectFPropertyManager - by default
     constraint is null rect meaning no constraint is applied

2.6: - QtGroupPropertyBrowser - don't force the layout to show the
     whole labels' contents for read only properties, show tooltips for
     them in addition.
     - QtTreePropertyBrowser - fixed painting of the editor for color
     property type when style sheet is used (QTSOLBUG-64).
     - Make it possible to change the style of the checkboxes with a
     stylesheet (QTSOLBUG-61).
     - Change the minimum size of a combobox so that it can show at
     least one character and an icon.
     - Make it possible to properly style custom embedded editors (e.g.
     the color editor provided with the solution).

