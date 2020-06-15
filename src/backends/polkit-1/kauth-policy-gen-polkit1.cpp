/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <policy-gen/policy-gen.h>

#include <cstdio>
#include <QDebug>
#include <QTextStream>

const char header[] = ""
                      "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                      "<!DOCTYPE policyconfig PUBLIC\n"
                      "\"-//freedesktop//DTD PolicyKit Policy Configuration 1.0//EN\"\n"
                      "\"http://www.freedesktop.org/standards/PolicyKit/1.0/policyconfig.dtd\">\n"
                      "<policyconfig>\n";

const char policy_tag[] = ""
                          "      <defaults>\n"
                          "         <allow_inactive>%1</allow_inactive>\n"
                          "         <allow_active>%2</allow_active>\n"
                          "      </defaults>\n";

const char dent[] = "   ";

void output(const QList<Action> &actions, const QMap<QString, QString> &domain)
{
    QTextStream out(stdout);
    out.setCodec("UTF-8");

    out << header;

    // Blacklisted characters + replacements
    QMap< QChar, QString > blacklist;
    blacklist.insert(QLatin1Char('&'), QLatin1String("&amp;"));

    if (domain.contains(QLatin1String("vendor"))) {
        QString vendor = domain[QLatin1String("vendor")];
        for (QMap< QChar, QString >::const_iterator blI = blacklist.constBegin(), total = blacklist.constEnd(); blI != total; ++blI) {
            vendor.replace(blI.key(), blI.value());
        }
        out << "<vendor>" << vendor << "</vendor>\n";
    }
    if (domain.contains(QLatin1String("vendorurl"))) {
        out << "<vendor_url>" << domain[QLatin1String("vendorurl")] << "</vendor_url>\n";
    }
    if (domain.contains(QLatin1String("icon"))) {
        out << "<icon_name>" << domain[QLatin1String("icon")] << "</icon_name>\n";
    }

    for (const Action &action : actions) {
        out << dent << "<action id=\"" << action.name << "\" >\n";

        // Not a typo, messages and descriptions are actually inverted
        for (QMap< QString, QString >::const_iterator i = action.messages.constBegin(); i != action.messages.constEnd(); ++i) {
            out << dent << dent << "<description";
            if (i.key() != QLatin1String("en")) {
                out << " xml:lang=\"" << i.key() << '"';
            }

            QMap< QChar, QString >::const_iterator blI;
            QString description = i.value();
            for (blI = blacklist.constBegin(); blI != blacklist.constEnd(); ++blI) {
                description.replace(blI.key(), blI.value());
            }

            out << '>' << description << "</description>\n";
        }

        for (QMap< QString, QString >::const_iterator i = action.descriptions.constBegin();
                i != action.descriptions.constEnd();
                ++i) {
            out << dent << dent << "<message";
            if (i.key() != QLatin1String("en")) {
                out << " xml:lang=\"" << i.key() << '"';
            }

            QMap< QChar, QString >::const_iterator blI;
            QString message = i.value();
            for (blI = blacklist.constBegin(); blI != blacklist.constEnd(); ++blI) {
                message.replace(blI.key(), blI.value());
            }

            out << '>' << message << "</message>\n";
        }

        QString policy = action.policy;
        QString policyInactive = action.policyInactive.isEmpty() ? QLatin1String("no") : action.policyInactive;
        if (!action.persistence.isEmpty() && policy != QLatin1String("yes") && policy !=
                QLatin1String("no")) {
            policy += QLatin1String("_keep");
        }
        if (!action.persistence.isEmpty() && policyInactive != QLatin1String("yes") && policyInactive !=
                QLatin1String("no")) {
            policyInactive += QLatin1String("_keep");
        }

        out << QString(QLatin1String(policy_tag)).arg(policyInactive, policy);

        out << dent << "</action>\n";
    }

    out << "</policyconfig>\n";
}
