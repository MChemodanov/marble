//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

// Self
#include "WikipediaPlugin.h"

// Marble
#include "WikipediaModel.h"
#include "PluginAboutDialog.h"
#include "MarbleDirs.h"

// Qt
#include <QtCore/QDebug>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>

using namespace Marble;

const quint32 maximumNumberOfItems = 99;

WikipediaPlugin::WikipediaPlugin()
    : m_isInitialized( false ),
      m_icon(),
      m_aboutDialog( 0 ),
      m_configDialog( 0 ),
      m_settings()
{
    setNameId( "wikipedia" );
    
    // Plugin is enabled by default
    setEnabled( true );
    // Plugin is not visible by default
    setVisible( false );
    
    configDialog();
    readSettings();
}

WikipediaPlugin::~WikipediaPlugin()
{
    delete m_aboutDialog;
    delete m_configDialog;
}
     
void WikipediaPlugin::initialize()
{
    WikipediaModel *model = new WikipediaModel( this );
    // Ensure that all settings get forwarded to the model.
    setModel( model );
    updateItemSettings();
    m_isInitialized = true;
}

bool WikipediaPlugin::isInitialized() const
{
    return m_isInitialized;
}

QString WikipediaPlugin::name() const
{
    return tr( "Wikipedia Articles" );
}

QString WikipediaPlugin::guiString() const
{
    return tr( "&Wikipedia" );
}
   
QString WikipediaPlugin::description() const
{
    return tr( "Automatically downloads Wikipedia articles and shows them on the right position on the map" );
}
    
QIcon WikipediaPlugin::icon() const
{
    return m_icon;
}

QDialog *WikipediaPlugin::aboutDialog() const
{
    if ( !m_aboutDialog ) {
        // Initializing about dialog
        m_aboutDialog = new PluginAboutDialog();
        m_aboutDialog->setName( "Wikipedia Plugin" );
        m_aboutDialog->setVersion( "0.1" );
        // FIXME: Can we store this string for all of Marble
        m_aboutDialog->setAboutText( tr( "<br />(c) 2009 The Marble Project<br /><br /><a href=\"http://edu.kde.org/marble\">http://edu.kde.org/marble</a>" ) );
        QList<Author> authors;
        Author bholst;
        bholst.name = "Bastian Holst";
        bholst.task = tr( "Developer" );
        bholst.email = "bastianholst@gmx.de";
        authors.append( bholst );
        m_aboutDialog->setAuthors( authors );
        m_aboutDialog->setDataText( tr( "Geo positions by geonames.org\nTexts by wikipedia.org" ) );
        m_icon.addFile( MarbleDirs::path( "svg/wikipedia_shadow.svg" ) );
        m_aboutDialog->setPixmap( m_icon.pixmap( 62, 53 ) );
    }
    return m_aboutDialog;
}

QDialog *WikipediaPlugin::configDialog() const
{
    if ( !m_configDialog ) {
        // Initializing configuration dialog
        m_configDialog = new QDialog();
        ui_configWidget.setupUi( m_configDialog );
        ui_configWidget.m_itemNumberSpinBox->setRange( 0, maximumNumberOfItems );
        connect( ui_configWidget.m_buttonBox, SIGNAL( accepted() ),
                                            SLOT( writeSettings() ) );
        connect( ui_configWidget.m_buttonBox, SIGNAL( rejected() ),
                                            SLOT( readSettings() ) );
        QPushButton *applyButton = ui_configWidget.m_buttonBox->button( QDialogButtonBox::Apply );
        connect( applyButton, SIGNAL( clicked() ),
                this,        SLOT( writeSettings() ) );
        connect( this, SIGNAL( changedNumberOfItems( quint32 ) ),
                this, SLOT( setDialogNumberOfItems( quint32 ) ) );
        connect( this, SIGNAL( settingsChanged( QString ) ),
                this, SLOT( updateItemSettings() ) );
    }
    return m_configDialog;
}

QHash<QString,QVariant> WikipediaPlugin::settings() const
{
    return m_settings;
}

void WikipediaPlugin::setSettings( QHash<QString,QVariant> settings )
{
    m_settings = settings;
    readSettings();
}

void WikipediaPlugin::setShowThumbnails( bool shown )
{
    if ( shown ) {
        ui_configWidget.m_showThumbnailCheckBox->setCheckState( Qt::Checked );
    }
    else {
        ui_configWidget.m_showThumbnailCheckBox->setCheckState( Qt::Unchecked );
    }

    WikipediaModel *wikipediaModel = qobject_cast<WikipediaModel*>( model() );

    m_settings.insert( "showThumbnails", shown );
    if ( wikipediaModel ) {
        wikipediaModel->setShowThumbnail( shown );
    }
}

void WikipediaPlugin::readSettings()
{
    setNumberOfItems( m_settings.value( "numberOfItems", 15 ).toUInt() );
    setDialogNumberOfItems( numberOfItems() );
    if ( !m_settings.contains( "showThumbnails" ) ) {
        m_settings.insert( "showThumbnails", true );
    }

    setShowThumbnails( m_settings.value( "showThumbnails" ).toBool() );
}

void WikipediaPlugin::writeSettings()
{
    setNumberOfItems( ui_configWidget.m_itemNumberSpinBox->value() );
    m_settings.insert( "numberOfItems", ui_configWidget.m_itemNumberSpinBox->value() );
    if ( ui_configWidget.m_showThumbnailCheckBox->checkState() == Qt::Checked ) {
        setShowThumbnails( true );
        m_settings.insert( "showThumbnails", true );
    }
    else {
        setShowThumbnails( false );
        m_settings.insert( "showThumbnails", false );
    }

    emit settingsChanged( nameId() );
}

void WikipediaPlugin::setDialogNumberOfItems( quint32 number )
{
    if ( number <= maximumNumberOfItems ) {
        ui_configWidget.m_itemNumberSpinBox->setValue( (int) number );
    }
    else {
        // Force a the number of items being lower or equal maximumNumberOfItems
        setNumberOfItems( maximumNumberOfItems );
    }
}

void WikipediaPlugin::updateItemSettings()
{
    AbstractDataPluginModel *abstractModel = model();
    if( abstractModel != 0 ) {
        abstractModel->setItemSettings( m_settings );
    }
}

Q_EXPORT_PLUGIN2(WikipediaPlugin, Marble::WikipediaPlugin)

#include "WikipediaPlugin.moc"
