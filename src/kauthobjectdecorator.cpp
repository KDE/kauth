/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009-2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kauthobjectdecorator.h"

#include "kauthaction.h"
#include "kauthexecutejob.h"
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
        : q(parent),
          decoratedObject(parent->parent())
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
    if (qobject_cast<QAbstractButton *>(decoratedObject)) {
        q->connect(decoratedObject, SIGNAL(clicked()),
                   q, SLOT(slotActivated()));
        return;
    }

    if (qobject_cast<QAction *>(decoratedObject)) {
        q->connect(decoratedObject, SIGNAL(triggered(bool)),
                   q, SLOT(slotActivated()));
        return;
    }

    qCWarning(KAUTH) << Q_FUNC_INFO << "We're not decorating an action or a button";
}

void ObjectDecoratorPrivate::linkActionToWidget()
{
    QWidget *widget = qobject_cast<QWidget *>(decoratedObject);
    if (widget) {
        // Set the WA_NativeWindow attribute to force the creation of the QWindow.
        // Without this QWidget::windowHandle() returns nullptr.
        widget->setAttribute(Qt::WA_NativeWindow, true);
        authAction.setParentWindow(widget->windowHandle());
        return;
    }

    QAction *action = qobject_cast<QAction *>(decoratedObject);
    if (action) {
        auto *w = action->parentWidget();
        if (w) {
            // Set the WA_NativeWindow attribute to force the creation of the QWindow.
            // Without this QWidget::windowHandle() returns nullptr.
            w->setAttribute(Qt::WA_NativeWindow, true);
            authAction.setParentWindow(w->windowHandle());
        }
        return;
    }

    qCWarning(KAUTH) << Q_FUNC_INFO << "We're not decorating an action or a widget";
}

void ObjectDecoratorPrivate::slotActivated()
{
    if (authAction.isValid()) {
        KAuth::ExecuteJob *job = authAction.execute(KAuth::Action::AuthorizeOnlyMode);
        q->connect(job, SIGNAL(statusChanged(KAuth::Action::AuthStatus)),
                   q, SLOT(authStatusChanged(KAuth::Action::AuthStatus)));
        if (job->exec()) {
            emit q->authorized(authAction);
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
    : QObject(parent), d(new ObjectDecoratorPrivate(this))
{
    d->connectDecorated();
}

ObjectDecorator::~ObjectDecorator()
{
    delete d;
}

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

#include "moc_kauthobjectdecorator.cpp"
