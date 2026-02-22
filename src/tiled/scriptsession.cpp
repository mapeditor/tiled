#include "scriptsession.h"

#include "session.h"

namespace Tiled {

ScriptSession::ScriptSession(QObject *parent)
    : QObject(parent)
{}

QString ScriptSession::fileName() const
{
    if (!Session::hasCurrent())
        return QString();
    return Session::current().fileName();
}

QVariant ScriptSession::get(const QString &key,
                            const QVariant &defaultValue) const
{
    if (!Session::hasCurrent())
        return defaultValue;

    const QByteArray latin1Key = key.toLatin1();
    auto &session = Session::current();

    if (!session.isSet(latin1Key.constData()))
        return defaultValue;

    return session.get<QVariant>(latin1Key.constData());
}

void ScriptSession::set(const QString &key, const QVariant &value)
{
    if (!Session::hasCurrent())
        return;
    Session::current().set(key.toLatin1().constData(), value);
}

bool ScriptSession::isSet(const QString &key) const
{
    if (!Session::hasCurrent())
        return false;
    return Session::current().isSet(key.toLatin1().constData());
}

QVariantMap ScriptSession::fileState(const QString &fileName) const
{
    if (!Session::hasCurrent())
        return {};
    return Session::current().fileState(fileName);
}

void ScriptSession::setFileState(const QString &fileName,
                                 const QVariantMap &fileState)
{
    if (!Session::hasCurrent())
        return;
    Session::current().setFileState(fileName, fileState);
}

void ScriptSession::setFileStateValue(const QString &fileName,
                                      const QString &name,
                                      const QVariant &value)
{
    if (!Session::hasCurrent())
        return;
    Session::current().setFileStateValue(fileName, name, value);
}

} // namespace Tiled

#include "moc_scriptsession.cpp"
