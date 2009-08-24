//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Eckhart Wörner <ewoerner@kde.org>
//

#include "GpsdConnection.h"

#include <QtCore/QDebug>



using namespace Marble;

GpsdConnection::GpsdConnection( QObject* parent )
    : QObject( parent ),
      m_timer( 0 )
{
    gps_data_t* data = m_gpsd.open();
    if ( data ) {
        connect( &m_timer, SIGNAL( timeout() ), this, SLOT( update() ) );
        m_timer.start( 1000 );
    } else
        qDebug() << "Connection to gpsd failed, no position info available.";
}

void GpsdConnection::update()
{
    gps_data_t* data = m_gpsd.query( "o" );
    if ( data )
        emit gpsdInfo( *data );
}



#include "GpsdConnection.moc"
