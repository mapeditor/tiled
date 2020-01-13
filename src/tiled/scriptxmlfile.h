
#pragma once

#include <QObject>
#include <QVector>
#include <QSharedPointer>
#include <QMap>
#include <QtXml/QDomDocument>
#include <QVariant>


namespace Tiled {

class ScriptXMLNode;
class ScriptXMLFile;
class ScriptXMLElement;
class ScriptTextFile;

class ScriptXMLAttributes : public QObject {
    Q_OBJECT

    Q_PROPERTY(QStringList keys READ keys)
    Q_PROPERTY(QStringList values READ values)
    Q_PROPERTY(int length READ length)
public:
    explicit ScriptXMLAttributes(QDomNamedNodeMap map, QDomElement parentNode, QObject *parent = nullptr);

    Q_INVOKABLE QVariant get(const QString &key) const;
    Q_INVOKABLE void set(const QString &key, const QString &value);
    Q_INVOKABLE void remove(const QString &key);
    Q_INVOKABLE void merge(const QVariantMap &other);
    Q_INVOKABLE QVariantMap toRaw();

    QStringList keys();
    QStringList values();
    int length();

private:
    QDomNamedNodeMap mMap;
    QDomElement mParentNode;
};

class ScriptXMLList : public QObject {
    Q_OBJECT

    Q_PROPERTY(int length READ length)

public:
    explicit ScriptXMLList(QDomNodeList list, QDomNode parentNode, QObject *parent = nullptr);

    Q_INVOKABLE Tiled::ScriptXMLNode *get(int index);
    Q_INVOKABLE void insert(int index, Tiled::ScriptXMLNode *node);
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE void set(int index, Tiled::ScriptXMLNode *node);
    Q_INVOKABLE void append(Tiled::ScriptXMLNode *node);
    Q_INVOKABLE void push(Tiled::ScriptXMLNode *node);
    Q_INVOKABLE void clear();

    Q_INVOKABLE QVariant find(Tiled::ScriptXMLNode *node);
    Q_INVOKABLE QList<ScriptXMLElement*> toRaw();

    int length();

private:
    QDomNodeList mList;
    QDomNode mParentNode;
};

class ScriptXMLNode : public QObject {
    Q_OBJECT

    Q_PROPERTY(Tiled::ScriptXMLList *children READ children)
    Q_PROPERTY(Tiled::ScriptXMLFile *root READ root)

public:
    explicit ScriptXMLNode(QDomNode *node, QObject *parent = nullptr);

    Q_INVOKABLE Tiled::ScriptXMLElement *childByName(const QString &name);
    Q_INVOKABLE QList<Tiled::ScriptXMLElement*> childrenByName(const QString &name);

    ScriptXMLList *children();
    ScriptXMLFile *root();

    QDomNode node();

private:
    QDomNode *mNode;
};

/**
 * Represents either an element node or a text node.
 */
class ScriptXMLElement : public ScriptXMLNode {
    Q_OBJECT

    Q_PROPERTY(Tiled::ScriptXMLAttributes *attributes READ attributes)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(Tiled::ScriptXMLNode *parent READ parent)
    Q_PROPERTY(bool isTextOnly READ isTextOnly)
    Q_PROPERTY(QString value READ value WRITE setValue)

public:
    explicit ScriptXMLElement(QDomNode element, QObject *parent = nullptr);
    Q_INVOKABLE explicit ScriptXMLElement(const QString &name = QString(), QObject *parent = nullptr);

    ScriptXMLAttributes *attributes();
    QString name();
    ScriptXMLNode *parent();
    QString value();
    bool isTextOnly();

    void setValue(const QString &text);
    void setName(const QString &name);
private:
    QDomNode mElement;
};

class ScriptXMLFile : public ScriptXMLNode {
    Q_OBJECT

public:
    Q_INVOKABLE explicit ScriptXMLFile(Tiled::ScriptTextFile *source, QObject *parent = nullptr);
    Q_INVOKABLE explicit ScriptXMLFile(const QString &source = {}, QObject *parent = nullptr);
    explicit ScriptXMLFile(QDomDocument document, QObject *parent = nullptr);

    Q_INVOKABLE QString writeToString();

    QDomDocument &document();

private:
    QDomDocument mDocument;
};

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::ScriptXMLElement*);
Q_DECLARE_METATYPE(Tiled::ScriptXMLAttributes*);
Q_DECLARE_METATYPE(Tiled::ScriptXMLList*);
Q_DECLARE_METATYPE(Tiled::ScriptXMLNode*);
Q_DECLARE_METATYPE(Tiled::ScriptXMLFile*);

