//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Eckhart Wörner <ewoerner@kde.org>
//

#include "GpsdPositionProviderPlugin.h"

#include "GpsdThread.h"
#include "MarbleDebug.h"

using namespace Marble;

QString GpsdPositionProviderPlugin::name() const
{
    return tr( "Gpsd position provider Plugin" );
}

QString GpsdPositionProviderPlugin::nameId() const
{
    return QString::fromLatin1( "Gpsd" );
}

QString GpsdPositionProviderPlugin::guiString() const
{
    return tr( "gpsd" );
}

QString GpsdPositionProviderPlugin::description() const
{
    return tr( "Reports the position of a GPS device." );
}

QIcon GpsdPositionProviderPlugin::icon() const
{
    return QIcon();
}

void GpsdPositionProviderPlugin::initialize()
{
    m_status = PositionProviderStatusAcquiring;
    emit statusChanged( m_status );

    m_thread = new GpsdThread;
    connect( m_thread, SIGNAL( gpsdInfo( gps_data_t ) ),
             this, SLOT( update( gps_data_t ) ) );
    connect( m_thread, SIGNAL( statusChanged( PositionProviderStatus ) ),
             this, SIGNAL( statusChanged( PositionProviderStatus ) ) );
    m_thread->start();
}

void GpsdPositionProviderPlugin::update( gps_data_t data )
{
    PositionProviderStatus oldStatus = m_status;
    GeoDataCoordinates oldPosition = m_position;
    if ( data.status == STATUS_NO_FIX )
        m_status = PositionProviderStatusUnavailable;
    else {
        m_status = PositionProviderStatusAvailable;
        m_position.set( data.fix.longitude, data.fix.latitude,
                        data.fix.altitude, GeoDataCoordinates::Degree );
        if (data.fix.mode == MODE_2D) {
            m_position.setAltitude(0);
        }
        m_accuracy.level = GeoDataAccuracy::Detailed;
        // FIXME: Add real values here
        m_accuracy.horizontal = 5;
        m_accuracy.vertical = 5;
    }
    if (m_status != oldStatus)
        emit statusChanged( m_status );
    if (!(oldPosition == m_position)) {
        emit positionChanged( m_position, m_accuracy );
    }
}

bool GpsdPositionProviderPlugin::isInitialized() const
{
    return m_thread;
}

PositionProviderPlugin* GpsdPositionProviderPlugin::newInstance() const
{
    return new GpsdPositionProviderPlugin;
}

PositionProviderStatus GpsdPositionProviderPlugin::status() const
{
    return m_status;
}

GeoDataCoordinates GpsdPositionProviderPlugin::position() const
{
    return m_position;
}

GeoDataAccuracy GpsdPositionProviderPlugin::accuracy() const
{
    return m_accuracy;
}

GpsdPositionProviderPlugin::GpsdPositionProviderPlugin() : m_thread( 0 )
{
}

GpsdPositionProviderPlugin::~GpsdPositionProviderPlugin()
{
    if ( m_thread ) {
        m_thread->exit();
        
        if ( !m_thread->wait( 5000 ) ) {
            mDebug() << "Failed to stop GpsdThread";
        }
        else {
            delete m_thread;
        }
    }
}

QString GpsdPositionProviderPlugin::error() const
{
    return m_thread->error();
}

Q_EXPORT_PLUGIN2( GpsdPositionProviderPlugin, Marble::GpsdPositionProviderPlugin )



#include "GpsdPositionProviderPlugin.moc"
