/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2009 Radek Novacek <rnovacek@redhat.com>
*   Copyright (C) 2009-2010 Dario Freddi <drf@kde.org>
*   Copyright (C) 2020 David Edmundson <davidedmundson@kde.org>
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

#include "Polkit1Backend.h"

#include <qplugin.h>
#include <QCoreApplication>
#include <QTimer>

#include <QApplication>
#include <QWidget>

#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <PolkitQt1/Authority>
#include <PolkitQt1/Subject>
#include <polkitqt1-version.h>

#include "kauthdebug.h"

namespace KAuth
{

PolkitResultEventLoop::PolkitResultEventLoop(QObject *parent)
    : QEventLoop(parent)
{
}

PolkitResultEventLoop::~PolkitResultEventLoop()
{
}

void PolkitResultEventLoop::requestQuit(const PolkitQt1::Authority::Result &result)
{
    m_result = result;
    quit();
}

PolkitQt1::Authority::Result PolkitResultEventLoop::result() const
{
    return m_result;
}

Polkit1Backend::Polkit1Backend()
    : AuthBackend()
{
    setCapabilities(AuthorizeFromHelperCapability | CheckActionExistenceCapability | PreAuthActionCapability);

    // Setup useful signals
    connect(PolkitQt1::Authority::instance(), SIGNAL(configChanged()),
            this, SLOT(checkForResultChanged()));
    connect(PolkitQt1::Authority::instance(), SIGNAL(consoleKitDBChanged()),
            this, SLOT(checkForResultChanged()));
}

Polkit1Backend::~Polkit1Backend()
{

}

void Polkit1Backend::preAuthAction(const QString &action, QWidget *parent)
{
    // If a parent was not specified, skip this
    if (!parent) {
        qCDebug(KAUTH) << "Parent widget does not exist, skipping";
        return;
    }

    // Are we running our KDE auth agent?
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.kde.polkit-kde-authentication-agent-1"))) {
        // Check if we actually are entitled to use GUI capabilities
        if (qApp == nullptr || !qobject_cast<QApplication *>(qApp)) {
            qCDebug(KAUTH) << "Not streaming parent as we are on a TTY application";
        }

        // Retrieve the dialog root window Id
        qulonglong wId = parent->effectiveWinId();

        // Send it over the bus to our agent
        QDBusMessage methodCall =
            QDBusMessage::createMethodCall(QLatin1String("org.kde.polkit-kde-authentication-agent-1"), QLatin1String("/org/kde/Polkit1AuthAgent"), QLatin1String("org.kde.Polkit1AuthAgent"),
                                           QLatin1String("setWIdForAction"));

        methodCall << action;
        methodCall << wId;

        QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(methodCall);
        call.waitForFinished();

        if (call.isError()) {
            qCWarning(KAUTH) << "ERROR while streaming the parent!!" << call.error();
        }
    } else {
        qCDebug(KAUTH) << "KDE polkit agent appears too old or not registered on the bus";
    }
}

Action::AuthStatus Polkit1Backend::authorizeAction(const QString &action)
{
    Q_UNUSED(action)
    // Always return Yes here, we'll authorize inside isCallerAuthorized
    return Action::AuthorizedStatus;
}

void Polkit1Backend::setupAction(const QString &action)
{
    m_cachedResults[action] = actionStatus(action);
}

Action::AuthStatus Polkit1Backend::actionStatus(const QString &action)
{
    PolkitQt1::SystemBusNameSubject subject(QString::fromUtf8(callerID()));
    auto authority = PolkitQt1::Authority::instance();
    PolkitQt1::Authority::Result r = authority->checkAuthorizationSync(action, subject,
                                     PolkitQt1::Authority::None);

    if (authority->hasError()) {
        qCDebug(KAUTH) << "Encountered error while checking action status, error code:" << authority->lastError() << authority->errorDetails();
        authority->clearError();
        return Action::InvalidStatus;
    }

    switch (r) {
    case PolkitQt1::Authority::Yes:
        return Action::AuthorizedStatus;
    case PolkitQt1::Authority::No:
    case PolkitQt1::Authority::Unknown:
        return Action::DeniedStatus;
    default:
        return Action::AuthRequiredStatus;
    }
}

QByteArray Polkit1Backend::callerID() const
{
        return QDBusConnection::systemBus().baseService().toUtf8();
}

AuthBackend::ExtraCallerIDVerificationMethod Polkit1Backend::extraCallerIDVerificationMethod() const
{
    return VerifyAgainstDBusServiceName;
}

bool Polkit1Backend::isCallerAuthorized(const QString &action, const QByteArray &callerID, const QVariantMap &details)
{
    PolkitQt1::SystemBusNameSubject subject(QString::fromUtf8(callerID));
    PolkitQt1::Authority *authority = PolkitQt1::Authority::instance();
    QMap<QString, QString> polkit1Details;
    for (auto it = details.cbegin(); it != details.cend(); ++it) {
        polkit1Details.insert(it.key(), it.value().toString());
    }
    PolkitResultEventLoop e;
    connect(authority, SIGNAL(checkAuthorizationFinished(PolkitQt1::Authority::Result)),
            &e, SLOT(requestQuit(PolkitQt1::Authority::Result)));
#if POLKITQT1_IS_VERSION(0, 113, 0)
    authority->checkAuthorizationWithDetails(action, subject, PolkitQt1::Authority::AllowUserInteraction, polkit1Details);
#else
    authority->checkAuthorization(action, subject, PolkitQt1::Authority::AllowUserInteraction);
#endif
    e.exec();

    if (authority->hasError()) {
        qCDebug(KAUTH) << "Encountered error while checking authorization, error code:" << authority->lastError() << authority->errorDetails();
        authority->clearError();
    }

    switch (e.result()) {
    case PolkitQt1::Authority::Yes:
        return true;
    default:
        return false;
    }
}

void Polkit1Backend::checkForResultChanged()
{
    for (auto it = m_cachedResults.begin(); it != m_cachedResults.end(); ++it) {
        const QString action = it.key();
        if (it.value() != actionStatus(action)) {
            *it = actionStatus(action);
            emit actionStatusChanged(action, *it);
        }
    }
}

bool Polkit1Backend::actionExists(const QString &action)
{
    return m_cachedResults.value(action) != Action::InvalidStatus;
}

QVariantMap Polkit1Backend::backendDetails(const DetailsMap &details)
{
    QVariantMap backendDetails;
    for (auto it = details.cbegin(); it != details.cend(); ++it) {
        switch (it.key()) {
            case Action::AuthDetail::DetailMessage:
                backendDetails.insert(QStringLiteral("polkit.message"), it.value());
                break;
            case Action::AuthDetail::DetailOther:
            default:
                backendDetails.insert(QStringLiteral("other_details"), it.value());
                break;
        }
    }
    return backendDetails;
}

} // namespace Auth

