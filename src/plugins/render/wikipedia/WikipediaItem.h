//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

#ifndef WIKIPEDIAITEM_H
#define WIKIPEDIAITEM_H

#include "AbstractDataPluginItem.h"

#include <QtCore/QHash>
#include <QtCore/QUrl>
#include <QtGui/QPixmap>
#include <QtGui/QIcon>

class QAction;

namespace Marble
{

class TinyWebBrowser;
 
class WikipediaItem : public AbstractDataPluginItem
{
    Q_OBJECT
    
 public:
    WikipediaItem( QObject *parent );
    
    ~WikipediaItem();
    
    QString name() const;

    void setName( const QString& name );
    
    QString itemType() const;
     
    bool initialized();
    
    void addDownloadedFile( const QString& url, const QString& type );
    
    void paint( GeoPainter *painter, ViewportParams *viewport,
                const QString& renderPos, GeoSceneLayer * layer = 0 );
                 
    bool operator<( const AbstractDataPluginItem *other ) const;
    
    qreal longitude();
    
    void setLongitude( qreal longitude );
    
    qreal latitude();
    
    void setLatitude( qreal latitude );
    
    QUrl url();
    
    void setUrl( const QUrl& url );
    
    QUrl thumbnailImageUrl();
    
    void setThumbnailImageUrl( const QUrl& thumbnailImageUrl );

    QString summary();

    void setSummary( const QString& summary );
    
    QAction *action();
    
    void setIcon( const QIcon& icon );

    void setSettings( QHash<QString, QVariant> settings );
    
 public Q_SLOTS:
    void openBrowser();
    
 private:
    void updateSize();
    void updateToolTip();
    bool showThumbnail();

    QUrl m_url;
    QUrl m_thumbnailImageUrl;
    QString m_summary;
    TinyWebBrowser *m_browser;
    QAction *m_action;

    QPixmap m_thumbnail;
    QIcon m_wikiIcon;
    QHash<QString, QVariant> m_settings;
};
    
}

#endif // WIKIPEDIAITEM_H
