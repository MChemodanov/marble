//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
//


// #include <zlib.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QStringList>


QString escapeXml( const QString &str )
{
    QString xml = str;
    xml.replace('&', "&amp;");
    xml.replace('<', "&lt;");
    xml.replace('>', "&gt;");
    xml.replace('\'', "&apos;");
    xml.replace('"', "&quot;");

    return xml;
}


int main(int argc, char *argv[])
{
    QString  sourcefilename;
    QString  targetfilename;

    QCoreApplication  app( argc, argv );

    for ( int i = 1; i < argc; ++i ) {
        if ( strcmp( argv[ i ], "-o" ) != 0 )
            continue;

        targetfilename = QString(argv[i+1]);
        sourcefilename = QString(argv[i+2]);

        qDebug() << "Source: " << sourcefilename;
        qDebug() << "Target: " << targetfilename;

        QFile  sourcefile( sourcefilename );
        sourcefile.open( QIODevice::ReadOnly );

        // Read the data serialized from the file.
        QTextStream  sourcestream( &sourcefile );
        sourcestream.setCodec("UTF-8");

        QFile  targetfile( targetfilename );
        targetfile.open( QIODevice::ReadWrite );

        QTextStream  targetstream( &targetfile );
        targetstream.setCodec("UTF-8");

        // gzFile gzDoc = gzopen( targetfilename.toLatin1(), "w");
        // QTextStream targetstream( new QString() );

        targetstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n"
                     << "<kml xmlns=\"http://earth.google.com/kml/2.1\"> \n"
                     << "<Document> \n";

        QString  rawline;
        QString  name;
        QString  state;
        QString  country;
        QString  role;
        QString  popstring;
        QString  latstring;
        QString  lngstring;
        float    lat;
        float    lng;
        int          population;
        QStringList  splitline;

        while ( !sourcestream.atEnd() ) {

            rawline=sourcestream.readLine();
            splitline = rawline.split('\t');

            name    = splitline[0];
            state   = splitline[1];
            country = splitline[2];
            role    = splitline[3];

            popstring = splitline[4];
            latstring = splitline[5];
            lngstring = splitline[6];

            population = (int) ( 1000 * popstring.toFloat() );

            lng = lngstring.left( lngstring.size() - 2 ).toFloat();
            if ( lngstring.contains( "W" ) )
                lng=-lng;

            lat = latstring.left( latstring.size() - 2 ).toFloat();
            if ( latstring.contains( "S" ) )
                lat=-lat;

            targetstream << "    <Placemark> \n";
            targetstream << "        <name>" << escapeXml( name ) << "</name> \n";
            targetstream << "        <state>" << escapeXml( state ) << "</state> \n";
            targetstream << "        <CountryNameCode>" << escapeXml( country.toUpper() ) << "</CountryNameCode>\n";
            targetstream << "        <role>" << escapeXml( role ) << "</role> \n";
            targetstream << "        <pop>"
                         << escapeXml( QString::number( population ) ) << "</pop> \n";
            targetstream << "        <Point>\n"
                         << "            <coordinates>"
                         << escapeXml( QString::number( lng ) )
                         << ","
                         << escapeXml( QString::number( lat ) )
                         << "</coordinates> \n"
                         << "        </Point> \n";
            targetstream << "    </Placemark> \n";
        }

        targetstream << "</Document> \n"
                     << "</kml> \n";
        qDebug("Putting");

        // gzputs( gzDoc, targetstream.readAll().toUtf8() );
        // gzclose( gzDoc );

        sourcefile.close();
        targetfile.close();

        qDebug("Finished!");
        return 0;
    }

    qDebug(" asc2kml -o targetfile sourcefile");
    app.exit();
}
