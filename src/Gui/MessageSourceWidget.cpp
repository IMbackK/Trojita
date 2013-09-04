/* Copyright (C) 2013  Ahmed Ibrahim Khalil <ahmedibrahimkhali@gmail.com>

   This file is part of the Trojita Qt IMAP e-mail client,
   http://trojita.flaska.net/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "MessageSourceWidget.h"
#include <QModelIndex>
#include "Imap/Model/FullMessageCombiner.h"

namespace Gui
{

MessageSourceWidget::MessageSourceWidget(QWidget *parent, const QModelIndex &messageIndex):
    QWebView(parent)
{
    Q_ASSERT(messageIndex.isValid());
    page()->setNetworkAccessManager(0);
    m_combiner = new Imap::Mailbox::FullMessageCombiner(messageIndex, this);
    connect(m_combiner, SIGNAL(completed()), this, SLOT(slotCompleted()));
    connect(m_combiner, SIGNAL(failed(QString)), this, SLOT(slotError(QString)));
    m_combiner->load();
}

void MessageSourceWidget::slotCompleted()
{
    setContent(m_combiner->data(), QLatin1String("text/plain"));
}

void MessageSourceWidget::slotError(const QString &message)
{
    setContent(message.toUtf8(), QLatin1String("text/plain; charset=utf-8"));
}

}
