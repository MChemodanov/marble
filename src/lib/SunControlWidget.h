//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      David Roberts  <dvdr18@gmail.com>
// Copyright 2008      Inge Wallin    <inge@lysator.liu.se>
// Copyright 2010      Harshit Jain   <hjain.itbhu@gmail.com>
//

#ifndef MARBLE_SUNCONTROLWIDGET_H
#define MARBLE_SUNCONTROLWIDGET_H

#include <QtCore/QDateTime>
#include <QtCore/QTime>
#include <QtGui/QDialog>

#include "marble_export.h"

namespace Ui
{
    class SunControlWidget;
}

namespace Marble
{
class SunLocator;

class MARBLE_EXPORT SunControlWidget : public QDialog
{
    Q_OBJECT
	
 public:
    SunControlWidget( SunLocator* sunLocator, QWidget* parent = 0 );
    virtual ~SunControlWidget();
    void setSunShading( bool );

 private Q_SLOTS:
    void apply();
	
 Q_SIGNALS:
    void showSun( bool show );
    void showSunInZenith( bool show );

 protected:
    Q_DISABLE_COPY( SunControlWidget )

    void showEvent(QShowEvent* event);

    Ui::SunControlWidget *m_uiWidget;
    SunLocator           *m_sunLocator;
    QString               m_shadow;
};

}

#endif
