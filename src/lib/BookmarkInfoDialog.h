//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Gaurav Gupta <1989.gaurav@googlemail.com>     
//

#ifndef MARBLE_INFODIALOG_H
#define MARBLE_INFODIALOG_H

#include "ui_BookmarkInfoDialog.h"
#include "MarbleWidget.h"
#include "marble_export.h"
#include "MarbleRunnerManager.h"

namespace Marble
{

class MARBLE_EXPORT BookmarkInfoDialog : public QDialog, private Ui::BookmarkInfoDialog
{

    Q_OBJECT

 public:

    explicit BookmarkInfoDialog( MarbleWidget *parent = 0);

    ~BookmarkInfoDialog();

    void initComboBox();


 public Q_SLOTS:

    void addBookmark();
    
    void openNewFolderDialog();

    void retrieveGeocodeResult( const GeoDataCoordinates &coordinates, const GeoDataPlacemark &placemark);

    QString append( const QString &bookmark, const QString &text);

 private:
    Q_DISABLE_COPY( BookmarkInfoDialog )
    MarbleWidget *m_widget;
    MarbleRunnerManager* m_manager;
};

}
#endif

