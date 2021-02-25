/*
    SPDX-FileCopyrightText: 2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "TestBackend.h"

#include <QDebug>

namespace KAuth
{
TestBackend::TestBackend()
    : AuthBackend()
{
    qDebug() << "Test backend loaded";
    setCapabilities(AuthorizeFromHelperCapability | CheckActionExistenceCapability);
}

void TestBackend::setNewCapabilities(AuthBackend::Capabilities capabilities)
{
    qDebug() << "Capabilities changing";
    setCapabilities(capabilities);
}

Action::AuthStatus TestBackend::authorizeAction(const QString &action)
{
    if (action == QLatin1String("doomed.to.fail")) {
        return Action::DeniedStatus;
    }

    return Action::AuthorizedStatus;
}

void TestBackend::setupAction(const QString &action)
{
    if (action == QLatin1String("doomed.to.fail")) {
        m_actionStatuses.insert(action, Action::DeniedStatus);
    } else if (action == QLatin1String("requires.auth") || action == QLatin1String("generates.error")) {
        m_actionStatuses.insert(action, Action::AuthRequiredStatus);
    } else if (action == QLatin1String("always.authorized")) {
        m_actionStatuses.insert(action, Action::AuthorizedStatus);
    } else if (action.startsWith(QLatin1String("org.kde.kf5auth.autotest"))) {
        m_actionStatuses.insert(action, Action::AuthRequiredStatus);
    }
}

Action::AuthStatus TestBackend::actionStatus(const QString &action)
{
    if (m_actionStatuses.contains(action)) {
        return m_actionStatuses.value(action);
    }

    return Action::InvalidStatus;
}

QByteArray TestBackend::callerID() const
{
    return QByteArray("a random caller Id");
}

bool TestBackend::isCallerAuthorized(const QString &action, const QByteArray &callerId, const QVariantMap &details)
{
    Q_UNUSED(details);

    if (action == QLatin1String("doomed.to.fail")) {
        return false;
    } else if (action == QLatin1String("requires.auth")) {
        m_actionStatuses.insert(action, Action::AuthorizedStatus);
        Q_EMIT actionStatusChanged(action, Action::AuthorizedStatus);
        return true;
    } else if (action == QLatin1String("generates.error")) {
        m_actionStatuses.insert(action, Action::ErrorStatus);
        Q_EMIT actionStatusChanged(action, Action::ErrorStatus);
        return false;
    } else if (action == QLatin1String("always.authorized")) {
        return true;
    } else if (action.startsWith(QLatin1String("org.kde.kf5auth.autotest"))) {
        qDebug() << "Caller ID:" << callerId;
        if (callerId == callerID()) {
            m_actionStatuses.insert(action, Action::AuthorizedStatus);
            Q_EMIT actionStatusChanged(action, Action::AuthorizedStatus);
            return true;
        } else {
            m_actionStatuses.insert(action, Action::DeniedStatus);
            Q_EMIT actionStatusChanged(action, Action::DeniedStatus);
        }
    }

    return false;
}

bool TestBackend::actionExists(const QString &action)
{
    qDebug() << "Checking if action " << action << "exists";
    /* clang-format off */
    if (action != QLatin1String("doomed.to.fail")
        && action != QLatin1String("requires.auth")
        && action != QLatin1String("generates.error")
        && action != QLatin1String("always.authorized")
        && action != QLatin1String("/safinvalid124%$&")
        && !action.startsWith(QLatin1String("org.kde.kf5auth.autotest"))) { /* clang-format on */
        return false;
    }

    return true;
}

} // namespace Auth
