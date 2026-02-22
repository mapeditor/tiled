
#pragma once

#include <QObject>
#include <QVariant>
#include <QVariantMap>

namespace Tiled {

/**
 * Exposes the current Session to the scripting API as tiled.session.
 *
 * Provides generic get/set access to session values by their string key,
 * as well as access to per-file states.
 */
class ScriptSession : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName)

public:
    explicit ScriptSession(QObject *parent = nullptr);

    QString fileName() const;

    Q_INVOKABLE QVariant get(const QString &key,
                              const QVariant &defaultValue = QVariant()) const;
    Q_INVOKABLE void set(const QString &key, const QVariant &value);
    Q_INVOKABLE bool isSet(const QString &key) const;

    Q_INVOKABLE QVariantMap fileState(const QString &fileName) const;
    Q_INVOKABLE void setFileState(const QString &fileName,
                                  const QVariantMap &fileState);
    Q_INVOKABLE void setFileStateValue(const QString &fileName,
                                       const QString &name,
                                       const QVariant &value);
};

} 
