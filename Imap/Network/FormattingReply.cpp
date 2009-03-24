#include <QDebug>
#include <QStringList>
#include <QTimer>

#include "FormattingReply.h"
#include "MsgPartNetAccessManager.h"
#include "MsgPartNetworkReply.h"
#include "Imap/Model/MailboxTree.h"

namespace Imap {

namespace Network {

FormattingReply::FormattingReply( QObject* parent, const Imap::Mailbox::Model* _model,
        Imap::Mailbox::TreeItemMessage* _msg, Imap::Mailbox::TreeItemPart* _part ):
    QNetworkReply( parent ), model(_model), msg(_msg), part(_part), pendingCount(0)
{
    setOpenMode( QIODevice::ReadOnly | QIODevice::Unbuffered );
    requestAnotherPart( part );
}

void FormattingReply::abort()
{
    close();
}

void FormattingReply::close()
{
    buffer.close();
}

qint64 FormattingReply::bytesAvailable() const
{
    return buffer.bytesAvailable() + QNetworkReply::bytesAvailable();
}

qint64 FormattingReply::readData( char* data, qint64 maxSize )
{
    qint64 res = buffer.read( data, maxSize );
    if ( res <= 0 )
        emit finished();
    else
        emit readyRead();
    return res;
}

void FormattingReply::requestAnotherPart( Imap::Mailbox::TreeItemPart* anotherPart )
{
    MsgPartNetworkReply* reply = new MsgPartNetworkReply( this, model, msg, anotherPart );
    replies.append( reply );
    pendingBitmap.append( true );
    ++pendingCount;
    if ( anotherPart->fetched() ) {
        // there will be no finished() signal
        reqAlreadyFetched << reply;
        QTimer::singleShot( 0, this, SLOT( handleAlreadyFinished() ) );
    } else {
        connect( reply, SIGNAL( finished() ), this, SLOT( anotherReplyFinished() ) );
    }
}

void FormattingReply::handleAlreadyFinished()
{
    for( QList<MsgPartNetworkReply*>::const_iterator it = reqAlreadyFetched.begin();
         it != reqAlreadyFetched.end(); ++it ) {
        anotherReplyFinished( *it );
    }
}

void FormattingReply::anotherReplyFinished()
{
    MsgPartNetworkReply* reply = qobject_cast<MsgPartNetworkReply*>( sender() );
    Q_ASSERT( reply );
    anotherReplyFinished( reply );
}

void FormattingReply::anotherReplyFinished( MsgPartNetworkReply* anotherReply )
{
    int offset = replies.indexOf( anotherReply );
    Q_ASSERT( offset != -1 );
    if ( pendingBitmap[ offset ] ) {
        pendingBitmap[ offset ] = false;
        --pendingCount;
    }
    disconnect( anotherReply, SIGNAL( finished() ), this, SLOT( anotherReplyFinished() ) );
    if ( offset == 0 )
        mainReplyFinished();
    else if ( pendingCount == 0 )
        everythingFinished();
}

void FormattingReply::mainReplyFinished()
{
    QString mimeType = part->mimeType();
    if ( mimeType.startsWith( QLatin1String( "text/" ) ) ) {
        if ( part->charset().isEmpty() )
            setHeader( QNetworkRequest::ContentTypeHeader, mimeType );
        else
            setHeader( QNetworkRequest::ContentTypeHeader,
                       QString("%1; charset=%2").arg( mimeType ).arg( part->charset() ) );
        buffer.setData( replies[0]->readAll() );
        emit everythingFinished();
    } else if ( mimeType.startsWith( QLatin1String( "multipart/signed" ) ) ) {
        // FIXME
    } else {
        setHeader( QNetworkRequest::ContentTypeHeader, mimeType );
        buffer.setData( replies[0]->readAll() );
        emit everythingFinished();
    }
}

void FormattingReply::everythingFinished()
{
    Q_ASSERT( ! buffer.isOpen() );
    buffer.open( QIODevice::ReadOnly );
    emit readyRead();
}

}
}
#include "FormattingReply.moc"
