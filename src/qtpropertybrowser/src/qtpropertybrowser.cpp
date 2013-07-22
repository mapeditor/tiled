/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "qtpropertybrowser.h"
#include <QSet>
#include <QMap>
#include <QIcon>
#include <QLineEdit>

#if defined(Q_CC_MSVC)
#    pragma warning(disable: 4786) /* MS VS 6: truncating debug info after 255 characters */
#endif

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

class QtPropertyPrivate
{
public:
    QtPropertyPrivate(QtAbstractPropertyManager *manager)
        : m_enabled(true),
          m_modified(false),
          m_manager(manager) {}
    QtProperty *q_ptr;

    QSet<QtProperty *> m_parentItems;
    QList<QtProperty *> m_subItems;

    QString m_toolTip;
    QString m_statusTip;
    QString m_whatsThis;
    QString m_name;
    bool m_enabled;
    bool m_modified;

    QtAbstractPropertyManager * const m_manager;
};

class QtAbstractPropertyManagerPrivate
{
    QtAbstractPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtAbstractPropertyManager)
public:
    void propertyDestroyed(QtProperty *property);
    void propertyChanged(QtProperty *property) const;
    void propertyRemoved(QtProperty *property,
                QtProperty *parentProperty) const;
    void propertyInserted(QtProperty *property, QtProperty *parentProperty,
                QtProperty *afterProperty) const;

    QSet<QtProperty *> m_properties;
};

/*!
    \class QtProperty

    \brief The QtProperty class encapsulates an instance of a property.

    Properties are created by objects of QtAbstractPropertyManager
    subclasses; a manager can create properties of a given type, and
    is used in conjunction with the QtAbstractPropertyBrowser class. A
    property is always owned by the manager that created it, which can
    be retrieved using the propertyManager() function.

    QtProperty contains the most common property attributes, and
    provides functions for retrieving as well as setting their values:

    \table
    \header \o Getter \o Setter
    \row
    \o propertyName() \o setPropertyName()
    \row
    \o statusTip() \o setStatusTip()
    \row
    \o toolTip() \o setToolTip()
    \row
    \o whatsThis() \o setWhatsThis()
    \row
    \o isEnabled() \o setEnabled()
    \row
    \o isModified() \o setModified()
    \row
    \o valueText() \o Nop
    \row
    \o valueIcon() \o Nop
    \endtable

    It is also possible to nest properties: QtProperty provides the
    addSubProperty(), insertSubProperty() and removeSubProperty() functions to
    manipulate the set of subproperties. Use the subProperties()
    function to retrieve a property's current set of subproperties.
    Note that nested properties are not owned by the parent property,
    i.e. each subproperty is owned by the manager that created it.

    \sa QtAbstractPropertyManager, QtBrowserItem
*/

/*!
    Creates a property with the given \a manager.

    This constructor is only useful when creating a custom QtProperty
    subclass (e.g. QtVariantProperty). To create a regular QtProperty
    object, use the QtAbstractPropertyManager::addProperty()
    function instead.

    \sa QtAbstractPropertyManager::addProperty()
*/
QtProperty::QtProperty(QtAbstractPropertyManager *manager)
{
    d_ptr = new QtPropertyPrivate(manager);
    d_ptr->q_ptr = this;
}

/*!
    Destroys this property.

    Note that subproperties are detached but not destroyed, i.e. they
    can still be used in another context.

    \sa QtAbstractPropertyManager::clear()

*/
QtProperty::~QtProperty()
{
    QSetIterator<QtProperty *> itParent(d_ptr->m_parentItems);
    while (itParent.hasNext()) {
        QtProperty *property = itParent.next();
        property->d_ptr->m_manager->d_ptr->propertyRemoved(this, property);
    }

    d_ptr->m_manager->d_ptr->propertyDestroyed(this);

    QListIterator<QtProperty *> itChild(d_ptr->m_subItems);
    while (itChild.hasNext()) {
        QtProperty *property = itChild.next();
        property->d_ptr->m_parentItems.remove(this);
    }

    itParent.toFront();
    while (itParent.hasNext()) {
        QtProperty *property = itParent.next();
        property->d_ptr->m_subItems.removeAll(this);
    }
    delete d_ptr;
}

/*!
    Returns the set of subproperties.

    Note that subproperties are not owned by \e this property, but by
    the manager that created them.

    \sa insertSubProperty(), removeSubProperty()
*/
QList<QtProperty *> QtProperty::subProperties() const
{
    return d_ptr->m_subItems;
}

/*!
    Returns a pointer to the manager that owns this property.
*/
QtAbstractPropertyManager *QtProperty::propertyManager() const
{
    return d_ptr->m_manager;
}

/*!
    Returns the property's  tool tip.

    \sa setToolTip()
*/
QString QtProperty::toolTip() const
{
    return d_ptr->m_toolTip;
}

/*!
    Returns the property's status tip.

    \sa setStatusTip()
*/
QString QtProperty::statusTip() const
{
    return d_ptr->m_statusTip;
}

/*!
    Returns the property's "What's This" help text.

    \sa setWhatsThis()
*/
QString QtProperty::whatsThis() const
{
    return d_ptr->m_whatsThis;
}

/*!
    Returns the property's name.

    \sa setPropertyName()
*/
QString QtProperty::propertyName() const
{
    return d_ptr->m_name;
}

/*!
    Returns whether the property is enabled.

    \sa setEnabled()
*/
bool QtProperty::isEnabled() const
{
    return d_ptr->m_enabled;
}

/*!
    Returns whether the property is modified.

    \sa setModified()
*/
bool QtProperty::isModified() const
{
    return d_ptr->m_modified;
}

/*!
    Returns whether the property has a value.

    \sa QtAbstractPropertyManager::hasValue()
*/
bool QtProperty::hasValue() const
{
    return d_ptr->m_manager->hasValue(this);
}

/*!
    Returns an icon representing the current state of this property.

    If the given property type can not generate such an icon, this
    function returns an invalid icon.

    \sa QtAbstractPropertyManager::valueIcon()
*/
QIcon QtProperty::valueIcon() const
{
    return d_ptr->m_manager->valueIcon(this);
}

/*!
    Returns a string representing the current state of this property.

    If the given property type can not generate such a string, this
    function returns an empty string.

    \sa QtAbstractPropertyManager::valueText()
*/
QString QtProperty::valueText() const
{
    return d_ptr->m_manager->valueText(this);
}

/*!
    Returns the display text according to the echo-mode set on the editor.

    When the editor is a QLineEdit, this will return a string equal to what
    is displayed.

    \sa QtAbstractPropertyManager::valueText()
*/
QString QtProperty::displayText() const
{
    return d_ptr->m_manager->displayText(this);
}

/*!
    Sets the property's tool tip to the given \a text.

    \sa toolTip()
*/
void QtProperty::setToolTip(const QString &text)
{
    if (d_ptr->m_toolTip == text)
        return;

    d_ptr->m_toolTip = text;
    propertyChanged();
}

/*!
    Sets the property's status tip to the given \a text.

    \sa statusTip()
*/
void QtProperty::setStatusTip(const QString &text)
{
    if (d_ptr->m_statusTip == text)
        return;

    d_ptr->m_statusTip = text;
    propertyChanged();
}

/*!
    Sets the property's "What's This" help text to the given \a text.

    \sa whatsThis()
*/
void QtProperty::setWhatsThis(const QString &text)
{
    if (d_ptr->m_whatsThis == text)
        return;

    d_ptr->m_whatsThis = text;
    propertyChanged();
}

