/* Copyright (C) 2007 - 2012 Jan Kundrát <jkt@flaska.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or version 3 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef IMAP_TASK_ENABLETASK_H
#define IMAP_TASK_ENABLETASK_H

#include "ImapTask.h"

namespace Imap
{
namespace Mailbox
{

/** @short Perform the ENABLE command from RFC 5161 */
class EnableTask : public ImapTask
{
    Q_OBJECT
public:
    EnableTask(Model *model, ImapTask *parentTask, const QList<QByteArray> &extensions);
    virtual void perform();

    virtual bool handleStateHelper(const Imap::Responses::State *const resp);
    virtual bool handleEnabled(const Responses::Enabled *const resp);
    virtual QVariant taskData(const int role) const;
    virtual bool needsMailbox() const {return false;}
private:
    CommandHandle tag;
    QList<QByteArray> extensions;
};

}
}

#endif // IMAP_TASK_ENABLETASK_H