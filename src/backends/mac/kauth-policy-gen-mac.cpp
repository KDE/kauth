/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
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

#include "../../policy-gen/policy-gen.h"

#include <iostream>
#include <Security/Security.h>

#include <QDebug>

using namespace std;

void output(QList<Action> actions, const QMap<QString, QString> &domain)
{
    AuthorizationRef auth;
    AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &auth);

    OSStatus err;

    foreach (const Action &action, actions) {

        err = AuthorizationRightGet(action.name.toLatin1().constData(), NULL);

        if (err != errAuthorizationSuccess) {

            QString rule;

            if (action.policy == QLatin1String("yes")) {
                rule = QString::fromLatin1(kAuthorizationRuleClassAllow);
            } else if (action.policy == QLatin1String("no")) {
                rule = QString::fromLatin1(kAuthorizationRuleClassDeny);
            } else if (action.policy == QLatin1String("auth_self")) {
                rule = QString::fromLatin1(kAuthorizationRuleAuthenticateAsSessionUser);
            } else if (action.policy == QLatin1String("auth_admin")) {
                rule = QString::fromLatin1(kAuthorizationRuleAuthenticateAsAdmin);
            }

            CFStringRef cfRule = CFStringCreateWithCString(NULL, rule.toLatin1().constData(), kCFStringEncodingASCII);
            CFStringRef cfPrompt = CFStringCreateWithCString(NULL, action.descriptions.value(QLatin1String("en")).toLatin1().constData(), kCFStringEncodingASCII);

            err = AuthorizationRightSet(auth, action.name.toLatin1().constData(), cfRule, cfPrompt, NULL, NULL);
            if (err != noErr) {
                cerr << "You don't have the right to edit the security database (try to run cmake with sudo): " << err << endl;
                exit(1);
            } else {
                qInfo() << "Created or updated rule" << rule << "for right entry" << action.name << "policy" << action.policy << "; domain=" << domain;
            }
        }
    }
}
