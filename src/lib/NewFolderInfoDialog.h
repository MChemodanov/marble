//
// This file is part of the Marble Virtual Globe.
//
// the source code.
//
// Copyright 2010      Gaurav Gupta <1989.gaurav@googlemail.com>     
//

#ifndef MARBLE_NEWFOLDERINFODIALOG_H
#define MARBLE_NEWFOLDERINFODIALOG_H

#include "ui_NameDialog.h"
#include "MarbleWidget.h"
#include "marble_export.h"
namespace Marble
{

/** @todo FIXME after freeze: Rename to AddBookmarkFolderDialog*/

class MARBLE_EXPORT NewFolderInfoDialog : public QDialog, private Ui::NameDialog
{

    Q_OBJECT

 public:

    explicit NewFolderInfoDialog( MarbleWidget *parent = 0);

    ~NewFolderInfoDialog();
 public Q_SLOTS:

   void addNewBookmarkFolder();


 private:
    Q_DISABLE_COPY( NewFolderInfoDialog )
    MarbleWidget *m_widget;
};

}
#endif