/*!
    \fn void QtProperty::setPropertyName(const QString &name)

    Sets the property's  name to the given \a name.

    \sa propertyName()
*/
void QtProperty::setPropertyName(const QString &text)
{
    if (d_ptr->m_name == text)
        return;

    d_ptr->m_name = text;
    propertyChanged();
}

/*!
    Enables or disables the property according to the passed \a enable value.

    \sa isEnabled()
*/
void QtProperty::setEnabled(bool enable)
{
    if (d_ptr->m_enabled == enable)
        return;

    d_ptr->m_enabled = enable;
    propertyChanged();
}

/*!
    Sets the property's modified state according to the passed \a modified value.

    \sa isModified()
*/
void QtProperty::setModified(bool modified)
{
    if (d_ptr->m_modified == modified)
        return;

    d_ptr->m_modified = modified;
    propertyChanged();
}

/*!
    Appends the given \a property to this property's subproperties.

    If the given \a property already is added, this function does
    nothing.

    \sa insertSubProperty(), removeSubProperty()
*/
void QtProperty::addSubProperty(QtProperty *property)
{
    QtProperty *after = 0;
    if (d_ptr->m_subItems.count() > 0)
        after = d_ptr->m_subItems.last();
    insertSubProperty(property, after);
}

/*!
    \fn void QtProperty::insertSubProperty(QtProperty *property, QtProperty *precedingProperty)

    Inserts the given \a property after the specified \a
    precedingProperty into this property's list of subproperties.  If
    \a precedingProperty is 0, the specified \a property is inserted
    at the beginning of the list.

    If the given \a property already is inserted, this function does
    nothing.

    \sa addSubProperty(), removeSubProperty()
*/
void QtProperty::insertSubProperty(QtProperty *property,
            QtProperty *afterProperty)
{
    if (!property)
        return;

    if (property == this)
        return;

    // traverse all children of item. if this item is a child of item then cannot add.
    QList<QtProperty *> pendingList = property->subProperties();
    QMap<QtProperty *, bool> visited;
    while (!pendingList.isEmpty()) {
        QtProperty *i = pendingList.first();
        if (i == this)
            return;
        pendingList.removeFirst();
        if (visited.contains(i))
            continue;
        visited[i] = true;
        pendingList += i->subProperties();
    }

    pendingList = subProperties();
    int pos = 0;
    int newPos = 0;
    QtProperty *properAfterProperty = 0;
    while (pos < pendingList.count()) {
        QtProperty *i = pendingList.at(pos);
        if (i == property)
            return; // if item is already inserted in this item then cannot add.
        if (i == afterProperty) {
            newPos = pos + 1;
            properAfterProperty = afterProperty;
        }
        pos++;
    }

    d_ptr->m_subItems.insert(newPos, property);
    property->d_ptr->m_parentItems.insert(this);

    d_ptr->m_manager->d_ptr->propertyInserted(property, this, properAfterProperty);
}

/*!
    Removes the given \a property from the list of subproperties
    without deleting it.

    \sa addSubProperty(), insertSubProperty()
*/
void QtProperty::removeSubProperty(QtProperty *property)
{
    if (!property)
        return;

    d_ptr->m_manager->d_ptr->propertyRemoved(property, this);

    QList<QtProperty *> pendingList = subProperties();
    int pos = 0;
    while (pos < pendingList.count()) {
        if (pendingList.at(pos) == property) {
            d_ptr->m_subItems.removeAt(pos);
            property->d_ptr->m_parentItems.remove(this);

            return;
        }
        pos++;
    }
}

/*!
    \internal
*/
void QtProperty::propertyChanged()
{
    d_ptr->m_manager->d_ptr->propertyChanged(this);
}

////////////////////////////////

void QtAbstractPropertyManagerPrivate::propertyDestroyed(QtProperty *property)
{
    if (m_properties.contains(property)) {
        emit q_ptr->propertyDestroyed(property);
        q_ptr->uninitializeProperty(property);
        m_properties.remove(property);
    }
}

void QtAbstractPropertyManagerPrivate::propertyChanged(QtProperty *property) const
{
    emit q_ptr->propertyChanged(property);
}

void QtAbstractPropertyManagerPrivate::propertyRemoved(QtProperty *property,
            QtProperty *parentProperty) const
{
    emit q_ptr->propertyRemoved(property, parentProperty);
}

void QtAbstractPropertyManagerPrivate::propertyInserted(QtProperty *property,
            QtProperty *parentProperty, QtProperty *afterProperty) const
{
    emit q_ptr->propertyInserted(property, parentProperty, afterProperty);
}

/*!
    \class QtAbstractPropertyManager

    \brief The QtAbstractPropertyManager provides an interface for
    property managers.

    A manager can create and manage properties of a given type, and is
    used in conjunction with the QtAbstractPropertyBrowser class.

    When using a property browser widget, the properties are created
    and managed by implementations of the QtAbstractPropertyManager
    class. To ensure that the properties' values will be displayed
    using suitable editing widgets, the managers are associated with
    objects of QtAbstractEditorFactory subclasses. The property browser
    will use these associations to determine which factories it should
    use to create the preferred editing widgets.

    The QtAbstractPropertyManager class provides common functionality
    like creating a property using the addProperty() function, and
    retrieving the properties created by the manager using the
    properties() function. The class also provides signals that are
    emitted when the manager's properties change: propertyInserted(),
    propertyRemoved(), propertyChanged() and propertyDestroyed().

    QtAbstractPropertyManager subclasses are supposed to provide their
    own type specific API. Note that several ready-made
    implementations are available:

    \list
    \o QtBoolPropertyManager
    \o QtColorPropertyManager
    \o QtDatePropertyManager
    \o QtDateTimePropertyManager
    \o QtDoublePropertyManager
    \o QtEnumPropertyManager
    \o QtFlagPropertyManager
    \o QtFontPropertyManager
    \o QtGroupPropertyManager
    \o QtIntPropertyManager
    \o QtPointPropertyManager
    \o QtRectPropertyManager
    \o QtSizePropertyManager
    \o QtSizePolicyPropertyManager
    \o QtStringPropertyManager
    \o QtTimePropertyManager
    \o QtVariantPropertyManager
    \endlist

    \sa QtAbstractEditorFactoryBase, QtAbstractPropertyBrowser, QtProperty
*/

/*!
    \fn void QtAbstractPropertyManager::propertyInserted(QtProperty *newProperty,
                QtProperty *parentProperty, QtProperty *precedingProperty)

    This signal is emitted when a new subproperty is inserted into an
    existing property, passing pointers to the \a newProperty, \a
    parentProperty and \a precedingProperty as parameters.

    If \a precedingProperty is 0, the \a newProperty was inserted at
    the beginning of the \a parentProperty's subproperties list.

    Note that signal is emitted only if the \a parentProperty is created
    by this manager.

    \sa QtAbstractPropertyBrowser::itemInserted()
*/

/*!
    \fn void QtAbstractPropertyManager::propertyChanged(QtProperty *property)

    This signal is emitted whenever a property's data changes, passing
    a pointer to the \a property as parameter.

    Note that signal is only emitted for properties that are created by
    this manager.

    \sa QtAbstractPropertyBrowser::itemChanged()
*/

/*!
    \fn void QtAbstractPropertyManager::propertyRemoved(QtProperty *property, QtProperty *parent)

    This signal is emitted when a subproperty is removed, passing
    pointers to the removed \a property and the \a parent property as
    parameters.

    Note that signal is emitted only when the \a parent property is
    created by this manager.

    \sa QtAbstractPropertyBrowser::itemRemoved()
*/

