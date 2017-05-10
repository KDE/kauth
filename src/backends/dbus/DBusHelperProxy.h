/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2009 Dario Freddi <drf@kde.org>
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

#ifndef DBUS_HELPER_PROXY_H
#define DBUS_HELPER_PROXY_H

#include "HelperProxy.h"
#include "kauthactionreply.h"

#include <QDBusConnection>
#include <QDBusContext>
#include <QVariant>

namespace KAuth
{

class DBusHelperProxy : public HelperProxy, protected QDBusContext
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.DBusHelperProxy")
    Q_INTERFACES(KAuth::HelperProxy)

    QObject *responder;
    QString m_name;
    QString m_currentAction;
    bool m_stopRequest;
    QList<QString> m_actionsInProgress;
    QDBusConnection m_busConnection;

    enum SignalType {
        ActionStarted, // The blob argument is empty
        ActionPerformed, // The blob argument contains the ActionReply
        DebugMessage, // The blob argument contains the debug level and the message (in this order)
        ProgressStepIndicator, // The blob argument contains the step indicator
        ProgressStepData    // The blob argument contains the QVariantMap
    };

public:
    DBusHelperProxy();
    DBusHelperProxy(const QDBusConnection &busConnection);

    virtual ~DBusHelperProxy();

    virtual void executeAction(const QString &action, const QString &helperID,
                               const QVariantMap &arguments, int timeout = -1) Q_DECL_OVERRIDE;
    Action::AuthStatus authorizeAction(const QString &action, const QString &helperID) Q_DECL_OVERRIDE;
    void stopAction(const QString &action, const QString &helperID) Q_DECL_OVERRIDE;

    bool initHelper(const QString &name) Q_DECL_OVERRIDE;
    void setHelperResponder(QObject *o) Q_DECL_OVERRIDE;
    bool hasToStopAction() Q_DECL_OVERRIDE;
    void sendDebugMessage(int level, const char *msg) Q_DECL_OVERRIDE;
    void sendProgressStep(int step) Q_DECL_OVERRIDE;
    void sendProgressStep(const QVariantMap &data) Q_DECL_OVERRIDE;

public Q_SLOTS:
    void stopAction(const QString &action);
    QByteArray performAction(const QString &action, const QByteArray &callerID, QByteArray arguments);
    uint authorizeAction(const QString &action, const QByteArray &callerID);

Q_SIGNALS:
    void remoteSignal(int type, const QString &action, const QByteArray &blob); // This signal is sent from the helper to the app

private Q_SLOTS:
    void remoteSignalReceived(int type, const QString &action, QByteArray blob);

private:
    bool isCallerAuthorized(const QString &action, const QByteArray &callerID);
};

} // namespace Auth

#endif
