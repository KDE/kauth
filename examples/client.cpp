/*
 *   Copyright (C) 2017 Elvis Angelaccio <elvis.angelaccio@kde.org>
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

#undef QT_NO_CAST_FROM_ASCII

#include <KAuth>

#include <QCoreApplication>
#include <QDebug>

using namespace KAuth;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QString filename = "foo.txt";

    //! [client_how_to_call_helper]
    QVariantMap args;
    args["filename"] = filename;
    Action readAction("org.kde.kf5auth.example.read");
    readAction.setHelperId("org.kde.kf5auth.example");
    readAction.setArguments(args);
    ExecuteJob *job = readAction.execute();
    if (!job->exec()) {
       qDebug() << "KAuth returned an error code:" << job->error();
    } else {
       QString contents = job->data()["contents"].toString();
    }
    //! [client_how_to_call_helper]

    return app.exec();
}

