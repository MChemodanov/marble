//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2008 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
//


#ifndef MARBLEWIDGET_H
#define MARBLEWIDGET_H


/** @file
 * This file contains the headers for MarbleWidget.
 *
 * @author Torsten Rahn <tackat@kde.org>
 * @author Inge Wallin  <inge@lysator.liu.se>
 */

#include <QtGui/QActionGroup>
#include <QtCore/QTimer>
#include <QtGui/QWidget>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtDesigner/QDesignerExportWidget>

#include "marble_export.h"
#include "GeoDataCoordinates.h"       // In geodata/data/
#include "MarbleWidgetInputHandler.h"
#include "Quaternion.h"
#include "global.h"             // types needed in all of marble.

// Qt
class QAbstractItemModel;
class QModelIndex;
class QItemSelectionModel;
class QSettings;

namespace Marble
{

// MarbleWidget 
// class MarbleWidgetInputHandler;
class MarbleWidgetPrivate;

// Marble
class MarbleMap;
class MarbleModel;
class TileCreator;
class SunLocator;
class GpxFileModel;
class FileViewModel;
class GeoDataLatLonBox;
class GeoPainter;
class RenderPlugin;
class AbstractFloatItem;
class GeoSceneDocument;

/**
 * @short A widget class that displays a view of the earth.
 *
 * This widget displays a view of the earth or any other globe,
 * depending on which dataset is used.  The user can navigate the
 * globe using either a control widget, e.g. the MarbleControlBox, or
 * the mouse.  The mouse and keyboard control is done through a
 * MarbleWidgetInputHandler. Only some aspects of the widget can be
 * controlled by the mouse and/or keyboard.
 *
 * By clicking on the globe and moving the mouse, the position can be
 * changed.  The user can also zoom by using the scroll wheel of the
 * mouse in the widget. The zoom value is not tied to any units, but
 * is an abstract value without any physical meaning. A value around
 * 1000 shows the full globe in a normal-sized window. Higher zoom
 * values give a more zoomed-in view.
 *
 * The MarbleWidget needs to be provided with a data model to
 * work. This model is contained in the MarbleModel class, and it is
 * painted by using a MarbleMap. The widget can also construct its own
 * map and model if none is given to the constructor.  A MarbleModel
 * contains 3 separate datatypes: <b>tiles</b> which provide the
 * background, <b>vectors</b> which provide things like country
 * borders and coastlines and <b>placemarks</b> which can show points
 * of interest, such as cities, mountain tops or the poles.
 *
 * In addition to navigating with the mouse, you can also use it to
 * get information about items on the map. You can either click on a
 * placemark with the left mouse button or with the right mouse button
 * anywhere on the map.
 *
 * The left mouse button opens up a menu with all the placemarks
 * within a certain distance from the mouse pointer. When you choose
 * one item from the menu, Marble will open up a dialog window with
 * some information about the placemark and also try to connect to
 * Wikipedia to retrieve an article about it. If there is such an
 * article, you will get a mini-browser window with the article in a tab.
 *
 * The right mouse button controls a distance tool.  The distance tool
 * is implemented as a menu where you can choose to either create or
 * remove so called Measure Points. Marble will keep track of the
 * Measure Points and show the total distance in the upper left of the
 * widget.  Measure Points are shown on the map as a little white
 * cross.
 *
 * @see MarbleControlBox
 * @see MarbleMap
 * @see MarbleModel
 */

class MARBLE_EXPORT MarbleWidget : public QWidget
{
    Q_OBJECT
#ifdef MARBLE_DBUS
    Q_CLASSINFO("D-Bus Interface", "org.kde.MarbleWidget")
#endif

    Q_PROPERTY(int zoom          READ zoom            WRITE zoomView)

    Q_PROPERTY(QString mapThemeId  READ mapThemeId    WRITE setMapThemeId)
    Q_PROPERTY(int projection    READ projection      WRITE setProjection)

    Q_PROPERTY(qreal longitude  READ centerLongitude WRITE setCenterLongitude)
    Q_PROPERTY(qreal latitude   READ centerLatitude  WRITE setCenterLatitude)

    Q_PROPERTY(bool showOverviewMap READ showOverviewMap    WRITE setShowOverviewMap)
    Q_PROPERTY(bool showScaleBar READ showScaleBar    WRITE setShowScaleBar)
    Q_PROPERTY(bool showCompass  READ showCompass     WRITE setShowCompass)
    Q_PROPERTY(bool showGrid     READ showGrid        WRITE setShowGrid)

