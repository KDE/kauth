/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2009-2010 Dario Freddi <drf@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 2.1 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*/

#include "DBusHelperProxy.h"

#include <QtCore/qplugin.h>
#include <QObject>
#include <QMap>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QTimer>

#include "BackendsManager.h"
#include "kf5authadaptor.h"
#include "kauthdebug.h"

namespace KAuth
{

static void debugMessageReceived(int t, const QString &message);

DBusHelperProxy::DBusHelperProxy()
    : responder(nullptr)
    , m_stopRequest(false)
    , m_busConnection(QDBusConnection::systemBus())
{
}

DBusHelperProxy::DBusHelperProxy(const QDBusConnection &busConnection)
    : responder(nullptr)
    , m_stopRequest(false)
    , m_busConnection(busConnection)
{
}

DBusHelperProxy::~DBusHelperProxy()
{
    // workaround for Qt 5.6.0 bug which leads to kded5 deadlocking when a program using kauth exits
    // See: https://bugreports.qt.io/browse/QTBUG-51648
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) && QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
    disconnect();
#endif
}

void DBusHelperProxy::stopAction(const QString &action, const QString &helperID)
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, QLatin1String("/"), QLatin1String("org.kde.kf5auth"), QLatin1String("stopAction"));

    QList<QVariant> args;
    args << action;
    message.setArguments(args);

    m_busConnection.asyncCall(message);
}

void DBusHelperProxy::executeAction(const QString &action, const QString &helperID, const QVariantMap &arguments, int timeout)
{
    QByteArray blob;
    {
        QDataStream stream(&blob, QIODevice::WriteOnly);
        stream << arguments;
    }

    //on unit tests we won't have a service, but the service will already be running
    const auto reply = m_busConnection.interface()->startService(helperID);
    if (!reply.isValid() && !m_busConnection.interface()->isServiceRegistered(helperID)) {
        ActionReply errorReply = ActionReply::DBusErrorReply();
        errorReply.setErrorDescription(tr("DBus Backend error: service start %1 failed: %2").arg(helperID, reply.error().message()));
        emit actionPerformed(action, errorReply);
        return;
    }

    const bool connected = m_busConnection.connect(helperID, QLatin1String("/"), QLatin1String("org.kde.kf5auth"), QLatin1String("remoteSignal"), this, SLOT(remoteSignalReceived(int,QString,QByteArray)));

    //if already connected reply will be false but we won't have an error or a reason to fail
    if (!connected && m_busConnection.lastError().isValid()) {
        ActionReply errorReply = ActionReply::DBusErrorReply();
        errorReply.setErrorDescription(tr("DBus Backend error: connection to helper failed. %1").arg(m_busConnection.lastError().message()));
        emit actionPerformed(action, errorReply);
        return;
    }

    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, QLatin1String("/"), QLatin1String("org.kde.kf5auth"), QLatin1String("performAction"));

    QList<QVariant> args;
    args << action << BackendsManager::authBackend()->callerID() << blob;
    message.setArguments(args);

    m_actionsInProgress.push_back(action);

    QDBusPendingCall pendingCall = m_busConnection.asyncCall(message, timeout);

    auto watcher = new QDBusPendingCallWatcher(pendingCall, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, action, watcher]() {
        watcher->deleteLater();

        const QDBusMessage reply = watcher->reply();

        if (reply.type() == QDBusMessage::ErrorMessage) {
            ActionReply r = ActionReply::DBusErrorReply();
            r.setErrorDescription(tr("DBus Backend error: could not contact the helper. "
                                    "Connection error: %1. Message error: %2").arg(reply.errorMessage(), m_busConnection.lastError().message()));
            qCWarning(KAUTH) << reply.errorMessage();

            emit actionPerformed(action, r);
        }
    });
}

Action::AuthStatus DBusHelperProxy::authorizeAction(const QString &action, const QString &helperID)
{
    if (!m_actionsInProgress.isEmpty()) {
        return Action::ErrorStatus;
    }

    m_busConnection.interface()->startService(helperID);

    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, QLatin1String("/"), QLatin1String("org.kde.kf5auth"), QLatin1String("authorizeAction"));

    QList<QVariant> args;
    args << action << BackendsManager::authBackend()->callerID();
    message.setArguments(args);

    m_actionsInProgress.push_back(action);

    QEventLoop e;
    QDBusPendingCall pendingCall = m_busConnection.asyncCall(message);
    QDBusPendingCallWatcher watcher(pendingCall, this);
    connect(&watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), &e, SLOT(quit()));
    e.exec();

    m_actionsInProgress.removeOne(action);

    QDBusMessage reply = pendingCall.reply();

    if (reply.type() == QDBusMessage::ErrorMessage || reply.arguments().size() != 1) {
        return Action::ErrorStatus;
    }

    return static_cast<Action::AuthStatus>(reply.arguments().first().toUInt());
}

bool DBusHelperProxy::initHelper(const QString &name)
{
    new Kf5authAdaptor(this);

    if (!m_busConnection.registerService(name)) {
        qDebug() << "couldn't register service" << m_busConnection.lastError();
        return false;
    }

    if (!m_busConnection.registerObject(QLatin1String("/"), this)) {
        qDebug() << "couldn't register object" << m_busConnection.lastError();
        return false;
    }

    m_name = name;

    return true;
}

