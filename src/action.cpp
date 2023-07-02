/*
    SPDX-FileCopyrightText: 2009-2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "action.h"

#include <QRegularExpression>
#include <QtGlobal>

class QWidget;

#include "executejob.h"

#include "BackendsManager.h"

namespace KAuth
{
class ActionData : public QSharedData
{
public:
    ActionData()
        : parent(nullptr)
        , timeout(-1)
    {
    }
    ActionData(const ActionData &other)
        : QSharedData(other)
        , name(other.name)
        , helperId(other.helperId)
        , details(other.details)
        , args(other.args)
        , parent(other.parent)
        , timeout(other.timeout)
    {
    }
    ~ActionData()
    {
    }

    QString name;
    QString helperId;
    Action::DetailsMap details;
    QVariantMap args;
    QWidget *parent = nullptr;
    int timeout;
};

// Constructors
Action::Action()
    : d(new ActionData())
{
}

Action::Action(const Action &action)
    : d(action.d)
{
}

Action::Action(const QString &name)
    : d(new ActionData())
{
    setName(name);
    BackendsManager::authBackend()->setupAction(d->name);
}

#if KAUTHCORE_BUILD_DEPRECATED_SINCE(5, 71)
Action::Action(const QString &name, const QString &details)
    : Action(name, DetailsMap{{AuthDetail::DetailOther, details}})
{
}
#endif

Action::Action(const QString &name, const DetailsMap &details)
    : d(new ActionData())
{
    setName(name);
    setDetailsV2(details);
    BackendsManager::authBackend()->setupAction(d->name);
}

Action::~Action()
{
}

// Operators
Action &Action::operator=(const Action &action)
{
    if (this == &action) {
        // Protect against self-assignment
        return *this;
    }

    d = action.d;
    return *this;
}

bool Action::operator==(const Action &action) const
{
    return d->name == action.d->name;
}

bool Action::operator!=(const Action &action) const
{
    return d->name != action.d->name;
}

// Accessors
QString Action::name() const
{
    return d->name;
}

void Action::setName(const QString &name)
{
    d->name = name;
}

// Accessors
int Action::timeout() const
{
    return d->timeout;
}

void Action::setTimeout(int timeout)
{
    d->timeout = timeout;
}

#if KAUTHCORE_BUILD_DEPRECATED_SINCE(5, 71)
QString Action::details() const
{
    return d->details.value(AuthDetail::DetailOther).toString();
}
#endif

#if KAUTHCORE_BUILD_DEPRECATED_SINCE(5, 71)
void Action::setDetails(const QString &details)
{
    d->details.clear();
    d->details.insert(AuthDetail::DetailOther, details);
}
#endif

Action::DetailsMap Action::detailsV2() const
{
    return d->details;
}

void Action::setDetailsV2(const DetailsMap &details)
{
    d->details = details;
}

bool Action::isValid() const
{
    if (d->name.isEmpty()) {
        return false;
    }

    // Does the backend support checking for known actions?
    if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::CheckActionExistenceCapability) {
        // In this case, just ask the backend
        return BackendsManager::authBackend()->actionExists(name());
    } else {
        // Otherwise, check through a regexp
        const QRegularExpression re(QRegularExpression::anchoredPattern(QStringLiteral("[0-z]+(\\.[0-z]+)*")));
        return re.match(name()).hasMatch();
    }
}

void Action::setArguments(const QVariantMap &arguments)
{
    d->args = arguments;
}

void Action::addArgument(const QString &key, const QVariant &value)
{
    d->args.insert(key, value);
}

QVariantMap Action::arguments() const
{
    return d->args;
}

QString Action::helperId() const
{
    return d->helperId;
}

// TODO: Check for helper id's syntax
void Action::setHelperId(const QString &id)
{
    d->helperId = id;
}

void Action::setParentWidget(QWidget *parent)
{
    d->parent = parent;
}

QWidget *Action::parentWidget() const
{
    return d->parent;
}

Action::AuthStatus Action::status() const
{
    if (!isValid()) {
        return Action::InvalidStatus;
    }

    return BackendsManager::authBackend()->actionStatus(d->name);
}

ExecuteJob *Action::execute(ExecutionMode mode)
{
    return new ExecuteJob(*this, mode, nullptr);
}

bool Action::hasHelper() const
{
    return !d->helperId.isEmpty();
}

} // namespace Auth

#include "moc_action.cpp"
