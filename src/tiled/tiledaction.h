#pragma once

#include <QAction>
#include <QString>

namespace Tiled {

class QmlAction : public QAction
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString iconSource READ iconSource WRITE setIconSource)
    Q_PROPERTY(QString shortcut READ shortcut WRITE setShortcutString)

public:
    explicit QmlAction(QObject *parent = nullptr);

    QString id() const;
    void setId(const QString &id);

    QString iconSource() const;
    void setIconSource(const QString &path);

    QString shortcut() const;
    void setShortcutString(const QString &shortcut);

signals:
    void triggeredFromQml();   // forwarded to QML

private:
    QString mId;
    QString mIconSource;
};

}
