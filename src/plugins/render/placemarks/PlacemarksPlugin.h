//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Patrick Spendrin      <ps_ml@gmx.de>
// Copyright 2008 Simon Schmeisser      <mail_to_wrt@gmx.de>
//

//
// This class is the placemark layer plugin.
//

#ifndef MARBLEPLACEMARKSPLUGIN_H
#define MARBLEPLACEMARKSPLUGIN_H

#include <QtCore/QObject>
#include <QtGui/QBrush>
#include <QtGui/QPen>

#include "RenderPlugin.h"


namespace Marble
{

class GeoDataGeometry;
class GeoDataFeature;
class GeoDataDocument;

/**
 * @short The class that specifies the Marble layer interface of the plugin.
 *
 * PlacemarksPlugin is the beginning of a plugin, that displays placemarks
 */

class PlacemarksPlugin : public RenderPlugin
{
    Q_OBJECT
    Q_INTERFACES( Marble::RenderPluginInterface )
    MARBLE_PLUGIN( PlacemarksPlugin )

    void setBrushStyle( GeoPainter *painter, GeoDataDocument* root, QString styleId );
    void setPenStyle( GeoPainter *painter, GeoDataDocument* root, QString styleId );
    bool renderGeoDataGeometry( GeoPainter *painter, GeoDataGeometry *geometry, QString styleUrl );
    bool renderGeoDataFeature( GeoPainter *painter, GeoDataFeature *feature );

    QBrush m_currentBrush;
    QPen m_currentPen;
    
 public:
    QStringList backendTypes() const;

    QString renderPolicy() const;

    QStringList renderPosition() const;

    QString name() const;

    QString guiString() const;

    QString nameId() const;

    QString description() const;

    QIcon icon () const;

    void initialize ();
    bool isInitialized () const;

    bool render( GeoPainter *painter, ViewportParams *viewport, const QString& renderPos, GeoSceneLayer * layer = 0 );

 private:
    bool m_isInitialized;
};

}

#endif
