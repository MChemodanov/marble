//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>"
// Copyright 2007-2008 Inge Wallin  <ingwa@kde.org>"
//

//
// PlaceMarkPainter is responsible for drawing the PlaceMarks on the map
//


#ifndef PLACEMARKPAINTER_H
#define PLACEMARKPAINTER_H


#include <QtGui/QFont>
#include <QtGui/QItemSelection>
#include <QtGui/QPainterPath>
#include <QtGui/QPixmap>
#include <QtCore/QRect>
#include <QtCore/QVector>

#include "GeoDataPlacemark.h"
#include "PlaceMarkLayout.h"


class ViewportParams;
class VisiblePlaceMark;

static const double s_labelOutlineWidth = 2.5;


class PlaceMarkPainter : public QObject
{
    Q_OBJECT

 public:
    PlaceMarkPainter(QObject *parent = 0);
    ~PlaceMarkPainter();

    void drawPlaceMarks( QPainter* painter, 
                         QVector<VisiblePlaceMark*> visiblePlaceMarks,
                         const QItemSelection &selection, 
                         ViewportParams *viewport );

    // FIXME: To be removed after MapTheme / KML refactoring
    void setDefaultLabelColor( const QColor& color );

 private:

    void drawLabelText( QPainter& textpainter, const QString &name, const QFont &labelFont );
    void drawLabelPixmap( VisiblePlaceMark *mark, bool isSelected );

    bool testXBug();

 private:
    bool m_useXWorkaround;  // Indicates need for an X windows workaround.

    // FIXME: To be removed after MapTheme / KML refactoring
    QColor m_defaultLabelColor;
};

#endif // PLACEMARKPAINTER_H
