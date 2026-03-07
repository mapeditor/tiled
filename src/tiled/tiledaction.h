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
    Q_PROPERTY(QString shortcut READ shortcut WRITE setShortcut)
    Q_PROPERTY(QString menu READ menu WRITE setMenu)
    Q_PROPERTY(QString context READ context WRITE setContext)

public:
    explicit QmlAction(QObject *parent = nullptr);

    QString id() const;
    void setId(const QString &id);

    QString iconSource() const;
    void setIconSource(const QString &path);

    QString shortcut() const;
    void setShortcut(const QString &shortcut);

    QString menu() const;
    void setMenu(const QString &menu);

    QString context() const;
    void setContext(const QString &context);

signals:
    void triggeredFromQml();

private:
    QString mId;
    QString mIconSource;
    QString mMenu = QStringLiteral("Extensions");  // default menu
    QString mContext;               // optional context
};

}