/*!
    \fn void QtAbstractPropertyManager::propertyDestroyed(QtProperty *property)

    This signal is emitted when the specified \a property is about to
    be destroyed.

    Note that signal is only emitted for properties that are created
    by this manager.

    \sa clear(), uninitializeProperty()
*/

/*!
    \fn void QtAbstractPropertyBrowser::currentItemChanged(QtBrowserItem *current)

    This signal is emitted when the current item changes. The current item is specified by \a current.

    \sa QtAbstractPropertyBrowser::setCurrentItem()
*/

/*!
    Creates an abstract property manager with the given \a parent.
*/
QtAbstractPropertyManager::QtAbstractPropertyManager(QObject *parent)
    : QObject(parent)
{
    d_ptr = new QtAbstractPropertyManagerPrivate;
    d_ptr->q_ptr = this;

}

/*!
    Destroys the manager. All properties created by the manager are
    destroyed.
*/
QtAbstractPropertyManager::~QtAbstractPropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Destroys all the properties that this manager has created.

    \sa propertyDestroyed(), uninitializeProperty()
*/
void QtAbstractPropertyManager::clear() const
{
    while (!properties().isEmpty()) {
        QSetIterator<QtProperty *> itProperty(properties());
        QtProperty *prop = itProperty.next();
        delete prop;
    }
}

/*!
    Returns the set of properties created by this manager.

    \sa addProperty()
*/
QSet<QtProperty *> QtAbstractPropertyManager::properties() const
{
    return d_ptr->m_properties;
}

/*!
    Returns whether the given \a property has a value.

    The default implementation of this function returns true.

    \sa QtProperty::hasValue()
*/
bool QtAbstractPropertyManager::hasValue(const QtProperty *property) const
{
    Q_UNUSED(property)
    return true;
}

/*!
    Returns an icon representing the current state of the given \a
    property.

    The default implementation of this function returns an invalid
    icon.

    \sa QtProperty::valueIcon()
*/
QIcon QtAbstractPropertyManager::valueIcon(const QtProperty *property) const
{
    Q_UNUSED(property)
    return QIcon();
}

/*!
    Returns a string representing the current state of the given \a
    property.

    The default implementation of this function returns an empty
    string.

    \sa QtProperty::valueText()
*/
QString QtAbstractPropertyManager::valueText(const QtProperty *property) const
{
    Q_UNUSED(property)
    return QString();
}

/*!
    Returns a string representing the current state of the given \a
    property.

    The default implementation of this function returns an empty
    string.

    \sa QtProperty::valueText()
*/
QString QtAbstractPropertyManager::displayText(const QtProperty *property) const
{
    Q_UNUSED(property)
    return QString();
}

/*!
    Returns the echo mode representing the current state of the given \a
    property.

    The default implementation of this function returns QLineEdit::Normal.

    \sa QtProperty::valueText()
*/
EchoMode QtAbstractPropertyManager::echoMode(const QtProperty *property) const
{
    Q_UNUSED(property)
    return QLineEdit::Normal;
}

/*!
    Creates a property with the given \a name which then is owned by this manager.

    Internally, this function calls the createProperty() and
    initializeProperty() functions.

    \sa initializeProperty(), properties()
*/
QtProperty *QtAbstractPropertyManager::addProperty(const QString &name)
{
    QtProperty *property = createProperty();
    if (property) {
        property->setPropertyName(name);
        d_ptr->m_properties.insert(property);
        initializeProperty(property);
    }
    return property;
}

/*!
    Creates a property.

    The base implementation produce QtProperty instances; Reimplement
    this function to make this manager produce objects of a QtProperty
    subclass.

    \sa addProperty(), initializeProperty()
*/
QtProperty *QtAbstractPropertyManager::createProperty()
{
    return new QtProperty(this);
}

/*!
    \fn void QtAbstractPropertyManager::initializeProperty(QtProperty *property) = 0

    This function is called whenever a new valid property pointer has
    been created, passing the pointer as parameter.

    The purpose is to let the manager know that the \a property has
    been created so that it can provide additional attributes for the
    new property, e.g. QtIntPropertyManager adds \l
    {QtIntPropertyManager::value()}{value}, \l
    {QtIntPropertyManager::minimum()}{minimum} and \l
    {QtIntPropertyManager::maximum()}{maximum} attributes. Since each manager
    subclass adds type specific attributes, this function is pure
    virtual and must be reimplemented when deriving from the
    QtAbstractPropertyManager class.

    \sa addProperty(), createProperty()
*/

/*!
    This function is called just before the specified \a property is destroyed.

    The purpose is to let the property manager know that the \a
    property is being destroyed so that it can remove the property's
    additional attributes.

    \sa clear(), propertyDestroyed()
*/
void QtAbstractPropertyManager::uninitializeProperty(QtProperty *property)
{
    Q_UNUSED(property)
}

////////////////////////////////////

/*!
    \class QtAbstractEditorFactoryBase

    \brief The QtAbstractEditorFactoryBase provides an interface for
    editor factories.

    An editor factory is a class that is able to create an editing
    widget of a specified type (e.g. line edits or comboboxes) for a
    given QtProperty object, and it is used in conjunction with the
    QtAbstractPropertyManager and QtAbstractPropertyBrowser classes.

    When using a property browser widget, the properties are created
    and managed by implementations of the QtAbstractPropertyManager
    class. To ensure that the properties' values will be displayed
    using suitable editing widgets, the managers are associated with
    objects of QtAbstractEditorFactory subclasses. The property browser
    will use these associations to determine which factories it should
    use to create the preferred editing widgets.

    Typically, an editor factory is created by subclassing the
    QtAbstractEditorFactory template class which inherits
    QtAbstractEditorFactoryBase. But note that several ready-made
    implementations are available:

    \list
    \o QtCheckBoxFactory
    \o QtDateEditFactory
    \o QtDateTimeEditFactory
    \o QtDoubleSpinBoxFactory
    \o QtEnumEditorFactory
    \o QtLineEditFactory
    \o QtScrollBarFactory
    \o QtSliderFactory
    \o QtSpinBoxFactory
    \o QtTimeEditFactory
    \o QtVariantEditorFactory
    \endlist

    \sa QtAbstractPropertyManager, QtAbstractPropertyBrowser
*/

/*!
    \fn virtual QWidget *QtAbstractEditorFactoryBase::createEditor(QtProperty *property,
        QWidget *parent) = 0

    Creates an editing widget (with the given \a parent) for the given
    \a property.

    This function is reimplemented in QtAbstractEditorFactory template class
    which also provides a pure virtual convenience overload of this
    function enabling access to the property's manager.

    \sa QtAbstractEditorFactory::createEditor()
*/

/*!
    \fn QtAbstractEditorFactoryBase::QtAbstractEditorFactoryBase(QObject *parent = 0)

    Creates an abstract editor factory with the given \a parent.
*/

/*!
    \fn virtual void QtAbstractEditorFactoryBase::breakConnection(QtAbstractPropertyManager *manager) = 0

    \internal

    Detaches property manager from factory.
    This method is reimplemented in QtAbstractEditorFactory template subclass.
    You don't need to reimplement it in your subclasses. Instead implement more convenient
    QtAbstractEditorFactory::disconnectPropertyManager() which gives you access to particular manager subclass.
*/

/*!
    \fn virtual void QtAbstractEditorFactoryBase::managerDestroyed(QObject *manager) = 0

    \internal

    This method is called when property manager is being destroyed.
    Basically it notifies factory not to produce editors for properties owned by \a manager.
    You don't need to reimplement it in your subclass. This method is implemented in
    QtAbstractEditorFactory template subclass.
*/