    Q_PROPERTY(bool showClouds   READ showClouds      WRITE setShowClouds)
    Q_PROPERTY(bool showAtmosphere READ showAtmosphere WRITE setShowAtmosphere)
    Q_PROPERTY(bool showCrosshairs READ showCrosshairs WRITE setShowCrosshairs)

    Q_PROPERTY(bool showPlaces   READ showPlaces      WRITE setShowPlaces)
    Q_PROPERTY(bool showCities   READ showCities      WRITE setShowCities)
    Q_PROPERTY(bool showTerrain  READ showTerrain     WRITE setShowTerrain)
    Q_PROPERTY(bool showOtherPlaces READ showOtherPlaces WRITE setShowOtherPlaces)

    Q_PROPERTY(bool showRelief   READ showRelief      WRITE setShowRelief)
    Q_PROPERTY(bool showElevationModel READ showElevationModel WRITE setShowElevationModel)

    Q_PROPERTY(bool showIceLayer READ showIceLayer    WRITE setShowIceLayer)
    Q_PROPERTY(bool showBorders  READ showBorders     WRITE setShowBorders)
    Q_PROPERTY(bool showRivers   READ showRivers      WRITE setShowRivers)
    Q_PROPERTY(bool showLakes    READ showLakes       WRITE setShowLakes)

    Q_PROPERTY(quint64 persistentTileCacheLimit  READ persistentTileCacheLimit  WRITE setPersistentTileCacheLimit)
    Q_PROPERTY(quint64 volatileTileCacheLimit    READ volatileTileCacheLimit    WRITE setVolatileTileCacheLimit)

 public:

    /**
     * @brief Construct a new MarbleWidget.
     * @param parent the parent widget
     *
     * This constructor should be used when you will only use one
     * MarbleWidget.  The widget will create its own MarbleModel when
     * created.
     */
    explicit MarbleWidget( QWidget *parent = 0);

    /**
     * @brief Construct a new MarbleWidget.
     * @param model  the data model for the widget.
     * @param parent the parent widget
     *
     * This constructor should be used when you plan to use more than
     * one MarbleWidget for the same MarbleModel (not yet supported,
     * but will be soon).
     */
    explicit MarbleWidget(MarbleMap *map, QWidget *parent = 0);

    virtual ~MarbleWidget();

    /**
     * @brief Return the map that this view shows.
     */
    MarbleMap  *map() const;

    /**
     * @brief Return the model that this view shows.
     */
    MarbleModel  *model() const;

    /**
     * Returns the current input handler
     */
    MarbleWidgetInputHandler *inputHandler() const;

    /**
     * @brief Set the input handler
     */
    void setInputHandler(MarbleWidgetInputHandler *handler);

    /**
     * @brief Return the active region in which it's possible to drag the view using the mouse.
     */
    const QRegion  activeRegion();

    /**
     * @brief Return the projected region which describes the (shape of the) projected surface.
     */
    const QRegion  mapRegion();

    /**
     * @brief  Return the radius of the globe in pixels.
     */
    int         radius() const;

    /**
     * @brief  Set the radius of the globe in pixels.
     * @param  radius  The new globe radius value in pixels.
     */
    void        setRadius(int radius);

    /**
     * @brief Return the current zoom amount.
     */
    int         zoom() const;

    /**
     * @brief Return the current distance.
     */
    qreal      distance() const;

    /**
     * @brief  Set the distance of the observer to the globe in km.
     * @param  distance  The new distance in km.
     */
    void        setDistance( qreal distance );

    /**
     * @brief Return the current distance string.
     */
    QString     distanceString() const;

    /**
     * @brief Return the minimum zoom value for the current map theme.
     */
    int         minimumZoom() const;

    /**
     * @brief Return the minimum zoom value for the current map theme.
     */
    int         maximumZoom() const;

    /**
     * @brief Return if the widget needs to be updated.
     */
    bool needsUpdate() const;
    /**
     * @brief Mark the widget as needing an update.
     */
    void setNeedsUpdate();

