//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011 Thibaut Gridel <tgridel@free.fr>
//

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QtCore/QMetaType>

#include "MarbleDebug.h"
#include "MarbleDirs.h"
#include "MarbleRunnerManager.h"
#include "PluginManager.h"
#include "GeoDataPlacemark.h"
#include "routing/RouteRequest.h"

#define addRow() QTest::newRow( QString("line %1").arg( __LINE__ ).toAscii().data() )

Q_DECLARE_METATYPE( QList<Marble::GeoDataCoordinates> )

namespace Marble
{

class MarbleRunnerManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testAsyncPlacemarks_data();
    void testAsyncPlacemarks();

    void testAsyncReverse_data();
    void testAsyncReverse();

    void testAsyncRouting_data();
    void testAsyncRouting();

    void testAsyncParsing_data();
    void testAsyncParsing();

public:
    PluginManager m_pluginManager;
    GeoDataCoordinates m_coords;
    GeoDataCoordinates m_coords2;
};

void MarbleRunnerManagerTest::initTestCase()
{
    // MarbleDebug::enable = true;
    MarbleDirs::setMarbleDataPath( DATA_PATH );
    MarbleDirs::setMarblePluginPath( PLUGIN_PATH );

    qRegisterMetaType<QList<GeoDataCoordinates> >( "QList<GeoDataCoordinates>" );

    qRegisterMetaType<QList<GeoDataCoordinates> >( "QList<GeoDataCoordinates>" );

    m_coords.setLatitude(52.50160, GeoDataCoordinates::Degree );
    m_coords.setLongitude(13.40233, GeoDataCoordinates::Degree );

    m_coords2.setLatitude(52.52665, GeoDataCoordinates::Degree );
    m_coords2.setLongitude(13.39032, GeoDataCoordinates::Degree );
}

void MarbleRunnerManagerTest::cleanupTestCase()
{
}


void MarbleRunnerManagerTest::testAsyncPlacemarks_data()
{
    QTest::addColumn<QString>( "name" );

    addRow() << QString( "Berlin" );
    addRow() << QString( "www.heise.de" );
}

void MarbleRunnerManagerTest::testAsyncPlacemarks()
{
    MarbleRunnerManager m_runnerManager(&m_pluginManager, this);

    QSignalSpy finishSpy( &m_runnerManager, SIGNAL( searchFinished( QString ) ) );
    QSignalSpy resultSpy( &m_runnerManager, SIGNAL( searchResultChanged( QVector<GeoDataPlacemark*> ) ) );

    QEventLoop loop;
    connect( &m_runnerManager, SIGNAL( searchFinished( QString ) ),
             &loop, SLOT( quit() ) );

    QFETCH( QString, name );
    m_runnerManager.findPlacemarks( name );

    loop.exec();

    QCOMPARE( resultSpy.count(), 1 );
    QCOMPARE( finishSpy.count(), 1 );
}

void MarbleRunnerManagerTest::testAsyncReverse_data()
{
    QTest::addColumn<GeoDataCoordinates>( "coordinates" );

    addRow() << m_coords;
}

void MarbleRunnerManagerTest::testAsyncReverse()
{
    MarbleRunnerManager m_runnerManager(&m_pluginManager, this);

    QSignalSpy finishSpy( &m_runnerManager, SIGNAL(reverseGeocodingFinished()) );
    QSignalSpy resultSpy( &m_runnerManager, SIGNAL(reverseGeocodingFinished(GeoDataCoordinates,GeoDataPlacemark)) );

    QEventLoop loop;
    connect( &m_runnerManager, SIGNAL( reverseGeocodingFinished() ),
             &loop, SLOT( quit() ) );

    QFETCH( GeoDataCoordinates, coordinates );
    m_runnerManager.reverseGeocoding( coordinates );

    loop.exec();

    QCOMPARE( resultSpy.count(), 1 );
    QCOMPARE( finishSpy.count(), 1 );
}

void MarbleRunnerManagerTest::testAsyncRouting_data()
{
    QTest::addColumn<QList<GeoDataCoordinates> >( "coordinatesList" );

    addRow() << ( QList<GeoDataCoordinates>() << m_coords << m_coords2 );
}

void MarbleRunnerManagerTest::testAsyncRouting()
{
    MarbleRunnerManager m_runnerManager(&m_pluginManager, this);

    QSignalSpy finishSpy( &m_runnerManager, SIGNAL(routingFinished()) );
    QSignalSpy resultSpy( &m_runnerManager, SIGNAL( routeRetrieved(GeoDataDocument*)) );

    QEventLoop loop;
    connect( &m_runnerManager, SIGNAL( routingFinished() ),
             &loop, SLOT( quit() ) );

    QFETCH( QList<GeoDataCoordinates>, coordinatesList );
    RouteRequest request;
    foreach( const GeoDataCoordinates &coordinates, coordinatesList ) {
        request.append( coordinates );
    }

    m_runnerManager.retrieveRoute( &request );

    loop.exec();

    QVERIFY( resultSpy.count() > 0 );
    QCOMPARE( finishSpy.count(), 1 );
}

void MarbleRunnerManagerTest::testAsyncParsing_data()
{
    QTest::addColumn<QString>( "fileName" );

    addRow() << MarbleDirs::path( "placemarks/otherplacemarks.cache" );
}

void MarbleRunnerManagerTest::testAsyncParsing()
{
    MarbleRunnerManager m_runnerManager(&m_pluginManager, this);

    QSignalSpy finishSpy( &m_runnerManager, SIGNAL( parsingFinished() ) );
    QSignalSpy resultSpy( &m_runnerManager, SIGNAL( parsingFinished(GeoDataDocument*,QString)) );

    QEventLoop loop;
    connect( &m_runnerManager, SIGNAL( parsingFinished() ),
             &loop, SLOT( quit() ) );

    QFETCH( QString, fileName );

    m_runnerManager.parseFile( fileName );

    loop.exec();

    QVERIFY( resultSpy.count() > 0 );
    QCOMPARE( finishSpy.count(), 1 );
}

}

QTEST_MAIN( Marble::MarbleRunnerManagerTest )

#include "MarbleRunnerManagerTest.moc"
