/*
    Copyright 2008 Henry de Valence <hdevalence@gmail.com>
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public 
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "MarbleRunnerManager.h"

#include "MarblePlacemarkModel.h"
#include "PlacemarkManager.h"
#include "GeoDataPlacemark.h"

#include "LatLonRunner.h"
#include "OnfRunner.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVector>

#include <QtCore/QtDebug>

namespace Marble
{

MarbleRunnerManager::MarbleRunnerManager( QObject *parent )
    : QObject(parent),
      m_activeRunners(0),
      m_lastString(""),
      m_model(new MarblePlacemarkModel)
{
    m_model->setPlacemarkContainer(&m_placemarkContainer);
    qRegisterMetaType<QVector<GeoDataPlacemark> >("QVector<GeoDataPlacemark>");
}

MarbleRunnerManager::~MarbleRunnerManager()
{
    foreach(MarbleAbstractRunner* runner, m_runners)
    {
        runner->quit();
        runner->wait();
        m_runners.removeOne(runner);
        delete runner;
    }

    delete m_model;
}

void MarbleRunnerManager::newText(QString text)
{
    m_lastString = text;
    m_placemarkContainer.clear();
    emit modelChanged( m_model );

    LatLonRunner* llrunner = new LatLonRunner;
    m_runners << dynamic_cast<MarbleAbstractRunner*>(llrunner);
    connect( llrunner, SIGNAL( runnerFinished( MarbleAbstractRunner*, QVector<GeoDataPlacemark> ) ),
             this,     SLOT( slotRunnerFinished( MarbleAbstractRunner*, QVector<GeoDataPlacemark> ) ));
    llrunner->parse(text);
    
    OnfRunner* onfrunner = new OnfRunner;
    m_runners << dynamic_cast<MarbleAbstractRunner*>(onfrunner);
    connect( onfrunner, SIGNAL( runnerFinished( MarbleAbstractRunner*, QVector<GeoDataPlacemark> ) ),
             this,      SLOT( slotRunnerFinished( MarbleAbstractRunner*, QVector<GeoDataPlacemark> ) ));
    onfrunner->parse(text);

    llrunner->start();
    onfrunner->start();
}

void MarbleRunnerManager::slotRunnerFinished( MarbleAbstractRunner* runner, QVector<GeoDataPlacemark> result )
{
    m_runners.removeOne(runner);
    runner->deleteLater();
    qDebug() << "Runner finished, active runners: " << m_runners.size();
    qDebug() << "Runner reports" << result.size() << "results";
    if( result.isEmpty() )
        return;

    int start = m_placemarkContainer.size();
    m_placemarkContainer << result;
    m_model->addPlacemarks( start, result.size() );
    emit modelChanged( m_model );
}

}

#include "MarbleRunnerManager.moc"
