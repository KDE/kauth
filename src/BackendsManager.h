/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KAUTH_BACKENDS_MANAGER_H
#define KAUTH_BACKENDS_MANAGER_H

#include "AuthBackend.h"
#include "HelperProxy.h"
#include "kauthcore_export.h"

namespace KAuth
{
class KAUTHCORE_EXPORT BackendsManager
{
public:
    ~BackendsManager();

    static BackendsManager &self();

    AuthBackend *authBackend();
    HelperProxy *helperProxy();

private:
    KAUTHCORE_NO_EXPORT void init();
    KAUTHCORE_NO_EXPORT QList<QObject *> retrieveInstancesIn(const QString &path);
    AuthBackend *auth = nullptr;
    HelperProxy *helper = nullptr;
};

} // namespace Auth

#endif