/*!
    \class QtAbstractEditorFactory

    \brief The QtAbstractEditorFactory is the base template class for editor
    factories.

    An editor factory is a class that is able to create an editing
    widget of a specified type (e.g. line edits or comboboxes) for a
    given QtProperty object, and it is used in conjunction with the
    QtAbstractPropertyManager and QtAbstractPropertyBrowser classes.

    Note that the QtAbstractEditorFactory functions are using the
    PropertyManager template argument class which can be any
    QtAbstractPropertyManager subclass. For example:

    \code
        QtSpinBoxFactory *factory;
        QSet<QtIntPropertyManager *> managers = factory->propertyManagers();
    \endcode

    Note that QtSpinBoxFactory by definition creates editing widgets
    \e only for properties created by QtIntPropertyManager.

    When using a property browser widget, the properties are created
    and managed by implementations of the QtAbstractPropertyManager
    class. To ensure that the properties' values will be displayed
    using suitable editing widgets, the managers are associated with
    objects of QtAbstractEditorFactory subclasses. The property browser will
    use these associations to determine which factories it should use
    to create the preferred editing widgets.

    A QtAbstractEditorFactory object is capable of producing editors for
    several property managers at the same time. To create an
    association between this factory and a given manager, use the
    addPropertyManager() function. Use the removePropertyManager() function to make
    this factory stop producing editors for a given property
    manager. Use the propertyManagers() function to retrieve the set of
    managers currently associated with this factory.

    Several ready-made implementations of the QtAbstractEditorFactory class
    are available:

    \list
    \o QtCheckBoxFactory
    \o QtDateEditFactory
    \o QtDateTimeEditFactory
    \o QtDoubleSpinBoxFactory
    \o QtEnumEditorFactory
    \o QtLineEditFactory
    \o QtScrollBarFactory
    \o QtSliderFactory
    \o QtSpinBoxFactory
    \o QtTimeEditFactory
    \o QtVariantEditorFactory
    \endlist

    When deriving from the QtAbstractEditorFactory class, several pure virtual
    functions must be implemented: the connectPropertyManager() function is
    used by the factory to connect to the given manager's signals, the
    createEditor() function is supposed to create an editor for the
    given property controlled by the given manager, and finally the
    disconnectPropertyManager() function is used by the factory to disconnect
    from the specified manager's signals.

    \sa QtAbstractEditorFactoryBase, QtAbstractPropertyManager
*/

/*!
    \fn QtAbstractEditorFactory::QtAbstractEditorFactory(QObject *parent = 0)

    Creates an editor factory with the given \a parent.

    \sa addPropertyManager()
*/

/*!
    \fn QWidget *QtAbstractEditorFactory::createEditor(QtProperty *property, QWidget *parent)

    Creates an editing widget (with the given \a parent) for the given
    \a property.
*/

/*!
    \fn void QtAbstractEditorFactory::addPropertyManager(PropertyManager *manager)

    Adds the given \a manager to this factory's set of managers,
    making this factory produce editing widgets for properties created
    by the given manager.

    The PropertyManager type is a template argument class, and represents the chosen
    QtAbstractPropertyManager subclass.

    \sa propertyManagers(), removePropertyManager()
*/

/*!
    \fn void QtAbstractEditorFactory::removePropertyManager(PropertyManager *manager)

    Removes the given \a manager from this factory's set of
    managers. The PropertyManager type is a template argument class, and may be
    any QtAbstractPropertyManager subclass.

    \sa propertyManagers(), addPropertyManager()
*/

/*!
    \fn virtual void QtAbstractEditorFactory::connectPropertyManager(PropertyManager *manager) = 0

    Connects this factory to the given \a manager's signals.  The
    PropertyManager type is a template argument class, and represents
    the chosen QtAbstractPropertyManager subclass.

    This function is used internally by the addPropertyManager() function, and
    makes it possible to update an editing widget when the associated
    property's data changes. This is typically done in custom slots
    responding to the signals emitted by the property's manager,
    e.g. QtIntPropertyManager::valueChanged() and
    QtIntPropertyManager::rangeChanged().

    \sa propertyManagers(), disconnectPropertyManager()
*/

/*!
    \fn virtual QWidget *QtAbstractEditorFactory::createEditor(PropertyManager *manager, QtProperty *property,
                QWidget *parent) = 0

    Creates an editing widget with the given \a parent for the
    specified \a property created by the given \a manager. The
    PropertyManager type is a template argument class, and represents
    the chosen QtAbstractPropertyManager subclass.

    This function must be implemented in derived classes: It is
    recommended to store a pointer to the widget and map it to the
    given \a property, since the widget must be updated whenever the
    associated property's data changes. This is typically done in
    custom slots responding to the signals emitted by the property's
    manager, e.g. QtIntPropertyManager::valueChanged() and
    QtIntPropertyManager::rangeChanged().

    \sa connectPropertyManager()
*/

/*!
    \fn virtual void QtAbstractEditorFactory::disconnectPropertyManager(PropertyManager *manager) = 0

    Disconnects this factory from the given \a manager's signals. The
    PropertyManager type is a template argument class, and represents
    the chosen QtAbstractPropertyManager subclass.

    This function is used internally by the removePropertyManager() function.

    \sa propertyManagers(), connectPropertyManager()
*/

/*!
    \fn QSet<PropertyManager *> QtAbstractEditorFactory::propertyManagers() const

    Returns the factory's set of associated managers.  The
    PropertyManager type is a template argument class, and represents
    the chosen QtAbstractPropertyManager subclass.

    \sa addPropertyManager(), removePropertyManager()
*/

/*!
    \fn PropertyManager *QtAbstractEditorFactory::propertyManager(QtProperty *property) const

    Returns the property manager for the given \a property, or 0 if
    the given \a property doesn't belong to any of this factory's
    registered managers.

    The PropertyManager type is a template argument class, and represents the chosen
    QtAbstractPropertyManager subclass.

    \sa propertyManagers()
*/

/*!
    \fn virtual void QtAbstractEditorFactory::managerDestroyed(QObject *manager)

    \internal
    \reimp
*/

////////////////////////////////////
class QtBrowserItemPrivate
{
public:
    QtBrowserItemPrivate(QtAbstractPropertyBrowser *browser, QtProperty *property, QtBrowserItem *parent)
        : m_browser(browser), m_property(property), m_parent(parent), q_ptr(0) {}

    void addChild(QtBrowserItem *index, QtBrowserItem *after);
    void removeChild(QtBrowserItem *index);

    QtAbstractPropertyBrowser * const m_browser;
    QtProperty *m_property;
    QtBrowserItem *m_parent;

    QtBrowserItem *q_ptr;

    QList<QtBrowserItem *> m_children;

};

void QtBrowserItemPrivate::addChild(QtBrowserItem *index, QtBrowserItem *after)
{
    if (m_children.contains(index))
        return;
    int idx = m_children.indexOf(after) + 1; // we insert after returned idx, if it was -1 then we set idx to 0;
    m_children.insert(idx, index);
}

void QtBrowserItemPrivate::removeChild(QtBrowserItem *index)
{
    m_children.removeAll(index);
}


