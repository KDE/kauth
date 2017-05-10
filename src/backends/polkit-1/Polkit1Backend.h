/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2009 Radek Novacek <rnovacek@redhat.com>
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

#ifndef POLKIT1BACKEND_H
#define POLKIT1BACKEND_H

#include "AuthBackend.h"

#include <QtCore/QHash>
#include <QtCore/QEventLoop>
#include <QtCore/QStringList>

#include <PolkitQt1/Authority>

class QByteArray;

namespace KAuth
{

class Polkit1Backend : public AuthBackend
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.Polkit1Backend")
    Q_INTERFACES(KAuth::AuthBackend)

public:
    Polkit1Backend();
    virtual ~Polkit1Backend();
    void setupAction(const QString &) Q_DECL_OVERRIDE;
    void preAuthAction(const QString &action, QWidget *parent) Q_DECL_OVERRIDE;
    Action::AuthStatus authorizeAction(const QString &) Q_DECL_OVERRIDE;
    Action::AuthStatus actionStatus(const QString &) Q_DECL_OVERRIDE;
    QByteArray callerID() const Q_DECL_OVERRIDE;
    ExtraCallerIDVerificationMethod extraCallerIDVerificationMethod() const Q_DECL_OVERRIDE;
    bool isCallerAuthorized(const QString &action, QByteArray callerID) Q_DECL_OVERRIDE;
    bool actionExists(const QString &action) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void checkForResultChanged();
    void updateCachedActions(const PolkitQt1::ActionDescription::List &actions);

private:
    QHash<QString, Action::AuthStatus> m_cachedResults;
    QStringList m_knownActions;
    bool m_flyingActions;
};

class PolkitResultEventLoop : public QEventLoop
{
    Q_OBJECT
public:
    PolkitResultEventLoop(QObject *parent = 0);
    virtual ~PolkitResultEventLoop();

    PolkitQt1::Authority::Result result() const;

public Q_SLOTS:
    void requestQuit(const PolkitQt1::Authority::Result &result);

private:
    PolkitQt1::Authority::Result m_result;
};

} // namespace Auth

#endif