    /**
     * @brief Get the screen coordinates corresponding to geographical coordinates in the widget.
     * @param lon    the lon coordinate of the requested pixel position
     * @param lat    the lat coordinate of the requested pixel position
     * @param x      the x coordinate of the pixel is returned through this parameter
     * @param y      the y coordinate of the pixel is returned through this parameter
     * @return @c true  if the geographical coordinates are visible on the screen
     *         @c false if the geographical coordinates are not visible on the screen
     */
    bool screenCoordinates( qreal lon, qreal lat,
                            qreal& x, qreal& y );

    /**
     * @brief Get the Y coordinate of the North Pole.
     * @return the pixel address of the geographical north pole.
     */
    int northPoleY();

    /**
     * @brief Get the earth coordinates corresponding to a pixel in the widget.
     * @param x      the x coordinate of the pixel
     * @param y      the y coordinate of the pixel
     * @param lon    the longitude angle is returned through this parameter
     * @param lat    the latitude angle is returned through this parameter
     * @return @c true  if the pixel (x, y) is within the globe
     *         @c false if the pixel (x, y) is outside the globe, i.e. in space.
     */
    bool geoCoordinates( int x, int y,
                         qreal& lon, qreal& lat,
                         GeoDataCoordinates::Unit = GeoDataCoordinates::Degree );

    /**
     * @brief Return the longitude of the center point.
     */
    qreal  centerLongitude() const;

    /**
     * @brief Return the latitude of the center point.
     */
    qreal  centerLatitude()  const;

    /**
     * @brief Returns the model for all the placemarks on the globe.
     */
    QAbstractItemModel *placemarkModel() const;

    /**
     * @brief Returns the selection model for all the placemarks on the globe.
     */
    QItemSelectionModel *placemarkSelectionModel() const;

    /**
     * @brief  Return how much the map will move if one of the move slots are called.
     * @return The move step.
     */
    qreal  moveStep();

    /**
     * @brief  Add a GeoDataPlacemark file to the model.
     * @param  filename  the filename of the file containing the Placemarks.
     */
    void addPlacemarkFile( const QString &filename );

    /**
     * @brief  Add GeoDataPlacemark data as string to the model.
     * @param  data  the string containing the Placemarks.
     * @param key  the string needed to identify the data
     */
    void addPlacemarkData( const QString& data, const QString& key = "data" );
    
    /**
     * @brief  remove data or files from the model.
     * @param key  either the filename or the string used to identify the data in addPlacemarkFile and addPlacemarkData
     */
    void removePlacemarkKey( const QString& key );

    /**
     * @brief  Return the quaternion that specifies the rotation of the globe.
     * @return The quaternion that describes the rotation of the globe.
     */
    Quaternion  planetAxis() const;

    /**
     * @brief  Return a QPixmap with the current contents of the widget.
     */
    QPixmap mapScreenShot();

    /**
     * @brief  Return whether the overview map is visible.
     * @return The overview map visibility.
     */
    bool  showOverviewMap() const;

    /**
     * @brief  Return whether the scale bar is visible.
     * @return The scale bar visibility.
     */
    bool  showScaleBar() const;

    /**
     * @brief  Return whether the compass bar is visible.
     * @return The compass visibility.
     */
    bool  showCompass() const;

    /**
     * @brief  Return whether the cloud cover is visible.
     * @return The cloud cover visibility.
     */
    bool  showClouds() const;

    /**
     * @brief  Return whether the atmospheric glow is visible.
     * @return The cloud cover visibility.
     */
    bool  showAtmosphere() const;

    /**
     * @brief  Return whether the crosshairs are visible.
     * @return The crosshairs' visibility.
     */
    bool  showCrosshairs() const;

    /**
     * @brief  Return whether the coordinate grid is visible.
     * @return The coordinate grid visibility.
     */
    bool  showGrid() const;

    /**
     * @brief  Return whether the place marks are visible.
     * @return The place mark visibility.
     */
    bool  showPlaces() const;

    /**
     * @brief  Return whether the city place marks are visible.
     * @return The city place mark visibility.
     */
    bool  showCities() const;

    /**
     * @brief  Return whether the terrain place marks are visible.
     * @return The terrain place mark visibility.
     */
    bool  showTerrain() const;

    /**
     * @brief  Return whether other places are visible.
     * @return The visibility of other places.
     */
    bool  showOtherPlaces() const;

    /**
     * @brief  Return whether the relief is visible.
     * @return The relief visibility.
     */
    bool  showRelief() const;