/*!
    \class QtBrowserItem

    \brief The QtBrowserItem class represents a property in
    a property browser instance.

    Browser items are created whenever a QtProperty is inserted to the
    property browser. A QtBrowserItem uniquely identifies a
    browser's item. Thus, if the same QtProperty is inserted multiple
    times, each occurrence gets its own unique QtBrowserItem. The
    items are owned by QtAbstractPropertyBrowser and automatically
    deleted when they are removed from the browser.

    You can traverse a browser's properties by calling parent() and
    children(). The property and the browser associated with an item
    are available as property() and browser().

    \sa QtAbstractPropertyBrowser, QtProperty
*/

/*!
    Returns the property which is accosiated with this item. Note that
    several items can be associated with the same property instance in
    the same property browser.

    \sa QtAbstractPropertyBrowser::items()
*/

QtProperty *QtBrowserItem::property() const
{
    return d_ptr->m_property;
}

/*!
    Returns the parent item of \e this item. Returns 0 if \e this item
    is associated with top-level property in item's property browser.

    \sa children()
*/

QtBrowserItem *QtBrowserItem::parent() const
{
    return d_ptr->m_parent;
}

/*!
    Returns the children items of \e this item. The properties
    reproduced from children items are always the same as
    reproduced from associated property' children, for example:

    \code
        QtBrowserItem *item;
        QList<QtBrowserItem *> childrenItems = item->children();

        QList<QtProperty *> childrenProperties = item->property()->subProperties();
    \endcode

    The \e childrenItems list represents the same list as \e childrenProperties.
*/

QList<QtBrowserItem *> QtBrowserItem::children() const
{
    return d_ptr->m_children;
}

/*!
    Returns the property browser which owns \e this item.
*/

QtAbstractPropertyBrowser *QtBrowserItem::browser() const
{
    return d_ptr->m_browser;
}

QtBrowserItem::QtBrowserItem(QtAbstractPropertyBrowser *browser, QtProperty *property, QtBrowserItem *parent)
{
    d_ptr = new QtBrowserItemPrivate(browser, property, parent);
    d_ptr->q_ptr = this;
}

QtBrowserItem::~QtBrowserItem()
{
    delete d_ptr;
}


////////////////////////////////////

typedef QMap<QtAbstractPropertyBrowser *, QMap<QtAbstractPropertyManager *,
                            QtAbstractEditorFactoryBase *> > Map1;
typedef QMap<QtAbstractPropertyManager *, QMap<QtAbstractEditorFactoryBase *,
                            QList<QtAbstractPropertyBrowser *> > > Map2;
Q_GLOBAL_STATIC(Map1, m_viewToManagerToFactory)
Q_GLOBAL_STATIC(Map2, m_managerToFactoryToViews)

class QtAbstractPropertyBrowserPrivate
{
    QtAbstractPropertyBrowser *q_ptr;
    Q_DECLARE_PUBLIC(QtAbstractPropertyBrowser)
public:
    QtAbstractPropertyBrowserPrivate();

    void insertSubTree(QtProperty *property,
            QtProperty *parentProperty);
    void removeSubTree(QtProperty *property,
            QtProperty *parentProperty);
    void createBrowserIndexes(QtProperty *property, QtProperty *parentProperty, QtProperty *afterProperty);
    void removeBrowserIndexes(QtProperty *property, QtProperty *parentProperty);
    QtBrowserItem *createBrowserIndex(QtProperty *property, QtBrowserItem *parentIndex, QtBrowserItem *afterIndex);
    void removeBrowserIndex(QtBrowserItem *index);
    void clearIndex(QtBrowserItem *index);

    void slotPropertyInserted(QtProperty *property,
            QtProperty *parentProperty, QtProperty *afterProperty);
    void slotPropertyRemoved(QtProperty *property, QtProperty *parentProperty);
    void slotPropertyDestroyed(QtProperty *property);
    void slotPropertyDataChanged(QtProperty *property);

    QList<QtProperty *> m_subItems;
    QMap<QtAbstractPropertyManager *, QList<QtProperty *> > m_managerToProperties;
    QMap<QtProperty *, QList<QtProperty *> > m_propertyToParents;

    QMap<QtProperty *, QtBrowserItem *> m_topLevelPropertyToIndex;
    QList<QtBrowserItem *> m_topLevelIndexes;
    QMap<QtProperty *, QList<QtBrowserItem *> > m_propertyToIndexes;

    QtBrowserItem *m_currentItem;
};

QtAbstractPropertyBrowserPrivate::QtAbstractPropertyBrowserPrivate() :
   m_currentItem(0)
{
}

void QtAbstractPropertyBrowserPrivate::insertSubTree(QtProperty *property,
            QtProperty *parentProperty)
{
    if (m_propertyToParents.contains(property)) {
        // property was already inserted, so its manager is connected
        // and all its children are inserted and theirs managers are connected
        // we just register new parent (parent has to be new).
        m_propertyToParents[property].append(parentProperty);
        // don't need to update m_managerToProperties map since
        // m_managerToProperties[manager] already contains property.
        return;
    }
    QtAbstractPropertyManager *manager = property->propertyManager();
    if (m_managerToProperties[manager].isEmpty()) {
        // connect manager's signals
        q_ptr->connect(manager, SIGNAL(propertyInserted(QtProperty *,
                            QtProperty *, QtProperty *)),
                q_ptr, SLOT(slotPropertyInserted(QtProperty *,
                            QtProperty *, QtProperty *)));
        q_ptr->connect(manager, SIGNAL(propertyRemoved(QtProperty *,
                            QtProperty *)),
                q_ptr, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));
        q_ptr->connect(manager, SIGNAL(propertyDestroyed(QtProperty *)),
                q_ptr, SLOT(slotPropertyDestroyed(QtProperty *)));
        q_ptr->connect(manager, SIGNAL(propertyChanged(QtProperty *)),
                q_ptr, SLOT(slotPropertyDataChanged(QtProperty *)));
    }
    m_managerToProperties[manager].append(property);
    m_propertyToParents[property].append(parentProperty);

    QList<QtProperty *> subList = property->subProperties();
    QListIterator<QtProperty *> itSub(subList);
    while (itSub.hasNext()) {
        QtProperty *subProperty = itSub.next();
        insertSubTree(subProperty, property);
    }
}

void QtAbstractPropertyBrowserPrivate::removeSubTree(QtProperty *property,
            QtProperty *parentProperty)
{
    if (!m_propertyToParents.contains(property)) {
        // ASSERT
        return;
    }

    m_propertyToParents[property].removeAll(parentProperty);
    if (!m_propertyToParents[property].isEmpty())
        return;

    m_propertyToParents.remove(property);
    QtAbstractPropertyManager *manager = property->propertyManager();
    m_managerToProperties[manager].removeAll(property);
    if (m_managerToProperties[manager].isEmpty()) {
        // disconnect manager's signals
        q_ptr->disconnect(manager, SIGNAL(propertyInserted(QtProperty *,
                            QtProperty *, QtProperty *)),
                q_ptr, SLOT(slotPropertyInserted(QtProperty *,
                            QtProperty *, QtProperty *)));
        q_ptr->disconnect(manager, SIGNAL(propertyRemoved(QtProperty *,
                            QtProperty *)),
                q_ptr, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));
        q_ptr->disconnect(manager, SIGNAL(propertyDestroyed(QtProperty *)),
                q_ptr, SLOT(slotPropertyDestroyed(QtProperty *)));
        q_ptr->disconnect(manager, SIGNAL(propertyChanged(QtProperty *)),
                q_ptr, SLOT(slotPropertyDataChanged(QtProperty *)));

        m_managerToProperties.remove(manager);
    }

    QList<QtProperty *> subList = property->subProperties();
    QListIterator<QtProperty *> itSub(subList);
    while (itSub.hasNext()) {
        QtProperty *subProperty = itSub.next();
        removeSubTree(subProperty, property);
    }
}

