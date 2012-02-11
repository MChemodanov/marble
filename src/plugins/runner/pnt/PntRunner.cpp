//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011 Thibaut Gridel <tgridel@free.fr>
// Copyright 2012 Bernhard Beschow <bbeschow@cs.tu-berlin.de>

#include "PntRunner.h"

#include "GeoDataDocument.h"
#include "MarbleDebug.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

namespace Marble
{

// distance of 180deg in arcminutes
const qreal INT2RAD = M_PI / 10800.0;

PntRunner::PntRunner(QObject *parent) :
    MarbleAbstractRunner(parent)
{
}

PntRunner::~PntRunner()
{
}

GeoDataFeature::GeoDataVisualCategory PntRunner::category() const
{
    return GeoDataFeature::Folder;
}

void PntRunner::parseFile( const QString &fileName, DocumentRole role = UnknownDocument )
{
    QFileInfo fileinfo( fileName );
    if( fileinfo.suffix().compare( "pnt", Qt::CaseInsensitive ) != 0 ) {
        emit parsingFinished( 0 );
        return;
    }

    QFile  file( fileName );
    if ( !file.exists() ) {
        qWarning( "File does not exist!" );
        emit parsingFinished( 0 );
        return;
    }

    file.open( QIODevice::ReadOnly );
    QDataStream stream( &file );  // read the data serialized from the file
    stream.setByteOrder( QDataStream::LittleEndian );

    GeoDataDocument *document = new GeoDataDocument();
    document->setDocumentRole( role );

    GeoDataPlacemark  *placemark = 0;
    placemark = new GeoDataPlacemark;
    document->append( placemark );
    GeoDataMultiGeometry *geom = new GeoDataMultiGeometry;
    placemark->setGeometry( geom );

    int coastLine = 1001;
    int countryBorder = 2000;
    int internalPoliticalBorder = 4000;
    int island = 5001;
    int lake = 6001;
    int river = 7001;
    int customGlaciersLakesIslands = 8000;
    int customPoliticalBorder = 9001;
    int customPoliticalBorder2 = 14001;
    int customDateline = 19000;

    int count = 0;
    bool error = false;
    while( !stream.atEnd() || error ){
        short  header = -1;
        short  iLat = -5400 - 1;
        short  iLon = -10800 - 1;

        stream >> header >> iLat >> iLon;

        // make sure iLat is within valid range
        if ( !( -5400 <= iLat && iLat <= 5400 ) ) {
            mDebug() << Q_FUNC_INFO << "invalid iLat =" << iLat << "(" << ( iLat * INT2RAD ) * RAD2DEG << ") in dataset" << count << "of file" << fileName;
            error = true;
        }

        // make sure iLon is within valid range
        if ( !( -10800 <= iLon && iLon <= 10800 ) ) {
            mDebug() << Q_FUNC_INFO << "invalid iLon =" << iLon << "(" << ( iLon * INT2RAD ) * RAD2DEG << ") in dataset" << count << "of file" << fileName;
            error = true;
        }

        if ( header < 1 ) {
            /* invalid header */
            mDebug() << Q_FUNC_INFO << "invalid header:" << header << "in" << fileName << "at" << count;
            error = true;
            break;
        }
        else if ( header <= 5 ) {
            /* header represents level of detail */
            /* nothing to do */
        }
        else if ( header == coastLine ) {
            /* header represents start of coastline */
            geom->append( new GeoDataLinearRing );
            coastLine++;
        }
        else if ( header == countryBorder ) {
            /* header represents start of country border */
            geom->append( new GeoDataLinearRing );
            countryBorder++;
        }
        else if ( header == internalPoliticalBorder ) {
            /* header represents start of internal political border */
            geom->append( new GeoDataLinearRing );
            internalPoliticalBorder++;
        }
        else if ( header == island ) {
            /* header represents start of island */
            geom->append( new GeoDataLinearRing );
            island++;
        }
        else if ( header == lake ) {
            /* header represents start of lake */
            geom->append( new GeoDataLineString );
            lake++;
        }
        else if ( header == river ) {
            /* header represents start of river */
            geom->append( new GeoDataLinearRing );
            river++;
        }
        else if ( header == customGlaciersLakesIslands ) {
            /* custom header represents start of glaciers, lakes or islands */
            geom->append( new GeoDataLinearRing );
            customGlaciersLakesIslands++;
        }
        else if ( header == customPoliticalBorder ) {
            /* custom header represents start of political borders */
            geom->append( new GeoDataLinearRing );
            customPoliticalBorder++;
        }
        else if ( header == customPoliticalBorder2 ) {
            /* custom header represents start of political borders */
            geom->append( new GeoDataLinearRing );
            customPoliticalBorder2++;
        }
        else if ( header == customDateline ) {
            /* custom header represents start of dateline */
            geom->append( new GeoDataLineString );
            customDateline++;
        }
        else {
            /* invalid header */
            mDebug() << Q_FUNC_INFO
                      << "invalid header:" << header << "in" << fileName << "at" << count << endl
                      << "expected:" << coastLine << ","
                                     << countryBorder << ","
                                     << internalPoliticalBorder << ","
                                     << island << ","
                                     << lake << ","
                                     << river << ","
                                     << customGlaciersLakesIslands << ","
                                     << customPoliticalBorder << ","
                                     << customPoliticalBorder2 << ", or"
                                     << customDateline;
            error = true;
            break;
        }

        GeoDataLineString *polyline = static_cast<GeoDataLineString*>(geom->child(geom->size()-1));

        // Transforming Range of Coordinates to iLat [0,ARCMINUTE] , iLon [0,2 * ARCMINUTE]
        polyline->append( GeoDataCoordinates( (qreal)(iLon) * INT2RAD, (qreal)(iLat) * INT2RAD,
                                                  0.0, GeoDataCoordinates::Radian, 5 ) );

        ++count;
    }

    file.close();
    if ( geom->size() == 0 || error ) {
        delete document;
        document = 0;
    }

    emit parsingFinished( document );
}

}

#include "PntRunner.moc"