    /**
     * @brief  Return whether the elevation model is visible.
     * @return The elevation model visibility.
     */
    bool  showElevationModel() const;

    /**
     * @brief  Return whether the ice layer is visible.
     * @return The ice layer visibility.
     */
    bool  showIceLayer() const;

    /**
     * @brief  Return whether the borders are visible.
     * @return The border visibility.
     */
    bool  showBorders() const;

    /**
     * @brief  Return whether the rivers are visible.
     * @return The rivers' visibility.
     */
    bool  showRivers() const;

    /**
     * @brief  Return whether the lakes are visible.
     * @return The lakes' visibility.
     */
    bool  showLakes() const;

    /**
     * @brief Return whether Gps Data is visible.
     * @return The Gps Data's visibility.
     */
    bool  showGps() const;

    /**
     * @brief  Return whether the frame rate gets displayed.
     * @return the frame rates visibility
     */
    bool  showFrameRate() const;

    /**
     * @brief  Returns the limit in kilobytes of the persistent (on hard disc) tile cache.
     * @return the limit of persistent tile cache
     */
    quint64  persistentTileCacheLimit() const;

    /**
     * @brief  Returns the limit in kilobytes of the volatile (in RAM) tile cache.
     * @return the limit of volatile tile cache
     */
    quint64  volatileTileCacheLimit() const;

    SunLocator* sunLocator();

    //These methods should all be removed
    // what does this do to binary compatibility?
    void setProxy( const QString& proxyHost, const quint16 proxyPort, const QString& user, const QString& password );
    QString proxyHost() const;
    quint16 proxyPort() const;
    QString user() const;
    QString password() const;

    /**
     * @brief Returns a list of all RenderPlugins on the widget, this includes float items
     * @return the list of RenderPlugins
     */
    QList<RenderPlugin *>      renderPlugins() const;
    /**
     * @brief Returns a list of all FloatItems on the widget
     * @return the list of the floatItems
     */
    QList<AbstractFloatItem *> floatItems()    const;

    /**
     * Reads the plugin settings from the passed QSettings.
     * You shouldn't use this in a KDE application as these use KConfig. Here you could
     * use MarblePart which is handling this automatically.
     * @param settings The QSettings object to be used.
     */
    void readPluginSettings( QSettings& settings );

    /**
     * Writes the plugin settings in the passed QSettings.
     * You shouldn't use this in a KDE application as these use KConfig. Here you could
     * use MarblePart which is handling this automatically.
     * @param settings The QSettings object to be used.
     */
    void writePluginSettings( QSettings& settings ) const;

    /**
     * @brief  Get the Projection used for the map
     * @return @c Spherical         a Globe
     * @return @c Equirectangular   a flat map
     * @return @c Mercator          another flat map
     */
    Projection  projection() const;
//    int         projection() const;

    /**
     * @brief Get the ID of the current map theme
     * To ensure that a unique identifier is being used the theme does NOT 
     * get represented by its name but the by relative location of the file 
     * that specifies the theme:
     *
     * Example: 
     *    mapThemeId = "earth/bluemarble/bluemarble.dgml"
     */
    QString mapThemeId() const;

    /**
     * @brief Get the GeoSceneDocument object of the current map theme
     */
    Marble::GeoSceneDocument * mapTheme() const;

    /**
     * @brief Return a QAbstractItemModel containing GPX files.
     */
    GpxFileModel * gpxFileModel();

    /**
     * @brief Return a QAbstractItemModel containing files.
     */
    FileViewModel * fileViewModel() const;

    /**
     * @brief Retrieve the map quality depending on the view context 
     */
    Marble::MapQuality mapQuality( Marble::ViewContext = Marble::Still );

    /**
     * @brief Retrieve the view context (i.e. still or animated map) 
     */
    Marble::ViewContext viewContext( ) const;

    /**
     * @brief Retrieve whether travels to a point should get animated 
     */
    bool animationsEnabled() const;

    Marble::AngleUnit defaultAngleUnit() const;
    void setDefaultAngleUnit( Marble::AngleUnit angleUnit );

    QFont defaultFont() const;
    void setDefaultFont( const QFont& font );

 public Q_SLOTS:

    void updateSun();
    void centerSun();
    void setInputEnabled( bool );
//    void repaintMap();
    
