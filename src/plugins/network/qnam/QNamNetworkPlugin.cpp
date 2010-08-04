//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009 Jens-Michael Hoffmann <jensmh@gmx.de>
//
#include "QNamNetworkPlugin.h"

#include "MarbleDebug.h"
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>

#include "QNamDownloadRequest.h"

namespace Marble
{

QNamNetworkPlugin::QNamNetworkPlugin()
    : m_networkAccessManager( new QNetworkAccessManager )
{
}

QNamNetworkPlugin::~QNamNetworkPlugin()
{
    delete m_networkAccessManager;
}

QString QNamNetworkPlugin::name() const
{
    return tr( "QNam Network Plugin" );
}

QString QNamNetworkPlugin::nameId() const
{
    return QString::fromLatin1( "QNamNetworkPlugin" );
}

QString QNamNetworkPlugin::guiString() const
{
    return tr( "QNetworkAccessManager based Network Plugin" );
}

QString QNamNetworkPlugin::description() const
{
    return tr( "A network plugin which keeps connected to the hosts used "
               "to deliver a better user experience." );
}

QIcon QNamNetworkPlugin::icon() const
{
    return QIcon();
}

void QNamNetworkPlugin::initialize()
{
}

bool QNamNetworkPlugin::isInitialized() const
{
    return true;
}

HttpRequest * QNamNetworkPlugin::createRequest( const QUrl & sourceUrl,
                                        const QString & destination,
                                        const QString & id )
{
    HttpRequest * const request = new QNamDownloadRequest( sourceUrl, destination, id, m_networkAccessManager );
    request->setUserAgentPluginId( "QNamNetworkPlugin" );
    return request;
}

NetworkPlugin * QNamNetworkPlugin::newInstance() const
{
    return new QNamNetworkPlugin;
}

} // namespace Marble

#include "QNamNetworkPlugin.moc"

Q_EXPORT_PLUGIN2( QNamNetworkPlugin, Marble::QNamNetworkPlugin )
