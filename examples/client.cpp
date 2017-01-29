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

#include "client.moc"