    /**
     * @brief  Zoom the view to a certain zoomlevel
     * @param  zoom  the new zoom level.
     *
     * The zoom level is an abstract value without physical
     * interpretation.  A zoom value around 1000 lets the viewer see
     * all of the earth in the default window.
     */
    void  zoomView(int zoom);

    /**
     * @brief  Zoom the view by a certain step
     * @param  zoomStep  the difference between the old zoom and the new
     */
    void  zoomViewBy(int zoomStep);

    /**
     * @brief  Zoom in by the amount zoomStep.
     */
    void  zoomIn();
    /**
     * @brief  Zoom out by the amount zoomStep.
     */
    void  zoomOut();

    /**
     * @brief  Rotate the view by the two angles phi and theta.
     * @param  deltaLon  an angle that specifies the change in terms of longitude
     * @param  deltaLat  an angle that specifies the change in terms of latitude
     *
     * This function rotates the view by two angles,
     * deltaLon ("theta") and deltaLat ("phi").
     * If we start at (0, 0), the result will be the exact equivalent
     * of (lon, lat), otherwise the resulting angle will be the sum of
     * the previous position and the two offsets.
     */
    void  rotateBy( const qreal &deltaLon, const qreal &deltaLat );

    /**
     * @brief  Rotate the view by the angle specified by a Quaternion.
     * @param  incRot a quaternion specifying the rotation
     */
    void  rotateBy(const Quaternion& incRot);

    /**
     * @brief  Center the view on a geographical point
     * @param  lat  an angle in degrees parallel to the latitude lines
     *              +90(N) - -90(S)
     * @param  lon  an angle in degrees parallel to the longitude lines
     *              +180(W) - -180(E)
     */
    void  centerOn( const qreal &lon, const qreal &lat, bool animated = false );

    /**
     * @brief  Center the view on a point
     * @param  index  an index for a QModel, indicating a city
     */
    void  centerOn( const QModelIndex& index, bool animated = false );

    /**
     * @brief  Center the view on a point
     * This method centers the Marble map on the point described by the latitude
     * and longitude in the GeoDataCoordinate parameter @c point. It also zooms
     * the map to be at the elevation described by the altitude. If this is
     * not the desired functionality or you do not have an accurate altitude
     * then use @see centerOn(qreal, qreal, bool)
     * @param point the point in 3 dimensions above the globe to move the view
     *              to. It will always be looking vertically down.
     */
    void  centerOn( const GeoDataCoordinates &point, bool animated = false );

    /**
     * @brief Center the view on a bounding box so that it completely fills the viewport
     * This method not only centers on the center of the GeoDataLatLon box but it also
     * adjusts the zoom of the marble widget so that the LatLon box provided fills
     * the viewport.
     * @param box The GeoDataLatLonBox to zoom and move the MarbleWidget to.
     */
    void centerOn( const GeoDataLatLonBox& box, bool animated = false );

    /**
     * @brief  Set the latitude for the center point
     * @param  lat  the new value for the latitude
     */
    void setCenterLatitude( qreal lat );

    /**
     * @brief  Set the longitude for the center point
     * @param  lon  the new value for the longitude
     */
    void setCenterLongitude( qreal lon );

    /**
     * @brief  Set the Projection used for the map
     * @param  projection projection type (e.g. Spherical, Equirectangular, Mercator)
     */
    void  setProjection( int        projection );
    void  setProjection( Projection projection );

    /**
     * @brief  Get the home point
     * @param  lon  the longitude of the home point.
     * @param  lat  the latitude of the home point.
     * @param  zoom the default zoom level of the home point.
     */
    void  home(qreal &lon, qreal &lat, int& zoom);
    /**
     * @brief  Set the home point
     * @param  lon  the longitude of the new home point.
     * @param  lat  the latitude of the new home point.
     * @param  zoom the default zoom level for the new home point.
     */
    void  setHome(qreal lon, qreal lat, int zoom = 1050);
    /**
     * @brief  Set the home point
     * @param  homePoint  the new home point.
     * @param  zoom       the default zoom level for the new home point.
     */
    void  setHome(const GeoDataCoordinates& homePoint, int zoom = 1050);

    /**
     * @brief  Move left by the moveStep.
     */
    void  moveLeft();
    /**
     * @brief  Move right by the moveStep.
     */
    void  moveRight();
    /**
     * @brief  Move up by the moveStep.
     */
    void  moveUp();
    /**
     * @brief  Move down by the moveStep.
     */
    void  moveDown();

