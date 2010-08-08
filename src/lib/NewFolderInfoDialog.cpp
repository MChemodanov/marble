//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010 Gaurav Gupta <1989.gaurav@googlemail.com>
//
//

#include "NewFolderInfoDialog.h"
#include "MarbleDebug.h"
#include <QLineEdit>
using namespace Marble;

NewFolderInfoDialog::NewFolderInfoDialog(MarbleWidget *parent)
    : QDialog(parent), m_widget(parent)
{
    setupUi(this);
    setWindowTitle( tr("New Folder Name") );
    connect( m_save, SIGNAL( clicked() ), this, SLOT( addNewBookmarkFolder() ) );
}

void NewFolderInfoDialog::addNewBookmarkFolder()
{
    mDebug() << " Adding New Bookmark Folder "<< m_name->text() ;
    m_widget->addNewBookmarkFolder( m_name->text() );
}


#include "NewFolderInfoDialog.moc"
