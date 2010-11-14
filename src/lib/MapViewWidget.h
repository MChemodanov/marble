//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2004-2007 Torsten Rahn  <tackat@kde.org>
// Copyright 2007      Inge Wallin   <ingwa@kde.org>
// Copyright 2007      Thomas Zander <zander@kde.org>
// Copyright 2010      Bastian Holst <bastianholst@gmx.de>
//

#ifndef MARBLE_MAPVIEWWIDGET_H
#define MARBLE_MAPVIEWWIDGET_H

// Marble
#include "global.h"
#include "marble_export.h"

// Qt
#include <QtGui/QWidget>

class QStandardItemModel;

namespace Marble
{

class MapViewWidgetPrivate;

class MarbleWidget;

class MARBLE_EXPORT MapViewWidget : public QWidget
{
    Q_OBJECT

 public:
    MapViewWidget( QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~MapViewWidget();

    /**
     * @brief Set a MarbleWidget associated to this widget.
     * @param widget  the MarbleWidget to be set.
     */
    void setMarbleWidget( MarbleWidget *widget );

    void updateCelestialModel();

 public Q_SLOTS:
    void selectTheme( const QString & );

    void selectProjection( Projection projection );

    void selectCurrentMapTheme( const QString& );

    /// whenever a new map gets inserted, the following slot will adapt the ListView accordingly
    void updateMapThemeView();

    void projectionSelected( int projectionIndex );

 Q_SIGNALS:
    void selectMapTheme( const QString& );
    void projectionSelected( Projection );

 private:
    Q_DISABLE_COPY( MapViewWidget )

    MapViewWidgetPrivate * const d;
};

}

#endif