    /**
     * @brief Center the view on the default start point with the default zoom.
     */
    void  goHome();

    /**
     * @brief Set a new map theme
     * @param maptheme  The ID of the new maptheme. To ensure that a unique 
     * identifier is being used the theme does NOT get represented by its 
     * name but the by relative location of the file that specifies the theme:
     *
     * Example: 
     *    maptheme = "earth/bluemarble/bluemarble.dgml" 
     */
    void  setMapThemeId( const QString& maptheme );

    /**
     * @brief  Sets the value of a map theme property
     * @param  value  value of the property (usually: visibility)
     * 
     * Later on we might add a "setPropertyType and a QVariant
     * if needed.
     */
    void setPropertyValue( const QString& name, bool value );

    /**
     * @brief  Set whether the overview map overlay is visible
     * @param  visible  visibility of the overview map
     */
    void setShowOverviewMap( bool visible );

    /**
     * @brief  Set whether the scale bar overlay is visible
     * @param  visible  visibility of the scale bar
     */
    void setShowScaleBar( bool visible );

    /**
     * @brief  Set whether the compass overlay is visible
     * @param  visible  visibility of the compass
     */
    void setShowCompass( bool visible );

    /**
     * @brief  Set whether the cloud cover is visible
     * @param  visible  visibility of the cloud cover
     */
    void setShowClouds( bool visible );

    /**
     * @brief  Set whether the atmospheric glow is visible
     * @param  visible  visibility of the atmospheric glow
     */
    void setShowAtmosphere( bool visible );

    /**
     * @brief  Set whether the crosshairs are visible
     * @param  visible  visibility of the crosshairs
     */
    void setShowCrosshairs( bool visible );

    /**
     * @brief  Set whether the coordinate grid overlay is visible
     * @param  visible  visibility of the coordinate grid
     */
    void setShowGrid( bool visible );

    /**
     * @brief  Set whether the place mark overlay is visible
     * @param  visible  visibility of the place marks
     */
    void setShowPlaces( bool visible );

    /**
     * @brief  Set whether the city place mark overlay is visible
     * @param  visible  visibility of the city place marks
     */
    void setShowCities( bool visible );

    /**
     * @brief  Set whether the terrain place mark overlay is visible
     * @param  visible  visibility of the terrain place marks
     */
    void setShowTerrain( bool visible );

    /**
     * @brief  Set whether the other places overlay is visible
     * @param  visible  visibility of other places
     */
    void setShowOtherPlaces( bool visible );

    /**
     * @brief  Set whether the relief is visible
     * @param  visible  visibility of the relief
     */
    void setShowRelief( bool visible );

    /**
     * @brief  Set whether the elevation model is visible
     * @param  visible  visibility of the elevation model
     */
    void setShowElevationModel( bool visible );

    /**
     * @brief  Set whether the ice layer is visible
     * @param  visible  visibility of the ice layer
     */
    void setShowIceLayer( bool visible );

    /**
     * @brief  Set whether the borders visible
     * @param  visible  visibility of the borders
     */
    void setShowBorders( bool visible );

    /**
     * @brief  Set whether the rivers are visible
     * @param  visible  visibility of the rivers
     */
    void setShowRivers( bool visible );

    /**
     * @brief  Set whether the lakes are visible
     * @param  visible  visibility of the lakes
     */
    void setShowLakes( bool visible );

    /**
     * @brief Set whether Gps Data is visible
     * @param visible  visibility of the Gps Data
     */
    void setShowGps( bool visible );

    /**
     * @brief Set whether the frame rate gets shown
     * @param visible  visibility of the frame rate
     */
    void setShowFrameRate( bool visible );

    /**
     * @brief Set whether the is tile is visible
     * NOTE: This is part of the transitional debug API
     *       and might be subject to changes until Marble 0.8
     * @param visible visibility of the tile
     */
    void setShowTileId( bool visible);

    /**
     * @brief Set the current Gps position
     * @param lat the new latitude value
     * @param lon the new longitude value
     */
    
    void changeCurrentPosition( qreal lon, qreal lat );

     /**
     * @brief Used to notify about the position of the mouse click
      */
    void notifyMouseClick( int x, int y );