void QtAbstractPropertyBrowserPrivate::createBrowserIndexes(QtProperty *property, QtProperty *parentProperty, QtProperty *afterProperty)
{
    QMap<QtBrowserItem *, QtBrowserItem *> parentToAfter;
    if (afterProperty) {
        QMap<QtProperty *, QList<QtBrowserItem *> >::ConstIterator it =
            m_propertyToIndexes.find(afterProperty);
        if (it == m_propertyToIndexes.constEnd())
            return;

        QList<QtBrowserItem *> indexes = it.value();
        QListIterator<QtBrowserItem *> itIndex(indexes);
        while (itIndex.hasNext()) {
            QtBrowserItem *idx = itIndex.next();
            QtBrowserItem *parentIdx = idx->parent();
            if ((parentProperty && parentIdx && parentIdx->property() == parentProperty) || (!parentProperty && !parentIdx))
                parentToAfter[idx->parent()] = idx;
        }
    } else if (parentProperty) {
        QMap<QtProperty *, QList<QtBrowserItem *> >::ConstIterator it =
                m_propertyToIndexes.find(parentProperty);
        if (it == m_propertyToIndexes.constEnd())
            return;

        QList<QtBrowserItem *> indexes = it.value();
        QListIterator<QtBrowserItem *> itIndex(indexes);
        while (itIndex.hasNext()) {
            QtBrowserItem *idx = itIndex.next();
            parentToAfter[idx] = 0;
        }
    } else {
        parentToAfter[0] = 0;
    }

    const QMap<QtBrowserItem *, QtBrowserItem *>::ConstIterator pcend = parentToAfter.constEnd();
    for (QMap<QtBrowserItem *, QtBrowserItem *>::ConstIterator it = parentToAfter.constBegin(); it != pcend; ++it)
        createBrowserIndex(property, it.key(), it.value());
}

QtBrowserItem *QtAbstractPropertyBrowserPrivate::createBrowserIndex(QtProperty *property,
        QtBrowserItem *parentIndex, QtBrowserItem *afterIndex)
{
    QtBrowserItem *newIndex = new QtBrowserItem(q_ptr, property, parentIndex);
    if (parentIndex) {
        parentIndex->d_ptr->addChild(newIndex, afterIndex);
    } else {
        m_topLevelPropertyToIndex[property] = newIndex;
        m_topLevelIndexes.insert(m_topLevelIndexes.indexOf(afterIndex) + 1, newIndex);
    }
    m_propertyToIndexes[property].append(newIndex);

    q_ptr->itemInserted(newIndex, afterIndex);

    QList<QtProperty *> subItems = property->subProperties();
    QListIterator<QtProperty *> itChild(subItems);
    QtBrowserItem *afterChild = 0;
    while (itChild.hasNext()) {
        QtProperty *child = itChild.next();
        afterChild = createBrowserIndex(child, newIndex, afterChild);
    }
    return newIndex;
}

void QtAbstractPropertyBrowserPrivate::removeBrowserIndexes(QtProperty *property, QtProperty *parentProperty)
{
    QList<QtBrowserItem *> toRemove;
    QMap<QtProperty *, QList<QtBrowserItem *> >::ConstIterator it =
        m_propertyToIndexes.find(property);
    if (it == m_propertyToIndexes.constEnd())
        return;

    QList<QtBrowserItem *> indexes = it.value();
    QListIterator<QtBrowserItem *> itIndex(indexes);
    while (itIndex.hasNext()) {
        QtBrowserItem *idx = itIndex.next();
        QtBrowserItem *parentIdx = idx->parent();
        if ((parentProperty && parentIdx && parentIdx->property() == parentProperty) || (!parentProperty && !parentIdx))
            toRemove.append(idx);
    }

    QListIterator<QtBrowserItem *> itRemove(toRemove);
    while (itRemove.hasNext()) {
        QtBrowserItem *index = itRemove.next();
        removeBrowserIndex(index);
    }
}

void QtAbstractPropertyBrowserPrivate::removeBrowserIndex(QtBrowserItem *index)
{
    QList<QtBrowserItem *> children = index->children();
    for (int i = children.count(); i > 0; i--) {
        removeBrowserIndex(children.at(i - 1));
    }

    q_ptr->itemRemoved(index);

    if (index->parent()) {
        index->parent()->d_ptr->removeChild(index);
    } else {
        m_topLevelPropertyToIndex.remove(index->property());
        m_topLevelIndexes.removeAll(index);
    }

    QtProperty *property = index->property();

    m_propertyToIndexes[property].removeAll(index);
    if (m_propertyToIndexes[property].isEmpty())
        m_propertyToIndexes.remove(property);

    delete index;
}

void QtAbstractPropertyBrowserPrivate::clearIndex(QtBrowserItem *index)
{
    QList<QtBrowserItem *> children = index->children();
    QListIterator<QtBrowserItem *> itChild(children);
    while (itChild.hasNext()) {
        clearIndex(itChild.next());
    }
    delete index;
}

void QtAbstractPropertyBrowserPrivate::slotPropertyInserted(QtProperty *property,
        QtProperty *parentProperty, QtProperty *afterProperty)
{
    if (!m_propertyToParents.contains(parentProperty))
        return;
    createBrowserIndexes(property, parentProperty, afterProperty);
    insertSubTree(property, parentProperty);
    //q_ptr->propertyInserted(property, parentProperty, afterProperty);
}

void QtAbstractPropertyBrowserPrivate::slotPropertyRemoved(QtProperty *property,
        QtProperty *parentProperty)
{
    if (!m_propertyToParents.contains(parentProperty))
        return;
    removeSubTree(property, parentProperty); // this line should be probably moved down after propertyRemoved call
    //q_ptr->propertyRemoved(property, parentProperty);
    removeBrowserIndexes(property, parentProperty);
}

void QtAbstractPropertyBrowserPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (!m_subItems.contains(property))
        return;
    q_ptr->removeProperty(property);
}

void QtAbstractPropertyBrowserPrivate::slotPropertyDataChanged(QtProperty *property)
{
    if (!m_propertyToParents.contains(property))
        return;

    QMap<QtProperty *, QList<QtBrowserItem *> >::ConstIterator it =
            m_propertyToIndexes.find(property);
    if (it == m_propertyToIndexes.constEnd())
        return;

    QList<QtBrowserItem *> indexes = it.value();
    QListIterator<QtBrowserItem *> itIndex(indexes);
    while (itIndex.hasNext()) {
        QtBrowserItem *idx = itIndex.next();
        q_ptr->itemChanged(idx);
    }
    //q_ptr->propertyChanged(property);
}

