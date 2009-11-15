//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Torsten Rahn  <rahn@kde.org>
//

// Own
#include "MarblePhysics.h"

#include "MarbleDebug.h"
#include <QtCore/QTimeLine>

#include "Quaternion.h"

using namespace Marble;

MarblePhysics::MarblePhysics( QObject * parent )
    : QObject( parent ), 
      m_jumpDuration( 2000 )
{
    m_timeLine = new QTimeLine( m_jumpDuration );
    m_timeLine->setFrameRange( 0, 500 );
    m_timeLine->setCurveShape( QTimeLine::EaseInOutCurve );
    m_timeLine->setUpdateInterval( 0 );
    connect( m_timeLine, SIGNAL( valueChanged( qreal ) ), SIGNAL( valueChanged( qreal ) ) );
}

MarblePhysics::~MarblePhysics()
{
    delete m_timeLine;
}

GeoDataCoordinates MarblePhysics::suggestedPosition() const
{
    qreal lon, lat;
    Quaternion  itpos;

    qreal t = m_timeLine->currentValue();

    // Spherical interpolation for current position between source position
    // and target position. We can't use Nlerp here, as the "t-velocity" needs to be constant.
    itpos.slerp( m_sourcePosition.quaternion(), m_targetPosition.quaternion(), t );
    itpos.getSpherical( lon, lat );

    // Purely cinematic approach to calculate the jump path

    qreal g = m_sourcePosition.altitude(); // Initial altitude
    qreal h = 3000.0;                      // Jump height

    // Parameters for the parabolic function that has the maximum at
    // the point H ( 0.5 * m_jumpDuration, g + h )
    qreal a = - h / ( (qreal)( 0.25 * m_jumpDuration * m_jumpDuration ) );
    qreal b = 2.0 * h / (qreal)( 0.5 * m_jumpDuration );

    qreal x = (qreal)(m_jumpDuration ) * t;

    qreal y = ( a * x + b ) * x + g;       // Parabolic function

    return GeoDataCoordinates( lon, lat, y );
}

void MarblePhysics::jumpTo( const GeoDataCoordinates &targetPosition )
{
    m_targetPosition = targetPosition;
    m_timeLine->start();
}

void MarblePhysics::setCurrentPosition( const GeoDataCoordinates &sourcePosition )
{
    m_timeLine->stop();
    m_sourcePosition = sourcePosition;
}

#include "MarblePhysics.moc"