void DBusHelperProxy::setHelperResponder(QObject *o)
{
    responder = o;
}

void DBusHelperProxy::remoteSignalReceived(int t, const QString &action, QByteArray blob)
{
    SignalType type = (SignalType)t;
    QDataStream stream(&blob, QIODevice::ReadOnly);

    if (type == ActionStarted) {
        emit actionStarted(action);
    } else if (type == ActionPerformed) {
        ActionReply reply = ActionReply::deserialize(blob);

        m_actionsInProgress.removeOne(action);
        emit actionPerformed(action, reply);
    } else if (type == DebugMessage) {
        int level;
        QString message;

        stream >> level >> message;

        debugMessageReceived(level, message);
    } else if (type == ProgressStepIndicator) {
        int step;
        stream >> step;

        emit progressStep(action, step);
    } else if (type == ProgressStepData) {
        QVariantMap data;
        stream >> data;

        emit progressStep(action, data);
    }
}

void DBusHelperProxy::stopAction(const QString &action)
{
    Q_UNUSED(action)
//#warning FIXME: The stop request should be action-specific rather than global
    m_stopRequest = true;
}

bool DBusHelperProxy::hasToStopAction()
{
    QEventLoop loop;
    loop.processEvents(QEventLoop::AllEvents);

    return m_stopRequest;
}

QByteArray DBusHelperProxy::performAction(const QString &action, const QByteArray &callerID, QByteArray arguments)
{
    if (!responder) {
        return ActionReply::NoResponderReply().serialized();
    }

    if (!m_currentAction.isEmpty()) {
        return ActionReply::HelperBusyReply().serialized();
    }

    QVariantMap args;
    QDataStream s(&arguments, QIODevice::ReadOnly);
    s >> args;

    m_currentAction = action;
    emit remoteSignal(ActionStarted, action, QByteArray());
    QEventLoop e;
    e.processEvents(QEventLoop::AllEvents);

    ActionReply retVal;

    QTimer *timer = responder->property("__KAuth_Helper_Shutdown_Timer").value<QTimer *>();
    timer->stop();

    if (BackendsManager::authBackend()->isCallerAuthorized(action, callerID)) {
        QString slotname = action;
        if (slotname.startsWith(m_name + QLatin1Char('.'))) {
            slotname = slotname.right(slotname.length() - m_name.length() - 1);
        }

        slotname.replace(QLatin1Char('.'), QLatin1Char('_'));

        bool success = QMetaObject::invokeMethod(responder, slotname.toLatin1().data(), Qt::DirectConnection,
                       Q_RETURN_ARG(ActionReply, retVal), Q_ARG(QVariantMap, args));

        if (!success) {
            retVal = ActionReply::NoSuchActionReply();
        }

    } else {
        retVal = ActionReply::AuthorizationDeniedReply();
    }

    timer->start();

    emit remoteSignal(ActionPerformed, action, retVal.serialized());
    e.processEvents(QEventLoop::AllEvents);
    m_currentAction.clear();
    m_stopRequest = false;

    return retVal.serialized();
}

uint DBusHelperProxy::authorizeAction(const QString &action, const QByteArray &callerID)
{
    if (!m_currentAction.isEmpty()) {
        return static_cast<uint>(Action::ErrorStatus);
    }

    m_currentAction = action;

    uint retVal;

    QTimer *timer = responder->property("__KAuth_Helper_Shutdown_Timer").value<QTimer *>();
    timer->stop();

    if (BackendsManager::authBackend()->isCallerAuthorized(action, callerID)) {
        retVal = static_cast<uint>(Action::AuthorizedStatus);
    } else {
        retVal = static_cast<uint>(Action::DeniedStatus);
    }

    timer->start();
    m_currentAction.clear();

    return retVal;
}

void DBusHelperProxy::sendDebugMessage(int level, const char *msg)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);

    stream << level << QString::fromLocal8Bit(msg);

    emit remoteSignal(DebugMessage, m_currentAction, blob);
}

void DBusHelperProxy::sendProgressStep(int step)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);

    stream << step;

    emit remoteSignal(ProgressStepIndicator, m_currentAction, blob);
}

void DBusHelperProxy::sendProgressStep(const QVariantMap &data)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);

    stream << data;

    emit remoteSignal(ProgressStepData, m_currentAction, blob);
}

void debugMessageReceived(int t, const QString &message)
{
    QtMsgType type = (QtMsgType)t;
    switch (type) {
    case QtDebugMsg:
        qDebug("Debug message from helper: %s", message.toLatin1().data());
        break;
    case QtInfoMsg:
        qInfo("Info message from helper: %s", message.toLatin1().data());
        break;
    case QtWarningMsg:
        qWarning("Warning from helper: %s", message.toLatin1().data());
        break;
    case QtCriticalMsg:
        qCritical("Critical warning from helper: %s", message.toLatin1().data());
        break;
    case QtFatalMsg:
        qFatal("Fatal error from helper: %s", message.toLatin1().data());
        break;
    }
}

} // namespace Auth
