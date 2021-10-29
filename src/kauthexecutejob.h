/*
    SPDX-FileCopyrightText: 2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef EXECUTE_JOB_H
#define EXECUTE_JOB_H

#include <kjob.h>

#include <kauthcore_export.h>

#include "kauthaction.h"
#include "kauthactionreply.h"

namespace KAuth
{
/**
 * @class ExecuteJob kauthexecutejob.h <KAuthExecuteJob>
 *
 * @brief Job for executing an Action
 *
 * To run the action synchonously use KJob::exec() and check the return code for
 * success.
 *
 * For longer tasks connect KJob::result(KJob*) and any other signals such as
 * percent(KJob*, unsigned long) and newData(const QVariantMap &) then run start().
 *
 * To check for authentiation success or problems connect to
 * statusChanged(KAuth::Action::AuthStatus status) signal.
 *
 * Use data() to get the return result of the action.
 *
 * @since 5.0
 */
class KAUTHCORE_EXPORT ExecuteJob : public KJob
{
    Q_OBJECT

    ExecuteJob(const KAuth::Action &action, KAuth::Action::ExecutionMode mode, QObject *parent);

    friend class Action;

    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void doExecuteAction())
    Q_PRIVATE_SLOT(d, void doAuthorizeAction())
    Q_PRIVATE_SLOT(d, void actionPerformedSlot(const QString &action, const KAuth::ActionReply &reply))
    Q_PRIVATE_SLOT(d, void progressStepSlot(const QString &action, int i))
    Q_PRIVATE_SLOT(d, void statusChangedSlot(const QString &action, KAuth::Action::AuthStatus status))

public:
    /// Virtual destructor
    ~ExecuteJob() override;

    /**
     * Starts the job asynchronously.
     * @see KJob::result
     * @see newData
     * @see statusChanged
     */
    void start() override;

    /**
     * @returns the action associated with this job
     */
    Action action() const;

    /**
     * Use this to get the data set in the action by
     * HelperSupport::progressStep(QVariant) or returned at the end of the
     * action.
     *
     * This function is particularly useful once the job has completed. During
     * execution, simply read the data in the newData signal.
     *
     * @see ExecuteJob::newData
     * @returns the data set by the helper
     */
    QVariantMap data() const;

public Q_SLOTS:
    /**
     * Attempts to halt the execution of the action associated with this job.
     * You should listen to the finished and result signals to work out whether
     * halting was successful (as long running operations can also take time
     * to shut down cleanly).
     * @see HelperSupport::isStopped()
     * @see KJob::result
     * @see KJob::finished
     * @return Always returns true
     */
    bool kill(KillVerbosity verbosity = Quietly);

Q_SIGNALS:
    /**
     * @brief Signal emitted by the helper to notify the action's progress
     *
     * This signal is emitted every time the helper's code calls the
     * HelperSupport::progressStep(QVariantMap) method. This is useful to let the
     * helper notify the execution status of a long action, also providing
     * some data, for example if you want to achieve some sort of progressive loading.
     * The meaning of the data passed here is totally application-dependent.
     * If you only need to pass some percentage, you can use the other signal that
     * pass an int.
     *
     * @param data The progress data from the helper
     */
    void newData(const QVariantMap &data);

    /**
     * @brief Signal emitted when the authentication status changes
     * @param status the new authentication status
     */
    void statusChanged(KAuth::Action::AuthStatus status);

private:
    Q_DISABLE_COPY(ExecuteJob)
};

} // namespace Auth

#endif
