//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2004-2007 Torsten Rahn <tackat@kde.org>"
// Copyright 2007      Inge Wallin  <ingwa@kde.org>"
//

//
// The about dialog 
//


#ifndef MARBLEABOUTDIALOG_H
#define MARBLEABOUTDIALOG_H


#include <QtGui/QDialog>

#include "marble_export.h"

namespace Marble
{

class MarbleAboutDialogPrivate;


class MARBLE_EXPORT MarbleAboutDialog : public QDialog
{
    Q_OBJECT

 public:
    explicit MarbleAboutDialog(QWidget *parent = 0);
    ~MarbleAboutDialog();

 private Q_SLOTS:
    void loadPageContents( int idx );

 private:
    Q_DISABLE_COPY( MarbleAboutDialog )
    MarbleAboutDialogPrivate  * const d;
};

}

#endif // KATLASABOUTDIALOG_H
