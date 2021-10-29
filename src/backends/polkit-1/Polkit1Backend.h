/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2009 Radek Novacek <rnovacek@redhat.com>
    SPDX-FileCopyrightText: 2009-2010 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef POLKIT1BACKEND_H
#define POLKIT1BACKEND_H

#include "AuthBackend.h"

#include <QEventLoop>
#include <QHash>
#include <QStringList>

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
    ~Polkit1Backend() override;
    void setupAction(const QString &) override;
    void preAuthAction(const QString &action, QWidget *parent) override;
    Action::AuthStatus authorizeAction(const QString &) override;
    Action::AuthStatus actionStatus(const QString &) override;
    QByteArray callerID() const override;
    ExtraCallerIDVerificationMethod extraCallerIDVerificationMethod() const override;
    virtual bool isCallerAuthorized(const QString &action, const QByteArray &callerID, const QVariantMap &details) override;
    bool actionExists(const QString &action) override;
    QVariantMap backendDetails(const DetailsMap &details) override;

private Q_SLOTS:
    void checkForResultChanged();

private:
    QHash<QString, Action::AuthStatus> m_cachedResults;
};

} // namespace Auth

#endif
