/*
    SPDX-FileCopyrightText: 2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BACKENDS_MANAGER_H
#define BACKENDS_MANAGER_H

#include "AuthBackend.h"
#include "HelperProxy.h"

namespace KAuth
{
class BackendsManager
{
public:
    ~BackendsManager();

    static BackendsManager &self();

    AuthBackend *authBackend();
    HelperProxy *helperProxy();
    void setProxyForThread(QThread *thread, HelperProxy *proxy);

private:
    void init();
    AuthBackend *auth = nullptr;
    QHash<QThread *, HelperProxy *> proxiesForThreads;
};

} // namespace Auth

#endif
