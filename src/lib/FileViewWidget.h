//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Bastian Holst <bastianholst@gmx.de>
//

#ifndef MARBLE_FILEVIEWWIDGET_H
#define MARBLE_FILEVIEWWIDGET_H

// Marble
#include "marble_export.h"

// Qt
#include <QtGui/QWidget>

class QModelIndex;

namespace Marble
{

class FileViewWidgetPrivate;

class MarbleWidget;

class MARBLE_EXPORT FileViewWidget : public QWidget
{
    Q_OBJECT

 public:
    FileViewWidget( QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~FileViewWidget();

    /**
     * @brief Set a MarbleWidget associated to this widget.
     * @param widget  the MarbleWidget to be set.
     */
    void setMarbleWidget( MarbleWidget *widget );

 public Q_SLOTS:
    void enableFileViewActions();

 private Q_SLOTS:
    void mapCenterOnTreeViewModel( const QModelIndex & );

 private:
    Q_DISABLE_COPY( FileViewWidget )

    FileViewWidgetPrivate * const d;
};

}

#endif