//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2004-2007 Torsten Rahn  <tackat@kde.org>
// Copyright 2007-2008 Inge Wallin   <ingwa@kde.org>
// Copyright 2007      Andrew Manson <g.real.ate@gmail.com>
//


#ifndef MARBLE_ABSTRACTLAYER_H
#define MARBLE_ABSTRACTLAYER_H


// WARNING: This class is obsolete and will be removed, so do not use it


#include <QtCore/QObject>

class QPoint;
class QPointF;
class QSize;

namespace Marble
{

class AbstractLayerContainer;
class ClipPainter;
class ViewParams;
class GeoDataCoordinates;


/**
 * @brief framework class for each display layer in Marble
 *
 * This class is intended to be used as a base class for all layers in
 * Marble. It contains convenience classes and can be used as a 
 * framework for implementing new layers. So far there is default
 * implementations for all of the virtual methods so that any
 * developer may implement a whole new layer with the least amount of
 * effort.
 **/
class AbstractLayer: public QObject
{
    //FIXME: make AbstractLayer inherit from AbstractLayerInterface
    //       and change the function paintLayer() to draw()
    Q_OBJECT
 public Q_SLOTS:
    /**
     * @brief method to paint the whole Layer
     * 
     * This method is intended to keep the painting of an entire
     * layer a simple function call.
     * @param painter pointer to the painter that will paint this
     *                layer
     * @param screenSize used to calculate the pixel position
     * @param radius globe radius, used as a measure of zoom level
     * @param rotAxis quaternion that represents the current rotation
     *                of the globe
     */
    virtual void paintLayer( ClipPainter* painter, 
			     const QSize& screenSize,
			     ViewParams *viewParams );

    /**
     * @brief  Set whether the Layer is visible
     * @param  visible  visibility of the Layer
     */
    void setVisible( bool visible );

    virtual void clearModel() = 0;

 public:
    /**
     * @brief Construct this layer with a parent
     */
    explicit AbstractLayer( QObject * parent=0 );

    /**
     * @brief destructor
     */
    ~AbstractLayer();

    /** 
     * @brief get screen pixel position from a geographical position
     * 
     * Method to simplify the retrieval of the screen pixel position 
     * from a longitude and latitude. This static method is not 
     * intended for use in a layer that implements AbstractLayerData 
     * objects because AbstractLayerData has a member function for 
     * this purpose
     * @param _lon the longitude of the point we want to find
     * @param _lat the latitude of the point we want to find 
     * @param invRotAxis inversion of the Quaternion status of the
     *                   globe
     * @param screenSize size of the screen
     * @param position QPoint that the position on screen will be 
     *                 saved into
     * @param radius the radius of the globe, a measure of zoom level
     * @return @c true if the pixel is visible on the screen
     *         @c false if the pixel is outside the screen
     *
     * 
     **/
    static bool getPixelPosFromGeoDataCoordinates(qreal _lon, qreal _lat, 
                                 const QSize &screenSize, 
                                 ViewParams *viewParams,
                                 QPoint *position);

     /**
      * @brief get screen pixel position. 
      * 
      * Method to simplify the retrieval of the screen pixel position
      * from a GeoDataCoordinates object.
      * @param geoPosition the position of the point we want to find
      * @param invRotAxis inversion of the Quaternion status of the
      *                   globe
      * @param screenSize size of the screen
      * @param position QPoint where the position on screen will be 
      *                 saved into
      * @param radius the radius of the globe, a measure of zoom level
      * @return @c true if the pixel is visible on the screen
      *         @c false if the pixel is outside the screen
      **/ 
    bool getPixelPosFromGeoDataCoordinates(const GeoDataCoordinates & geoPosition,
                                 const QSize &screenSize,
                                 ViewParams *viewParams,
                                 QPoint *position);

    /**
     * @brief  Return whether the Layer is visible.
     * @return The Layer visibility.
     */
    bool  visible() const;

    /**
     * @brief convenience method to find the distance between 2 points
     * 
     * This is not intended to be used as a distance between 2 
     * geoPoints, the intended use is only for measuring points on the
     * view for drawing purposes.
     */
    static qreal distance ( const QPoint &, const QPoint & );

    /**
     * @brief overloaded method to allow for double precision distance
     */
    static qreal distance ( const QPointF &, const QPointF & );

 private:
    /**
     * @brief visibility of this layer
     * 
     * Any layer should only be drawn if this variable is @c true, it
     * defaults as @c false.
     */
    bool m_visible;
    
 protected:
    /**
     * @brief a collection of all containers that hold the data for
     *        this layer
     * 
     * To draw a layer you should be able to iterate through this
     * collection and call each container's @c draw() function
     */
    QVector<AbstractLayerContainer *> *m_containers;

    Q_DISABLE_COPY( AbstractLayer )
};

}

#endif
