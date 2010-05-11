// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "DownloadRegionDialog.h"

#include <cmath>

#include <QtGui/QDialogButtonBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QVBoxLayout>

#include "AbstractScanlineTextureMapper.h"
#include "GeoDataLatLonBox.h"
#include "GeoSceneTexture.h"
#include "MarbleDebug.h"
#include "MarbleMath.h"
#include "LatLonBoxWidget.h"
#include "TileId.h"
#include "TileLevelRangeWidget.h"
#include "TileLoaderHelper.h"
#include "ViewportParams.h"

namespace Marble
{

int const maxTilesCount = 100000;

class DownloadRegionDialog::Private
{
public:
    Private( ViewportParams const * const viewport,
             AbstractScanlineTextureMapper const * const textureMapper,
             QDialog * const dialog );
    QWidget * createSelectionMethodBox();
    QLayout * createTilesCounter();
    QWidget * createOkCancelButtonBox();

    int rad2PixelX( qreal const lon ) const;
    int rad2PixelY( qreal const lat ) const;

    QDialog * m_dialog;
    LatLonBoxWidget * m_latLonBoxWidget;
    TileLevelRangeWidget * m_tileLevelRangeWidget;
    QLabel * m_tilesCountLabel;
    QLabel * m_tilesCountLimitInfo;
    QPushButton * m_okButton;
    QPushButton * m_applyButton;
    int m_originatingTileLevel;
    int m_minimumAllowedTileLevel;
    int m_maximumAllowedTileLevel;
    ViewportParams const * const m_viewport;
    AbstractScanlineTextureMapper const * const m_textureMapper;
    GeoSceneTexture const * const m_textureLayer;
    GeoDataLatLonBox m_visibleRegion;
};

DownloadRegionDialog::Private::Private( ViewportParams const * const viewport,
                                        AbstractScanlineTextureMapper const * const textureMapper,
                                        QDialog * const dialog )
    : m_dialog( dialog ),
      m_latLonBoxWidget( new LatLonBoxWidget ),
      m_tileLevelRangeWidget( new TileLevelRangeWidget ),
      m_tilesCountLabel( 0 ),
      m_tilesCountLimitInfo( 0 ),
      m_okButton( 0 ),
      m_applyButton( 0 ),
      m_originatingTileLevel( textureMapper->tileZoomLevel() ),
      m_minimumAllowedTileLevel( -1 ),
      m_maximumAllowedTileLevel( -1 ),
      m_viewport( viewport ),
      m_textureMapper( textureMapper ),
      m_textureLayer( textureMapper->textureLayer() ),
      m_visibleRegion( viewport->viewLatLonAltBox() )
{
    m_latLonBoxWidget->setEnabled( false );
    m_latLonBoxWidget->setLatLonBox( m_visibleRegion );
    m_tileLevelRangeWidget->setDefaultLevel( m_originatingTileLevel );
}

QWidget * DownloadRegionDialog::Private::createSelectionMethodBox()
{
    QRadioButton * const visibleRegionMethodButton = new QRadioButton( tr( "Visible region" ));
    visibleRegionMethodButton->setChecked( true );
    QRadioButton * const latLonBoxMethodButton = new QRadioButton( tr( "Specify region" ));
    connect( latLonBoxMethodButton, SIGNAL( toggled( bool )),
             m_dialog, SLOT( toggleSelectionMethod() ));

    QVBoxLayout * const layout = new QVBoxLayout;
    layout->addWidget( visibleRegionMethodButton );
    layout->addWidget( latLonBoxMethodButton );
    layout->addWidget( m_latLonBoxWidget );

    QGroupBox * const selectionMethodBox = new QGroupBox( tr( "Selection method" ));
    selectionMethodBox->setLayout( layout );
    return selectionMethodBox;
}

QLayout * DownloadRegionDialog::Private::createTilesCounter()
{
    QLabel * const description = new QLabel( tr( "Number of tiles to download:" ));
    m_tilesCountLabel = new QLabel;
    m_tilesCountLimitInfo = new QLabel;

    QHBoxLayout * const tilesCountLayout = new QHBoxLayout;
    tilesCountLayout->addWidget( description );
    tilesCountLayout->addWidget( m_tilesCountLabel );

    QVBoxLayout * const layout = new QVBoxLayout;
    layout->addLayout( tilesCountLayout );
    layout->addWidget( m_tilesCountLimitInfo );
    return layout;
}

QWidget * DownloadRegionDialog::Private::createOkCancelButtonBox()
{
    QDialogButtonBox * const buttonBox = new QDialogButtonBox;
    m_okButton = buttonBox->addButton( QDialogButtonBox::Ok );
    m_applyButton = buttonBox->addButton( QDialogButtonBox::Apply );
    buttonBox->addButton( QDialogButtonBox::Cancel );
    connect( buttonBox, SIGNAL( accepted() ), m_dialog, SLOT( accept() ));
    connect( buttonBox, SIGNAL( rejected() ), m_dialog, SLOT( reject() ));
    connect( m_applyButton, SIGNAL( clicked() ), m_dialog, SIGNAL( applied() ));
    return buttonBox;
}

// copied from AbstractScanlineTextureMapper and slightly adjusted
int DownloadRegionDialog::Private::rad2PixelX( qreal const lon ) const
{
    qreal const globalWidth = m_textureMapper->tileSize().width()
        * TileLoaderHelper::levelToColumn( m_textureLayer->levelZeroColumns(),
                                           m_originatingTileLevel );
    return static_cast<int>( globalWidth * 0.5 + lon * ( globalWidth / ( 2.0 * M_PI ) ));
}

// copied from AbstractScanlineTextureMapper and slightly adjusted
int DownloadRegionDialog::Private::rad2PixelY( qreal const lat ) const
{
    qreal const globalHeight = m_textureMapper->tileSize().height()
        * TileLoaderHelper::levelToRow( m_textureLayer->levelZeroRows(), m_originatingTileLevel );
    qreal const normGlobalHeight = globalHeight / M_PI;
    switch ( m_textureLayer->projection() ) {
    case GeoSceneTexture::Equirectangular:
        return static_cast<int>( globalHeight * 0.5 - lat * normGlobalHeight );
    case GeoSceneTexture::Mercator:
        if ( fabs( lat ) < 1.4835 )
            return static_cast<int>( globalHeight * 0.5 - gdInv( lat ) * 0.5 * normGlobalHeight );
        if ( lat >= +1.4835 )
            return static_cast<int>( globalHeight * 0.5 - 3.1309587 * 0.5 * normGlobalHeight );
        if ( lat <= -1.4835 )
            return static_cast<int>( globalHeight * 0.5 + 3.1309587 * 0.5 * normGlobalHeight );
    }

    // Dummy value to avoid a warning.
    return 0;
}

DownloadRegionDialog::DownloadRegionDialog( ViewportParams const * const viewport,
                                            AbstractScanlineTextureMapper const * const textureMapper,
                                            QWidget * const parent, Qt::WindowFlags const f )
    : QDialog( parent, f ),
      d( new Private( viewport, textureMapper, this ))
{
    setWindowTitle( tr( "Download Region" ));

    QVBoxLayout * const layout = new QVBoxLayout;
    layout->addWidget( d->createSelectionMethodBox() );
    layout->addWidget( d->m_tileLevelRangeWidget );
    layout->addLayout( d->createTilesCounter() );
    layout->addWidget( d->createOkCancelButtonBox() );
    setLayout( layout );

    connect( d->m_latLonBoxWidget, SIGNAL( valueChanged() ), SLOT( updateTilesCount() ));
    connect( d->m_tileLevelRangeWidget, SIGNAL( topLevelChanged( int )),
             SLOT( updateTilesCount() ));
    connect( d->m_tileLevelRangeWidget, SIGNAL( bottomLevelChanged( int )),
             SLOT( updateTilesCount() ));
    updateTilesCount();
}

void DownloadRegionDialog::setAllowedTileLevelRange( int const minimumTileLevel,
                                                     int const maximumTileLevel )
{
    d->m_minimumAllowedTileLevel = minimumTileLevel;
    d->m_maximumAllowedTileLevel = maximumTileLevel;
    d->m_tileLevelRangeWidget->setAllowedLevelRange( minimumTileLevel, maximumTileLevel );
}

void DownloadRegionDialog::setOriginatingTileLevel( int const tileLevel )
{
    d->m_originatingTileLevel = tileLevel;
    d->m_tileLevelRangeWidget->setDefaultLevel( tileLevel );
}

TileCoordsPyramid DownloadRegionDialog::region() const
{
    // check whether "visible region" or "lat/lon region" is selection method
    GeoDataLatLonBox downloadRegion = d->m_visibleRegion;
    if ( d->m_latLonBoxWidget->isEnabled() )
        downloadRegion = d->m_latLonBoxWidget->latLonBox();

    int const westX = d->rad2PixelX( downloadRegion.west() );
    int const northY = d->rad2PixelY( downloadRegion.north() );
    int const eastX = d->rad2PixelX( downloadRegion.east() );
    int const southY = d->rad2PixelY( downloadRegion.south() );

    // FIXME: remove this stuff
    mDebug() << "DownloadRegionDialog downloadRegion:"
             << "north:" << downloadRegion.north()
             << "south:" << downloadRegion.south()
             << "east:" << downloadRegion.east()
             << "west:" << downloadRegion.west();
    mDebug() << "north/west (x/y):" << westX << northY;
    mDebug() << "south/east (x/y):" << eastX << southY;

    int const tileWidth = d->m_textureMapper->tileSize().width();
    int const tileHeight = d->m_textureMapper->tileSize().height();

    int const visibleLevelX1 = qMin( westX, eastX );
    int const visibleLevelY1 = qMin( northY, southY );
    int const visibleLevelX2 = qMax( westX, eastX );
    int const visibleLevelY2 = qMax( northY, southY );

    mDebug() << "visible level pixel coords (level/x1/y1/x2/y2):" << d->m_originatingTileLevel
             << visibleLevelX1 << visibleLevelY1 << visibleLevelX2 << visibleLevelY2;

    int bottomLevelX1, bottomLevelY1, bottomLevelX2, bottomLevelY2;
    // the pixel coords calculated above are referring to the originating ("visible") tile level,
    // if the bottom level is a different level, we have to take it into account
    if ( d->m_originatingTileLevel > d->m_tileLevelRangeWidget->bottomLevel() ) {
        int const deltaLevel = d->m_originatingTileLevel - d->m_tileLevelRangeWidget->bottomLevel();
        bottomLevelX1 = visibleLevelX1 >> deltaLevel;
        bottomLevelY1 = visibleLevelY1 >> deltaLevel;
        bottomLevelX2 = visibleLevelX2 >> deltaLevel;
        bottomLevelY2 = visibleLevelY2 >> deltaLevel;
    }
    else if ( d->m_originatingTileLevel < d->m_tileLevelRangeWidget->bottomLevel() ) {
        int const deltaLevel = d->m_tileLevelRangeWidget->bottomLevel() - d->m_originatingTileLevel;
        bottomLevelX1 = visibleLevelX1 << deltaLevel;
        bottomLevelY1 = visibleLevelY1 << deltaLevel;
        bottomLevelX2 = visibleLevelX2 << deltaLevel;
        bottomLevelY2 = visibleLevelY2 << deltaLevel;
    }
    else {
        bottomLevelX1 = visibleLevelX1;
        bottomLevelY1 = visibleLevelY1;
        bottomLevelX2 = visibleLevelX2;
        bottomLevelY2 = visibleLevelY2;
    }
    mDebug() << "bottom level pixel coords (level/x1/y1/x2/y2):"
             << d->m_tileLevelRangeWidget->bottomLevel()
             << bottomLevelX1 << bottomLevelY1 << bottomLevelX2 << bottomLevelY2;

    TileCoordsPyramid coordsPyramid( d->m_tileLevelRangeWidget->topLevel(),
                                     d->m_tileLevelRangeWidget->bottomLevel() );
    QRect bottomLevelTileCoords;
    bottomLevelTileCoords.setCoords
        ( bottomLevelX1 / tileWidth,
          bottomLevelY1 / tileHeight,
          bottomLevelX2 / tileWidth + ( bottomLevelX2 % tileWidth > 0 ? 1 : 0 ),
          bottomLevelY2 / tileHeight + ( bottomLevelY2 % tileHeight > 0 ? 1 : 0 ));
    mDebug() << "bottom level tile coords: (x1/y1/size):" << bottomLevelTileCoords;
    coordsPyramid.setBottomLevelCoords( bottomLevelTileCoords );
    mDebug() << "tiles count:" << coordsPyramid.tilesCount();
    return coordsPyramid;
}

void DownloadRegionDialog::setMapTheme( QString const & mapThemeId )
{
    mDebug() << "DownloadRegionDialog::setMapTheme" << mapThemeId;
}

void DownloadRegionDialog::setVisibleLatLonAltBox( GeoDataLatLonAltBox const & region )
{
    d->m_visibleRegion = region;
    updateTilesCount();
}

void DownloadRegionDialog::toggleSelectionMethod()
{
    d->m_latLonBoxWidget->setEnabled( !d->m_latLonBoxWidget->isEnabled() );
}

void DownloadRegionDialog::updateTilesCount()
{
    TileCoordsPyramid const pyramid = region();
    qint64 const tilesCount = pyramid.tilesCount();
    if ( tilesCount > maxTilesCount ) {
        d->m_tilesCountLimitInfo->setText( tr( "There is a limit of %n tiles to download.", "",
                                               maxTilesCount ));
    } else {
        d->m_tilesCountLimitInfo->clear();
    }
    d->m_tilesCountLabel->setText( QString::number( tilesCount ));
    bool const tilesCountWithinLimits = tilesCount > 0 && tilesCount <= maxTilesCount;
    d->m_okButton->setEnabled( tilesCountWithinLimits );
    d->m_applyButton->setEnabled( tilesCountWithinLimits );
}

}

#include "DownloadRegionDialog.moc"
