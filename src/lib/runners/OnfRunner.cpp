//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Henry de Valence <hdevalence@gmail.com>

#include "OnfRunner.h"

#include "MarbleAbstractRunner.h"
#include "GeoOnfParser.h"
#include "GeoDataDocument.h"
#include "GeoDataPlacemark.h"
#include "MarbleDebug.h"

#include <QtCore/QString>
#include <QtCore/QBuffer>
#include <QtCore/QVector>
#include <QtCore/QUrl>

#include <QtNetwork/QHttp>

namespace Marble
{

/*
TODO: grade results individually instead of giving them all a score of 100
*/

OnfRunner::OnfRunner( QObject *parent ) : MarbleAbstractRunner( parent )
{
    m_http = new QHttp("gazetteer.openstreetmap.org");
    m_buffer = 0;
    
    connect( m_http, SIGNAL( requestFinished( int, bool ) ),
             this, SLOT( slotRequestFinished( int, bool ) ) );
}

OnfRunner::~OnfRunner()
{
    delete m_http;
    delete m_buffer;
}

GeoDataFeature::GeoDataVisualCategory OnfRunner::category() const
{
    return GeoDataFeature::OsmSite;
}

void OnfRunner::fail()
{
    //The manager needs to know when parsing starts and stops
    //in order to have a balanced count of active runners. So
    //we emit runnerFinished() to balance the previous failed runnerStarted()
    QVector<GeoDataPlacemark*> empty;
    emit runnerFinished( this, empty );
    return;
}

void OnfRunner::run()
{
    if( m_input.isEmpty() ) {
        fail();
    }
    //no point to keep downloading if we're doing a new one
    /* //causes crash
    if( m_http->currentId() != 0 ) {
        m_http->abort();
        //aborted parse request fails; this one keeps going.
        fail();
    }
    */
    //make a new buffer
    delete m_buffer;
    m_buffer = new QBuffer;
    QString base = "/namefinder/search.xml?find=%1";
    QString input = QUrl::toPercentEncoding(m_input);
    QString request = base.arg(input);
    mDebug() << "ONF search: GET " << request;
    m_http->get( request, m_buffer );
}

void OnfRunner::slotRequestFinished( int id, bool error )
{
    if( error ) {
        mDebug() << "ONF request" << id << "failed:" << m_http->error()
                 << m_http->errorString();
        fail();
        return;
    }
    
    mDebug() << "ONF search result buffer size:" << m_buffer->size();
    //mDebug() << m_buffer->data();
    
    QByteArray array = m_buffer->data();
    QBuffer data( &array );
    data.open( QIODevice::ReadOnly );
    
    GeoOnfParser parser;
    if( !parser.read( &data ) ) {
        qWarning() << "Could not parse ONF buffer!!";
        fail();
        return;
    }
    
    GeoDataDocument *results = static_cast<GeoDataDocument*>( parser.releaseDocument() );
    Q_ASSERT( results );
    QVector<GeoDataPlacemark*> placemarks = results->placemarkList();

    // feed a new vector, because docPlacemarks and its placemarks will get
    // deleted along with results
    QVector<GeoDataPlacemark*> returnPlacemarks;

    QVector<GeoDataPlacemark*>::iterator it = placemarks.begin();
    QVector<GeoDataPlacemark*>::iterator end = placemarks.end();
    for(; it != end; ++it ) {
        (*it)->setVisualCategory( category() );
        returnPlacemarks.append( new GeoDataPlacemark( **it ) );
    }
    emit runnerFinished( this, returnPlacemarks );
    delete results;
    return;
}

} // namespace Marble

#include "OnfRunner.moc"
