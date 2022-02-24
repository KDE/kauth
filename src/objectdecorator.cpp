/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009-2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "objectdecorator.h"

#include "action.h"
#include "executejob.h"
#include "kauthdebug.h"

#include <QAbstractButton>
#include <QAction>
#include <QIcon>

namespace KAuth
{
class ObjectDecoratorPrivate
{
public:
    ObjectDecoratorPrivate(ObjectDecorator *parent)
        : q(parent)
        , decoratedObject(parent->parent())
    {
    }

    ObjectDecorator *const q;

    QObject *const decoratedObject;
    KAuth::Action authAction;
    // TODO: Remove whenever QIcon overlays will get fixed
    QIcon oldIcon;

    void connectDecorated();
    void linkActionToWidget();
    void slotActivated();
    void authStatusChanged(KAuth::Action::AuthStatus status);
};

void ObjectDecoratorPrivate::connectDecorated()
{
    if (auto *button = qobject_cast<QAbstractButton *>(decoratedObject)) {
        q->connect(button, &QAbstractButton::clicked, q, [this] {
            slotActivated();
        });
        return;
    }

    if (auto *action = qobject_cast<QAction *>(decoratedObject)) {
        q->connect(action, &QAction::triggered, q, [this] {
            slotActivated();
        });
        return;
    }

    qCWarning(KAUTH) << Q_FUNC_INFO << "We're not decorating an action or a button";
}

void ObjectDecoratorPrivate::linkActionToWidget()
{
    QWidget *widget = qobject_cast<QWidget *>(decoratedObject);
    if (widget) {
        authAction.setParentWidget(widget);
        return;
    }

    QAction *action = qobject_cast<QAction *>(decoratedObject);
    if (action) {
        authAction.setParentWidget(action->parentWidget());
        return;
    }

    qCWarning(KAUTH) << Q_FUNC_INFO << "We're not decorating an action or a widget";
}

void ObjectDecoratorPrivate::slotActivated()
{
    if (authAction.isValid()) {
        KAuth::ExecuteJob *job = authAction.execute(KAuth::Action::AuthorizeOnlyMode);
        q->connect(job, &KAuth::ExecuteJob::statusChanged, q, [this](KAuth::Action::AuthStatus status) {
            authStatusChanged(status);
        });

        if (job->exec()) {
            Q_EMIT q->authorized(authAction);
        } else {
            decoratedObject->setProperty("enabled", false);
        }
    }
}

void ObjectDecoratorPrivate::authStatusChanged(KAuth::Action::AuthStatus status)
{
    switch (status) {
    case KAuth::Action::AuthorizedStatus:
        decoratedObject->setProperty("enabled", true);
        if (!oldIcon.isNull()) {
            decoratedObject->setProperty("icon", QVariant::fromValue(oldIcon));
            oldIcon = QIcon();
        }
        break;
    case KAuth::Action::AuthRequiredStatus:
        decoratedObject->setProperty("enabled", true);
        oldIcon = decoratedObject->property("icon").value<QIcon>();
        decoratedObject->setProperty("icon", QIcon::fromTheme(QLatin1String("dialog-password")));
        break;
    default:
        decoratedObject->setProperty("enabled", false);
        if (!oldIcon.isNull()) {
            decoratedObject->setProperty("icon", QVariant::fromValue(oldIcon));
            oldIcon = QIcon();
        }
    }
}

ObjectDecorator::ObjectDecorator(QObject *parent)
    : QObject(parent)
    , d(new ObjectDecoratorPrivate(this))
{
    d->connectDecorated();
}

ObjectDecorator::~ObjectDecorator() = default;

KAuth::Action ObjectDecorator::authAction() const
{
    return d->authAction;
}

void ObjectDecorator::setAuthAction(const QString &actionName)
{
    if (actionName.isEmpty()) {
        setAuthAction(KAuth::Action());
    } else {
        setAuthAction(KAuth::Action(actionName));
    }
}

void ObjectDecorator::setAuthAction(const KAuth::Action &action)
{
    if (d->authAction == action) {
        return;
    }

    if (d->authAction.isValid()) {
        if (!d->oldIcon.isNull()) {
            d->decoratedObject->setProperty("icon", QVariant::fromValue(d->oldIcon));
            d->oldIcon = QIcon();
        }
    }

    if (action.isValid()) {
        d->authAction = action;

        // Set the parent widget
        d->linkActionToWidget();

        d->authStatusChanged(d->authAction.status());
    }
}

} // namespace KAuth

#include "moc_objectdecorator.cpp"