/*!
    \class QtAbstractPropertyBrowser

    \brief QtAbstractPropertyBrowser provides a base class for
    implementing property browsers.

    A property browser is a widget that enables the user to edit a
    given set of properties.  Each property is represented by a label
    specifying the property's name, and an editing widget (e.g. a line
    edit or a combobox) holding its value. A property can have zero or
    more subproperties.

    \image qtpropertybrowser.png

    The top level properties can be retrieved using the
    properties() function. To traverse each property's
    subproperties, use the QtProperty::subProperties() function. In
    addition, the set of top level properties can be manipulated using
    the addProperty(), insertProperty() and removeProperty()
    functions. Note that the QtProperty class provides a corresponding
    set of functions making it possible to manipulate the set of
    subproperties as well.

    To remove all the properties from the property browser widget, use
    the clear() function. This function will clear the editor, but it
    will not delete the properties since they can still be used in
    other editors.

    The properties themselves are created and managed by
    implementations of the QtAbstractPropertyManager class. A manager
    can handle (i.e. create and manage) properties of a given type. In
    the property browser the managers are associated with
    implementations of the QtAbstractEditorFactory: A factory is a
    class able to create an editing widget of a specified type.

    When using a property browser widget, managers must be created for
    each of the required property types before the properties
    themselves can be created. To ensure that the properties' values
    will be displayed using suitable editing widgets, the managers
    must be associated with objects of the preferred factory
    implementations using the setFactoryForManager() function. The
    property browser will use these associations to determine which
    factory it should use to create the preferred editing widget.

    Note that a factory can be associated with many managers, but a
    manager can only be associated with one single factory within the
    context of a single property browser.  The associations between
    managers and factories can at any time be removed using the
    unsetFactoryForManager() function.

    Whenever the property data changes or a property is inserted or
    removed, the itemChanged(), itemInserted() or
    itemRemoved() functions are called, respectively. These
    functions must be reimplemented in derived classes in order to
    update the property browser widget. Be aware that some property
    instances can appear several times in an abstract tree
    structure. For example:

    \table 100%
    \row
    \o
    \code
        QtProperty *property1, *property2, *property3;

        property2->addSubProperty(property1);
        property3->addSubProperty(property2);

        QtAbstractPropertyBrowser *editor;

        editor->addProperty(property1);
        editor->addProperty(property2);
        editor->addProperty(property3);
    \endcode
    \o  \image qtpropertybrowser-duplicate.png
    \endtable

    The addProperty() function returns a QtBrowserItem that uniquely
    identifies the created item.

    To make a property editable in the property browser, the
    createEditor() function must be called to provide the
    property with a suitable editing widget.

    Note that there are two ready-made property browser
    implementations:

    \list
        \o QtGroupBoxPropertyBrowser
        \o QtTreePropertyBrowser
    \endlist

    \sa QtAbstractPropertyManager, QtAbstractEditorFactoryBase
*/

/*!
    \fn void QtAbstractPropertyBrowser::setFactoryForManager(PropertyManager *manager,
                    QtAbstractEditorFactory<PropertyManager> *factory)

    Connects the given \a manager to the given \a factory, ensuring
    that properties of the \a manager's type will be displayed with an
    editing widget suitable for their value.

    For example:

    \code
        QtIntPropertyManager *intManager;
        QtDoublePropertyManager *doubleManager;

        QtProperty *myInteger = intManager->addProperty();
        QtProperty *myDouble = doubleManager->addProperty();

        QtSpinBoxFactory  *spinBoxFactory;
        QtDoubleSpinBoxFactory *doubleSpinBoxFactory;

        QtAbstractPropertyBrowser *editor;
        editor->setFactoryForManager(intManager, spinBoxFactory);
        editor->setFactoryForManager(doubleManager, doubleSpinBoxFactory);

        editor->addProperty(myInteger);
        editor->addProperty(myDouble);
    \endcode

    In this example the \c myInteger property's value is displayed
    with a QSpinBox widget, while the \c myDouble property's value is
    displayed with a QDoubleSpinBox widget.

    Note that a factory can be associated with many managers, but a
    manager can only be associated with one single factory.  If the
    given \a manager already is associated with another factory, the
    old association is broken before the new one established.

    This function ensures that the given \a manager and the given \a
    factory are compatible, and it automatically calls the
    QtAbstractEditorFactory::addPropertyManager() function if necessary.

    \sa unsetFactoryForManager()
*/

/*!
    \fn virtual void QtAbstractPropertyBrowser::itemInserted(QtBrowserItem *insertedItem,
        QtBrowserItem *precedingItem) = 0

    This function is called to update the widget whenever a property
    is inserted or added to the property browser, passing pointers to
    the \a insertedItem of property and the specified
    \a precedingItem as parameters.

    If \a precedingItem is 0, the \a insertedItem was put at
    the beginning of its parent item's list of subproperties. If
    the parent of \a insertedItem is 0, the \a insertedItem was added as a top
    level property of \e this property browser.

    This function must be reimplemented in derived classes. Note that
    if the \a insertedItem's property has subproperties, this
    method will be called for those properties as soon as the current call is finished.

    \sa insertProperty(), addProperty()
*/

/*!
    \fn virtual void QtAbstractPropertyBrowser::itemRemoved(QtBrowserItem *item) = 0

    This function is called to update the widget whenever a property
    is removed from the property browser, passing the pointer to the
    \a item of the property as parameters. The passed \a item is
    deleted just after this call is finished.

    If the the parent of \a item is 0, the removed \a item was a
    top level property in this editor.

    This function must be reimplemented in derived classes. Note that
    if the removed \a item's property has subproperties, this
    method will be called for those properties just before the current call is started.

    \sa removeProperty()
*/

/*!
    \fn virtual void QtAbstractPropertyBrowser::itemChanged(QtBrowserItem *item) = 0

    This function is called whenever a property's data changes,
    passing a pointer to the \a item of property as parameter.

    This function must be reimplemented in derived classes in order to
    update the property browser widget whenever a property's name,
    tool tip, status tip, "what's this" text, value text or value icon
    changes.

    Note that if the property browser contains several occurrences of
    the same property, this method will be called once for each
    occurrence (with a different item each time).

    \sa QtProperty, items()
*/

/*!
    Creates an abstract property browser with the given \a parent.
*/
QtAbstractPropertyBrowser::QtAbstractPropertyBrowser(QWidget *parent)
    : QWidget(parent)
{
    d_ptr = new QtAbstractPropertyBrowserPrivate;
    d_ptr->q_ptr = this;

}

/*!
    Destroys the property browser, and destroys all the items that were
    created by this property browser.

    Note that the properties that were displayed in the editor are not
    deleted since they still can be used in other editors. Neither
    does the destructor delete the property managers and editor
    factories that were used by this property browser widget unless
    this widget was their parent.

    \sa QtAbstractPropertyManager::~QtAbstractPropertyManager()
*/
QtAbstractPropertyBrowser::~QtAbstractPropertyBrowser()
{
    QList<QtBrowserItem *> indexes = topLevelItems();
    QListIterator<QtBrowserItem *> itItem(indexes);
    while (itItem.hasNext())
        d_ptr->clearIndex(itItem.next());
    delete d_ptr;
}

/*!
    Returns the property browser's list of top level properties.

    To traverse the subproperties, use the QtProperty::subProperties()
    function.

    \sa addProperty(), insertProperty(), removeProperty()
*/
QList<QtProperty *> QtAbstractPropertyBrowser::properties() const
{
    return d_ptr->m_subItems;
}

/*!
    Returns the property browser's list of all items associated
    with the given \a property.

    There is one item per instance of the property in the browser.

    \sa topLevelItem()
*/

QList<QtBrowserItem *> QtAbstractPropertyBrowser::items(QtProperty *property) const
{
    return d_ptr->m_propertyToIndexes.value(property);
}

/*!
    Returns the top-level items associated with the given \a property.

    Returns 0 if \a property wasn't inserted into this property
    browser or isn't a top-level one.

    \sa topLevelItems(), items()
*/

QtBrowserItem *QtAbstractPropertyBrowser::topLevelItem(QtProperty *property) const
{
    return d_ptr->m_topLevelPropertyToIndex.value(property);
}

/*!
    Returns the list of top-level items.

    \sa topLevelItem()
*/

