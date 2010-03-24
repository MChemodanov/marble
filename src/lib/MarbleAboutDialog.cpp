/* This file is part of the KDE project
 *
 * This file is part of the Marble Desktop Globe.
 *
 * Copyright 2004-2007 Torsten Rahn <tackat@kde.org>
 * Copyright 2007      Inge Wallin  <ingwa@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#include "MarbleAboutDialog.h"
#include "ui_MarbleAboutDialog.h"

#include <QtCore/QFile>
#include <QtGui/QTextFrame>
#include <QtGui/QTabWidget>
#include <QtCore/QTextStream>
#include <QtGui/QPixmap>

#include "global.h"
#include "MarbleDirs.h"

namespace Marble
{

class MarbleAboutDialogPrivate
{
public: 
    Ui::MarbleAboutDialog  uiWidget;

    bool authorsLoaded;
    bool dataLoaded;
    bool licenseLoaded;
};


MarbleAboutDialog::MarbleAboutDialog(QWidget *parent)
    : QDialog( parent ),
      d( new MarbleAboutDialogPrivate )
{
    d->uiWidget.setupUi( this );

    d->authorsLoaded = false;
    d->dataLoaded = false;
    d->licenseLoaded = false;

    if( MarbleGlobal::getInstance()->profile() == MarbleGlobal::MobileInternetDevice ) {
        d->uiWidget.m_pMarbleTitleLabel->hide();
        d->uiWidget.m_pMarbleLogoLabel->hide();
    }
    else {
        d->uiWidget.m_pMarbleLogoLabel->setPixmap( 
                QPixmap( MarbleDirs::path("svg/marble-logo-72dpi.png") ) );
    }
    d->uiWidget.m_pMarbleVersionLabel->setText( tr("Version %1").arg(MARBLE_VERSION_STRING) );

    connect( d->uiWidget.tabWidget, SIGNAL( currentChanged( int ) ), 
             this, SLOT( loadPageContents( int ) ) );

    QTextBrowser* browser = d->uiWidget.m_pMarbleAboutBrowser;
    browser->setHtml( tr("<br />(c) 2007, 2008, 2009 The Marble Project<br /><br /><a href=\"http://edu.kde.org/marble\">http://edu.kde.org/marble</a>") );

    QTextFrameFormat  format = browser->document()->rootFrame()->frameFormat();
    format.setMargin(12);
    browser->document()->rootFrame()->setFrameFormat( format );

}

MarbleAboutDialog::~MarbleAboutDialog()
{
   delete d;
}

void MarbleAboutDialog::loadPageContents( int idx )
{
    QTextBrowser* browser = 0;

    if ( idx == 1 && !d->authorsLoaded )
    {
        d->authorsLoaded = true;
        browser = d->uiWidget.m_pMarbleAuthorsBrowser;
        browser->setHtml( tr("<b>Active Development Team of Marble 0.8 and 0.7</b>")+
        tr("<p>Torsten Rahn <a href=\"mailto:rahn@kde.org\">rahn@kde.org</a><br>"
           "<i>Core Developer and Original Author</i></p>")+
        tr("<p>Patrick Spendrin <a href=\"pspendrin@gmail.com\">pspendrin@gmail.com</a><br>"
           "<i>Core Developer: KML and Windows support</i></p>")+
        tr("<p>Jens-Michael Hoffmann <a href=\"mailto:jensmh@gmx.de\">jensmh@gmx.de</a><br>"
           "<i>Core Developer: OpenStreetMap support, Download Management</i></p>")+
        tr("<p>Henry de Valence <a href=\"mailto:hdevalence@gmail.com\">hdevalence@gmail.com</a><br>"
           "<i>Core Developer: Marble Runners, World-Clock Plasmoid</i></p>")+
        tr("<p>Bastian Holst, <a href=\"mailto:bastianholst@gmx.de\">bastianholst@gmx.de</a><br>"
           "<i>Online Services support</i></p>")+
        tr("<p>Andrew Manson <a href=\"mailto:g.real.ate@gmail.com\">g.real.ate@gmail.com</a><br>"
           "<i>Proxy support</i></p>")+
        tr("<p>Pino Toscano <a href=\"mailto:pino@kde.org\">pino@kde.org</a><br>"
           "<i>Network plugins</i></p>")+
        tr("<p>Harshit Jain <a href=\"mailto:sonu.itbhu@googlemail.com\">sonu.itbhu@googlemail.com</a><br>"
           "<i>Planet Filter, bugfixes</i></p>")+
        tr("<p>M&eacute;d&eacute;ric Boquien <a href=\"mailto:mboquien@free.fr\">mboquien@free.fr</a><br>"
           "<i>Astronomical Observatories</i></p>")+
        tr("<p> Magnus Valle<br><i>Historical Maps</i></p>")+
        tr("<p>Eckhart W&ouml;rner <a href=\"mailto:kde@ewsoftware.de\">kde@ewsoftware.de</a><br>"
           "<i>Bugfixes</i></p>")+
        tr("<b>Join us</b>")+
        tr("<p>You can reach the developers of the Marble Project at <a href=\"mailto:marble-devel@kde.org\">marble-devel@kde.org</a></p>")+
        tr("<b>Google Summer of Code</b>")+
        tr("<p> The Marble Team would like to thank its members who participated "
           "in the Google Summer of Code for their successful work on Marble:</p>")+
        tr("<p><b>2008</b></p>")+
        tr("<p>Patrick Spendrin <a href=\"pspendrin@gmail.com\">pspendrin@gmail.com</a><br>"
           "<i>Project: Vector Tiles for Marble</i></p>")+
        tr("<p>Shashank Singh <a href=\"mailto:shashank.personal@gmail.com\">shashank.personal@gmail.com</a><br>"
           "<i>Project: Panoramio / Wikipedia -photo support for Marble</i></p>")+
        tr("<b>2007</b>")+
        tr("<p>Carlos Licea <a href=\"mailto:carlos.licea@kdemail.net\">carlos.licea@kdemail.net</a><br>"
           "<i>Project: Equirectangular Projection (\"Flat Map\")</i></p>")+
        tr("<p>Andrew Manson <a href=\"mailto:g.real.ate@gmail.com\">g.real.ate@gmail.com</a><br>"
           "<i>Project: GPS Support for Marble</i></p>")+
        tr("<p>Murad Tagirov <a href=\"mailto:tmurad@gmail.com\">tmurad@gmail.com</a><br>"
           "<i>Project: KML Support for Marble</i></p>")+
        tr("<p>... and of course we'd like to thank the people at Google Inc. for making these projects possible.</p>")+
        tr("<b>Developers</b>")+
        tr("<p>Inge Wallin <a href=\"mailto:inge@lysator.liu.se\">inge@lysator.liu.se</a><br>"
           "<i>Core Developer and Co-Maintainer</i></p>")+
        tr("<p><i>Development &amp; Patches:</i> Simon Schmeisser, Claudiu Covaci, David Roberts, Nikolas Zimmermann, Jan Becker, "
           "Stefan Asserh&auml;ll, Laurent Montel, Prashanth Udupa, Anne-Marie Mahfouf, Josef Spillner, Frerich Raabe, "
           "Frederik Gladhorn, Fredrik H&ouml;glund, Albert Astals Cid, Thomas Zander, Joseph Wenninger, Kris Thomsen, "
           "Daniel Molkentin  </p>")+
        tr("<p><i>Platforms &amp; Distributions:</i>"
           "Tim Sutton, Christian Ehrlicher, Ralf Habacker, Steffen Joeris, Marcus Czeslinski, Marcus D. Hanwell, Chitlesh Goorah.</p>")+
        tr("<p><i>Artwork:</i> Nuno Pinheiro, Torsten Rahn</p>")+
        tr("<b>Credits</b>")+
        tr("<p><i>Various Suggestions &amp; Testing:</i> Stefan Jordan, Robert Scott, Lubos Petrovic, Benoit Sigoure, "
           "Martin Konold, Matthias Welwarsky, Rainer Endres, Luis Silva, Ralf Gesellensetter, Tim Alder</p>")+
        tr("<p> We'd especially like to thank John Layt who provided an important source of inspiration "
           "by creating Marble's predecessor \"Kartographer\".</p>"));
        QTextFrameFormat  format = browser->document()->rootFrame()->frameFormat();
        format.setMargin(12);
        browser->document()->rootFrame()->setFrameFormat( format );
    }

    if ( idx == 2 && !d->dataLoaded )
    {
        d->dataLoaded = true;
        browser = d->uiWidget.m_pMarbleDataBrowser;
        browser->setHtml( tr("<b>Maps</b>")+
        tr("<p><i>Blue Marble Next Generation (500 m / pixel)</i><br>" 
           "NASA Goddard Space Flight Center Earth Observatory "
           "<a href=\"http://earthobservatory.nasa.gov/Newsroom/BlueMarble/\">"
           "http://earthobservatory.nasa.gov/Newsroom/BlueMarble/</a></p>")+
        tr("<p><i>Earth's City Lights</i><br>"
           "Data courtesy Marc Imhoff of NASA GSFC and Christopher Elvidge of NOAA NGDC. "
           "Image by Craig Mayhew and Robert Simmon, NASA GSFC.</p>")+
        tr("<p><i>Shuttle Radar Topography Mission (SRTM30, 1 km / pixel )</i><br>" 
           "NASA Jet Propulsion Laboratory <a href=\"http://www2.jpl.nasa.gov/srtm/\">"
           "http://www2.jpl.nasa.gov/srtm/</a></p>")+
        tr("<p><i>Micro World Data Bank in Polygons (\"MWDB-POLY / MWDBII\")</i><br>"
           "CIA ; Global Associates, Ltd.; Fred Pospeschil and Antonio Rivera</p>")+
        tr("<p><i>Temperature and Precipitation Maps (July and December)</i><br>"
           "A combination of two datasets:"
           "<ul>"
           "<li>Legates, D.R. and Willmott, C.J. 1989. Average Monthly Surface Air Temperature and "
           "Precipitation. Digital Raster Data on a .5 degree Geographic (lat/long) 361x721 grid "
           "(centroid-registered on .5 degree meridians). Boulder CO: National Center for Atmospheric Research. "
           "<a href=\"http://www.ngdc.noaa.gov/ecosys/cdroms/ged_iia/datasets/a04/lw.htm\">"
           "http://www.ngdc.noaa.gov/ecosys/cdroms/ged_iia/datasets/a04/lw.htm</a></li>"
           "<li>CRU CL 2.0: New, M., Lister, D., Hulme, M. and Makin, I., 2002: A high-resolution "
           "data set of surface climate over global land areas. Climate Research 21."
           "<a href=\"http://www.cru.uea.ac.uk/cru/data/hrg\">"
           "http://www.cru.uea.ac.uk/cru/data/hrg</a></li></ul></p>")+
        tr("<b>Street Map</b>")+
        tr("<p><i>OpenStreetMap</i><br>"
           "The street maps used in Marble via download are provided by the <a href=\""
           "http://www.openstreetmap.org\">OpenStreetMap</a> Project (\"OSM\"). "
           "OSM is an open community which creates free editable maps."
           "<i>License</i>: OpenStreetMap data can be used freely under the terms of the "
           "<a href=\"http://wiki.openstreetmap.org/index.php/OpenStreetMap_License\">"
           "Creative Commons Attribution-ShareAlike 2.0license</a>.</p>")+
        tr("<b>Cities and Locations</b>")+
        tr("<p><i>World Gazetteer</i><br>Stefan Helders "
           "<a href=\"http://www.world-gazetteer.com\">http://www.world-gazetteer.com</a></p>")+
        tr("<p><i>Geonames.org</i><br>"
           "<a href=\"http://www.geonames.org/\">http://www.geonames.org/</a>"
           "<i>License</i>: Geonames.org data can be used freely under the terms of the "
           "<a href=\"http://creativecommons.org/licenses/by/3.0/\">Creative Commons Attribution 3.0 license</a>.</p>")+
        tr("<p><i>Czech Statistical Office</i><br>Public database "
           "<a href=\"http://www.czso.cz/eng/redakce.nsf/i/home\">http://www.czso.cz</a></p>")+
        tr("<b>Flags</b>")+
        tr("<p><i>Flags of the World</i><br>The flags were taken from Wikipedia "
           "(<a href=\"http://www.wikipedia.org\">http://www.wikipedia.org</a>) "
           "which in turn took a subset from <a href=\"http://www.openclipart.org\">"
           "http://www.openclipart.org</a> and reworked them. "
           "All flags are under the public domain (see comments inside the svg files).</p>")+
        tr("<b>Stars</b>")+
        tr("<p><i>The Bright Star Catalogue</i><br>5th Revised Ed. (Preliminary Version) "
           "Hoffleit D., Warren Jr W.H., Astronomical Data Center, NSSDC/ADC (1991)"
           "<a href=\"http://adc.gsfc.nasa.gov/adc-cgi/cat.pl?/catalogs/5/5050\"> "
           "http://adc.gsfc.nasa.gov</a></p>") );
        QTextFrameFormat  format = browser->document()->rootFrame()->frameFormat();
        format.setMargin(12);
        browser->document()->rootFrame()->setFrameFormat( format );
    }

    if ( idx == 3 && !d->licenseLoaded )
    {
        d->licenseLoaded = true;
        browser = d->uiWidget.m_pMarbleLicenseBrowser;
        QString filename = MarbleDirs::path( "LICENSE.txt" );
        if( !filename.isEmpty() )
        {
            QFile  f( filename );
            if( f.open( QIODevice::ReadOnly ) ) 
            {
                QTextStream ts( &f );
                browser->setText( ts.readAll() );
            }
            f.close();
        }
        QTextFrameFormat  format = browser->document()->rootFrame()->frameFormat();
        format.setMargin(12);
        browser->document()->rootFrame()->setFrameFormat( format );
    }
}

}

#include "MarbleAboutDialog.moc"