    /**
     * @brief Updates the gps tracking point by polling
     */
    void updateGps();

    /**
     * @brief Opens a gpx file for viewing on the Marble Widget
     */
    void openGpxFile( QString &filename );

    void clearPersistentTileCache();
    /**
     * @brief  Set the limit of the persistent (on hard disc) tile cache.
     * @param  kilobytes The limit in kilobytes, 0 means no limit.
     */
    void setPersistentTileCacheLimit( quint64 kiloBytes );

    void clearVolatileTileCache();
    /**
     * @brief  Set the limit of the volatile (in RAM) tile cache.
     * @param  kilobytes The limit in kilobytes.
     */
    void setVolatileTileCacheLimit( quint64 kiloBytes );

    /**
     * @brief A slot that is called when the model starts to create new tiles.
     * @param creator the tile creator object.
     * @param name  the name of the created theme.
     * @param description  a descriptive text that can be shown in a dialog.
     * @see    creatingTilesProgress
     *
     * This function is connected to the models signal with the same
     * name.  When the model needs to create a cache of tiles in
     * several different resolutions, it will emit creatingTilesStart
     * once with a name of the theme and a descriptive text.  The
     * widget can then pop up a dialog to explain why there is a
     * delay.  The model will then call creatingTilesProgress several
     * times until the parameter reaches 100 (100%), after which the
     * creation process is finished.  After this there will be no more
     * calls to creatingTilesProgress, and the poup dialog can then be
     * closed.
     */
    void creatingTilesStart( TileCreator *creator, const QString& name, const QString& description );

    /**
     * @brief Update the widget because the model changed.
     */
    void updateChangedMap();

    /**
     * Schedule repaint
     */
    void scheduleRepaint( QRegion dirtyRegion );

    /**
     * @brief Set the map quality depending on the view context 
     */
    void setMapQuality( Marble::MapQuality, Marble::ViewContext = Marble::Still );

    /**
     * @brief Set the view context (i.e. still or animated map) 
     */
    void setViewContext( Marble::ViewContext viewContext );

    /**
     * @brief Set whether travels to a point should get animated 
     */
    void setAnimationsEnabled( bool enabled );

    void setSelection(const QRect& region);

 private Q_SLOTS:

    void updateAnimation( qreal currentValue );

 Q_SIGNALS:
    /**
     * @brief Signal that the zoom has changed, and to what.
     * @param zoom  The new zoom value.
     * @see  zoomView()
     */
    void  zoomChanged( int zoom );
    void  distanceChanged( const QString& distanceString );

    /**
     * @brief Signal that the theme has changed
     * @param theme  Name of the new theme.
     */
    void  themeChanged( QString theme );

    void  projectionChanged( Projection );

    void  mouseMoveGeoPosition( QString );

    void  mouseClickGeoPosition( qreal lon, qreal lat, GeoDataCoordinates::Unit );

    void  timeout();

    void  framesPerSecond( qreal fps );

    /** This signal is emit when a new rectangle region is selected over the map 
     *  The list of double values include coordinates in degrees using this order:
     *  lon1, lat1, lon2, lat2 (or West, North, East, South) as left/top, right/bottom rectangle.
     */
    void  regionSelected(const QList<double>&);

    /**
     * This signal is emit when the settings of a plugin changed.
     */
    void pluginSettingsChanged();
    
    /**
     * @brief Signal that a render item has been initialized
     */
    void renderPluginInitialized( RenderPlugin *renderPlugin );

 protected:
    /**
     * @brief Reimplementation of the leaveEvent() function in QWidget.
     */
    void  leaveEvent( QEvent *event );

    /**
     * @brief Reimplementation of the paintEvent() function in QWidget.
     */
    void  paintEvent( QPaintEvent *event );

    /**
     * @brief Enables custom drawing onto the MarbleWidget straight after
     * @brief the globe and before all other layers has been rendered.
     * @param painter 
     */
    virtual void  customPaint( GeoPainter *painter );

    /**
     * @brief Reimplementation of the resizeEvent() function in QWidget.
     */
    void  resizeEvent( QResizeEvent* );

    void  connectNotify ( const char * signal );
    void  disconnectNotify ( const char * signal );

 private:
    Q_DISABLE_COPY( MarbleWidget )
    MarbleWidgetPrivate  * const d;
};

}

#endif // MARBLEWIDGET_H