QList<QtBrowserItem *> QtAbstractPropertyBrowser::topLevelItems() const
{
    return d_ptr->m_topLevelIndexes;
}

/*!
    Removes all the properties from the editor, but does not delete
    them since they can still be used in other editors.

    \sa removeProperty(), QtAbstractPropertyManager::clear()
*/
void QtAbstractPropertyBrowser::clear()
{
    QList<QtProperty *> subList = properties();
    QListIterator<QtProperty *> itSub(subList);
    itSub.toBack();
    while (itSub.hasPrevious()) {
        QtProperty *property = itSub.previous();
        removeProperty(property);
    }
}

/*!
    Appends the given \a property (and its subproperties) to the
    property browser's list of top level properties. Returns the item
    created by property browser which is associated with the \a property.
    In order to get all children items created by the property
    browser in this call, the returned item should be traversed.

    If the specified \a property is already added, this function does
    nothing and returns 0.

    \sa insertProperty(), QtProperty::addSubProperty(), properties()
*/
QtBrowserItem *QtAbstractPropertyBrowser::addProperty(QtProperty *property)
{
    QtProperty *afterProperty = 0;
    if (d_ptr->m_subItems.count() > 0)
        afterProperty = d_ptr->m_subItems.last();
    return insertProperty(property, afterProperty);
}

/*!
    \fn QtBrowserItem *QtAbstractPropertyBrowser::insertProperty(QtProperty *property,
            QtProperty *afterProperty)

    Inserts the given \a property (and its subproperties) after
    the specified \a afterProperty in the browser's list of top
    level properties. Returns item created by property browser which
    is associated with the \a property. In order to get all children items
    created by the property browser in this call returned item should be traversed.

    If the specified \a afterProperty is 0, the given \a property is
    inserted at the beginning of the list.  If \a property is
    already inserted, this function does nothing and returns 0.

    \sa addProperty(), QtProperty::insertSubProperty(), properties()
*/
QtBrowserItem *QtAbstractPropertyBrowser::insertProperty(QtProperty *property,
            QtProperty *afterProperty)
{
    if (!property)
        return 0;

    // if item is already inserted in this item then cannot add.
    QList<QtProperty *> pendingList = properties();
    int pos = 0;
    int newPos = 0;
    while (pos < pendingList.count()) {
        QtProperty *prop = pendingList.at(pos);
        if (prop == property)
            return 0;
        if (prop == afterProperty) {
            newPos = pos + 1;
        }
        pos++;
    }
    d_ptr->createBrowserIndexes(property, 0, afterProperty);

    // traverse inserted subtree and connect to manager's signals
    d_ptr->insertSubTree(property, 0);

    d_ptr->m_subItems.insert(newPos, property);
    //propertyInserted(property, 0, properAfterProperty);
    return topLevelItem(property);
}

/*!
    Removes the specified \a property (and its subproperties) from the
    property browser's list of top level properties. All items
    that were associated with the given \a property and its children
    are deleted.

    Note that the properties are \e not deleted since they can still
    be used in other editors.

    \sa clear(), QtProperty::removeSubProperty(), properties()
*/
void QtAbstractPropertyBrowser::removeProperty(QtProperty *property)
{
    if (!property)
        return;

    QList<QtProperty *> pendingList = properties();
    int pos = 0;
    while (pos < pendingList.count()) {
        if (pendingList.at(pos) == property) {
            d_ptr->m_subItems.removeAt(pos); //perhaps this two lines
            d_ptr->removeSubTree(property, 0); //should be moved down after propertyRemoved call.
            //propertyRemoved(property, 0);

            d_ptr->removeBrowserIndexes(property, 0);

            // when item is deleted, item will call removeItem for top level items,
            // and itemRemoved for nested items.

            return;
        }
        pos++;
    }
}

/*!
    Creates an editing widget (with the given \a parent) for the given
    \a property according to the previously established associations
    between property managers and editor factories.

    If the property is created by a property manager which was not
    associated with any of the existing factories in \e this property
    editor, the function returns 0.

    To make a property editable in the property browser, the
    createEditor() function must be called to provide the
    property with a suitable editing widget.

    Reimplement this function to provide additional decoration for the
    editing widgets created by the installed factories.

    \sa setFactoryForManager()
*/
QWidget *QtAbstractPropertyBrowser::createEditor(QtProperty *property,
                QWidget *parent)
{
    QtAbstractEditorFactoryBase *factory = 0;
    QtAbstractPropertyManager *manager = property->propertyManager();

    if (m_viewToManagerToFactory()->contains(this) &&
        (*m_viewToManagerToFactory())[this].contains(manager)) {
        factory = (*m_viewToManagerToFactory())[this][manager];
    }

    if (!factory)
        return 0;
    return factory->createEditor(property, parent);
}

bool QtAbstractPropertyBrowser::addFactory(QtAbstractPropertyManager *abstractManager,
            QtAbstractEditorFactoryBase *abstractFactory)
{
    bool connectNeeded = false;
    if (!m_managerToFactoryToViews()->contains(abstractManager) ||
        !(*m_managerToFactoryToViews())[abstractManager].contains(abstractFactory)) {
        connectNeeded = true;
    } else if ((*m_managerToFactoryToViews())[abstractManager][abstractFactory]
                    .contains(this)) {
        return connectNeeded;
    }

    if (m_viewToManagerToFactory()->contains(this) &&
        (*m_viewToManagerToFactory())[this].contains(abstractManager)) {
        unsetFactoryForManager(abstractManager);
    }

    (*m_managerToFactoryToViews())[abstractManager][abstractFactory].append(this);
    (*m_viewToManagerToFactory())[this][abstractManager] = abstractFactory;

    return connectNeeded;
}

/*!
    Removes the association between the given \a manager and the
    factory bound to it, automatically calling the
    QtAbstractEditorFactory::removePropertyManager() function if necessary.

    \sa setFactoryForManager()
*/
void QtAbstractPropertyBrowser::unsetFactoryForManager(QtAbstractPropertyManager *manager)
{
    if (!m_viewToManagerToFactory()->contains(this) ||
        !(*m_viewToManagerToFactory())[this].contains(manager)) {
        return;
    }

    QtAbstractEditorFactoryBase *abstractFactory =
                (*m_viewToManagerToFactory())[this][manager];
    (*m_viewToManagerToFactory())[this].remove(manager);
    if ((*m_viewToManagerToFactory())[this].isEmpty()) {
        (*m_viewToManagerToFactory()).remove(this);
    }

    (*m_managerToFactoryToViews())[manager][abstractFactory].removeAll(this);
    if ((*m_managerToFactoryToViews())[manager][abstractFactory].isEmpty()) {
        (*m_managerToFactoryToViews())[manager].remove(abstractFactory);
        abstractFactory->breakConnection(manager);
        if ((*m_managerToFactoryToViews())[manager].isEmpty()) {
            (*m_managerToFactoryToViews()).remove(manager);
        }
    }
}

/*!
    Returns the current item in the property browser.

    \sa setCurrentItem()
*/
QtBrowserItem *QtAbstractPropertyBrowser::currentItem() const
{
    return d_ptr->m_currentItem;
}

/*!
    Sets the current item in the property browser to \a item.

    \sa currentItem(), currentItemChanged()
*/
void QtAbstractPropertyBrowser::setCurrentItem(QtBrowserItem *item)
{
    QtBrowserItem *oldItem = d_ptr->m_currentItem;
    d_ptr->m_currentItem = item;
    if (oldItem != item)
        emit  currentItemChanged(item);
}

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif

#include "moc_qtpropertybrowser.cpp"
