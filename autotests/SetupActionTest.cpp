/*
    SPDX-FileCopyrightText: 2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "BackendsManager.h"
#include <QTest>
#include <kauth/actionreply.h>
#include <kauth/executejob.h>

class SetupActionTest : public QObject
{
    Q_OBJECT

public:
    SetupActionTest(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:
    void initTestCase();
    void init()
    {
    }

    void testNonExistentAction();
#if KAUTHCORE_BUILD_DEPRECATED_SINCE(5, 71)
    void testBasicActionPropertiesDeprecated();
#endif
    void testBasicActionProperties();
    void testUserAuthorization();
    void testAuthorizationFail();

    void cleanup()
    {
    }
    void cleanupTestCase()
    {
    }

Q_SIGNALS:
    void changeCapabilities(KAuth::AuthBackend::Capabilities capabilities);

private:
};

void SetupActionTest::initTestCase()
{
    connect(this,
            SIGNAL(changeCapabilities(KAuth::AuthBackend::Capabilities)),
            KAuth::BackendsManager::authBackend(),
            SLOT(setNewCapabilities(KAuth::AuthBackend::Capabilities)));
}

void SetupActionTest::testNonExistentAction()
{
    Q_EMIT changeCapabilities(KAuth::AuthBackend::AuthorizeFromHelperCapability | KAuth::AuthBackend::CheckActionExistenceCapability);
    KAuth::Action action(QLatin1String("i.do.not.exist"));
    QVERIFY(!action.isValid());

    action = KAuth::Action(QLatin1String("/safinvalid124%$&"));
    QVERIFY(action.isValid());

    // Now with regexp check
    Q_EMIT changeCapabilities(KAuth::AuthBackend::NoCapability);

    action = KAuth::Action(QLatin1String("/safinvalid124%$&"));
    QVERIFY(!action.isValid());
}

#if KAUTHCORE_BUILD_DEPRECATED_SINCE(5, 71)
void SetupActionTest::testBasicActionPropertiesDeprecated()
{
    Q_EMIT changeCapabilities(KAuth::AuthBackend::AuthorizeFromHelperCapability | KAuth::AuthBackend::CheckActionExistenceCapability);
    KAuth::Action::DetailsMap detailsMap{{KAuth::Action::AuthDetail::DetailOther, QLatin1String("details")}};
    KAuth::Action action(QLatin1String("always.authorized"), QLatin1String("details"));
    QVERIFY(action.isValid());

    QCOMPARE(action.name(), QLatin1String("always.authorized"));
    QCOMPARE(action.details(), QLatin1String("details"));
    QCOMPARE(action.detailsV2(), detailsMap);
    QVERIFY(!action.hasHelper());
    QVERIFY(action.helperId().isEmpty());
    QCOMPARE(action.status(), KAuth::Action::AuthorizedStatus);

    QVERIFY(action.arguments().isEmpty());
    QVariantMap args;
    args.insert(QLatin1String("akey"), QVariant::fromValue(42));
    action.setArguments(args);
    QCOMPARE(action.arguments(), args);

    action.setName(QLatin1String("i.do.not.exist"));
    QVERIFY(!action.isValid());

    Q_EMIT changeCapabilities(KAuth::AuthBackend::NoCapability);

    action = KAuth::Action(QLatin1String("i.do.not.exist"), QLatin1String("details"));

    QVERIFY(action.isValid());
    QCOMPARE(action.name(), QLatin1String("i.do.not.exist"));
    QCOMPARE(action.details(), QLatin1String("details"));
    QCOMPARE(action.detailsV2(), detailsMap);
    QVERIFY(!action.hasHelper());
    QVERIFY(action.helperId().isEmpty());
    QCOMPARE(action.status(), KAuth::Action::InvalidStatus);
}
#endif

void SetupActionTest::testBasicActionProperties()
{
    Q_EMIT changeCapabilities(KAuth::AuthBackend::AuthorizeFromHelperCapability | KAuth::AuthBackend::CheckActionExistenceCapability);
    KAuth::Action::DetailsMap detailsMap{{KAuth::Action::AuthDetail::DetailOther, QLatin1String("details")}};
    KAuth::Action action(QLatin1String("always.authorized"), detailsMap);
    QVERIFY(action.isValid());

    QCOMPARE(action.name(), QLatin1String("always.authorized"));
#if KAUTHCORE_BUILD_DEPRECATED_SINCE(5, 71)
    QCOMPARE(action.details(), QLatin1String("details"));
#endif
    QCOMPARE(action.detailsV2(), detailsMap);
    QVERIFY(!action.hasHelper());
    QVERIFY(action.helperId().isEmpty());
    QCOMPARE(action.status(), KAuth::Action::AuthorizedStatus);

    QVERIFY(action.arguments().isEmpty());
    QVariantMap args;
    args.insert(QLatin1String("akey"), QVariant::fromValue(42));
    action.setArguments(args);
    QCOMPARE(action.arguments(), args);

    action.setName(QLatin1String("i.do.not.exist"));
    QVERIFY(!action.isValid());

    Q_EMIT changeCapabilities(KAuth::AuthBackend::NoCapability);

    action = KAuth::Action(QLatin1String("i.do.not.exist"), detailsMap);

    QVERIFY(action.isValid());
    QCOMPARE(action.name(), QLatin1String("i.do.not.exist"));
#if KAUTHCORE_BUILD_DEPRECATED_SINCE(5, 71)
    QCOMPARE(action.details(), QLatin1String("details"));
#endif
    QCOMPARE(action.detailsV2(), detailsMap);
    QVERIFY(!action.hasHelper());
    QVERIFY(action.helperId().isEmpty());
    QCOMPARE(action.status(), KAuth::Action::InvalidStatus);
}

void SetupActionTest::testUserAuthorization()
{
    Q_EMIT changeCapabilities(KAuth::AuthBackend::CheckActionExistenceCapability);

    KAuth::Action::DetailsMap detailsMap{{KAuth::Action::AuthDetail::DetailOther, QLatin1String("details")}};
    KAuth::Action action(QLatin1String("requires.auth"), detailsMap);
    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::AuthRequiredStatus);
    KAuth::ExecuteJob *job = action.execute();

    QVERIFY(!job->exec());

    QCOMPARE(job->error(), (int)KAuth::ActionReply::BackendError);

    Q_EMIT changeCapabilities(KAuth::AuthBackend::CheckActionExistenceCapability | KAuth::AuthBackend::AuthorizeFromClientCapability);

    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::AuthRequiredStatus);
    job = action.execute();

    QVERIFY(job->exec());

    QVERIFY(!job->error());
    QVERIFY(job->data().isEmpty());
}

void SetupActionTest::testAuthorizationFail()
{
    Q_EMIT changeCapabilities(KAuth::AuthBackend::CheckActionExistenceCapability | KAuth::AuthBackend::AuthorizeFromClientCapability);

    KAuth::Action::DetailsMap detailsMap{{KAuth::Action::AuthDetail::DetailOther, QLatin1String("details")}};
    KAuth::Action action(QLatin1String("doomed.to.fail"), detailsMap);
    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::DeniedStatus);
    KAuth::ExecuteJob *job = action.execute();

    QVERIFY(!job->exec());

    QCOMPARE(job->error(), (int)KAuth::ActionReply::AuthorizationDeniedError);
    QVERIFY(job->data().isEmpty());
}

QTEST_MAIN(SetupActionTest)
#include "SetupActionTest.moc"
