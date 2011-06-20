//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Eckhart Wörner <ewoerner@kde.org>
//

#include "GpsdConnection.h"

#include "MarbleDebug.h"

#include <errno.h>
#include <clocale>

using namespace Marble;
/* TRANSLATOR Marble::GpsdConnection */

GpsdConnection::GpsdConnection( QObject* parent )
    : QObject( parent ),
#if defined( GPSD_API_MAJOR_VERSION ) && ( GPSD_API_MAJOR_VERSION >= 5 )
      m_gpsd("localhost", DEFAULT_GPSD_PORT),
#endif
      m_timer( 0 )
{
    m_oldLocale = setlocale( LC_NUMERIC, NULL );
    setlocale( LC_NUMERIC, "C" );
    connect( &m_timer, SIGNAL( timeout() ), this, SLOT( update() ) );
}

GpsdConnection::~GpsdConnection()
{
    setlocale( LC_NUMERIC, m_oldLocale );
}

void GpsdConnection::initialize()
{
    m_timer.stop();
#if defined( GPSD_API_MAJOR_VERSION ) && ( GPSD_API_MAJOR_VERSION >= 5 ) && defined( WATCH_ENABLE )
    gps_data_t* data = m_gpsd.read();
#else
    gps_data_t* data = m_gpsd.open();
#endif
    if ( data ) {
        m_status = PositionProviderStatusAcquiring;
        emit statusChanged( m_status );

#if defined( GPSD_API_MAJOR_VERSION ) && ( GPSD_API_MAJOR_VERSION >= 3 ) && defined( WATCH_ENABLE )
        m_gpsd.stream( WATCH_ENABLE );
#endif
        m_timer.start( 1000 );
    }
    else {
        // There is also gps_errstr() for libgps version >= 2.90,
        // but it doesn't return a sensible error description
        switch ( errno ) {
            case NL_NOSERVICE:
                m_error = tr("Internal gpsd error (cannot get service entry)");
                break;
            case NL_NOHOST:
                m_error = tr("Internal gpsd error (cannot get host entry)");
                break;
            case NL_NOPROTO:
                m_error = tr("Internal gpsd error (cannot get protocol entry)");
                break;
            case NL_NOSOCK:
                m_error = tr("Internal gpsd error (unable to create socket)");
                break;
            case NL_NOSOCKOPT:
                m_error = tr("Internal gpsd error (unable to set socket option)");
                break;
            case NL_NOCONNECT:
                m_error = tr("No GPS device found by gpsd.");
                break;
            default:
                m_error = tr("Unknown error when opening gpsd connection");
                break;
        }

        m_status = PositionProviderStatusError;
        emit statusChanged( m_status );

        mDebug() << "Connection to gpsd failed, no position info available: " << m_error;
    }
}

void GpsdConnection::update()
{
#if defined( GPSD_API_MAJOR_VERSION ) && ( GPSD_API_MAJOR_VERSION >= 3 ) && defined( PACKET_SET )
#if defined( GPSD_API_MAJOR_VERSION ) && ( GPSD_API_MAJOR_VERSION >= 5 )
    if ( m_gpsd.waiting(0) ) {
#else
    if ( m_gpsd.waiting() ) {
#endif
#if defined( GPSD_API_MAJOR_VERSION ) && ( GPSD_API_MAJOR_VERSION >= 5 )
        gps_data_t* data = m_gpsd.read();
#else
        gps_data_t* data = m_gpsd.poll();
#endif
        if ( data && data->set & PACKET_SET ) {
            emit gpsdInfo( *data );
        }
    }
#else
    gps_data_t* data = m_gpsd.query( "o" );

    if ( data ) {
        emit gpsdInfo( *data );
    }
    else if ( m_status != PositionProviderStatusAcquiring ) {
        mDebug() << "Lost connection to gpsd, trying to re-open.";
        initialize();
    }
#endif
}

QString GpsdConnection::error() const
{
    return m_error;
}

#include "GpsdConnection.moc"
