/* Copyright (C) 2007 - 2008 Jan Kundrát <jkt@gentoo.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef IMAP_MAILBOXTREE_H
#define IMAP_MAILBOXTREE_H

#include <QList>
#include <QString>
#include "../Parser/Response.h"
#include "../Parser/Message.h"
#include "MailboxMetadata.h"

namespace Imap {

namespace Mailbox {

class Model;

class TreeItem {
    friend class Model; // for _loading and _fetched
    void operator=( const TreeItem& ); // don't implement
protected:
    TreeItem* _parent;
    QList<TreeItem*> _children;
    bool _fetched, _loading;
public:
    TreeItem( TreeItem* parent );
    TreeItem* parent() const { return _parent; };
    int row() const;

    virtual ~TreeItem();
    virtual unsigned int childrenCount( const Model* const model );
    virtual TreeItem* child( const int offset, const Model* const model );
    virtual QList<TreeItem*> setChildren( const QList<TreeItem*> items );
    virtual void fetch( const Model* const model ) = 0;
    virtual unsigned int columnCount( const Model* const model ) { return 1; };
    virtual unsigned int rowCount( const Model* const model ) = 0;
    virtual QVariant data( const Model* const model, int role ) = 0;
    virtual bool hasChildren( const Model* const model ) = 0;
    virtual bool fetched() const { return _fetched; };
};

class TreeItemPart;

class TreeItemMailbox: public TreeItem {
    void operator=( const TreeItem& ); // don't implement
    MailboxMetadata _metadata;
public:
    TreeItemMailbox( TreeItem* parent );
    TreeItemMailbox( TreeItem* parent, Responses::List );

    virtual QList<TreeItem*> setChildren( const QList<TreeItem*> items );
    virtual void fetch( const Model* const model );
    virtual unsigned int rowCount( const Model* const model );
    virtual QVariant data( const Model* const model, int role );
    virtual bool hasChildren( const Model* const model );
    virtual TreeItem* child( const int offset, const Model* const model );

    bool hasChildMailboxes( const Model* const model );

    QString mailbox() const { return _metadata.mailbox; };
    QString separator() const { return _metadata.separator; };
    const MailboxMetadata& mailboxMetadata() const { return _metadata; };
    /** @short Update internal tree with the results of a FETCH response

      If \a changedPart is not null, it will be updated to point to the message
      part whose content got fetched.
    */
    void handleFetchResponse( const Model* const model,
                              const Responses::Fetch& response,
                              TreeItemPart** changedPart=0 );
    void finalizeFetch( const Model* const model, const Responses::Status& response );
    void rescanForChildMailboxes( const Model* const model );
private:
    TreeItemPart* partIdToPtr( const Model* model, const int msgNumber, const QString& msgId );
};

class TreeItemMsgList: public TreeItem {
    void operator=( const TreeItem& ); // don't implement
    friend class TreeItemMailbox;
public:
    TreeItemMsgList( TreeItem* parent );

    virtual void fetch( const Model* const model );
    virtual unsigned int rowCount( const Model* const model );
    virtual QVariant data( const Model* const model, int role );
    virtual bool hasChildren( const Model* const model );
};

class TreeItemMessage: public TreeItem {
    void operator=( const TreeItem& ); // don't implement
    friend class TreeItemMailbox;
    Message::Envelope _envelope;
public:
    TreeItemMessage( TreeItem* parent );

    virtual void fetch( const Model* const model );
    virtual unsigned int rowCount( const Model* const model );
    virtual QVariant data( const Model* const model, int role );
    virtual bool hasChildren( const Model* const model ) { return true; };
};

class TreeItemPart: public TreeItem {
    void operator=( const TreeItem& ); // don't implement
    friend class TreeItemMailbox; // needs access to _data
    QString _mimeType;
    QString _charset;
    QByteArray _data;
public:
    TreeItemPart( TreeItem* parent, const QString& mimeType );

    virtual unsigned int childrenCount( const Model* const model );
    virtual TreeItem* child( const int offset, const Model* const model );
    virtual QList<TreeItem*> setChildren( const QList<TreeItem*> items );

    virtual void fetch( const Model* const model );
    virtual unsigned int rowCount( const Model* const model );
    virtual QVariant data( const Model* const model, int role );
    virtual bool hasChildren( const Model* const model );

    QString partId() const;
    TreeItemMessage* message() const;

    /** @short Provide access to the internal buffer holding data

        It is safe to access the obtained pointer as long as this object is not
        deleted. This function violates the classic concept of object
        encapsulation, but is really useful for the implementation of
        Imap::Network::MsgPartNetworkReply.
     */
    QByteArray* dataPtr();
    QString mimeType() const { return _mimeType; };
    QString charset() const { return _charset; };
    void setCharset( const QString& ch ) { _charset = ch; };
private:
    bool isTopLevelMultiPart() const;
};

}

}

#endif // IMAP_MAILBOXTREE_H
