/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BACKENDS_MANAGER_H
#define BACKENDS_MANAGER_H

#include "AuthBackend.h"
#include "HelperProxy.h"
#include <kauthcore_export.h>

namespace KAuth
{
class KAUTHCORE_EXPORT BackendsManager
{
    static AuthBackend *auth;
    static HelperProxy *helper;

    BackendsManager();

public:
    static AuthBackend *authBackend();
    static HelperProxy *helperProxy();

private:
    static void init();
    static QList<QObject *> retrieveInstancesIn(const QString &path);
};

} // namespace Auth

#endif
