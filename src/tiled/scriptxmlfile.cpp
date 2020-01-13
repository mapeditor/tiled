/*
 * scriptxmlfile.cpp
 * Copyright 2019, Phlosioneer <mattmdrr2@gmail.com>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scriptxmlfile.h"

#include "scriptmanager.h"
#include "scriptfile.h"

#include <QTextStream>
#include <QtXml/QDomNode>

namespace Tiled {

ScriptXMLAttributes::ScriptXMLAttributes(QDomNamedNodeMap map, QDomElement parentNode, QObject *parent)
    : QObject(parent)
    , mMap(map)
    , mParentNode(parentNode)
{}

QVariant ScriptXMLAttributes::get(const QString &key) const
{
    auto node = mMap.namedItem(key);
    if (node.isNull())
        return QVariant();
    else
        return QVariant(node.nodeValue());
}

void ScriptXMLAttributes::set(const QString &key, const QString &value)
{
    auto attribute = mParentNode.ownerDocument().createAttribute(key);
    attribute.setValue(value);
    mMap.setNamedItem(attribute);
}

void ScriptXMLAttributes::remove(const QString &key)
{
    mMap.removeNamedItem(key);
}

void ScriptXMLAttributes::merge(const QVariantMap &other)
{
    for (auto it = other.begin(); it != other.end(); ++it)
        set(it.key(), it.value().toString());
}

QVariantMap ScriptXMLAttributes::toRaw()
{
    QVariantMap ret;
    for (int i = 0; i < mMap.size(); i++) {
        auto attribute = mMap.item(i);
        ret.insert(attribute.nodeName(), attribute.nodeValue());
    }
    return ret;
}

QStringList ScriptXMLAttributes::keys()
{
    QStringList ret;
    for (int i = 0; i < mMap.size(); i++) {
        auto attribute = mMap.item(i);
        ret.append(attribute.nodeName());
    }
    return ret;
}

QStringList ScriptXMLAttributes::values()
{
    QStringList ret;
    for (int i = 0; i < mMap.size(); i++) {
        auto attribute = mMap.item(i);
        ret.append(attribute.nodeValue());
    }
    return ret;
}

int ScriptXMLAttributes::length()
{
    return mMap.length();
}

ScriptXMLList::ScriptXMLList(QDomNodeList list, QDomNode parentNode, QObject *parent)
    : QObject(parent)
    , mList(list)
    , mParentNode(parentNode)
{}

ScriptXMLNode *ScriptXMLList::get(int index)
{
    if (index >= length())
        return nullptr;
    else
        return new ScriptXMLElement(mList.at(index));
}

void ScriptXMLList::insert(int index, ScriptXMLNode *node)
{
    if (index >= length())
        append(node);
    else
        mParentNode.insertBefore(node->node(), mList.at(index));
}

void ScriptXMLList::remove(int index)
{
    if (index >= length())
        return;
    mParentNode.removeChild(mList.at(index));
}

void ScriptXMLList::set(int index, ScriptXMLNode *node)
{
    if (index >= length())
        append(node);
    else
        mParentNode.replaceChild(node->node(), mList.at(index));
}

void ScriptXMLList::append(ScriptXMLNode *node)
{
    mParentNode.appendChild(node->node());
}

void ScriptXMLList::push(ScriptXMLNode *node)
{
    append(node);
}

void ScriptXMLList::clear()
{
    while (mParentNode.hasChildNodes())
        mParentNode.removeChild(mParentNode.lastChild());
}

QVariant ScriptXMLList::find(ScriptXMLNode *node)
{
    auto domNode = node->node();
    for (int i = 0; i < mList.length(); i++) {
        if (mList.at(i) == domNode)
            return QVariant(i);
    }
    return QVariant();
}

QList<ScriptXMLElement*> ScriptXMLList::toRaw()
{
    QList<ScriptXMLElement*> ret;
    ret.reserve(mList.length());
    for (int i = 0; i < mList.length(); i++) {
        ret.append(new ScriptXMLElement(mList.at(i)));
    }
    return ret;
}

int ScriptXMLList::length()
{
    return mList.length();
}

ScriptXMLNode::ScriptXMLNode(QDomNode *node, QObject *parent)
    : QObject(parent)
    , mNode(node)
{
    Q_ASSERT(node);
}

ScriptXMLElement *ScriptXMLNode::childByName(const QString &name)
{
    auto node = mNode->firstChildElement(name);
    if (node.isNull())
        return nullptr;
    else
        return new ScriptXMLElement(node);
}

QList<ScriptXMLElement*> ScriptXMLNode::childrenByName(const QString &name)
{
    QList<ScriptXMLElement*> ret;
    auto children = mNode->childNodes();
    for (int i = 0; i < children.length(); i++) {
        auto current = children.at(i);
        if (current.nodeName().compare(name, Qt::CaseInsensitive) == 0)
            ret.append(new ScriptXMLElement(current));
    }
    return ret;
}

ScriptXMLList *ScriptXMLNode::children()
{
    if (mNode->hasChildNodes())
        return new ScriptXMLList(mNode->childNodes(), *mNode);
    else
        return nullptr;
}

ScriptXMLFile *ScriptXMLNode::root()
{
    auto document = mNode->ownerDocument();
    if (document.isNull())
        return nullptr;
    else
        return new ScriptXMLFile(document);
}

QDomNode ScriptXMLNode::node()
{
    return *mNode;
}

ScriptXMLElement::ScriptXMLElement(QDomNode element, QObject *parent)
    : ScriptXMLNode(&mElement, parent)
    , mElement(element)
{}

ScriptXMLElement::ScriptXMLElement(const QString &name, QObject *parent)
    : ScriptXMLNode(&mElement, parent)
    , mElement(QDomElement())
{
    mElement.toElement().setTagName(name);
}

ScriptXMLAttributes *ScriptXMLElement::attributes()
{
    if (mElement.isElement())
        return new ScriptXMLAttributes(mElement.toElement().attributes(), mElement.toElement());
    else
        return nullptr;
}

QString ScriptXMLElement::name()
{
    return mElement.nodeName();
}

ScriptXMLNode *ScriptXMLElement::parent()
{
    if (mElement.parentNode().isDocument())
        return root();
    else
        return new ScriptXMLElement(mElement.parentNode().toElement(), root());
}

QString ScriptXMLElement::value()
{
    if (mElement.isText())
        return mElement.nodeValue();
    if (mElement.childNodes().count() == 1 && mElement.firstChild().isText())
        return mElement.firstChild().nodeValue();

    // TODO: Should we throw an error here, or log a warning?
    return QString();
}

bool ScriptXMLElement::isTextOnly()
{
    return mElement.isText();
}

void ScriptXMLElement::setValue(const QString &text)
{
    // TODO: This breaks if mElement is neither a text node nor an element node.
    // (e.g. a comment)
    if (mElement.isText()) {
        mElement.toText().setNodeValue(text);
    } else if (mElement.childNodes().length() == 1 && mElement.firstChild().isText()) {
        mElement.firstChild().toText().setNodeValue(text);
    } else if (!mElement.hasChildNodes()) {
        auto textNode = root()->document().createTextNode(text);
        mElement.appendChild(textNode);
    } else {
        while (mElement.hasChildNodes())
            mElement.removeChild(mElement.lastChild());
        auto textNode = root()->document().createTextNode(text);
        mElement.appendChild(textNode);
    }
}

void ScriptXMLElement::setName(const QString &name)
{
    if (mElement.isElement())
        mElement.toElement().setTagName(name);
    else {
        // TODO: Should we throw an error here, or log a warning?
    }
}

ScriptXMLFile::ScriptXMLFile(const QString &source, QObject *parent)
    : ScriptXMLNode(&mDocument, parent)
{
    if (source.isEmpty())
        return;

    QString errorMessage;
    int lineNumber;
    int columnNumber;
    if (!mDocument.setContent(source, false, &errorMessage, &lineNumber, &columnNumber)) {
        auto message = QStringLiteral("Error on line %1, column %2: %3")
                .arg(lineNumber).arg(columnNumber).arg(errorMessage);
        ScriptManager::instance().throwError(message);
        mDocument.setContent(QString());
        return;
    }
}

ScriptXMLFile::ScriptXMLFile(ScriptTextFile *source, QObject *parent)
    : ScriptXMLFile(source->readAll(), parent)
{}

ScriptXMLFile::ScriptXMLFile(QDomDocument document, QObject *parent)
    : ScriptXMLNode(&mDocument, parent)
    , mDocument(document)
{}

QString ScriptXMLFile::writeToString()
{
    QString buffer;
    QTextStream stream(&buffer);
    mDocument.save(stream, 0);
    return buffer;
}

QDomDocument &ScriptXMLFile::document() {
    return mDocument;
}

}
