/**
*-------------------------------------------------
*  Copyright 2016 by Tidop Research Group <daguilera@usal.se>
*
* This file is part of GRAPHOS - inteGRAted PHOtogrammetric Suite.
*
* GRAPHOS - inteGRAted PHOtogrammetric Suite is free software: you can redistribute
* it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation, either
* version 3 of the License, or (at your option) any later version.
*
* GRAPHOS - inteGRAted PHOtogrammetric Suite is distributed in the hope that it will
* be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*
* @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
*-------------------------------------------------
*/

#include <QDebug>
#include <QVariant>
#include <QFile>
#include <QDir>
//DHL_20140405
#include <QSqlDriver>
//#include <sqlite3.h>
//DHL_20140405
#include "PersistenceManager.h"
#include "RadialExtended.h"
#include "FraserModel.h"
#include "FishEyeModel.h"
#include "libPW.h"
#include "TiePointsIO.h"
#include "ImagePointCloud.h"
#include "PWGraphImages.h"
#include "PWMatchesCategory.h"

using namespace PW;

PersistenceManager::PersistenceManager(QString dbURL, QObject *parent) :
    QObject(parent),
    mDBURL(dbURL)
{
}

int PersistenceManager::updateDBVersion(QString from, QString to)
{
    QSqlQuery query(mDB);

    //From 0.x to 1.0  (0.x don't save versión in qsettings and version file)
    if(from.isEmpty() || from.startsWith("0.")){
        //Add Metadata field:
        QString queryString = "ALTER TABLE pw_projects ADD metadata CHAR(5000) DEFAULT ''";
        if(query.prepare(queryString)){
            query.exec();
            mDB.commit();
        }
        //        else{
        //            qCritical() << mDB.lastError();
        //            return 1;
        //        }
    }
    if(from.isEmpty() || from.toFloat()<1.7){
        //Add Metadata field:
        QString queryString = "ALTER TABLE pw_projects ADD tiepoints_name CHAR(100) DEFAULT ''";
        if(query.prepare(queryString)){
            query.exec();
            mDB.commit();
        }
        queryString = "ALTER TABLE pw_projects ADD cloud_uuid CHAR(50) DEFAULT ''";
        if(query.prepare(queryString)){
            query.exec();
            mDB.commit();
        }
        //        else{
        //            qCritical() << mDB.lastError();
        //            return 1;
        //        }
    }
    return 0;
}

bool PW::PersistenceManager::openDatasource()
{
    mDB = QSqlDatabase::addDatabase("QSQLITE","PW_DATA");
    mDB.setDatabaseName(mDBURL);
    //    QVariant v = mDB.driver()->handle();
    //    if (v.isValid() && qstrcmp(v.typeName(), "sqlite3*")==0)
    //    {
    //        // v.data() returns a pointer to the handle
    //        sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
    //        if (handle != 0)
    //        { // check that it is not NULL
    //            sqlite3_enable_load_extension(handle,1);
    //        }
    //    }
    if(mDB.open()){
        QSqlQuery query(mDB);
        if(query.prepare("PRAGMA foreign_keys = ON")){
            query.exec();
        }
        else{
            qCritical() << tr("Unsupported SQLITE Version.");
            return false;
        }
        if(query.prepare("PRAGMA synchronous = OFF")){
            query.exec();
        }
        else{
            qCritical() << tr("Unsupported SQLITE Version.");
            return false;
        }
        if(query.prepare("PRAGMA journal_mode = MEMORY")){
            query.exec();
        }
        else{
            qCritical() << tr("Unsupported SQLITE Version.");
            return false;
        }
        return true;
    }qCritical() << mDB.lastError();

    return false;
}

int PersistenceManager::vacuum()
{
    if(mDB.isOpen()){
        QSqlQuery query(mDB);
        if(query.prepare("VACUUM")){
            query.exec();
            return 0;
        }
        else{
            qCritical() << mDB.lastError();
            return 1;
        }
    }qCritical() << mDB.lastError();

    return 1;
}

int PW::PersistenceManager::openAuxDatasource()
{
    mDB = QSqlDatabase::addDatabase("QSQLITE","PW_DATA_AUX");
    mDB.setDatabaseName(mDBURL);
    if(mDB.open()){
        QSqlQuery query(mDB);
        if(query.prepare("PRAGMA foreign_keys = ON")){
            query.exec();
        }
        else{
            qCritical() << tr("Unsupported SQLITE Version.");
            return 1;
        }
        return 0;
    }qCritical() << mDB.lastError();

    return 1;
}

int PersistenceManager::closeDatasource()
{
    if (mDB.isOpen())
        mDB.close();
    return 0;
}

int PW::PersistenceManager::deleteProject(QString projectName)
{
    QSqlQuery query(mDB);

    //Deletting Porject if exists:
    QString queryString = "DELETE FROM pw_projects WHERE name = '" + projectName + "'";
    if(query.prepare(queryString)){
        query.exec();
    }
    else{
        qCritical() << mDB.lastError();
        return 1;
    }

    //Deletting Camera models:
    queryString = "Delete FROM pw_camera_models WHERE id NOT IN (SELECT DISTINCT id_active_model FROM pw_images)";
    if(query.prepare(queryString)){
        query.exec();
    }
    else{
        qCritical() << mDB.lastError();
        return 1;
    }

    return(0);
}

// DHL_20140405
int PersistenceManager::enableSpatialite()
{
    //    QVariant v = mDB.driver()->handle();
    //    if (v.isValid() && qstrcmp(v.typeName(), "sqlite3*")==0)
    //    {
    //        // v.data() returns a pointer to the handle
    //        sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
    //        if (handle != 0)
    //        { // check that it is not NULL
    //            sqlite3_enable_load_extension(handle,1);
    //        }
    //    }
    QSqlQuery tSqlQuery(mDB);
    QString sql_command_text;
    sql_command_text = QString("SELECT load_extension('libspatialite-4.dll')");
    tSqlQuery.prepare(sql_command_text);
    if (!tSqlQuery.exec())
    {
        return 1;
    }
    return(0);
}

int PersistenceManager::getMatchesCategories(int projectId,
                                             QVector<int> &idMatchesCategories,
                                             QStringList &metadataMatchesCategories,
                                             QStringList &descriptionMatchesCategories,
                                             QStringList &timeCalculationCategories)
{
    idMatchesCategories.clear();
    metadataMatchesCategories.clear();
    descriptionMatchesCategories.clear();
    timeCalculationCategories.clear();
    mDB.transaction();
    QSqlQuery query(mDB);
    QString queryString="SELECT id,metadata,description,time_calculation FROM pw_matches_categories WHERE id_project='";
    queryString+=QString::number(projectId);
    queryString+="'";
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            int id=query.record().value("id").toInt();
            QString metadata=query.record().value("metadata").toString();
            QString description=query.record().value("description").toString();
            QString time_calculation=query.record().value("time_calculation").toString();
            idMatchesCategories.push_back(id);
            metadataMatchesCategories.append(metadata);
            descriptionMatchesCategories.append(description);
            timeCalculationCategories.append(time_calculation);
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatchesCategories error getting matching categories for project Id:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    if(!mDB.commit())
    {
        qCritical() << mDB.lastError();
        return PERSISTENCEMANAGER_ERROR;
    }
    return PERSISTENCEMANAGER_NO_ERROR;
}

bool PersistenceManager::copyMatches(int projectId,
                                     QMap<QString, QVector<QString> > imagesIdsTiePoints,
                                     QMap<QString, QMap<QString,QVector<float> > > imagesFirstColumnsTiePoints,
                                     QMap<QString, QMap<QString,QVector<float> > > imagesFirstRowsTiePoints,
                                     QMap<QString, QMap<QString,QVector<float> > > imagesSecondColumnsTiePoints,
                                     QMap<QString, QMap<QString,QVector<float> > > imagesSecondRowsTiePoints,
                                     Project *destinationProject)
{
    //Consulto la lista de categorías del proyecto.
    mDB.transaction();
    QSqlQuery query(mDB);
    QString queryString="SELECT count(*) as size FROM pw_matches AS m,pw_matches_categories AS c WHERE c.id_project='";
    queryString+=QString::number(projectId);

    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QStringList categories;
            categories << query.record().value("description").toString();
            //getMatches
            QMap<QString,QVector<QString> > imagesIdsTiePoints;
            QMap<QString,QMap<QString,QVector<double> > > imagesFirstColumnsTiePoints;
            QMap<QString,QMap<QString,QVector<double> > > imagesFirstRowsTiePoints;
            QMap<QString,QMap<QString,QVector<double> > > imagesSecondColumnsTiePoints;
            QMap<QString,QMap<QString,QVector<double> > > imagesSecondRowsTiePoints;
            if(PERSISTENCEMANAGER_NO_ERROR!=getMatches(projectId,
                                                       categories,
                                                       imagesIdsTiePoints,
                                                       imagesFirstColumnsTiePoints,
                                                       imagesFirstRowsTiePoints,
                                                       imagesSecondColumnsTiePoints,
                                                       imagesSecondRowsTiePoints))
            {
                QString msg=tr("PersistenceManager::copyMatches error getting matches for category: \n%1").arg(categories.at(0));
                qCritical() << msg;
                return PERSISTENCEMANAGER_ERROR;
            }

            //writeMatches
            if(0!=writeMatches(destinationProject,
                               query.record().value("metadata").toString(),
                               categories.at(0),
                               query.record().value("time_calculation").toString(),
                               imagesIdsTiePoints,
                               imagesFirstColumnsTiePoints,
                               imagesFirstRowsTiePoints,
                               imagesSecondColumnsTiePoints,
                               imagesSecondRowsTiePoints
                               ))
            {
                QString msg=tr("PersistenceManager::copyMatches error writing matches for category: \n%1").arg(categories.at(0));
                qCritical() << msg;
                return PERSISTENCEMANAGER_ERROR;
            }

        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting number of images and keypoints ids for matches for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }
}

int PersistenceManager::writeMatchesCategory(PWMatchesCategory *category, int &categoryId)
{
    QString strProjectID=QString::number(category->getProject()->getID());
    QString queryString = "INSERT INTO pw_matches_categories (metadata, description, time_calculation, id_project) ";
    queryString+="VALUES ('"+category->getMetadata()+"','"+category->getDescription()+"','"+category->getTimeCalculation()+"',"+strProjectID+")";
    QSqlQuery query(mDB);
    if(query.prepare(queryString))
    {
        query.exec();
        categoryId = query.lastInsertId().toInt();
    }
    else
    {
        QString msg=tr("PersistenceManager::writeMatchesCategory error writing matching catgory:\n%1").arg(category->getDescription());
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }
}

int PersistenceManager::writeMatchesCategories(Project *project)
{
    QString queryString = "INSERT INTO pw_image_pairs (id_img1, id_img2, id_category) ";
    queryString+="VALUES (:imgId1,:imgId2,:categoryId)";
    QSqlQuery query(mDB);
    if(!query.prepare(queryString))
    {
        QString msg=tr("PersistenceManager::writeMatchesCategories error writing image pairs.");
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    QList <PWMatchesCategory *> categories = project->getMatchesCategories();
    int categoryId;
    for(int i=0; i< categories.count();i++){
        writeMatchesCategory(categories.at(i),categoryId);

        //Writting Image Matching graph:
        QMap<QString, QVector<QString> > imagesPairs;
        PWMatchesCategory * category = project->getMatchesCategories().at(i);
        category->getImagePairs(imagesPairs);
        QString originalImagesExtension = project->getImagesFileName().first().split(".").last();
        for (int j=0; j<imagesPairs.keys().count();j++)
            for(int k=0; k<imagesPairs.values().at(j).count();k++)
            {
                int imageId1 = project->getImageByName(imagesPairs.keys().at(j).split(".").first().append(".").append(originalImagesExtension))->getID();
                int imageId2 = project->getImageByName(imagesPairs.values().at(j)[k].split(".").first().append(".").append(originalImagesExtension))->getID();
                query.bindValue(":imgId1",QString::number(imageId1));
                query.bindValue(":imgId2",QString::number(imageId2));
                query.bindValue(":categoryId",QString::number(categoryId));
                query.exec();
            }
    }
}

bool PersistenceManager::getMatchesOLD(int projectId,
                                    QStringList descriptionMatchesCategories,
                                    QMap<QString, int> &imagesIds,
                                    QMap<QString, QVector<QString> > &imagesIdsTiePoints,
                                    QMap<QString, QMap<QString,QVector<float> > > &imagesFirstColumnsTiePoints,
                                    QMap<QString, QMap<QString,QVector<float> > > &imagesFirstRowsTiePoints,
                                    QMap<QString, QMap<QString,QVector<float> > >& imagesSecondColumnsTiePoints,
                                    QMap<QString, QMap<QString,QVector<float> > >& imagesSecondRowsTiePoints,
                                    QMap<QString, QMap<QString, QVector<int> > > &matchesIds)
{
    imagesIdsTiePoints.clear();
    imagesFirstColumnsTiePoints.clear();
    imagesFirstColumnsTiePoints.clear();
    imagesSecondColumnsTiePoints.clear();
    imagesSecondRowsTiePoints.clear();
    matchesIds.clear();
    double factorConversionImageCoordinates=1.0/pow(10.0,1.0*LIBPW_IMAGE_COORDINATES_PRECISION);
    if(descriptionMatchesCategories.isEmpty())
    {
        QString msg=tr("Error in PersistenceManager::getMatches: descriptionMatchesCategories is empty");
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }
    imagesIdsTiePoints.clear();
    imagesFirstColumnsTiePoints.clear();
    imagesFirstRowsTiePoints.clear();
    imagesSecondColumnsTiePoints.clear();
    imagesSecondRowsTiePoints.clear();
    mDB.transaction();
    QSqlQuery query(mDB);
    QString queryString;

    QStringList idMatchesCategories;
    queryString="SELECT id from pw_matches_categories WHERE (id_project="+QString::number(projectId)+" AND ";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString descriptionMatchesCategory=descriptionMatchesCategories.at(i);
        queryString+="description='";
        queryString+=descriptionMatchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QString imgUrl=query.record().value("id").toString();
            idMatchesCategories.push_back(imgUrl);
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    // Obtenemos los url de todas las imagenes
    QMap<QString,QString> imageIdByIntIds;
    queryString="SELECT i.id as imgId,i.url as imgUrl from pw_images as i WHERE i.id_project='";
    queryString+=QString::number(projectId);
    queryString+="'";
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QString imgUrl=query.record().value("imgUrl").toString();
            QString imgId=query.record().value("imgId").toString();
            imageIdByIntIds[imgId]=imgUrl;
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    // Obtenemos los pares distintosse
    queryString="SELECT m.id as id,m.id_image_first as imgId1,m.id_image_second as imgId2,m.column1,m.row1,m.column2,m.row2 FROM pw_matches as m ";
    queryString+="WHERE (";
    //    queryString+="WHERE c.id_project='";
    //    queryString+=QString::number(projectId);
    //    queryString+="' AND (";
    for(int i=0;i<idMatchesCategories.size();i++)
    {
        QString idMatchesCategory=idMatchesCategories.at(i);
        queryString+="m.id_matches_category='";
        queryString+=idMatchesCategory;
        if(i<(idMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QString imgIntId1=query.record().value("imgId1").toString();
            QString imgId1=imageIdByIntIds[imgIntId1];
            QString imgIntId2=query.record().value("imgId2").toString();
            QString imgId2=imageIdByIntIds[imgIntId2];
            int matchId=query.record().value("id").toInt();
            float column1 = query.record().value("column1").toFloat()*factorConversionImageCoordinates;
            float row1 = query.record().value("row1").toFloat()*factorConversionImageCoordinates;
            float column2 = query.record().value("column2").toFloat()*factorConversionImageCoordinates;
            float row2 = query.record().value("row2").toFloat()*factorConversionImageCoordinates;
            if(!imagesIdsTiePoints.contains(imgId1))
            {
                QVector<QString> imagePairsIdsTiePoints;
                QMap<QString, QVector<int> > pairMatchesIds;
                QVector<int> matchIds;
                matchIds.push_back(matchId);
                pairMatchesIds[imgId2]=matchIds;
                QMap<QString,QVector<float> > imagePairsFirstColumnsTiePoints;
                QMap<QString,QVector<float> > imagePairsFirstRowsTiePoints;
                QMap<QString,QVector<float> > imagePairsSecondColumnsTiePoints;
                QMap<QString,QVector<float> > imagePairsSecondRowsTiePoints;
                QVector<float> firstColumnsTiePoints;
                QVector<float> firstRowsTiePoints;
                QVector<float> secondColumnsTiePoints;
                QVector<float> secondRowsTiePoints;
                firstColumnsTiePoints.push_back(column1);
                firstRowsTiePoints.push_back(row1);
                secondColumnsTiePoints.push_back(column2);
                secondRowsTiePoints.push_back(row2);
                imagePairsFirstColumnsTiePoints[imgId2]=firstColumnsTiePoints;
                imagePairsFirstRowsTiePoints[imgId2]=firstRowsTiePoints;
                imagePairsSecondColumnsTiePoints[imgId2]=secondColumnsTiePoints;
                imagePairsSecondRowsTiePoints[imgId2]=secondRowsTiePoints;
                imagePairsIdsTiePoints.push_back(imgId2);
                imagesFirstColumnsTiePoints[imgId1]=imagePairsFirstColumnsTiePoints;
                imagesFirstRowsTiePoints[imgId1]=imagePairsFirstRowsTiePoints;
                imagesSecondColumnsTiePoints[imgId1]=imagePairsSecondColumnsTiePoints;
                imagesSecondRowsTiePoints[imgId1]=imagePairsSecondRowsTiePoints;
                matchesIds[imgId1]=pairMatchesIds;
                imagesIdsTiePoints[imgId1]=imagePairsIdsTiePoints;
            }
            else
            {
                if(!imagesIdsTiePoints[imgId1].contains(imgId2))
                {
                    imagesIdsTiePoints[imgId1].push_back(imgId2);
                    QVector<int> matchIds;
                    matchIds.push_back(matchId);
                    QVector<float> firstColumnsTiePoints;
                    QVector<float> firstRowsTiePoints;
                    QVector<float> secondColumnsTiePoints;
                    QVector<float> secondRowsTiePoints;
                    firstColumnsTiePoints.push_back(column1);
                    firstRowsTiePoints.push_back(row1);
                    secondColumnsTiePoints.push_back(column2);
                    secondRowsTiePoints.push_back(row2);
                    imagesFirstColumnsTiePoints[imgId1][imgId2]=firstColumnsTiePoints;
                    imagesFirstRowsTiePoints[imgId1][imgId2]=firstRowsTiePoints;
                    imagesSecondColumnsTiePoints[imgId1][imgId2]=secondColumnsTiePoints;
                    imagesSecondRowsTiePoints[imgId1][imgId2]=secondRowsTiePoints;
                    matchesIds[imgId1][imgId2]=matchIds;
                }
                else
                {
                    imagesFirstColumnsTiePoints[imgId1][imgId2].push_back(column1);
                    imagesFirstRowsTiePoints[imgId1][imgId2].push_back(row1);
                    imagesSecondColumnsTiePoints[imgId1][imgId2].push_back(column2);
                    imagesSecondRowsTiePoints[imgId1][imgId2].push_back(row2);
                    matchesIds[imgId1][imgId2].push_back(matchId);
                }
            }
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting second images for project name:\n%1").arg(projectId);
        qCritical() << msg;
        imagesIdsTiePoints.clear();
        imagesFirstColumnsTiePoints.clear();
        imagesFirstColumnsTiePoints.clear();
        imagesSecondColumnsTiePoints.clear();
        imagesSecondRowsTiePoints.clear();
        matchesIds.clear();
        return PERSISTENCEMANAGER_ERROR;
    }
    return PERSISTENCEMANAGER_NO_ERROR;
}

bool PersistenceManager::getMatchesNEW(int projectId,
                                    QStringList descriptionMatchesCategories,
                                    QMap<QString, int> &imagesIds,
                                    QMap<QString, QVector<QString> > &imagesIdsTiePoints,
                                    QMap<QString, QMap<QString,QVector<float> > > &imagesFirstColumnsTiePoints,
                                    QMap<QString, QMap<QString,QVector<float> > > &imagesFirstRowsTiePoints,
                                    QMap<QString, QMap<QString,QVector<float> > >& imagesSecondColumnsTiePoints,
                                    QMap<QString, QMap<QString,QVector<float> > >& imagesSecondRowsTiePoints,
                                    QMap<QString, QMap<QString, QVector<int> > > &matchesIds)
{
    imagesIdsTiePoints.clear();
    imagesFirstColumnsTiePoints.clear();
    imagesFirstColumnsTiePoints.clear();
    imagesSecondColumnsTiePoints.clear();
    imagesSecondRowsTiePoints.clear();
    matchesIds.clear();
    double factorConversionImageCoordinates=1.0/pow(10.0,1.0*LIBPW_IMAGE_COORDINATES_PRECISION);
    if(descriptionMatchesCategories.isEmpty())
    {
        QString msg=tr("Error in PersistenceManager::getMatches: descriptionMatchesCategories is empty");
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }
    imagesIdsTiePoints.clear();
    imagesFirstColumnsTiePoints.clear();
    imagesFirstRowsTiePoints.clear();
    imagesSecondColumnsTiePoints.clear();
    imagesSecondRowsTiePoints.clear();
    mDB.transaction();
    QSqlQuery query(mDB);
    QString queryString;

    QStringList idMatchesCategories;
    queryString="SELECT id from pw_matches_categories WHERE (id_project="+QString::number(projectId)+" AND ";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString descriptionMatchesCategory=descriptionMatchesCategories.at(i);
        queryString+="description='";
        queryString+=descriptionMatchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QString categoryId=query.record().value("id").toString();
            idMatchesCategories.push_back(categoryId);
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    // Obtenemos los url de todas las imagenes
    QMap<QString,QString> imageIdByIntIds;
    queryString="SELECT i.id as imgId,i.url as imgUrl from pw_images as i WHERE i.id_project='";
    queryString+=QString::number(projectId);
    queryString+="'";
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QString imgUrl=query.record().value("imgUrl").toString();
            QString imgId=query.record().value("imgId").toString();
            imageIdByIntIds[imgId]=imgUrl;
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    QSqlQuery query2(mDB);
    QSqlQuery query3(mDB);
    QSqlQuery query4(mDB);

    //prepare queries:**************
    queryString="SELECT m.id as id,m.column1 as column1 ,m.row1 as row1 ,m.column2 as column2 ,m.row2 as row2 FROM pw_matches as m ";
    queryString+="WHERE (";
    for(int i=0;i<idMatchesCategories.size();i++)
    {
        QString idMatchesCategory=idMatchesCategories.at(i);
        queryString+="m.id_matches_category='";
        queryString+=idMatchesCategory;
        if(i<(idMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="' ";
        }
    }
    queryString+= "AND m.id_image_first= :imgId1 AND m.id_image_second= :imgId2)";
    query4.prepare(queryString);

    queryString="SELECT DISTINCT m.id_image_second as imgId2 FROM pw_matches as m ";
    queryString+="WHERE (";
    for(int i=0;i<idMatchesCategories.size();i++)
    {
        QString idMatchesCategory=idMatchesCategories.at(i);
        queryString+="m.id_matches_category='";
        queryString+=idMatchesCategory;
        if(i<(idMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }

    queryString+= "AND m.id_image_first = :imgId1";
    query2.prepare(queryString);
    //*******************************************************************


    queryString="SELECT DISTINCT m.id_image_first as imgId1 FROM pw_matches as m ";
    queryString+="WHERE (";
    for(int i=0;i<idMatchesCategories.size();i++)
    {
        QString idMatchesCategory=idMatchesCategories.at(i);
        queryString+="m.id_matches_category='";
        queryString+=idMatchesCategory;
        if(i<(idMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }

    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QVector<QString> imagePairsIdsTiePoints;
            QMap<QString, QVector<int> > pairMatchesIds;
            QMap<QString,QVector<float> > imagePairsFirstColumnsTiePoints;
            QMap<QString,QVector<float> > imagePairsFirstRowsTiePoints;
            QMap<QString,QVector<float> > imagePairsSecondColumnsTiePoints;
            QMap<QString,QVector<float> > imagePairsSecondRowsTiePoints;

            QString imgId1 = query.record().value("imgId1").toString();
            QString imgId1Str=imageIdByIntIds[imgId1];

            imagesFirstColumnsTiePoints[imgId1Str]=imagePairsFirstColumnsTiePoints;
            imagesFirstRowsTiePoints[imgId1Str]=imagePairsFirstRowsTiePoints;
            imagesSecondColumnsTiePoints[imgId1Str]=imagePairsSecondColumnsTiePoints;
            imagesSecondRowsTiePoints[imgId1Str]=imagePairsSecondRowsTiePoints;
            imagesIdsTiePoints[imgId1Str]=imagePairsIdsTiePoints;
            matchesIds[imgId1Str]=pairMatchesIds;

            query2.bindValue(":imgId1",imgId1);

            query2.exec();
            while(query2.next())
            {
                queryString="SELECT count(*) as matchesCount FROM pw_matches as m ";
                queryString+="WHERE (";
                for(int i=0;i<idMatchesCategories.size();i++)
                {
                    QString idMatchesCategory=idMatchesCategories.at(i);
                    queryString+="m.id_matches_category='";
                    queryString+=idMatchesCategory;
                    if(i<(idMatchesCategories.size()-1))
                    {
                        queryString+="' OR ";
                    }
                    else
                    {
                        queryString+="' ";
                    }
                }
                QString imgId2 = query2.record().value("imgId2").toString();
                queryString+= "AND m.id_image_first=" + imgId1 +
                        " AND m.id_image_second=" + imgId2 + ") ";

                QVector<int> matchIds;
                QVector<float> firstColumnsTiePoints;
                QVector<float> firstRowsTiePoints;
                QVector<float> secondColumnsTiePoints;
                QVector<float> secondRowsTiePoints;

                QString imgId2Str=imageIdByIntIds[imgId2];

                if(query3.prepare(queryString))
                {
                    query3.exec();

                    if(query3.next())
                    {
                        int matchesCount = query3.record().value("matchesCount").toInt();
                        matchIds.resize(matchesCount);
                        firstColumnsTiePoints.resize(matchesCount);
                        firstRowsTiePoints.resize(matchesCount);
                        secondColumnsTiePoints.resize(matchesCount);
                        secondRowsTiePoints.resize(matchesCount);

                        matchesIds[imgId1Str][imgId2Str]=matchIds;

                        imagesFirstColumnsTiePoints[imgId1Str][imgId2Str]=firstColumnsTiePoints;
                        imagesFirstRowsTiePoints[imgId1Str][imgId2Str]=firstRowsTiePoints;
                        imagesSecondColumnsTiePoints[imgId1Str][imgId2Str]=secondColumnsTiePoints;
                        imagesSecondRowsTiePoints[imgId1Str][imgId2Str]=secondRowsTiePoints;
                        imagesIdsTiePoints[imgId1Str].push_back(imgId2Str);
                    }
                }
                else
                {
                    QString msg=tr("PersistenceManager::getMatches error getting matches for project name:\n%1").arg(projectId);
                    qCritical() << msg;
                    imagesIdsTiePoints.clear();
                    imagesFirstColumnsTiePoints.clear();
                    imagesFirstColumnsTiePoints.clear();
                    imagesSecondColumnsTiePoints.clear();
                    imagesSecondRowsTiePoints.clear();
                    matchesIds.clear();
                    return PERSISTENCEMANAGER_ERROR;
                }

                query4.bindValue(":imgId1",imgId1);
                query4.bindValue(":imgId2",imgId2);
                query4.exec();
                int matchesCount=0;
                while(query4.next())
                {
                    int matchId=query4.record().value("id").toInt();
                    float column1 = query4.record().value("column1").toFloat()*factorConversionImageCoordinates;
                    float row1 = query4.record().value("row1").toFloat()*factorConversionImageCoordinates;
                    float column2 = query4.record().value("column2").toFloat()*factorConversionImageCoordinates;
                    float row2 = query4.record().value("row2").toFloat()*factorConversionImageCoordinates;

                    matchIds[matchesCount]=matchId;

                    imagesFirstColumnsTiePoints[imgId1Str][imgId2Str][matchesCount]=column1;
                    imagesFirstRowsTiePoints[imgId1Str][imgId2Str][matchesCount]=row1;
                    imagesSecondColumnsTiePoints[imgId1Str][imgId2Str][matchesCount]=column2;
                    imagesSecondRowsTiePoints[imgId1Str][imgId2Str][matchesCount]=row2;

                    matchesCount++;
                }

            }
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting firsts images for project name:\n%1").arg(projectId);
        qCritical() << msg;
        imagesIdsTiePoints.clear();
        imagesFirstColumnsTiePoints.clear();
        imagesFirstColumnsTiePoints.clear();
        imagesSecondColumnsTiePoints.clear();
        imagesSecondRowsTiePoints.clear();
        matchesIds.clear();
        return PERSISTENCEMANAGER_ERROR;
    }

    return PERSISTENCEMANAGER_NO_ERROR;
}

bool PersistenceManager::getMatches(int projectId,
                                       QStringList descriptionMatchesCategories,
                                       QMap<QString, int> &imagesIds,
                                       QMap<QString, QVector<QString> > &imagesIdsTiePoints,
                                       QMap<QString, QMap<QString,QVector<double> > > &imagesFirstColumnsTiePoints,
                                       QMap<QString, QMap<QString,QVector<double> > > &imagesFirstRowsTiePoints,
                                       QMap<QString, QMap<QString,QVector<double> > >& imagesSecondColumnsTiePoints,
                                       QMap<QString, QMap<QString,QVector<double> > >& imagesSecondRowsTiePoints,
                                       QMap<QString, QMap<QString, QVector<int> > > &matchesIds)
{
    imagesIdsTiePoints.clear();
    imagesFirstColumnsTiePoints.clear();
    imagesFirstColumnsTiePoints.clear();
    imagesSecondColumnsTiePoints.clear();
    imagesSecondRowsTiePoints.clear();
    matchesIds.clear();
    double factorConversionImageCoordinates=1.0/pow(10.0,1.0*LIBPW_IMAGE_COORDINATES_PRECISION);
    if(descriptionMatchesCategories.isEmpty())
    {
        QString msg=tr("Error in PersistenceManager::getMatches: descriptionMatchesCategories is empty");
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }
    imagesIdsTiePoints.clear();
    imagesFirstColumnsTiePoints.clear();
    imagesFirstRowsTiePoints.clear();
    imagesSecondColumnsTiePoints.clear();
    imagesSecondRowsTiePoints.clear();
    mDB.transaction();
    QSqlQuery query(mDB);
    QString queryString;

    QStringList idMatchesCategories;
    queryString="SELECT id from pw_matches_categories WHERE (id_project="+QString::number(projectId)+" AND ";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString descriptionMatchesCategory=descriptionMatchesCategories.at(i);
        queryString+="description='";
        queryString+=descriptionMatchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QString categoryId=query.record().value("id").toString();
            idMatchesCategories.push_back(categoryId);
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    // Obtenemos los url de todas las imagenes
    QMap<QString,QString> imageIdByIntIds;
    queryString="SELECT i.id as imgId,i.url as imgUrl from pw_images as i WHERE i.id_project='";
    queryString+=QString::number(projectId);
    queryString+="'";
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QString imgUrl=query.record().value("imgUrl").toString();
            QString imgId=query.record().value("imgId").toString();
            imageIdByIntIds[imgId]=imgUrl;
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    QSqlQuery query2(mDB);
    QSqlQuery query3(mDB);
    QSqlQuery query4(mDB);

    //prepare queries:**************
    queryString="SELECT m.id as id,m.column1 as column1 ,m.row1 as row1 ,m.column2 as column2 ,m.row2 as row2 FROM pw_matches as m ";
    queryString+="WHERE (";
    for(int i=0;i<idMatchesCategories.size();i++)
    {
        QString idMatchesCategory=idMatchesCategories.at(i);
        queryString+="m.id_matches_category='";
        queryString+=idMatchesCategory;
        if(i<(idMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="' ";
        }
    }
    queryString+= "AND m.id_image_first= :imgId1 AND m.id_image_second= :imgId2)";
    query4.prepare(queryString);

    queryString="SELECT DISTINCT m.id_image_second as imgId2 FROM pw_matches as m ";
    queryString+="WHERE (";
    for(int i=0;i<idMatchesCategories.size();i++)
    {
        QString idMatchesCategory=idMatchesCategories.at(i);
        queryString+="m.id_matches_category='";
        queryString+=idMatchesCategory;
        if(i<(idMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }

    queryString+= "AND m.id_image_first = :imgId1";
    query2.prepare(queryString);


    queryString="SELECT count(*) as matchesCount FROM pw_matches as m ";
    queryString+="WHERE (";
    for(int i=0;i<idMatchesCategories.size();i++)
    {
        QString idMatchesCategory=idMatchesCategories.at(i);
        queryString+="m.id_matches_category='";
        queryString+=idMatchesCategory;
        if(i<(idMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="' ";
        }
    }
    queryString+= "AND m.id_image_first= :imgId1 AND m.id_image_second= :imgId2)";
    query3.prepare(queryString);
    //*******************************************************************


    queryString="SELECT DISTINCT m.id_image_first as imgId1 FROM pw_matches as m ";
    queryString+="WHERE (";
    for(int i=0;i<idMatchesCategories.size();i++)
    {
        QString idMatchesCategory=idMatchesCategories.at(i);
        queryString+="m.id_matches_category='";
        queryString+=idMatchesCategory;
        if(i<(idMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }

    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QVector<QString> imagePairsIdsTiePoints;
            QMap<QString, QVector<int> > pairMatchesIds;
            QMap<QString,QVector<double> > imagePairsFirstColumnsTiePoints;
            QMap<QString,QVector<double> > imagePairsFirstRowsTiePoints;
            QMap<QString,QVector<double> > imagePairsSecondColumnsTiePoints;
            QMap<QString,QVector<double> > imagePairsSecondRowsTiePoints;

            QString imgId1 = query.record().value("imgId1").toString();
            QString imgId1Str=imageIdByIntIds[imgId1];

            imagesFirstColumnsTiePoints[imgId1Str]=imagePairsFirstColumnsTiePoints;
            imagesFirstRowsTiePoints[imgId1Str]=imagePairsFirstRowsTiePoints;
            imagesSecondColumnsTiePoints[imgId1Str]=imagePairsSecondColumnsTiePoints;
            imagesSecondRowsTiePoints[imgId1Str]=imagePairsSecondRowsTiePoints;
            imagesIdsTiePoints[imgId1Str]=imagePairsIdsTiePoints;
            matchesIds[imgId1Str]=pairMatchesIds;

            query2.bindValue(":imgId1",imgId1);

            query2.exec();
            while(query2.next())
            {
                QVector<int> matchIds;
                QVector<double> firstColumnsTiePoints;
                QVector<double> firstRowsTiePoints;
                QVector<double> secondColumnsTiePoints;
                QVector<double> secondRowsTiePoints;

                QString imgId2 = query2.record().value("imgId2").toString();
                QString imgId2Str=imageIdByIntIds[imgId2];

                query3.bindValue(":imgId1",imgId1);
                query3.bindValue(":imgId2",imgId2);
                query3.exec();

                if(query3.next())
                {
                    int matchesCount = query3.record().value("matchesCount").toInt();

                    matchesIds[imgId1Str][imgId2Str]=matchIds;

                    imagesFirstColumnsTiePoints[imgId1Str][imgId2Str]=firstColumnsTiePoints;
                    imagesFirstRowsTiePoints[imgId1Str][imgId2Str]=firstRowsTiePoints;
                    imagesSecondColumnsTiePoints[imgId1Str][imgId2Str]=secondColumnsTiePoints;
                    imagesSecondRowsTiePoints[imgId1Str][imgId2Str]=secondRowsTiePoints;
                    imagesIdsTiePoints[imgId1Str].push_back(imgId2Str);

                    matchesIds[imgId1Str][imgId2Str].resize(matchesCount);
                    imagesFirstColumnsTiePoints[imgId1Str][imgId2Str].resize(matchesCount);
                    imagesFirstRowsTiePoints[imgId1Str][imgId2Str].resize(matchesCount);
                    imagesSecondColumnsTiePoints[imgId1Str][imgId2Str].resize(matchesCount);
                    imagesSecondRowsTiePoints[imgId1Str][imgId2Str].resize(matchesCount);
                }

                query4.bindValue(":imgId1",imgId1);
                query4.bindValue(":imgId2",imgId2);
                query4.exec();
                int matchesIndex=0;
                while(query4.next())
                {
                    int matchId=query4.record().value("id").toInt();
                    float column1 = query4.record().value("column1").toFloat()*factorConversionImageCoordinates;
                    float row1 = query4.record().value("row1").toFloat()*factorConversionImageCoordinates;
                    float column2 = query4.record().value("column2").toFloat()*factorConversionImageCoordinates;
                    float row2 = query4.record().value("row2").toFloat()*factorConversionImageCoordinates;

                    matchesIds[imgId1Str][imgId2Str][matchesIndex]=matchId;

                    imagesFirstColumnsTiePoints[imgId1Str][imgId2Str][matchesIndex]=column1;
                    imagesFirstRowsTiePoints[imgId1Str][imgId2Str][matchesIndex]=row1;
                    imagesSecondColumnsTiePoints[imgId1Str][imgId2Str][matchesIndex]=column2;
                    imagesSecondRowsTiePoints[imgId1Str][imgId2Str][matchesIndex]=row2;

                    matchesIndex++;
                }

            }
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting firsts images for project name:\n%1").arg(projectId);
        qCritical() << msg;
        imagesIdsTiePoints.clear();
        imagesFirstColumnsTiePoints.clear();
        imagesFirstColumnsTiePoints.clear();
        imagesSecondColumnsTiePoints.clear();
        imagesSecondRowsTiePoints.clear();
        matchesIds.clear();
        return PERSISTENCEMANAGER_ERROR;
    }

    return PERSISTENCEMANAGER_NO_ERROR;
}

bool PersistenceManager::getMatches_20140821(int projectId,
                                             QStringList descriptionMatchesCategories,
                                             QMap<QString, int> &activeImagesIds,
                                             QMap<QString, QVector<QString> > &imagesIdsTiePoints,
                                             QMap<QString, QMap<QString,QVector<float> > > &imagesFirstColumnsTiePoints,
                                             QMap<QString, QMap<QString,QVector<float> > > &imagesFirstRowsTiePoints,
                                             QMap<QString, QMap<QString,QVector<float> > >& imagesSecondColumnsTiePoints,
                                             QMap<QString, QMap<QString,QVector<float> > >& imagesSecondRowsTiePoints,
                                             QMap<QString, QMap<QString, QVector<int> > > &matchesIds)
{
    imagesIdsTiePoints.clear();
    imagesFirstColumnsTiePoints.clear();
    imagesFirstColumnsTiePoints.clear();
    imagesSecondColumnsTiePoints.clear();
    imagesSecondRowsTiePoints.clear();
    matchesIds.clear();
    double factorConversionImageCoordinates=1.0/pow(10.0,1.0*LIBPW_IMAGE_COORDINATES_PRECISION);
    if(descriptionMatchesCategories.isEmpty())
    {
        QString msg=tr("Error in PersistenceManager::getMatches: descriptionMatchesCategories is empty");
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }
    imagesIdsTiePoints.clear();
    imagesFirstColumnsTiePoints.clear();
    imagesFirstRowsTiePoints.clear();
    imagesSecondColumnsTiePoints.clear();
    imagesSecondRowsTiePoints.clear();
    mDB.transaction();
    QSqlQuery query(mDB);
    QString queryString="SELECT count(*) as size FROM pw_matches AS m,pw_matches_categories AS c WHERE c.id_project='";
    queryString+=QString::number(projectId);
    queryString+="' AND (";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString matchesCategory=descriptionMatchesCategories.at(i);
        queryString+="c.description='";
        queryString+=matchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    queryString+="AND c.id=m.id_matches_category";
    int numberOfMatches;
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            numberOfMatches=query.record().value("size").toInt();
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting number of images and keypoints ids for matches for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    QVector<QString> firstImageUrls(numberOfMatches);
    QVector<int> firstKpColumns(numberOfMatches);
    QVector<int> firstKpRows(numberOfMatches);
    QVector<int> firstMatchIds(numberOfMatches);
    QVector<QString> secondImageUrls(numberOfMatches);
    QVector<int> secondKpColumns(numberOfMatches);
    QVector<int> secondKpRows(numberOfMatches);
    QVector<int> secondMatchIds(numberOfMatches);

    //    queryString="SELECT i.url as imgUrl,ST_X(k.pixel_point_geom) as x,ST_Y(k.pixel_point_geom) as y FROM pw_matches AS m,";
    queryString="SELECT i.url as imgUrl,k.x as x, k.y as y, m.id as matchId FROM pw_matches AS m,";
    queryString+="pw_matches_categories AS c,pw_images as i,pw_keypoints as k WHERE c.id_project='";
    queryString+=QString::number(projectId);
    queryString+="' AND (";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString matchesCategory=descriptionMatchesCategories.at(i);
        queryString+="c.description='";
        queryString+=matchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    queryString+="AND c.id=m.id_matches_category AND k.id=m.id_keypoint_first AND i.id=k.id_image";
    if(query.prepare(queryString))
    {
        query.exec();
        int cont=-1;
        while(query.next())
        {
            cont++;
            QString imgUrl=query.record().value("imgUrl").toString();
            int column=query.record().value("x").toInt();
            int row=query.record().value("y").toInt();
            int matchId=query.record().value("matchId").toInt();
            firstImageUrls[cont]=imgUrl;
            firstKpColumns[cont]=column;
            firstKpRows[cont]=row;
            firstMatchIds[cont]=matchId;
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images and keypoints for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    //    queryString="SELECT i.url as imgUrl,ST_X(k.pixel_point_geom) as x,ST_Y(k.pixel_point_geom) as y FROM pw_matches AS m,";
    queryString="SELECT i.url as imgUrl,k.x as x, k.y as y, m.id as matchId FROM pw_matches AS m,";
    queryString+="pw_matches_categories AS c,pw_images as i,pw_keypoints as k WHERE c.id_project='";
    queryString+=QString::number(projectId);
    queryString+="' AND (";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString matchesCategory=descriptionMatchesCategories.at(i);
        queryString+="c.description='";
        queryString+=matchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    queryString+="AND c.id=m.id_matches_category AND k.id=m.id_keypoint_second AND i.id=k.id_image";
    if(query.prepare(queryString))
    {
        query.exec();
        int cont=-1;
        while(query.next())
        {
            cont++;
            QString imgUrl=query.record().value("imgUrl").toString();
            int column=query.record().value("x").toInt();
            int row=query.record().value("y").toInt();
            int matchId=query.record().value("matchId").toInt();
            secondImageUrls[cont]=imgUrl;
            secondKpColumns[cont]=column;
            secondKpRows[cont]=row;
            secondMatchIds[cont]=matchId;
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images and keypoints for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    if(!mDB.commit())
    {
        qCritical() << mDB.lastError();
        return PERSISTENCEMANAGER_ERROR;
    }

    // Inicializo los contenedores
    for(int i=0;i<firstImageUrls.size();i++)
    {
        QString firstImageUrl=firstImageUrls[i];
        QString secondImageUrl=secondImageUrls[i];
        if(activeImagesIds.size()>0)
            if(!activeImagesIds.contains(firstImageUrl)||!activeImagesIds.contains(secondImageUrl))
                continue;
        QString url=firstImageUrls.at(i);
        //        if(activeImagesIds.size()>0)
        //            if(!activeImagesIds.contains(url))
        //                continue;
        if(!imagesIdsTiePoints.contains(url))
        {
            QVector<QString> imageMatches;
            imagesIdsTiePoints[url]=imageMatches;
            QMap<QString,QVector<float> > imageFirstColumns;
            QMap<QString,QVector<float> > imageFirstRows;
            QMap<QString,QVector<float> > imageSecondColumns;
            QMap<QString,QVector<float> > imageSecondRows;
            QMap<QString,QVector<int> > matchesId;
            imagesFirstColumnsTiePoints[url]=imageFirstColumns;
            imagesFirstRowsTiePoints[url]=imageFirstRows;
            imagesSecondColumnsTiePoints[url]=imageSecondColumns;
            imagesSecondRowsTiePoints[url]=imageSecondRows;
            matchesIds[url]=matchesId;
        }
    }
    for(int i=0;i<firstImageUrls.size();i++)
    {
        QString firstImageUrl=firstImageUrls[i];
        QString secondImageUrl=secondImageUrls[i];
        if(activeImagesIds.size()>0)
            if(!activeImagesIds.contains(firstImageUrl)||!activeImagesIds.contains(secondImageUrl))
                continue;
        double firstKpColumn=factorConversionImageCoordinates*firstKpColumns[i];
        double firstKpRow=factorConversionImageCoordinates*firstKpRows[i];
        double secondKpColumn=factorConversionImageCoordinates*secondKpColumns[i];
        double secondKpRow=factorConversionImageCoordinates*secondKpRows[i];
        int matchId=firstMatchIds[i];
        if(!imagesIdsTiePoints[firstImageUrl].contains(secondImageUrl))
        {
            imagesIdsTiePoints[firstImageUrl].push_back(secondImageUrl);
            QVector<float> firstColumns;
            QVector<float> firstRows;
            QVector<float> secondColumns;
            QVector<float> secondRows;
            QVector<int> matchsId;
            firstColumns.push_back(firstKpColumn);
            firstRows.push_back(firstKpRow);
            secondColumns.push_back(secondKpColumn);
            secondRows.push_back(secondKpRow);
            matchsId.push_back(matchId);
            imagesFirstColumnsTiePoints[firstImageUrl][secondImageUrl]=firstColumns;
            imagesFirstRowsTiePoints[firstImageUrl][secondImageUrl]=firstRows;
            imagesSecondColumnsTiePoints[firstImageUrl][secondImageUrl]=secondColumns;
            imagesSecondRowsTiePoints[firstImageUrl][secondImageUrl]=secondRows;
            matchesIds[firstImageUrl][secondImageUrl]=matchsId;
        }
        else
        {
            imagesFirstColumnsTiePoints[firstImageUrl][secondImageUrl].push_back(firstKpColumn);
            imagesFirstRowsTiePoints[firstImageUrl][secondImageUrl].push_back(firstKpRow);
            imagesSecondColumnsTiePoints[firstImageUrl][secondImageUrl].push_back(secondKpColumn);
            imagesSecondRowsTiePoints[firstImageUrl][secondImageUrl].push_back(secondKpRow);
            matchesIds[firstImageUrl][secondImageUrl].push_back(matchId);
        }
    }
    return PERSISTENCEMANAGER_NO_ERROR;
}

bool PersistenceManager::getMatches(int projectId,
                                    QStringList descriptionMatchesCategories,
                                    QMap<QString, QVector<QString> > &imagesIdsTiePoints,
                                    QMap<QString, QMap<QString,QVector<double> > > &imagesFirstColumnsTiePoints,
                                    QMap<QString, QMap<QString,QVector<double> > > &imagesFirstRowsTiePoints,
                                    QMap<QString, QMap<QString,QVector<double> > >& imagesSecondColumnsTiePoints,
                                    QMap<QString, QMap<QString,QVector<double> > >& imagesSecondRowsTiePoints)
{
    if(descriptionMatchesCategories.isEmpty())
    {
        QString msg=tr("Error in PersistenceManager::getMatches: descriptionMatchesCategories is empty");
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }
    imagesIdsTiePoints.clear();
    imagesFirstColumnsTiePoints.clear();
    imagesFirstRowsTiePoints.clear();
    imagesSecondColumnsTiePoints.clear();
    imagesSecondRowsTiePoints.clear();
    mDB.transaction();
    QSqlQuery query(mDB);
    QString queryString="SELECT count(*) as size FROM pw_matches AS m,pw_matches_categories AS c WHERE c.id_project='";
    queryString+=QString::number(projectId);
    queryString+="' AND (";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString matchesCategory=descriptionMatchesCategories.at(i);
        queryString+="c.description='";
        queryString+=matchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    queryString+="AND c.id=m.id_matches_category";
    int numberOfMatches;
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            numberOfMatches=query.record().value("size").toInt();
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting number of images and keypoints ids for matches for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    QVector<QString> firstImageUrls(numberOfMatches);
    QVector<float> firstKpColumns(numberOfMatches);
    QVector<float> firstKpRows(numberOfMatches);
    QVector<QString> secondImageUrls(numberOfMatches);
    QVector<float> secondKpColumns(numberOfMatches);
    QVector<float> secondKpRows(numberOfMatches);

    //    queryString="SELECT i.url as imgUrl,ST_X(k.pixel_point_geom) as x,ST_Y(k.pixel_point_geom) as y FROM pw_matches AS m,";
    queryString="SELECT i.url as imgUrl,k.x as x, k.y as y FROM pw_matches AS m,";
    queryString+="pw_matches_categories AS c,pw_images as i,pw_keypoints as k WHERE c.id_project='";
    queryString+=QString::number(projectId);
    queryString+="' AND (";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString matchesCategory=descriptionMatchesCategories.at(i);
        queryString+="c.description='";
        queryString+=matchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    queryString+="AND c.id=m.id_matches_category AND k.id=m.id_keypoint_first AND i.id=k.id_image";
    if(query.prepare(queryString))
    {
        query.exec();
        int cont=-1;
        while(query.next())
        {
            cont++;
            QString imgUrl=query.record().value("imgUrl").toString();
            float column=query.record().value("x").toFloat();
            float row=query.record().value("y").toFloat();
            firstImageUrls[cont]=imgUrl;
            firstKpColumns[cont]=column;
            firstKpRows[cont]=row;
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images and keypoints for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    //    queryString="SELECT i.url as imgUrl,ST_X(k.pixel_point_geom) as x,ST_Y(k.pixel_point_geom) as y FROM pw_matches AS m,";
    queryString="SELECT i.url as imgUrl,k.x as x, k.y as y FROM pw_matches AS m,";
    queryString+="pw_matches_categories AS c,pw_images as i,pw_keypoints as k WHERE c.id_project='";
    queryString+=QString::number(projectId);
    queryString+="' AND (";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString matchesCategory=descriptionMatchesCategories.at(i);
        queryString+="c.description='";
        queryString+=matchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    queryString+="AND c.id=m.id_matches_category AND k.id=m.id_keypoint_second AND i.id=k.id_image";
    if(query.prepare(queryString))
    {
        query.exec();
        int cont=-1;
        while(query.next())
        {
            cont++;
            QString imgUrl=query.record().value("imgUrl").toString();
            float column=query.record().value("x").toFloat();
            float row=query.record().value("y").toFloat();
            secondImageUrls[cont]=imgUrl;
            secondKpColumns[cont]=column;
            secondKpRows[cont]=row;
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images and keypoints for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    if(!mDB.commit())
    {
        qCritical() << mDB.lastError();
        return PERSISTENCEMANAGER_ERROR;
    }

    // Inicializo los contenedores
    for(int i=0;i<firstImageUrls.size();i++)
    {
        QString url=firstImageUrls.at(i);
        if(!imagesIdsTiePoints.contains(url))
        {
            QVector<QString> imageMatches;
            imagesIdsTiePoints[url]=imageMatches;
        }
        QMap<QString,QVector<double> > imageFirstColumns;
        QMap<QString,QVector<double> > imageFirstRows;
        QMap<QString,QVector<double> > imageSecondColumns;
        QMap<QString,QVector<double> > imageSecondRows;
        imagesFirstColumnsTiePoints[url]=imageFirstColumns;
        imagesFirstRowsTiePoints[url]=imageFirstRows;
        imagesSecondColumnsTiePoints[url]=imageSecondColumns;
        imagesSecondRowsTiePoints[url]=imageSecondRows;
    }
    for(int i=0;i<firstImageUrls.size();i++)
    {
        QString firstImageUrl=firstImageUrls[i];
        QString secondImageUrl=secondImageUrls[i];
        float firstKpColumn=firstKpColumns[i];
        float firstKpRow=firstKpRows[i];
        float secondKpColumn=secondKpColumns[i];
        float secondKpRow=secondKpRows[i];
        if(!imagesIdsTiePoints[firstImageUrl].contains(secondImageUrl))
        {
            imagesIdsTiePoints[firstImageUrl].push_back(secondImageUrl);
            QVector<double> firstColumns;
            QVector<double> firstRows;
            QVector<double> secondColumns;
            QVector<double> secondRows;
            firstColumns.push_back(firstKpColumn);
            firstRows.push_back(firstKpRow);
            secondColumns.push_back(secondKpColumn);
            secondRows.push_back(secondKpRow);
            imagesFirstColumnsTiePoints[firstImageUrl][secondImageUrl]=firstColumns;
            imagesFirstRowsTiePoints[firstImageUrl][secondImageUrl]=firstRows;
            imagesSecondColumnsTiePoints[firstImageUrl][secondImageUrl]=secondColumns;
            imagesSecondRowsTiePoints[firstImageUrl][secondImageUrl]=secondRows;
        }
        else
        {
            imagesFirstColumnsTiePoints[firstImageUrl][secondImageUrl].push_back(firstKpColumn);
            imagesFirstRowsTiePoints[firstImageUrl][secondImageUrl].push_back(firstKpRow);
            imagesSecondColumnsTiePoints[firstImageUrl][secondImageUrl].push_back(secondKpColumn);
            imagesSecondRowsTiePoints[firstImageUrl][secondImageUrl].push_back(secondKpRow);
        }
    }
    return PERSISTENCEMANAGER_NO_ERROR;
}

bool PersistenceManager::getMatchingPoints(int projectId,
                                           QStringList descriptionMatchesCategories,
                                           int imageFirstID,
                                           int imageSecondID,
                                           QList<QVector<float>>& matching_coords)

{
    if(descriptionMatchesCategories.isEmpty())
    {
        QString msg=tr("Error in PersistenceManager::getMatches: descriptionMatchesCategories is empty");
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    double factorConversionImageCoordinates=1.0/pow(10.0,1.0*LIBPW_IMAGE_COORDINATES_PRECISION);

    //    mDB.transaction(); // Solo hay una consulta

    QSqlQuery query(mDB);
    //    QString queryString="SELECT ka.id, kb.id"
    //    QString queryString="SELECT ST_X(ka.pixel_point_geom) as x1,ST_Y(ka.pixel_point_geom) as y1,ST_X(kb.pixel_point_geom) as x2,ST_Y(kb.pixel_point_geom) as y2"

    QString queryString="SELECT m.column1,m.row1,m.column2,m.row2,m.accuracy"
            " FROM pw_matches as m,pw_matches_categories as c"
            " LEFT JOIN pw_images as i1 ON i1.id=m.id_image_first"
            " LEFT JOIN pw_images as i2 ON i1.id=m.id_image_second"
            " WHERE m.id_image_first=";
    queryString+= QString::number(imageFirstID);
    queryString+= " AND m.id_image_second=";
    queryString+= QString::number(imageSecondID);
    queryString+= " AND  c.id_project ='";
    queryString+= QString::number(projectId);
    queryString+="' AND (";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString matchesCategory=descriptionMatchesCategories.at(i);
        queryString+="c.description='";
        queryString+=matchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    queryString+="AND c.id=m.id_matches_category";
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QVector<float> points_pair(5);
            points_pair[0] = query.record().value("column1").toFloat()*factorConversionImageCoordinates;
            points_pair[1] = query.record().value("row1").toFloat()*factorConversionImageCoordinates;
            points_pair[2] = query.record().value("column2").toFloat()*factorConversionImageCoordinates;
            points_pair[3] = query.record().value("row2").toFloat()*factorConversionImageCoordinates;
            points_pair[4] = query.record().value("accuracy").toFloat()*factorConversionImageCoordinates;  //Accuracy
            matching_coords.append(points_pair);
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting matching points");
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }
    if(matching_coords.size()==0)
    {
        QList<QVector<float>> reverse_matching_coords;
        bool success=getMatchingPoints(projectId,descriptionMatchesCategories,
                                       imageSecondID,imageFirstID,reverse_matching_coords);
        for(int nP=0;nP<reverse_matching_coords.size();nP++)
        {
            QVector<float> coordinates;
            coordinates.push_back(reverse_matching_coords[nP][2]);
            coordinates.push_back(reverse_matching_coords[nP][3]);
            coordinates.push_back(reverse_matching_coords[nP][0]);
            coordinates.push_back(reverse_matching_coords[nP][1]);
            coordinates.push_back(reverse_matching_coords[nP][4]); //Accuracy????????????????
            matching_coords.append(coordinates);
        }
        return(success);
    }

    //    if(!mDB.commit())
    //    {
    //        qCritical() << mDB.lastError();
    //        return PERSISTENCEMANAGER_ERROR;
    //    }

    return PERSISTENCEMANAGER_NO_ERROR;
}

bool PersistenceManager::getMatchingPoints_20140821(int projectId,
                                                    QStringList descriptionMatchesCategories,
                                                    int imageFirstID,
                                                    int imageSecondID,
                                                    QList<QVector<float>>& matching_coords)

{
    if(descriptionMatchesCategories.isEmpty())
    {
        QString msg=tr("Error in PersistenceManager::getMatches: descriptionMatchesCategories is empty");
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    double factorConversionImageCoordinates=1.0/pow(10.0,1.0*LIBPW_IMAGE_COORDINATES_PRECISION);

    //    mDB.transaction(); // Solo hay una consulta

    QSqlQuery query(mDB);
    //    QString queryString="SELECT ka.id, kb.id"
    //    QString queryString="SELECT ST_X(ka.pixel_point_geom) as x1,ST_Y(ka.pixel_point_geom) as y1,ST_X(kb.pixel_point_geom) as x2,ST_Y(kb.pixel_point_geom) as y2"

    QString queryString="SELECT ka.x as x1,ka.y as y1,kb.x as x2,kb.y as y2"
            " FROM pw_matches_categories AS c,pw_matches as m"
            " LEFT JOIN pw_keypoints as ka ON ka.id=m.id_keypoint_first"
            " LEFT JOIN pw_keypoints as kb ON kb.id=m.id_keypoint_second"
            " WHERE ka.id_image=";
    queryString+= QString::number(imageFirstID);
    queryString+= " AND kb.id_image=";
    queryString+= QString::number(imageSecondID);
    queryString+= " AND  c.id_project ='";
    queryString+= QString::number(projectId);
    queryString+="' AND (";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString matchesCategory=descriptionMatchesCategories.at(i);
        queryString+="c.description='";
        queryString+=matchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    queryString+="AND c.id=m.id_matches_category";
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QVector<float> points_pair(4);
            points_pair[0] = query.record().value("x1").toFloat()*factorConversionImageCoordinates;
            points_pair[1] = query.record().value("y1").toFloat()*factorConversionImageCoordinates;
            points_pair[2] = query.record().value("x2").toFloat()*factorConversionImageCoordinates;
            points_pair[3] = query.record().value("y2").toFloat()*factorConversionImageCoordinates;

            matching_coords.append(points_pair);
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting matching points");
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }
    if(matching_coords.size()==0)
    {
        QList<QVector<float>> reverse_matching_coords;
        bool success=getMatchingPoints(projectId,descriptionMatchesCategories,
                                       imageSecondID,imageFirstID,reverse_matching_coords);
        for(int nP=0;nP<reverse_matching_coords.size();nP++)
        {
            QVector<float> coordinates;
            coordinates.push_back(reverse_matching_coords[nP][2]);
            coordinates.push_back(reverse_matching_coords[nP][3]);
            coordinates.push_back(reverse_matching_coords[nP][0]);
            coordinates.push_back(reverse_matching_coords[nP][1]);
            matching_coords.append(coordinates);
        }
        return(success);
    }

    //    if(!mDB.commit())
    //    {
    //        qCritical() << mDB.lastError();
    //        return PERSISTENCEMANAGER_ERROR;
    //    }

    return PERSISTENCEMANAGER_NO_ERROR;
}

bool PersistenceManager::getMatchesCategoriesImagePairs(int idProject,
                                                        QVector<int> idMatchesCategories,
                                                        bool twoWay,
                                                        QMap<QString, QVector<QString> > &imagesIdsPairs)
{
    if(idMatchesCategories.isEmpty())
    {
        QString msg=tr("Error in PersistenceManager::getMatches: idMatchesCategories is empty");
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }
    imagesIdsPairs.clear();

    //mDB.transaction();
    QSqlQuery query(mDB);
    QString queryString;

    // Obtenemos los url de todas las imagenes
    QMap<QString,QString> imageIdByIntIds;
    queryString="SELECT i.id as imgId,i.url as imgUrl from pw_images as i WHERE i.id_project='";
    queryString+=QString::number(idProject);
    queryString+="'";
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QString imgUrl=query.record().value("imgUrl").toString();
            QString imgId=query.record().value("imgId").toString();
            imageIdByIntIds[imgId]=imgUrl;
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images for project id:\n%1").arg(QString::number(idProject));
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    // Obtenemos los pares distintos
    queryString="SELECT DISTINCT m.id_image_first as imgId1,m.id_image_second as imgId2 FROM pw_matches as m ";
    queryString+="WHERE (";
    for(int i=0;i<idMatchesCategories.size();i++)
    {
        int idMatchesCategory=idMatchesCategories.at(i);
        queryString+="m.id_matches_category='";
        queryString+=QString::number(idMatchesCategory);
        if(i<(idMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QString imgIntId1=query.record().value("imgId1").toString();
            QString imgId1=imageIdByIntIds[imgIntId1];
            QString imgIntId2=query.record().value("imgId2").toString();
            QString imgId2=imageIdByIntIds[imgIntId2];
            if(!imagesIdsPairs.contains(imgId1))
            {
                QVector<QString> aux;
                aux.push_back(imgId2);
                imagesIdsPairs[imgId1]=aux;
            }
            else
            {
                imagesIdsPairs[imgId1].push_back(imgId2);
            }
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting second images for project id:\n%1").arg(QString::number(idProject));
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }
    //    if(!mDB.commit())
    //    {
    //        qCritical() << mDB.lastError();
    //        return PERSISTENCEMANAGER_ERROR;
    //    }

    // Los pongo en los dos sentidos
    if(twoWay)
    {
        QMap<QString, QVector<QString> > aux=imagesIdsPairs;
        QMap<QString, QVector<QString> >::const_iterator iter=aux.begin();
        while(iter!=aux.end())
        {
            QString firstImage=iter.key();
            QVector<QString> secondImages=iter.value();
            for(int nSi=0;nSi<secondImages.size();nSi++)
            {
                QString secondImage=secondImages[nSi];
                if(!imagesIdsPairs.contains(secondImage))
                {
                    QVector<QString> auxSecond;
                    imagesIdsPairs[secondImage]=auxSecond;
                }
                if(imagesIdsPairs[secondImage].indexOf(firstImage)==-1)
                {
                    imagesIdsPairs[secondImage].push_back(firstImage);
                }
            }
            iter++;
        }
    }
    return PERSISTENCEMANAGER_NO_ERROR;
}

bool PersistenceManager::getMatches_20140821(int projectId,
                                             QStringList descriptionMatchesCategories,
                                             QMap<QString, QVector<QString> > &imagesIdsPairs)
{
    if(descriptionMatchesCategories.isEmpty())
    {
        QString msg=tr("Error in PersistenceManager::getMatches: descriptionMatchesCategories is empty");
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }
    imagesIdsPairs.clear();

    mDB.transaction();
    QSqlQuery query(mDB);
    QString queryString;

    QStringList idMatchesCategories;
    queryString="SELECT id from pw_matches_categories WHERE (";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString matchesCategory=descriptionMatchesCategories.at(i);
        queryString+="metadata='";
        queryString+=matchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QString imgUrl=query.record().value("id").toString();
            idMatchesCategories.push_back(imgUrl);
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    QVector<QString> firstImageIds;
    queryString="SELECT i.url as imgUrl from pw_images as i WHERE i.id_project='";
    queryString+=QString::number(projectId);
    queryString+="'";
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            QString imgUrl=query.record().value("imgUrl").toString();
            firstImageIds.push_back(imgUrl);
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    for(int nI=0;nI<firstImageIds.size();nI++)
    {
        QString firstImgUrl=firstImageIds[nI];
        QVector<QString> secondImageUrls;
        queryString="SELECT DISTINCT i.url AS imgUrl FROM pw_images AS i,";
        queryString+="pw_matches_categories AS c,pw_keypoints AS k,pw_matches AS m ";
        queryString+="WHERE c.id_project='";
        queryString+=QString::number(projectId);
        queryString+="' AND (";
        for(int i=0;i<idMatchesCategories.size();i++)
        {
            QString idMatchesCategory=idMatchesCategories.at(i);
            queryString+="c.id='";
            queryString+=idMatchesCategory;
            if(i<(idMatchesCategories.size()-1))
            {
                queryString+="' OR ";
            }
            else
            {
                queryString+="') ";
            }
        }
        queryString+="AND m.id_keypoint_second=k.id AND i.id=k.id_image";
        queryString+=" AND (";
        for(int i=0;i<idMatchesCategories.size();i++)
        {
            QString idMatchesCategory=idMatchesCategories.at(i);
            queryString+="m.id_matches_category='";
            queryString+=idMatchesCategory;
            if(i<(idMatchesCategories.size()-1))
            {
                queryString+="' OR ";
            }
            else
            {
                queryString+="') ";
            }
        }
        queryString+=" AND m.id IN ";
        queryString+="(SELECT m.id FROM pw_matches_categories AS c,pw_matches AS m,";
        queryString+="pw_images AS i,pw_keypoints AS k WHERE c.id_project='";
        queryString+=QString::number(projectId);
        queryString+="' AND (";
        for(int i=0;i<idMatchesCategories.size();i++)
        {
            QString idMatchesCategory=idMatchesCategories.at(i);
            queryString+="c.id='";
            queryString+=idMatchesCategory;
            if(i<(idMatchesCategories.size()-1))
            {
                queryString+="' OR ";
            }
            else
            {
                queryString+="') ";
            }
        }
        queryString+="AND m.id_keypoint_first=k.id AND i.id=k.id_image AND i.url='";
        queryString+=firstImgUrl;
        queryString+="' AND (";
        for(int i=0;i<idMatchesCategories.size();i++)
        {
            QString idMatchesCategory=idMatchesCategories.at(i);
            queryString+="m.id_matches_category='";
            queryString+=idMatchesCategory;
            if(i<(idMatchesCategories.size()-1))
            {
                queryString+="' OR ";
            }
            else
            {
                queryString+="')";
            }
        }
        queryString+=")";
        if(query.prepare(queryString))
        {
            query.exec();
            while(query.next())
            {
                QString imgUrl=query.record().value("imgUrl").toString();
                secondImageUrls.push_back(imgUrl);
            }
        }
        else
        {
            QString msg=tr("PersistenceManager::getMatches error getting second images for project name:\n%1").arg(projectId);
            qCritical() << msg;
            return PERSISTENCEMANAGER_ERROR;
        }
        if(secondImageUrls.size()>0)
        {
            imagesIdsPairs[firstImgUrl]=secondImageUrls;
        }
    }
    if(!mDB.commit())
    {
        qCritical() << mDB.lastError();
        return PERSISTENCEMANAGER_ERROR;
    }
    // Los pongo en los dos sentidos
    QMap<QString, QVector<QString> > aux=imagesIdsPairs;
    QMap<QString, QVector<QString> >::const_iterator iter=aux.begin();
    while(iter!=aux.end())
    {
        QString firstImage=iter.key();
        QVector<QString> secondImages=iter.value();
        for(int nSi=0;nSi<secondImages.size();nSi++)
        {
            QString secondImage=secondImages[nSi];
            if(!imagesIdsPairs.contains(secondImage))
            {
                QVector<QString> auxSecond;
                imagesIdsPairs[secondImage]=auxSecond;
            }
            if(imagesIdsPairs[secondImage].indexOf(firstImage)==-1)
            {
                imagesIdsPairs[secondImage].push_back(firstImage);
            }
        }
        iter++;
    }
    return PERSISTENCEMANAGER_NO_ERROR;
}

bool PersistenceManager::setMatchesCategories(Project *project)
{
    QStringList metadataMatchesCategories,descriptionMatchesCategories,timeCalculationMatchesCategories;
    QVector<int> idMatchesCategories;
    int idProject=project->getID();
    QVector<QString> imagesFileName=project->getImagesFileName();
    if(PERSISTENCEMANAGER_ERROR==getMatchesCategories(idProject,
                                                      idMatchesCategories,
                                                      metadataMatchesCategories,
                                                      descriptionMatchesCategories,
                                                      timeCalculationMatchesCategories))
    {
        return PERSISTENCEMANAGER_ERROR;
    }
    for(int nC=0;nC<idMatchesCategories.size();nC++)
    {
        QString description=descriptionMatchesCategories.at(nC);
        if(project->getMatchesCategory(description)!=NULL)
            continue;
        int id=idMatchesCategories.at(nC);
        QString metadata=metadataMatchesCategories.at(nC);
        QString timeCalculation=timeCalculationMatchesCategories.at(nC);
        PWMatchesCategory* matchesCategory=new PWMatchesCategory(project,id,description,metadata,timeCalculation);
        PWGraphImages* graphImages=new PWGraphImages();
        QMap<QString, QVector<QString> > imagePairs;
        QVector<int> auxIdMatchesCategories;
        auxIdMatchesCategories.push_back(id);
        bool twoWay=true;
        if(PERSISTENCEMANAGER_ERROR==getMatchesCategoriesImagePairs(idProject,auxIdMatchesCategories,twoWay,imagePairs))
            return(false);
        if(!graphImages->createGraph(imagesFileName,
                                     imagePairs))
        {
            continue;//graphIsConnected=false; ¿Que hacer?
        }
        matchesCategory->setGraphImages(graphImages);
        matchesCategory->setImagePairs(imagePairs);
        project->insertMatchesCategory(matchesCategory);
    }
    return PERSISTENCEMANAGER_NO_ERROR;
}

bool PersistenceManager::deleteMatches(QList<int> imagesIdList)
{
    QString imagesIdString="";
    int listSize = 0;
    for (int i=0; i<imagesIdList.size();i++){
        imagesIdString = imagesIdString + QString::number(imagesIdList.at(i)) + ",";
        listSize++;
        if (i==imagesIdList.size()-1 && listSize>0 )
            imagesIdString=imagesIdString.left(imagesIdString.size()-1);
    }
    mDB.transaction();
    QSqlQuery query(mDB);

    //Delete camera_models of deleted images:
    QString queryString =
            "DELETE FROM pw_matches"
            " WHERE id_image_first IN ("+imagesIdString+") OR id_image_second IN ("+imagesIdString+")";
    if(query.prepare(queryString)){
        query.exec();
    }
    else{
        qCritical() << mDB.lastError();
        return 1;
    }

    //Delete empty categories
    queryString =
            "Delete FROM pw_matches_categories WHERE id NOT IN (SELECT DISTINCT id_matches_category FROM pw_matches)";
    if(query.prepare(queryString)){
        query.exec();
    }
    else{
        qCritical() << mDB.lastError();
        return 1;
    }

    if(!mDB.commit())
    {
        qCritical() << mDB.lastError();
        return PERSISTENCEMANAGER_ERROR;
    }
    return PERSISTENCEMANAGER_NO_ERROR;
}

bool PersistenceManager::writeMatchesAccuracies(QMap<QString, QMap<QString, QVector<double> > > &orientationImagesFirstColumnsTiePoints,
                                                QMap<QString, QMap<QString, QVector<double> > > &orientationImagesFirstRowsTiePoints,
                                                QMap<QString, QMap<QString, QVector<double> > > &orientationImagesSecondColumnsTiePoints,
                                                QMap<QString, QMap<QString, QVector<double> > > &orientationImagesSecondRowsTiePoints,
                                                QMap<QString, QMap<QString, QVector<int> > > &orientationMatchesIds,
                                                QMap<QString, QMap<QString, QVector<double> > >& orientationMatchesAccuracies)
{
    mDB.transaction();
    QSqlQuery query(mDB);
    QString queryString;
    double factorConversionFromImageCoordinates=pow(10.0,1.0*LIBPW_IMAGE_COORDINATES_PRECISION);

    bool useUnion=false;
    if(useUnion)
    {
        /*
        QString fileName="e:/kk_insert.sql";
        QFile sqlFile(fileName);
        if(QFile::exists(fileName))
            QFile::remove(fileName);
        sqlFile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&sqlFile);
        QMap<QString, QMap<QString, QVector<int> > >::const_iterator iter1=orientationMatchesIds.begin();
        int maxInsertsInUnion=PERSISTENCEMANAGER_MAX_INSERTS_IN_UNION;
        int numberOfInserts=0;
        queryString="INSERT INTO pw_matches_filtered(id,column1,row1,column2,row2,accuracy)\n";
        bool isFirstInsert=true;
        while(iter1!=orientationMatchesIds.end())
        {
            QString firstImageId=iter1.key();
            QMap<QString, QVector<int> > orientationMatchesSeconds=iter1.value();
            QMap<QString, QVector<int> >::const_iterator iter2=orientationMatchesSeconds.begin();
            while(iter2!=orientationMatchesSeconds.end())
            {
                QString secondImageId=iter2.key();
                QVector<int> matchesIds=iter2.value();
                for(int nM=0;nM<matchesIds.size();nM++)
                {
                    int id=matchesIds[nM];
                    float floatAccuracy=orientationMatchesAccuracies[firstImageId][secondImageId].at(nM);
                    int intAccuracy=(int)(factorConversionFromImageCoordinates*floatAccuracy);
                    float floatFirstImageColumn=orientationImagesFirstColumnsTiePoints[firstImageId][secondImageId].at(nM);
                    float floatFirstImageRow=orientationImagesFirstRowsTiePoints[firstImageId][secondImageId].at(nM);
                    float floatSecondImageColumn=orientationImagesSecondColumnsTiePoints[firstImageId][secondImageId].at(nM);
                    float floatSecondImageRow=orientationImagesSecondRowsTiePoints[firstImageId][secondImageId].at(nM);
                    int column1=(int)(factorConversionFromImageCoordinates*floatFirstImageColumn);
                    int row1=(int)(factorConversionFromImageCoordinates*floatFirstImageRow);
                    int column2=(int)(factorConversionFromImageCoordinates*floatSecondImageColumn);
                    int row2=(int)(factorConversionFromImageCoordinates*floatSecondImageRow);
    //                queryString="INSERT pw_matches_filtered (id,column1,row1,column2,row2,accuracy) VALUES (";
                    if(!isFirstInsert)
                    {
                        queryString+="UNION\n";
                    }
                    else
                        isFirstInsert=false;
                    queryString+="SELECT \"";
                    queryString+=QString::number(id);
                    queryString+="\",\"";
                    queryString+=QString::number(column1);
                    queryString+="\",\"";
                    queryString+=QString::number(row1);
                    queryString+="\",\"";
                    queryString+=QString::number(column2);
                    queryString+="\",\"";
                    queryString+=QString::number(row2);
                    queryString+="\",\"";
                    queryString+=QString::number(intAccuracy);
                    queryString+="\"\n";
                    numberOfInserts++;
                    if(numberOfInserts>=maxInsertsInUnion)
                    {
                        numberOfInserts=0;
                        out<<queryString<<"\n";
                        if(query.prepare(queryString))
                        {
                            query.exec();
                        }
                        else
                        {
                            QString msg=tr("PersistenceManager::writeMatchesAccuracies error writting accuracies");
                            qCritical() << msg;
                            return PERSISTENCEMANAGER_ERROR;
                        }
                        queryString="INSERT INTO pw_matches_filtered(id,column1,row1,column2,row2,accuracy)\n";
                        isFirstInsert=true;
                    }
                }
    //            if(query.prepare(queryString))
    //            {
    //                query.exec();
    //            }
    //            else
    //            {
    //                QString msg=tr("PersistenceManager::writeMatchesAccuracies error writting matches accuracies");
    //                qCritical() << msg;
    //                return PERSISTENCEMANAGER_ERROR;
    //            }
                iter2++;
            }
            iter1++;
        }
        if(numberOfInserts>0)
        {
            out<<queryString<<"\n";
            if(query.prepare(queryString))
            {
                query.exec();
            }
            else
            {
                QString msg=tr("PersistenceManager::writeMatchesAccuracies error writting accuracies");
                qCritical() << msg;
                return PERSISTENCEMANAGER_ERROR;
            }
        }
        sqlFile.close();
        */
    }
    else
    {
        queryString="UPDATE pw_matches SET accuracy = :strValue WHERE id = :id";
        if(!query.prepare(queryString))
        {
            QString msg=tr("PersistenceManager::writeMatchesAccuracies error writting accuracy")
                    + ": " + mDB.lastError().text();
            qCritical() << msg;
            return PERSISTENCEMANAGER_ERROR;
        }

        QMap<QString, QMap<QString, QVector<int> > >::const_iterator iter1=orientationMatchesIds.begin();
        while(iter1!=orientationMatchesIds.end())
        {
            QString firstImageId=iter1.key();
            QMap<QString, QVector<int> > orientationMatchesSeconds=iter1.value();
            QMap<QString, QVector<int> >::const_iterator iter2=orientationMatchesSeconds.begin();
            while(iter2!=orientationMatchesSeconds.end())
            {
                QString secondImageId=iter2.key();
                QVector<int> matchesIds=iter2.value();
                for(int nM=0;nM<matchesIds.size();nM++)
                {
                    int id=matchesIds[nM];
                    float floatAccuracy=orientationMatchesAccuracies[firstImageId][secondImageId].at(nM);
                    int intAccuracy=(int)(factorConversionFromImageCoordinates*floatAccuracy);
                    QString strValue=QString::number(intAccuracy);
//                    queryString="UPDATE pw_matches SET accuracy='";
//                    queryString+=strValue;
//                    queryString+="' WHERE id='";
//                    queryString+=QString::number(id);
//                    queryString+="';";
                    query.bindValue(":strValue",strValue);
                    query.bindValue(":id",QString::number(id));

                    query.exec();
                }
                iter2++;
            }
            iter1++;
        }
    }
    if(!mDB.commit())
    {
        qCritical() << mDB.lastError();
        return PERSISTENCEMANAGER_ERROR;
    }
    return PERSISTENCEMANAGER_NO_ERROR;
}

/*

bool PersistenceManager::getMatches(int projectId, QStringList descriptionMatchesCategories, QMap<QString, QVector<QString> > &imagesIdsPairs)
{
    if(descriptionMatchesCategories.isEmpty())
    {
        QString msg=tr("Error in PersistenceManager::getMatches: descriptionMatchesCategories is empty");
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }
    imagesIdsPairs.clear();

    mDB.transaction();
    QSqlQuery query(mDB);
    QString queryString="SELECT count(*) as size FROM pw_matches AS m,pw_matches_categories AS c WHERE c.id_project='";
    queryString+=QString::number(projectId);
    queryString+="' AND (";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString matchesCategory=descriptionMatchesCategories.at(i);
        queryString+="c.description='";
        queryString+=matchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    queryString+="AND c.id=m.id_matches_category";
    int numberOfMatches;
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            numberOfMatches=query.record().value("size").toInt();
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting number of images and keypoints ids for matches for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    QVector<QString> firstImageUrls(numberOfMatches);
    QVector<QString> secondImageUrls(numberOfMatches);

    queryString="SELECT i.url as imgUrl FROM pw_matches AS m,";
    queryString+="pw_matches_categories AS c,pw_images as i,pw_keypoints as k WHERE c.id_project='";
    queryString+=QString::number(projectId);
    queryString+="' AND (";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString matchesCategory=descriptionMatchesCategories.at(i);
        queryString+="c.description='";
        queryString+=matchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    queryString+="AND c.id=m.id_matches_category AND k.id=m.id_keypoint_first AND i.id=k.id_image";
    if(query.prepare(queryString))
    {
        query.exec();
        int cont=-1;
        while(query.next())
        {
            cont++;
            QString imgUrl=query.record().value("imgUrl").toString();
            firstImageUrls[cont]=imgUrl;
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images and keypoints for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    queryString="SELECT i.url as imgUrl FROM pw_matches AS m,";
    queryString+="pw_matches_categories AS c,pw_images as i,pw_keypoints as k WHERE c.id_project='";
    queryString+=QString::number(projectId);
    queryString+="' AND (";
    for(int i=0;i<descriptionMatchesCategories.size();i++)
    {
        QString matchesCategory=descriptionMatchesCategories.at(i);
        queryString+="c.description='";
        queryString+=matchesCategory;
        if(i<(descriptionMatchesCategories.size()-1))
        {
            queryString+="' OR ";
        }
        else
        {
            queryString+="') ";
        }
    }
    queryString+="AND c.id=m.id_matches_category AND k.id=m.id_keypoint_second AND i.id=k.id_image";
    if(query.prepare(queryString))
    {
        query.exec();
        int cont=-1;
        while(query.next())
        {
            cont++;
            QString imgUrl=query.record().value("imgUrl").toString();
            secondImageUrls[cont]=imgUrl;
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::getMatches error getting first images and keypoints for project name:\n%1").arg(projectId);
        qCritical() << msg;
        return PERSISTENCEMANAGER_ERROR;
    }

    if(!mDB.commit())
    {
        qCritical() << mDB.lastError();
        return PERSISTENCEMANAGER_ERROR;
    }

    // Inicializo los contenedores
    for(int i=0;i<firstImageUrls.size();i++)
    {
        QString url=firstImageUrls.at(i);
        if(!imagesIdsPairs.contains(url))
        {
            QVector<QString> imageMatches;
            imagesIdsPairs[url]=imageMatches;
        }
    }
    for(int i=0;i<firstImageUrls.size();i++)
    {
        QString firstImageUrl=firstImageUrls[i];
        QString secondImageUrl=secondImageUrls[i];
        if(!imagesIdsPairs[firstImageUrl].contains(secondImageUrl))
        {
            imagesIdsPairs[firstImageUrl].push_back(secondImageUrl);
        }
    }
    return PERSISTENCEMANAGER_NO_ERROR;
}
*/

// DHL_20140405
int PW::PersistenceManager::
writeProject(PW::Project *project, bool writeCams, bool writeMatchesData, int previousProyectID)
{
    mDB.transaction();
    QSqlQuery query(mDB);

    //Delete matches categories in the project:

//    QString queryString =
//            "DELETE FROM pw_matches_categories WHERE id_project = " +
//            QString::number(project->getID());
//    if(query.prepare(queryString)){
//        query.exec();
//    }
//    else{
//        qCritical() << mDB.lastError();
//        return 1;
//    }

    //Delete camera_models in the project:

    QString queryString =
            "DELETE FROM pw_camera_models WHERE id IN (SELECT pw_images.id_active_model FROM pw_images, pw_projects WHERE pw_projects.id = pw_images.id_project AND pw_projects.name = '" +
            project->getName() + "')";
    if(query.prepare(queryString)){
        query.exec();
    }
    else{
        qCritical() << mDB.lastError();
        return 1;
    }

    //Delete Porject if exists:
    queryString = "DELETE FROM pw_projects WHERE name = '" + project->getName() + "'";
    if(query.prepare(queryString)){
        query.exec();
    }
    else{
        qCritical() << mDB.lastError();
        return 1;
    }

    //Writting Project:
    queryString = "INSERT INTO pw_projects (name, description, base_path, sparse_path, dense_path, ori_name, match_format, metadata, tiepoints_name, cloud_uuid, pre_processing_path, capture_type) VALUES ('" +
            project->getName() + "','" +
            project->getDescription() + "','" +
            project->getBasePath() + "','" +
            project->getSparseModelRelativePath() + "','" +
            project->getDenseModelRelativePath() + "','" +
            project->getCurrentOrientation() + "'," +
            QString::number(project->getMatchingFormat()) + ",'" +
            project->getProcessMetadata() + "','" +
            project->getActiveTiePointsSet() + "','" +
            project->getCloudUUID() + "','" +
            project->getPreProcessingPath() + "','" +
            project->getCaptureType() + "'" +
            ")";

    int projectID;
    if(query.prepare(queryString)){
        query.exec();
        projectID = query.lastInsertId().toInt();
        project->setID(projectID);
    }
    else{
        qCritical() << mDB.lastError();
        return 1;
    }

    QMap<CameraModel *, int> writtenModels;

    for(int i=0; i<project->getImages().count(); i++){

        PWImage *image = project->getImages().at(i);

        //Getting Camera id:
        Camera *camera = image->getCamera();
        int cameraID = 0;
        if (camera != NULL){
            queryString = "SELECT * FROM pw_cameras WHERE name = '" + image->getCamera()->getName() + "'";
            if(query.prepare(queryString)){
                query.exec();
                if(query.next()){
                    cameraID = query.record().value("id").toInt();
                }
                else if (writeCams)
                    cameraID = writeCamera(camera);
            }
            else{
                qCritical() << mDB.lastError();
                return 1;
            }
        }

        //Writting camera models:
        PhotogrammetricModel * cameraModel = (PhotogrammetricModel *)image->getCamera()->getCameraModel();
        int modelID = writtenModels.value(cameraModel);
        if(!modelID){
            /*TODO: Generalizar a otros modelos*/
            if(cameraModel != NULL){
                //Writting Camera Model:
                queryString = "INSERT INTO pw_camera_models (focal, xp, yp, id_camera) VALUES (" +
                        QString::number(cameraModel->getFocal(),'f',15) + "," +
                        QString::number(cameraModel->getXp(),'f',15) + "," +
                        QString::number(cameraModel->getYp(),'f',15) + "," +
                        QString::number(cameraID) + ")";
                if(query.prepare(queryString)){
                    query.exec();
                    modelID = query.lastInsertId().toInt();
                    writtenModels.insert(cameraModel, modelID);
                }
                else{
                    qCritical() << mDB.lastError();
                    return 1;
                }

                /*TODO: Generalizar a otros modelos*/

                DistortionModel *distortion = cameraModel->getDistortionModel();
                RadialExtended *extended = dynamic_cast<RadialExtended*>(distortion);

                if(extended != NULL){ //Radial Extended

                    QString kn = QString::number(extended->getK1(),'f',15) + " " +
                            QString::number(extended->getK2(),'g',15) + " " +
                            QString::number(extended->getK3(),'f',15) + " " +
                            QString::number(extended->getK4(),'f',15) + " " +
                            QString::number(extended->getK5(),'f',15);

                    queryString = "INSERT INTO pw_distortions (xcd, ycd, kn, id_camera_model) VALUES (" +
                            QString::number(extended->getXcd(),'f',15) + "," +
                            QString::number(extended->getYcd(),'f',15) + "," +
                            "'" + kn + "',"
                            + QString::number(modelID) +")";

                }else{
                    RadialBasic *basic = dynamic_cast<RadialBasic*>(distortion);
                    if(basic != NULL){
                        QString kn = QString::number(basic->getK1(),'f',15) + " " +
                                QString::number(basic->getK2(),'g',15);

                        queryString = "INSERT INTO pw_distortions (kn, id_camera_model) VALUES (" +
                                QString("'") + kn + "',"
                                + QString::number(modelID) +")";
                    }else{
                        FraserModel *fraser = dynamic_cast<FraserModel*>(distortion);
                        if(fraser != NULL){
                            QString coefs = QString::number(fraser->getK1(),'g',15) + " " +
                                    QString::number(fraser->getK2(),'g',15) + " " +
                                    QString::number(fraser->getK3(),'g',15) + " " +
                                    QString::number(fraser->getP1(),'g',15) + " " +
                                    QString::number(fraser->getP2(),'g',15) + " " +
                                    QString::number(fraser->getB1(),'g',15) + " " +
                                    QString::number(fraser->getB2(),'g',15);

                            queryString = "INSERT INTO pw_distortions (xcd, ycd, kn, id_camera_model) VALUES (" +
                                    QString::number(fraser->getXcd(),'f',15) + "," +
                                    QString::number(fraser->getYcd(),'f',15) + "," +
                                    "'" + coefs + "',"
                                    + QString::number(modelID) +")";
                        }
                        else{
                            FishEyeModel *fishEye = dynamic_cast<FishEyeModel*>(distortion);
                            if(fishEye != NULL){
                                QString params;
                                for (int i=0; i<fishEye->getParamsCount();i++)
                                    params = params+QString::number(fishEye->getParam(i))+" ";
                                queryString = "INSERT INTO pw_distortions (xcd, ycd, kn, id_camera_model) VALUES (" +
                                        QString::number(fishEye->getXcd(),'f',15) + "," +
                                        QString::number(fishEye->getYcd(),'f',15) + "," +
                                        "'" + params + "',"
                                        + QString::number(modelID) +")";
                            }
                        }
                    }
                }

                if(query.prepare(queryString)){
                    query.exec();
                }
                else{
                    qCritical() << mDB.lastError();
                    return 1;
                }
            }
        }

        //Writting Image:
        if(modelID)
            queryString = "INSERT INTO pw_images (url, id_project, id_camera, id_active_model) VALUES ('" +
                    image->getFileName() + "'," +
                    QString::number(projectID) + "," +
                    QString::number(cameraID) + "," +
                    QString::number(modelID) +
                    ")";
        else
            queryString = "INSERT INTO pw_images (url, id_project, id_camera) VALUES ('" +
                    image->getFileName() + "'," +
                    QString::number(projectID) + "," +
                    QString::number(cameraID) +
                    ")";

        int imageID;
        if(query.prepare(queryString)){
            query.exec();
            imageID = query.lastInsertId().toInt();
            image->setID(imageID);
        }
        else{
            qCritical() << mDB.lastError();
            return 1;
        }

        //Writting External orientation:
        ExteriorOrientation *exteriorOrientation = image->getExteriorOrientation();
        if(exteriorOrientation != NULL){
            QString r = "";
            for (int f=0; f < exteriorOrientation->getR()->count(); f++)
                for (int c = 0; c<exteriorOrientation->getR()->at(f).count(); c++)
                    r += QString::number(exteriorOrientation->getR()->at(f).at(c),'f',15) + ",";
            r = r.left(r.count()-1);
            queryString = "INSERT INTO pw_ext_orient (xcp, ycp, zcp, r, id_image) VALUES (" +
                    QString::number(exteriorOrientation->getCP()->at(0),'f',15) + "," +
                    QString::number(exteriorOrientation->getCP()->at(1),'f',15) + "," +
                    QString::number(exteriorOrientation->getCP()->at(2),'f',15) + ",'" +
                    r + "'," +
                    QString::number(imageID, 'f',15) + ")";

            if(query.prepare(queryString)){
                query.exec();
            }
            else{
                qCritical() << mDB.lastError();
                return 1;
            }
        }

        //Writting Mask:
        /*TODO: Múltiples máscaras*/
        QPolygonF mask = image->getMask();
        if(mask.count()>0){
            QString wktMask = "";
            for (int p = 0; p<mask.count(); p++){
                wktMask += QString::number(mask.at(p).x()) + " " + QString::number(mask.at(p).y()) + ",";
            }
            wktMask = wktMask.left(wktMask.count()-1);
            queryString = "INSERT INTO pw_masks (wkt_poligon, id_image) VALUES ('" +
                    wktMask + "'," +
                    QString::number(imageID) + ")";

            int maskID;
            if(query.prepare(queryString)){
                query.exec();
                maskID = query.lastInsertId().toInt();
            }
            else{
                qCritical() << mDB.lastError();
                return 1;
            }
        }

        //Writting Image Points:
        for(int ip=0; ip< image->getControlPoints()->count(); ip++){
            PW2dPoint *imagePoint = image->getControlPoints()->at(ip);
            queryString = "INSERT INTO pw_image_points (x, y, id_image, name) VALUES (" +
                    QString::number(imagePoint->x(),'f',5) + "," +
                    QString::number(imagePoint->y(),'f',5) + "," +
                    QString::number(imageID) + ",'" +
                    imagePoint->getName() + "')";
            if(query.prepare(queryString)){
                query.exec();
            }
            else{
                qCritical() << mDB.lastError();
                return 1;
            }
        }
    }

    //Writting Terrain Points:
    for(int tp=0; tp< project->getModelCPoints()->count(); tp++){
        PW3dPoint *terrainPoint = project->getModelCPoints()->at(tp);
        queryString = "INSERT INTO pw_terrain_points (x, y, z, id_project,name) VALUES (" +
                QString::number(terrainPoint->x(),'f',5) + "," +
                QString::number(terrainPoint->y(),'f',5) + "," +
                QString::number(terrainPoint->z(),'f',5) + "," +
                QString::number(projectID) + ",'" +
                terrainPoint->getName() + "')";

        if(query.prepare(queryString)){
            query.exec();
        }
        else{
            qCritical() << mDB.lastError();
            return 1;
        }
    }

    if(!mDB.commit()){
        qCritical() << mDB.lastError();
        return 1;
    }    

    //Writing mateches categories:
    writeMatchesCategories(project);
//    QList <PWMatchesCategory *> categories = project->getMatchesCategories();
//    for(int i=0; i< categories.count();i ++){
//        writeMatchesCategory(categories.at(i));
//    }


    //Copy matches:
//    if (previousProyectID && writeMatchesData)
//    {

////        QStringList matchesMetadatas;
////        QStringList matchesDescriptions;
////        QStringList matchesTimes;

//        QStringList metadataMatchesCategories,descriptionMatchesCategories,timeCalculationMatchesCategories;
//        QVector<bool> validityMatchesCategories;
//        project->getMatchesCategoriesInformation(metadataMatchesCategories,
//                                                  descriptionMatchesCategories,
//                                                  timeCalculationMatchesCategories,
//                                                  validityMatchesCategories);

////        getMatchesCategories(previousProyectID,
////                             matchesMetadatas,
////                             matchesDescriptions,
////                             matchesTimes);

//        QList<PWImage*> images=project->getImages();
//        QMap<QString,int> imagesIds;
//        for(int nI=0;nI<images.size();nI++)
//        {
//            QString image=images[nI]->getFileName();
//            int imageId=images[nI]->getID();
//            imagesIds[image]=imageId;
//        }

//        for(int i = 0; i<metadataMatchesCategories.count(); i++){

//            QMap<QString,QVector<QString> > idsTiePoints;
//            QMap<QString,QMap<QString,QVector<float> > > firstColumnsTiePoints;
//            QMap<QString,QMap<QString,QVector<float> > > firstRowsTiePoints;
//            QMap<QString,QMap<QString,QVector<float> > > secondColumnsTiePoints;
//            QMap<QString,QMap<QString,QVector<float> > > secondRowsTiePoints;
//            QMap<QString,QMap<QString,QVector<int> > > matchesIds;

//            QStringList description;
//            description << descriptionMatchesCategories.at(i);

//            if(PERSISTENCEMANAGER_NO_ERROR==getMatches(previousProyectID,
//                                                       description,
//                                                       imagesIds,
//                                                       idsTiePoints,
//                                                       firstColumnsTiePoints,
//                                                       firstRowsTiePoints,
//                                                       secondColumnsTiePoints,
//                                                       secondRowsTiePoints,
//                                                       matchesIds
//                                                       )){
//                writeMatches(project,
//                             metadataMatchesCategories.at(i),
//                             descriptionMatchesCategories.at(i),
//                             timeCalculationMatchesCategories.at(i),
//                             idsTiePoints,
//                             firstColumnsTiePoints,
//                             firstRowsTiePoints,
//                             secondColumnsTiePoints,
//                             secondRowsTiePoints
//                             );
//            }
//        }

//    }

    project->setNeedToSave(false);
    return 0;
}

//TODO: write image pairs.
int PersistenceManager::updateProject(Project *project)
{
    QString imagesIdList="";
    int listSize = 0;
    for (int i=0; i<project->getImages().count();i++){
        if(project->getImages().at(i)->getID()){
            imagesIdList = imagesIdList + QString::number(project->getImages().at(i)->getID()) + ",";
            listSize++;
        }
        if (i==project->getImages().count()-1 && listSize>0 )
            imagesIdList=imagesIdList.left(imagesIdList.size()-1);
    }
    mDB.transaction();
    QSqlQuery query(mDB);

    //Delete camera_models of deleted images:
    QString queryString =
            "DELETE FROM pw_camera_models"
            " WHERE id IN (SELECT pw_images.id_active_model FROM pw_images, pw_projects"
            " WHERE pw_projects.id = pw_images.id_project AND pw_projects.id = " +
            QString::number(project->getID()) + " AND pw_images.id NOT IN ("+imagesIdList+"))";
    if(query.prepare(queryString)){
        query.exec();
    }
    else{
        qCritical() << mDB.lastError();
        return 1;
    }

    //Deleting images deleted in project:
    queryString = "DELETE FROM pw_images WHERE id_project="+QString::number(project->getID()) +
            " AND id NOT IN ("+imagesIdList+")";
    if(query.prepare(queryString)){
        query.exec();
    }
    else{
        qCritical() << mDB.lastError();
        return 1;
    }

    //Deleting terrain points:
    queryString = "DELETE FROM pw_terrain_points WHERE id_project="+QString::number(project->getID());
    if(query.prepare(queryString)){
        query.exec();
    }
    else{
        qCritical() << mDB.lastError();
        return 1;
    }

    //Updating Project:
    queryString = "UPDATE pw_projects SET name='"+
            project->getName() + "'," +
            "description='"+project->getDescription() + "'," +
            "base_path='"+project->getBasePath() + "'," +
            "sparse_path='"+project->getSparseModelRelativePath() + "'," +
            "dense_path='"+project->getDenseModelRelativePath() + "'," +
            "ori_name='"+project->getCurrentOrientation() + "'," +
            "match_format="+QString::number(project->getMatchingFormat()) + "," +
            "metadata='"+project->getProcessMetadata() + "'," +
            "tiepoints_name='"+project->getActiveTiePointsSet() + "'," +
            "cloud_uuid='"+project->getCloudUUID() + "'," +
            "pre_processing_path='" + project->getPreProcessingPath() + "'," +
            "capture_type='" + project->getCaptureType() + "'" +
            " WHERE id=" + QString::number(project->getID());

    if(query.prepare(queryString)){
        query.exec();
    }
    else{
        qCritical() << mDB.lastError();
        return 1;
    }
    int projectID = project->getID();

    QMap<CameraModel *, int> writtenModels;
    for(int i=0; i<project->getImages().count(); i++){

        PWImage *image = project->getImages().at(i);

        bool update = true;
        if(image->getID()==0)
            update = false;

        //Getting Camera id:
        Camera *camera = image->getCamera();
        int cameraID = 0;
        if (camera != NULL){
            queryString = "SELECT * FROM pw_cameras WHERE name = '" + image->getCamera()->getName() + "'";
            if(query.prepare(queryString)){
                query.exec();
                if(query.next())
                    cameraID = query.record().value("id").toInt();
            }
            else{
                qCritical() << mDB.lastError();
                return 1;
            }
        }

        //Writting camera models:
        if(update)
            queryString = "DELETE FROM pw_camera_models WHERE id IN"
                    "(SELECT id_active_model FROM pw_images WHERE id = "+QString::number(image->getID())+ ")";
        if(query.prepare(queryString)){
            query.exec();
        }
        else{
            qCritical() << mDB.lastError();
            return 1;
        }

        PhotogrammetricModel * cameraModel = (PhotogrammetricModel *)image->getCamera()->getCameraModel();
        int modelID = writtenModels.value(cameraModel);
        if(!modelID){
            /*TODO: Generalizar a otros modelos*/
            if(cameraModel != NULL){
                //Writting Camera Model:
                queryString = "INSERT INTO pw_camera_models (focal, xp, yp, id_camera) VALUES (" +
                        QString::number(cameraModel->getFocal(),'f',15) + "," +
                        QString::number(cameraModel->getXp(),'f',15) + "," +
                        QString::number(cameraModel->getYp(),'f',15) + "," +
                        QString::number(cameraID) + ")";
                if(query.prepare(queryString)){
                    query.exec();
                    modelID = query.lastInsertId().toInt();
                    writtenModels.insert(cameraModel, modelID);
                }
                else{
                    qCritical() << mDB.lastError();
                    return 1;
                }

                /*TODO: Generalizar a otros modelos*/

                DistortionModel *distortion = cameraModel->getDistortionModel();
                RadialExtended *extended = dynamic_cast<RadialExtended*>(distortion);

                if(extended != NULL){ //Radial Extended

                    QString kn = QString::number(extended->getK1(),'f',15) + " " +
                            QString::number(extended->getK2(),'g',15) + " " +
                            QString::number(extended->getK3(),'f',15) + " " +
                            QString::number(extended->getK4(),'f',15) + " " +
                            QString::number(extended->getK5(),'f',15);

                    queryString = "INSERT INTO pw_distortions (xcd, ycd, kn, id_camera_model) VALUES (" +
                            QString::number(extended->getXcd(),'f',15) + "," +
                            QString::number(extended->getYcd(),'f',15) + "," +
                            "'" + kn + "',"
                            + QString::number(modelID) +")";

                }else{
                    RadialBasic *basic = dynamic_cast<RadialBasic*>(distortion);
                    if(basic != NULL){
                        QString kn = QString::number(basic->getK1(),'f',15) + " " +
                                QString::number(basic->getK2(),'g',15);

                        queryString = "INSERT INTO pw_distortions (kn, id_camera_model) VALUES (" +
                                QString("'") + kn + "',"
                                + QString::number(modelID) +")";
                    }else{
                        FraserModel *fraser = dynamic_cast<FraserModel*>(distortion);
                        if(fraser != NULL){
                            QString coefs = QString::number(fraser->getK1(),'g',15) + " " +
                                    QString::number(fraser->getK2(),'g',15) + " " +
                                    QString::number(fraser->getK3(),'g',15) + " " +
                                    QString::number(fraser->getP1(),'g',15) + " " +
                                    QString::number(fraser->getP2(),'g',15) + " " +
                                    QString::number(fraser->getB1(),'g',15) + " " +
                                    QString::number(fraser->getB2(),'g',15);

                            queryString = "INSERT INTO pw_distortions (xcd, ycd, kn, id_camera_model) VALUES (" +
                                    QString::number(fraser->getXcd(),'f',15) + "," +
                                    QString::number(fraser->getYcd(),'f',15) + "," +
                                    "'" + coefs + "',"
                                    + QString::number(modelID) +")";
                        }
                        else{
                            FishEyeModel *fishEye = dynamic_cast<FishEyeModel*>(distortion);
                            if(fishEye != NULL){
                                QString params;
                                for (int i=0; i<fishEye->getParamsCount();i++)
                                    params = params+QString::number(fishEye->getParam(i))+" ";
                                queryString = "INSERT INTO pw_distortions (xcd, ycd, kn, id_camera_model) VALUES (" +
                                        QString::number(fishEye->getXcd(),'f',15) + "," +
                                        QString::number(fishEye->getYcd(),'f',15) + "," +
                                        "'" + params + "',"
                                        + QString::number(modelID) +")";
                            }
                        }
                    }
                }

                if(query.prepare(queryString)){
                    query.exec();
                }
                else{
                    qCritical() << mDB.lastError();
                    return 1;
                }
            }
        }

        //Writting Image:

        if(update)
            queryString = "DELETE FROM pw_masks WHERE id_image = "+QString::number(image->getID());
        if(query.prepare(queryString)){
            query.exec();
        }
        else{
            qCritical() << mDB.lastError();
            return 1;
        }
        if(update)
            queryString = "DELETE FROM pw_ext_orient WHERE id_image = "+QString::number(image->getID());
        if(query.prepare(queryString)){
            query.exec();
        }
        else{
            qCritical() << mDB.lastError();
            return 1;
        }
        if(update)
            queryString = "DELETE FROM pw_image_points WHERE id_image = "+QString::number(image->getID());
        if(query.prepare(queryString)){
            query.exec();
        }
        else{
            qCritical() << mDB.lastError();
            return 1;
        }

        int imageID = image->getID();
        if (!update)
        {
            if(modelID)
                queryString = "INSERT INTO pw_images (url, id_project, id_camera, id_active_model) VALUES ('" +
                        image->getFileName() + "'," +
                        QString::number(projectID) + "," +
                        QString::number(cameraID) + "," +
                        QString::number(modelID) +
                        ")";
            else
                queryString = "INSERT INTO pw_images (url, id_project, id_camera) VALUES ('" +
                        image->getFileName() + "'," +
                        QString::number(projectID) + "," +
                        QString::number(cameraID) +
                        ")";
            if(query.prepare(queryString)){
                query.exec();
                imageID = query.lastInsertId().toInt();
                image->setID(imageID);
            }
            else{
                qCritical() << mDB.lastError();
                return 1;
            }
        }else {
            if(modelID)
                queryString = "UPDATE pw_images SET "
                        "url = '" + image->getFileName() + "'," +
                        "id_project = " + QString::number(projectID) + "," +
                        "id_camera = " + QString::number(cameraID) + "," +
                        "id_active_model = " + QString::number(modelID) +
                        " WHERE id="+QString::number(imageID);
            else
                queryString = "UPDATE pw_images SET "
                        "url = '" + image->getFileName() + "'," +
                        "id_project = " + QString::number(projectID) + "," +
                        "id_camera = " + QString::number(cameraID) +
                        " WHERE id="+QString::number(imageID);
            if(query.prepare(queryString)){
                query.exec();
                //                imageID = query.lastInsertId().toInt();
            }
            else{
                qCritical() << mDB.lastError();
                return 1;
            }
        }

        //Writting External orientation:
        ExteriorOrientation *exteriorOrientation = image->getExteriorOrientation();
        if(exteriorOrientation != NULL){
            QString r = "";
            for (int f=0; f < exteriorOrientation->getR()->count(); f++)
                for (int c = 0; c<exteriorOrientation->getR()->at(f).count(); c++)
                    r += QString::number(exteriorOrientation->getR()->at(f).at(c),'f',15) + ",";
            r = r.left(r.count()-1);
            queryString = "INSERT INTO pw_ext_orient (xcp, ycp, zcp, r, id_image) VALUES (" +
                    QString::number(exteriorOrientation->getCP()->at(0),'f',15) + "," +
                    QString::number(exteriorOrientation->getCP()->at(1),'f',15) + "," +
                    QString::number(exteriorOrientation->getCP()->at(2),'f',15) + ",'" +
                    r + "'," +
                    QString::number(imageID, 'f',15) + ")";

            if(query.prepare(queryString)){
                query.exec();
            }
            else{
                qCritical() << mDB.lastError();
                return 1;
            }
        }

        //Writting Mask:
        /*TODO: Múltiples máscaras*/
        QPolygonF mask = image->getMask();
        if(mask.count()>0){
            QString wktMask = "";
            for (int p = 0; p<mask.count(); p++){
                wktMask += QString::number(mask.at(p).x()) + " " + QString::number(mask.at(p).y()) + ",";
            }
            wktMask = wktMask.left(wktMask.count()-1);
            queryString = "INSERT INTO pw_masks (wkt_poligon, id_image) VALUES ('" +
                    wktMask + "'," +
                    QString::number(imageID) + ")";

            int maskID;
            if(query.prepare(queryString)){
                query.exec();
                maskID = query.lastInsertId().toInt();
            }
            else{
                qCritical() << mDB.lastError();
                return 1;
            }
        }

        //Writting Image Points:
        for(int ip=0; ip< image->getControlPoints()->count(); ip++){
            PW2dPoint *imagePoint = image->getControlPoints()->at(ip);
            queryString = "INSERT INTO pw_image_points (x, y, id_image, name) VALUES (" +
                    QString::number(imagePoint->x(),'f',5) + "," +
                    QString::number(imagePoint->y(),'f',5) + "," +
                    QString::number(imageID) + ",'" +
                    imagePoint->getName() + "')";
            if(query.prepare(queryString)){
                query.exec();
            }
            else{
                qCritical() << mDB.lastError();
                return 1;
            }
        }
    }

    //Writting Terrain Points:
    for(int tp=0; tp< project->getModelCPoints()->count(); tp++){
        PW3dPoint *terrainPoint = project->getModelCPoints()->at(tp);
        queryString = "INSERT INTO pw_terrain_points (x, y, z, id_project,name) VALUES (" +
                QString::number(terrainPoint->x(),'f',5) + "," +
                QString::number(terrainPoint->y(),'f',5) + "," +
                QString::number(terrainPoint->z(),'f',5) + "," +
                QString::number(projectID) + ",'" +
                terrainPoint->getName() + "')";

        if(query.prepare(queryString)){
            query.exec();
        }
        else{
            qCritical() << mDB.lastError();
            return 1;
        }
    }
    if(!mDB.commit()){
        qCritical() << mDB.lastError();
        return 1;
    }

    //Deleting - Writing matches categoris:
    //Delete matches categories in the project:

    queryString =
            "DELETE FROM pw_matches_categories WHERE id_project = " +
            QString::number(project->getID());
    if(query.prepare(queryString)){
        query.exec();
    }
    else{
        qCritical() << mDB.lastError();
        return 1;
    }
    writeMatchesCategories(project);

    project->setNeedToSave(false);
    return 0;
}

PW::Project * PW::PersistenceManager::readProject(QString projectName)
{
    QSqlQuery query(mDB);

    //Read project:
    Project *project = new Project();
    QString queryString;
    if(projectName.isEmpty())
        queryString = "SELECT * FROM pw_projects";
    else
        queryString = "SELECT * FROM pw_projects WHERE name = '" + projectName + "'";

    if(query.prepare(queryString)){
        query.exec();
        query.next();
        project->setName(query.record().value("name").toString());
        project->setDescription(query.record().value("description").toString());
        project->setBasePath(query.record().value("base_path").toString());
        project->setSparseModelRelativePath(query.record().value("sparse_path").toString());
        project->setDenseModelRelativePath(query.record().value("dense_path").toString());
        project->setCurrentOrientation(query.record().value("ori_name").toString());
        project->setMatchingFormat(query.record().value("match_format").toInt());
        project->setProcessMetadata(query.record().value("metadata").toString());
        project->setActiveTiePointsSet(query.record().value("tiepoints_name").toString());
        project->setCloudUUID(query.record().value("cloud_uuid").toString());
        project->setPreprocessingPath(query.record().value("pre_processing_path").toString());
        project->setCaptureType(query.record().value("capture_type").toString());
        int projectID = query.record().value("id").toInt();
        project->setID(projectID);

        QList<PW::PWImage *> *images = getImages(project, projectID, project->getBasePath());
        if(images != NULL){
            project->addImputImages(*images);
            delete images;

            project->getModelCPoints()->append(*getTerrainPoints(projectID));
        }
        else
            return NULL;

        if(!readCategories(project))
        {
            qCritical() << mDB.lastError();
            return NULL;
        }
        //        queryString = "SELECT * FROM pw_matches_categories WHERE id_project = " + QString::number(projectID);
//        if(query.prepare(queryString)){
//            query.exec();
//            while(query.next()){
//                project->appendTiePointsSet(query.record().value("description").toString());
//            }
//        }
//        else{
//            qCritical() << mDB.lastError();
//            return NULL;
//        }

    }
    else{
        qCritical() << mDB.lastError();
        return NULL;
    }

    project->setNeedToSave(false);
    return project;
}

QList<Project> PersistenceManager::readProjectsDescriptors()
{
    QSqlQuery query(mDB);
    QList <Project> projects;

    QString queryString;
    queryString = "SELECT * FROM pw_projects";
    if(query.prepare(queryString)){
        query.exec();
        while(query.next()){
            Project project;
            project.setName(query.record().value("name").toString());
            project.setDescription(query.record().value("description").toString());
            project.setBasePath(query.record().value("base_path").toString());
            project.setSparseModelRelativePath(query.record().value("sparse_path").toString());
            project.setDenseModelRelativePath(query.record().value("dense_path").toString());
            project.setCurrentOrientation(query.record().value("ori_name").toString());
            project.setMatchingFormat(query.record().value("match_format").toInt());
            project.setProcessMetadata(query.record().value("metadata").toString());
            project.setActiveTiePointsSet(query.record().value("tiepoints_name").toString());
            project.setCloudUUID(query.record().value("cloud_uuid").toString());
            project.setPreprocessingPath(query.record().value("pre_processing_path").toString());
            int projectID = query.record().value("id").toInt();
            project.setID(projectID);

            projects.append(project);
        }
    }
    return projects;
}

bool PersistenceManager::readCategories(Project *project)
{
    QSqlQuery query(mDB);
    QSqlQuery queryPairs(mDB);
    QString queryStringCategories = "SELECT * FROM pw_matches_categories WHERE id_project = " + QString::number(project->getID());
    QString queryStringPairs = "SELECT  imagesA.url as imageNameA, imagesB.url as imageNameB, pairs.id FROM pw_image_pairs as pairs, pw_images as imagesA, pw_images as imagesB WHERE id_category = :idCategory AND pairs.id_Img1= imagesA.id  AND pairs.id_Img2= imagesB.id ORDER BY  imagesA.url";
    if (!queryPairs.prepare(queryStringPairs)){
        qCritical() << mDB.lastError();
        return false;
    }
    if(query.prepare(queryStringCategories)){
        query.exec();
        while(query.next()){
            project->appendTiePointsSet(query.record().value("description").toString());

            PWMatchesCategory * category = new PWMatchesCategory(project,
                                                                 query.record().value("id").toInt(),
                                                                 query.record().value("description").toString(),
                                                                 query.record().value("metadata").toString(),
                                                                 query.record().value("time_calculation").toString());

            QMap<QString, QVector<QString> > imagePairs;
            queryPairs.bindValue(":idCategory", QString::number(category->getId()));
            queryPairs.exec();

            QVector<QString> imagesNameVector;
            while(queryPairs.next()){
                QString imageNameA = queryPairs.record().value("imageNameA").toString();
                QString imageNameB = queryPairs.record().value("imageNameB").toString();

                if(!imagePairs.contains(imageNameA))
                    imagesNameVector.clear();

                imagesNameVector.append(imageNameB);
                imagePairs.insert(imageNameA,imagesNameVector);
            }
            PWGraphImages* graphImages=new PWGraphImages();
            if(!graphImages->createGraph(project->getImagesFileName(),
                                         imagePairs))
            {
                //TODO: graphIsConnected=false; ¿Que hacer?
            }
            category->setGraphImages(graphImages);
            category->setImagePairs(imagePairs);

            project->insertMatchesCategory(category);
        }
    }
    else{
        qCritical() << mDB.lastError();
        return false;
    }

    return true;
}

QList<PW::PWImage *> *PW::PersistenceManager::getImages(Project *project, int dbProjectID, QString basePath)
{
    QSqlQuery query(mDB);

    QList<PW::PWImage *> *images = new QList<PW::PWImage *>();

    QString queryString = "SELECT * FROM pw_images WHERE id_project = " + QString::number(dbProjectID);
    if(query.prepare(queryString)){
        query.exec();
        while (query.next()) {
            PWImage * image = new PWImage(basePath + "/" + query.record().value("url").toString());
            int imageID = query.record().value("id").toInt();
            image->setID(imageID);
            int activeCameraModelID = query.record().value("id_active_model").toInt();

            //Read Camera:
            QSqlQuery query2(mDB);
            int cameraID = query.record().value("id_camera").toInt();
            queryString = "SELECT * FROM pw_cameras WHERE id = " + QString::number(cameraID);
            if(query2.prepare(queryString)){
                query2.exec();
                query2.next();

                QString name = query2.record().value("name").toString();
                QString lens = query2.record().value("lens").toString();
                QString key = name;
                if (!lens.isEmpty())
                    key = key + " - " + lens;
                Camera *camera = project->getCamera(key);

                if (camera == NULL){

                    camera = new Camera(name,
                                        lens,
                                        query2.record().value("sensor_width").toDouble(),
                                        query2.record().value("sensor_height").toDouble(),
                                        query2.record().value("focal").toDouble());
                    camera->setCameraModel(getCameraModel(activeCameraModelID));

                    project->addCamera(camera);
                }

                image->setCamera(camera);
                image->setExteriorOrientation(getExternalOrientation(imageID));
                QPolygon mask = getMask(imageID);
                if (mask.count()>2)
                    image->setMask(getMask(imageID));
                QList<PW::PW2dPoint *> * imagePoints = getImagePoints(imageID);
                if (imagePoints != NULL){
                    image->getControlPoints()->append(*imagePoints);
                    delete imagePoints;
                }

            }
            else{
                qCritical() << mDB.lastError();
                return NULL;
            }
            images->append(image);
        }
    }
    else{
        qCritical() << mDB.lastError();
        return NULL;
    }

    return images;
}

QList<PW::PW2dPoint *> *PW::PersistenceManager::getImagePoints(int dbImageID)
{
    QSqlQuery query(mDB);

    QList<PW::PW2dPoint *> *points = new QList<PW::PW2dPoint *>();

    QString queryString = "SELECT * FROM pw_image_points WHERE id_image = " + QString::number(dbImageID);
    if(query.prepare(queryString)){
        query.exec();
        while (query.next()) {
            points->append(new PW2dPoint(query.record().value("x").toDouble(),
                                         query.record().value("y").toDouble(),query.record().value("name").toString()));

        }
        if (points->count()>0)
            return points;
        else
            return NULL;

    }
    else{
        qCritical() << mDB.lastError();
        return NULL;
    }

}


PW::ExteriorOrientation * PW::PersistenceManager::getExternalOrientation(int imageID)
{
    QSqlQuery query(mDB);
    //Read External orientation:
    QString queryString = "SELECT * FROM pw_ext_orient WHERE id_image = " + QString::number(imageID);
    if(query.prepare(queryString)){
        query.exec();
        if(query.next()){
            double xcp = query.record().value("xcp").toDouble();
            double ycp = query.record().value("ycp").toDouble();
            double zcp = query.record().value("zcp").toDouble();
            QString rString = query.record().value("r").toString();
            QVector<QVector <double> > r;
            QStringList rList = rString.split(",");
            r.append(QVector <double>() << rList.at(0).toDouble() << rList.at(1).toDouble() << rList.at(2).toDouble());
            r.append(QVector <double>() << rList.at(3).toDouble() << rList.at(4).toDouble() << rList.at(5).toDouble());
            r.append(QVector <double>() << rList.at(6).toDouble() << rList.at(7).toDouble() << rList.at(8).toDouble());

            ExteriorOrientation * orientation = new ExteriorOrientation(QVector<double>() << xcp << ycp << zcp, r);

            return orientation;
        }
        else return NULL;

    }
    else{
        qCritical() << mDB.lastError();
        return NULL;
    }
}

PW::PhotogrammetricModel * PW::PersistenceManager::getCameraModel(int activeCameraModelID)
{
    PhotogrammetricModel *cameraModel = new PhotogrammetricModel();

    QSqlQuery query(mDB);

    int cameraModelID;
    QString queryString = "SELECT * FROM pw_camera_models WHERE id = " + QString::number(activeCameraModelID);
    if(query.prepare(queryString)){
        query.exec();
        if (!query.next())
            return NULL;
        cameraModel->setXp(query.record().value("xp").toDouble());
        cameraModel->setYp(query.record().value("yp").toDouble());
        cameraModel->setFocal(query.record().value("focal").toDouble());
        cameraModelID = query.record().value("id").toInt();
    }
    else{
        qCritical() << mDB.lastError();
        return NULL;
    }

    queryString = "SELECT * FROM pw_distortions WHERE id_camera_model = " + QString::number(cameraModelID);
    if(query.prepare(queryString)){
        query.exec();
        if (!query.next())
            return NULL;

        QStringList k =  query.record().value("kn").toString().split(" ");
        if(k.count()==5){
            RadialExtended * distortionModel = new RadialExtended();
            distortionModel->setXcd(query.record().value("xcd").toDouble());
            distortionModel->setYcd(query.record().value("ycd").toDouble());

            distortionModel->setK1(k.at(0).toDouble());
            distortionModel->setK2(k.at(1).toDouble());
            distortionModel->setK3(k.at(2).toDouble());
            distortionModel->setK4(k.at(3).toDouble());
            distortionModel->setK5(k.at(4).toDouble());

            cameraModel->setDistortionModel(distortionModel);
        }else if((k.count()==2)){
            RadialBasic * distortionModel = new RadialBasic();

            distortionModel->setK1(k.at(0).toDouble());
            distortionModel->setK2(k.at(1).toDouble());

            cameraModel->setDistortionModel(distortionModel);
        }else if(k.count() == 7){ //Fraser:
            FraserModel *distortionModel = new FraserModel();
            distortionModel->setXcd(query.record().value("xcd").toDouble());
            distortionModel->setYcd(query.record().value("ycd").toDouble());

            distortionModel->setK1(k.at(0).toDouble());
            distortionModel->setK2(k.at(1).toDouble());
            distortionModel->setK3(k.at(2).toDouble());
            distortionModel->setP1(k.at(3).toDouble());
            distortionModel->setP2(k.at(4).toDouble());
            distortionModel->setB1(k.at(5).toDouble());
            distortionModel->setB2(k.at(6).toDouble());

            if(cameraModel->getXp() == distortionModel->getXcd() && cameraModel->getYp() == distortionModel->getYcd())
                distortionModel->setBasic(true);

            cameraModel->setDistortionModel(distortionModel);
        }else if(k.count() > 45){ //FishEye
            FishEyeModel *distortionModel = new FishEyeModel();
            distortionModel->setXcd(query.record().value("xcd").toDouble());
            distortionModel->setYcd(query.record().value("ycd").toDouble());
            for(int i=0; i< k.count(); i++)
                distortionModel->addParam(k.at(i).toDouble());
            cameraModel->setDistortionModel(distortionModel);
        }
    }
    else{
        qCritical() << mDB.lastError();
        return NULL;
    }
    return cameraModel;
}

QPolygon PW::PersistenceManager::getMask(int imageID)
{
    QPolygon mask;

    QSqlQuery query(mDB);

    QString queryString = "SELECT * FROM pw_masks WHERE id_image = " + QString::number(imageID);
    if(query.prepare(queryString)){
        query.exec();
        query.next();
        QString wktPoligon = query.record().value("wkt_poligon").toString();
        QStringList points = wktPoligon.split(",");
        for (int i=0; i<points.count(); i++){
            QStringList point = points.at(i).split(" ");
            if (point.count() == 2)
                mask.putPoints(i,1,point.at(0).toDouble(),point.at(1).toDouble());
        }
    }
    else{
        qCritical() << mDB.lastError();
        return mask;
    }

    return mask;
}

QList<PW::PW3dPoint *> * PW::PersistenceManager::getTerrainPoints(int projectID)
{
    QSqlQuery query(mDB);

    QList<PW::PW3dPoint *> *points = new QList<PW::PW3dPoint *>();

    QString queryString = "SELECT * FROM pw_terrain_points WHERE id_project = " + QString::number(projectID);
    if(query.prepare(queryString)){
        query.exec();
        while (query.next()) {
            points->append(new PW3dPoint(query.record().value("x").toDouble(),
                                         query.record().value("y").toDouble(),
                                         query.record().value("z").toDouble(),query.record().value("name").toString()));

        }
        return points;

    }
    else{
        qCritical() << mDB.lastError();
        return NULL;
    }
}

PW::Camera * PW::PersistenceManager::readCamera(QString cameraName, QString lens)
{
    QSqlQuery query(mDB);
    Camera * camera = NULL;

    QString queryString = "SELECT * FROM pw_cameras WHERE name = '" + cameraName + "'";
    //    if (!lens.isEmpty())
    queryString = queryString + " AND lens = '" + lens + "'";
    if(query.prepare(queryString)){
        query.exec();
        if(query.next())
            camera = new Camera(cameraName,
                                query.record().value("lens").toString(),
                                query.record().value("sensor_width").toDouble(),
                                query.record().value("sensor_height").toDouble(),
                                query.record().value("focal").toDouble(),
                                query.record().value("scale_factor").toDouble());
        else return NULL;
    }
    else{
        qCritical() << mDB.lastError();
        return NULL;
    }

    return camera;

}

PW::Camera * PW::PersistenceManager::readCamera(QString cameraName)
{
    QSqlQuery query(mDB);
    Camera * camera = NULL;

    QString queryString = "SELECT * FROM pw_cameras WHERE name = '" + cameraName + "'";
    if(query.prepare(queryString)){
        query.exec();
        if(query.next())
            camera = new Camera(cameraName,
                                query.record().value("lens").toString(),
                                query.record().value("sensor_width").toDouble(),
                                query.record().value("sensor_height").toDouble(),
                                query.record().value("focal").toDouble(),
                                query.record().value("scale_factor").toDouble());
        else return NULL;
    }
    else{
        qCritical() << mDB.lastError();
        return NULL;
    }
    return camera;
}

PW::Camera * PW::PersistenceManager::readCameraByLens(QString lensName)
{
    QSqlQuery query(mDB);
    Camera * camera = NULL;

    if(lensName.isEmpty())
        return camera;

    QString queryString = "SELECT * FROM pw_cameras WHERE lens = '" + lensName + "'";
    if(query.prepare(queryString)){
        query.exec();
        if(query.next())
            camera = new Camera(lensName,
                                query.record().value("lens").toString(),
                                query.record().value("sensor_width").toDouble(),
                                query.record().value("sensor_height").toDouble(),
                                query.record().value("focal").toDouble(),
                                query.record().value("scale_factor").toDouble());
        else return NULL;
    }
    else{
        qCritical() << mDB.lastError();
        return NULL;
    }
    return camera;
}

QList<Camera *> PersistenceManager::readCameras(QString cameraName, QString lens)
{
    QSqlQuery query(mDB);
    QList<Camera *> cameras;
    QString queryString = "";

    if (cameraName.isEmpty() && lens.isEmpty())
        queryString = "SELECT * FROM pw_cameras";
    else if(cameraName.isEmpty()){
        queryString = "SELECT * FROM pw_cameras WHERE lens = '" + lens + "'";
    }else{
        queryString = "SELECT * FROM pw_cameras WHERE name = '" + cameraName + "'";
        if (!lens.isEmpty())
            queryString = queryString + " AND lens = '" + lens + "'";
    }

    if(query.prepare(queryString)){
        query.exec();
        while(query.next()){
            Camera * camera = new Camera(query.record().value("name").toString(),
                                         query.record().value("lens").toString(),
                                         query.record().value("sensor_width").toDouble(),
                                         query.record().value("sensor_height").toDouble(),
                                         query.record().value("focal").toDouble(),
                                         query.record().value("scale_factotr").toDouble());
            cameras.append(camera);
        }

    }
    else{
        qCritical() << mDB.lastError();
        return cameras;
    }

    return cameras;
}

QSqlDatabase * PW::PersistenceManager::getDataBase()
{
    return &mDB;
}

int PW::PersistenceManager::writeMatches(Project *project,
                                         QString metadata,
                                         QString description,
                                         QString timeCalculation,
                                         QMap<QString, QVector<QString> > imagesIdsTiePoints,
                                         QMap<QString, QMap<QString,QVector<double> > > imagesFirstColumnsTiePoints,
                                         QMap<QString, QMap<QString,QVector<double> > > imagesFirstRowsTiePoints,
                                         QMap<QString, QMap<QString,QVector<double> > > imagesSecondColumnsTiePoints,
                                         QMap<QString, QMap<QString,QVector<double> > > imagesSecondRowsTiePoints)
{
    mDB.transaction();
    QSqlQuery query(mDB);
    QString queryString="INSERT INTO pw_matches_categories (metadata, description, time_calculation, id_project)"
            "VALUES ('"+metadata+"', '"+description+"', '"+ timeCalculation +"', "+QString::number(project->getID())+")";

    int categoryId;
    if(query.prepare(queryString))
    {
        query.exec();
        categoryId = query.lastInsertId().toInt();
    }
    else
    {
        qCritical() << mDB.lastError();
        return PERSISTENCEMANAGER_ERROR;
    }

    for(int p=0; p<imagesIdsTiePoints.count(); p++) {
        for(int i=0; i< imagesIdsTiePoints.values().at(p).size(); i++) {
            //Comprobar que las dos imágenes siguen estando en el proyecto:
            if(!project->getImageByName(imagesIdsTiePoints.keys().at(p)) ||
                    !project->getImageByName(imagesIdsTiePoints.values().at(p).at(i)))
                continue;

            QVector<double> vectorXfirst = imagesFirstColumnsTiePoints.value(imagesIdsTiePoints.keys().at(p)).value(imagesIdsTiePoints.values().at(p).at(i));
            QVector<double> vectorYfirst = imagesFirstRowsTiePoints.value(imagesIdsTiePoints.keys().at(p)).value(imagesIdsTiePoints.values().at(p).at(i));
            QVector<double> vectorXsecond = imagesSecondColumnsTiePoints.value(imagesIdsTiePoints.keys().at(p)).value(imagesIdsTiePoints.values().at(p).at(i));
            QVector<double> vectorYsecond = imagesSecondRowsTiePoints.value(imagesIdsTiePoints.keys().at(p)).value(imagesIdsTiePoints.values().at(p).at(i));

            //Get images ids for new project:
            queryString = "SELECT id FROM pw_images WHERE url='"+imagesIdsTiePoints.keys().at(p)+
                    "' AND id_project = "+QString::number(project->getID());
            QString img1Id;
            if(query.prepare(queryString))
            {
                query.exec();
                if(query.next())
                    img1Id = query.record().value("id").toString();
                else
                {
                    qCritical() << mDB.lastError();
                    return PERSISTENCEMANAGER_ERROR;
                }
            }
            else
            {
                qCritical() << mDB.lastError();
                return PERSISTENCEMANAGER_ERROR;
            }

            queryString = "SELECT id FROM pw_images WHERE url='" + imagesIdsTiePoints.values().at(p).at(i)+
                    "' AND id_project = " + QString::number(project->getID());
            QString img2Id;
            if(query.prepare(queryString))
            {
                query.exec();
                if(query.next())
                    img2Id = query.record().value("id").toString();
                else
                {
                    qCritical() << mDB.lastError();
                    return PERSISTENCEMANAGER_ERROR;
                }
            }
            else
            {
                qCritical() << mDB.lastError();
                return PERSISTENCEMANAGER_ERROR;
            }

            for (int m = 0; m< vectorXfirst.size();m++){

                //Insert in pw_keypoints 1
                //                QString strWktFirstPoint="GeomFromText('POINT(";
                //                strWktFirstPoint+=QString::number(vectorXfirst[m],'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                //                strWktFirstPoint+=" ";
                //                strWktFirstPoint+=QString::number(vectorYfirst[m],'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                //                strWktFirstPoint+=")',";
                //                strWktFirstPoint+=QString::number(LIBPW_IMAGE_COORDINATES_CRS);
                //                strWktFirstPoint+=")";

                //                QString queryString="INSERT INTO pw_keypoints (id_image, pixel_point_geom) "
                //                        "VALUES("+img1Id+","+strWktFirstPoint+")";
                QString queryString="INSERT INTO pw_keypoints (id_image, x, y) "
                        "VALUES ("+img1Id+","+QString::number(vectorXfirst[m])+
                        ","+QString::number(vectorYfirst[m])+")";
                int firstKeyPointID;
                if(query.prepare(queryString))
                {
                    query.exec();
                    firstKeyPointID = query.lastInsertId().toInt();
                }
                else
                {
                    qCritical() << mDB.lastError();
                    return PERSISTENCEMANAGER_ERROR;
                }

                //Insert in pw_keypoints 2
                //                QString strWktSecondPoint="GeomFromText('POINT(";
                //                strWktSecondPoint+=QString::number(vectorXsecond[m],'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                //                strWktSecondPoint+=" ";
                //                strWktSecondPoint+=QString::number(vectorYsecond[m],'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                //                strWktSecondPoint+=")',";
                //                strWktSecondPoint+=QString::number(LIBPW_IMAGE_COORDINATES_CRS);
                //                strWktSecondPoint+=")";

                //                queryString="INSERT INTO pw_keypoints (id_image, pixel_point_geom) "
                //                       "VALUES ("+img2Id+","+strWktSecondPoint+")";
                queryString="INSERT INTO pw_keypoints (id_image, x, y) "
                        "VALUES ("+img2Id+","+QString::number(vectorXsecond[m])+
                        ","+QString::number(vectorYsecond[m])+")";
                int secondKeyPointID;
                if(query.prepare(queryString))
                {
                    query.exec();
                    secondKeyPointID = query.lastInsertId().toInt();
                }
                else
                {
                    qCritical() << mDB.lastError();
                    return PERSISTENCEMANAGER_ERROR;
                }

                //Insert in pw_maches
                QString strEnabled=QString::number(1);
                QString strAccuracy="-1.0";
                queryString="INSERT INTO pw_matches (id_matches_category,id_keypoint_first,id_keypoint_second,enabled, accuracy) "
                        "VALUES ("+QString::number(categoryId)+","+
                        QString::number(firstKeyPointID)+","+
                        QString::number(secondKeyPointID)+","+
                        strEnabled+","+
                        strAccuracy+")";

                if(query.prepare(queryString))
                {
                    query.exec();
                }
                else
                {
                    qCritical() << mDB.lastError();
                    return PERSISTENCEMANAGER_ERROR;
                }
            }
        }
    }

    if(!mDB.commit()){
        qCritical() << mDB.lastError();
        return PERSISTENCEMANAGER_ERROR;;
    }

    return PERSISTENCEMANAGER_NO_ERROR;
}


int PW::PersistenceManager::writeMatches(Project *project,
                                         QString &metadata,
                                         QString &description,
                                         QString &timeCalculation,
                                         QString &matchesPath)
{
    QDir auxDir=QDir::currentPath();
    if(!auxDir.exists(matchesPath))
    {
        QString msg=tr("PersistenceManager::writeMatches error: not exists path:\n%1").arg(matchesPath);
        qCritical() << mDB.lastError();
        return(PERSISTENCEMANAGER_ERROR);
    }
    QDir sourceDir(matchesPath);
    QStringList dirs = sourceDir.entryList(QDir::Dirs);
    QVector<QString> firstImagesFile;
    QVector<QString> secondImagesFile;
    QVector<QVector<double> > firstImagesPixelsColumn;
    QVector<QVector<double> > firstImagesPixelsRow;
    QVector<QVector<double> > secondImagesPixelsColumn;
    QVector<QVector<double> > secondImagesPixelsRow;
    QMap<QString,int> imagesIdByImageFile;
    QMap<QString,ImagePointCloud*> imagesPointCloudByImageFile;

    QSqlQuery query(mDB);

    for(int i = 0; i< dirs.count(); i++)
    {
        QString srcPath = dirs[i];
        if(srcPath.compare(".")==0
                ||srcPath.compare("..")==0)
        {
            continue;
        }
        if(!srcPath.contains(TAPIOCA_PASTIS_PATH,Qt::CaseInsensitive))
        {
            QString msg=tr("PersistenceManager::writeMatches error: path is not valid:\n%1").arg(srcPath);
            qCritical() << mDB.lastError();
            return(PERSISTENCEMANAGER_ERROR);
        }
        QString completeSrcPath=matchesPath+"/"+srcPath;
        QString firstImageFile=srcPath.remove(TAPIOCA_PASTIS_PATH);
        PW::PWImage* firstImage=project->getImageByName(firstImageFile);
        if(firstImage==NULL)
        {
            QString msg=tr("PersistenceManager::writeMatches error: not exists image:\n%1").arg(firstImageFile);
            qCritical() << mDB.lastError();
            return(PERSISTENCEMANAGER_ERROR);
        }
        if(!imagesIdByImageFile.contains(firstImageFile))
        {
            int firstImageId=firstImage->getID();
            imagesIdByImageFile[firstImageFile]=firstImageId;
            QSize imageSize=firstImage->getSize();
            ImagePointCloud* ptrImagePointCloud=new ImagePointCloud(0,0,imageSize.width()-1,imageSize.height()-1,LIBPW_IMAGE_POINTCLOUD_GRIDSIZE);
            imagesPointCloudByImageFile[firstImageFile]=ptrImagePointCloud;
        }
        QDir sourceFiles(completeSrcPath);
        QStringList files = sourceFiles.entryList(QDir::Files);
        for(int j = 0; j< files.count(); j++)
        {
            QString filePath = files[j];
            if(filePath.compare(".")==0
                    ||filePath.compare("..")==0)
            {
                continue;
            }
            QString completeFilePath=completeSrcPath+"/"+filePath;
            QString secondImageFile=filePath.remove(MATCHING_TXT_FILE_EXTENSION);
            PW::PWImage* secondImage=project->getImageByName(secondImageFile);
            if(secondImage==NULL)
            {
                QString msg=tr("PersistenceManager::writeMatches error: not exists image:\n%1").arg(secondImageFile);
                qCritical() << msg;
                return(PERSISTENCEMANAGER_ERROR);
            }
            firstImagesFile.push_back(firstImageFile);
            secondImagesFile.push_back(secondImageFile);
            if(!imagesIdByImageFile.contains(secondImageFile))
            {
                int secondImageId=secondImage->getID();
                imagesIdByImageFile[secondImageFile]=secondImageId;
                QSize imageSize=secondImage->getSize();
                ImagePointCloud* ptrImagePointCloud=new ImagePointCloud(0,0,imageSize.width()-1,imageSize.height()-1,LIBPW_IMAGE_POINTCLOUD_GRIDSIZE);
                imagesPointCloudByImageFile[secondImageFile]=ptrImagePointCloud;
            }
            QVector<double> firstImagePixelsColumn,firstImagePixelsRow;
            QVector<double> secondImagePixelsColumn,secondImagePixelsRow;
            QString strError;
            if(TIEPOINTSIO_NO_ERROR!=TiePointsIO::readTiePointsFile(completeFilePath,
                                                                    firstImagePixelsColumn,
                                                                    firstImagePixelsRow,
                                                                    secondImagePixelsColumn,
                                                                    secondImagePixelsRow,
                                                                    strError))
            {
                qCritical() << strError;
                return(PERSISTENCEMANAGER_ERROR);
            }
            firstImagesPixelsColumn.push_back(firstImagePixelsColumn);
            firstImagesPixelsRow.push_back(firstImagePixelsRow);
            secondImagesPixelsColumn.push_back(secondImagePixelsColumn);
            secondImagesPixelsRow.push_back(secondImagePixelsRow);
        }
    }

    int projectID=project->getID();
    // la primera, posible error
    QString queryString;
    int idOldMatchesCategories=0;
    int existsMatchesCategories;
    queryString="select count(*) as count, id from pw_matches_categories where metadata='"+metadata;
    queryString+="' AND id_project="+QString::number(projectID);
    if(query.prepare(queryString))
    {
        query.exec();
        query.next();
        existsMatchesCategories = query.record().value("count").toInt();
        idOldMatchesCategories = query.record().value("id").toInt();
    }
    else
    {
        qCritical() << mDB.lastError();
        for(int i=0;i<firstImagesFile.size();i++)
        {
            QString firstImageFile=firstImagesFile.at(i);
            if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[firstImageFile]);
                imagesPointCloudByImageFile[firstImageFile]=NULL;
            }
            QString secondImageFile=secondImagesFile.at(i);
            if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[secondImageFile]);
                imagesPointCloudByImageFile[secondImageFile]=NULL;
            }
        }
        return PERSISTENCEMANAGER_ERROR;
    }
    if(existsMatchesCategories>0) // Elimina los matches
    {
        int numberOfKeypointsToDelete=0;
        queryString ="DELETE FROM pw_matches_categories WHERE metadata = '"+metadata+"' AND id_project=";
        queryString += QString::number(projectID);
        if(query.prepare(queryString))
        {
            query.exec();
        }
        else
        {
            qCritical() << mDB.lastError();
            for(int i=0;i<firstImagesFile.size();i++)
            {
                QString firstImageFile=firstImagesFile.at(i);
                if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                {
                    delete(imagesPointCloudByImageFile[firstImageFile]);
                    imagesPointCloudByImageFile[firstImageFile]=NULL;
                }
                QString secondImageFile=secondImagesFile.at(i);
                if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                {
                    delete(imagesPointCloudByImageFile[secondImageFile]);
                    imagesPointCloudByImageFile[secondImageFile]=NULL;
                }
            }
            return PERSISTENCEMANAGER_ERROR;
        }
    }

    QString strProjectID=QString::number(projectID);
    queryString = "INSERT INTO pw_matches_categories (metadata, description, time_calculation, id_project) ";
    queryString+="VALUES ('"+metadata+"','"+description+"','"+timeCalculation+"',"+strProjectID+")";
    int matchesCategoriesID;
    if(query.prepare(queryString))
    {
        query.exec();
        matchesCategoriesID = query.lastInsertId().toInt();
    }
    else
    {
        qCritical() << mDB.lastError();
        for(int i=0;i<firstImagesFile.size();i++)
        {
            QString firstImageFile=firstImagesFile.at(i);
            if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[firstImageFile]);
                imagesPointCloudByImageFile[firstImageFile]=NULL;
            }
            QString secondImageFile=secondImagesFile.at(i);
            if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[secondImageFile]);
                imagesPointCloudByImageFile[secondImageFile]=NULL;
            }
        }
        return 1;
    }

    QString strMatchesCategoriesID=QString::number(matchesCategoriesID);
    QString strImageCoordinatesCoincidenceTolerance=QString::number(LIBPW_IMAGE_COORDINATES_COINCIDENCE_TOLERANCE,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
    QString strError;

    // DHL-20140721
    QTime t;
    t.start();
    mDB.transaction();

    bool useUnion=false;
    if(useUnion)
    {
        QString fileName="e:/insert_matches_union.sql";
        QFile sqlFile(fileName);
        if(QFile::exists(fileName))
            QFile::remove(fileName);
        sqlFile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&sqlFile);
        int maxInsertsInUnion=PERSISTENCEMANAGER_MAX_INSERTS_IN_UNION;
        int numberOfInserts=0;
        int numberOfTotalInserts=0;
        queryString="INSERT INTO pw_matches(id_matches_category,id_image_first,column1,row1,";
        queryString+="id_image_second,column2,row2,enabled,accuracy)\n";
        bool isFirstInsert=true;
        QString strEnabled="1";
        QString strAccuracy="-1";
        double pixelCoordinatesFactorDoubleToInteger=pow(10.0,LIBPW_IMAGE_COORDINATES_PRECISION*1.0);
        for(int i=0;i<firstImagesFile.size();i++)
        {
            QString firstImageFile=firstImagesFile.at(i);
            QString strFirstImageId=QString::number(imagesIdByImageFile[firstImageFile]);
            ImagePointCloud* ptrFirstImagePointCloud=imagesPointCloudByImageFile[firstImageFile];

            QString secondImageFile=secondImagesFile.at(i);
            QString strSecondImageId=QString::number(imagesIdByImageFile[secondImageFile]);
            ImagePointCloud* ptrSecondImagePointCloud=imagesPointCloudByImageFile[secondImageFile];

            QString strError;
            for(int j=0;j<firstImagesPixelsColumn[i].size();j++)
            {
                double firstImagePixelColumn=firstImagesPixelsColumn[i][j];
                double firstImagePixelRow=firstImagesPixelsRow[i][j];
                double secondImagePixelColumn=secondImagesPixelsColumn[i][j];
                double secondImagePixelRow=secondImagesPixelsRow[i][j];

                QString strFirstImagePixelColumn=QString::number(qRound(firstImagePixelColumn*pixelCoordinatesFactorDoubleToInteger));
                QString strFirstImagePixelRow=QString::number(qRound(firstImagePixelRow*pixelCoordinatesFactorDoubleToInteger));
                QString strSecondImagePixelColumn=QString::number(qRound(secondImagePixelColumn*pixelCoordinatesFactorDoubleToInteger));
                QString strSecondImagePixelRow=QString::number(qRound(secondImagePixelRow*pixelCoordinatesFactorDoubleToInteger));

                if(!isFirstInsert)
                {
                    queryString+="UNION\n";
                }
                else
                    isFirstInsert=false;
                queryString+="SELECT \"";
                queryString+=strMatchesCategoriesID;
                queryString+="\",\"";
                queryString+=strFirstImageId;
                queryString+="\",\"";
                queryString+=strFirstImagePixelColumn;
                queryString+="\",\"";
                queryString+=strFirstImagePixelRow;
                queryString+="\",\"";
                queryString+=strSecondImageId;
                queryString+="\",\"";
                queryString+=strSecondImagePixelColumn;
                queryString+="\",\"";
                queryString+=strSecondImagePixelRow;
                queryString+="\",\"";
                queryString+=strEnabled;
                queryString+="\",\"";
                queryString+=strAccuracy;
                queryString+="\"\n";
                numberOfInserts++;
                numberOfTotalInserts++;
                if(numberOfInserts>=maxInsertsInUnion)
                {
                    numberOfInserts=0;
                    out<<queryString<<"\n";
                    if(query.prepare(queryString))
                    {
                        query.exec();
                    }
                    else
                    {
                        QString msg=tr("PersistenceManager::writeMatches error");
                        qCritical() << msg;
                        for(int i=0;i<firstImagesFile.size();i++)
                        {
                            QString firstImageFile=firstImagesFile.at(i);
                            if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                            {
                                delete(imagesPointCloudByImageFile[firstImageFile]);
                                imagesPointCloudByImageFile[firstImageFile]=NULL;
                            }
                            QString secondImageFile=secondImagesFile.at(i);
                            if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                            {
                                delete(imagesPointCloudByImageFile[secondImageFile]);
                                imagesPointCloudByImageFile[secondImageFile]=NULL;
                            }
                        }
                        return PERSISTENCEMANAGER_ERROR;
                    }
                    queryString="INSERT INTO pw_matches(id_matches_category,id_image_first,column1,row1,";
                    queryString+="id_image_second,column2,row2,enabled,accuracy)\n";
                    isFirstInsert=true;
                }
            }
        }
        if(numberOfInserts>0)
        {
            out<<queryString<<"\n";
            if(query.prepare(queryString))
            {
                query.exec();
            }
            else
            {
                QString msg=tr("PersistenceManager::writeMatchesAccuracies error writting accuracies");
                qCritical() << msg;
                for(int i=0;i<firstImagesFile.size();i++)
                {
                    QString firstImageFile=firstImagesFile.at(i);
                    if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[firstImageFile]);
                        imagesPointCloudByImageFile[firstImageFile]=NULL;
                    }
                    QString secondImageFile=secondImagesFile.at(i);
                    if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[secondImageFile]);
                        imagesPointCloudByImageFile[secondImageFile]=NULL;
                    }
                }
                return PERSISTENCEMANAGER_ERROR;
            }
        }
        sqlFile.close();
    }
    else
    {
        //        QString fileName="e:/insert_matches.sql";
        //        QFile sqlFile(fileName);
        //        if(QFile::exists(fileName))
        //            QFile::remove(fileName);
        //        sqlFile.open(QIODevice::WriteOnly | QIODevice::Text);
        //        QTextStream out(&sqlFile);
        QString strEnabled="1";
        QString strAccuracy="-1";
        double pixelCoordinatesFactorDoubleToInteger=pow(10.0,LIBPW_IMAGE_COORDINATES_PRECISION*1.0);

        queryString="INSERT INTO pw_matches(id_matches_category,id_image_first,column1,row1,";
        queryString+="id_image_second,column2,row2,enabled,accuracy) ";
        queryString+="VALUES (:strMatchesCategoriesID,:strFirstImageId,:strFirstImagePixelColumn,:strFirstImagePixelRow,:strSecondImageId,:strSecondImagePixelColumn,:strSecondImagePixelRow,:strEnabled,:strAccuracy)";

        if(!query.prepare(queryString))
        {
            QString msg=tr("PersistenceManager::writeMatchesAccuracies error writting accuracies");
            qCritical() << msg;
            return PERSISTENCEMANAGER_ERROR;
        }

        for(int i=0;i<firstImagesFile.size();i++)
        {
            QString firstImageFile=firstImagesFile.at(i);
            QString strFirstImageId=QString::number(imagesIdByImageFile[firstImageFile]);
            ImagePointCloud* ptrFirstImagePointCloud=imagesPointCloudByImageFile[firstImageFile];

            QString secondImageFile=secondImagesFile.at(i);
            QString strSecondImageId=QString::number(imagesIdByImageFile[secondImageFile]);
            ImagePointCloud* ptrSecondImagePointCloud=imagesPointCloudByImageFile[secondImageFile];

            QString strError;
            for(int j=0;j<firstImagesPixelsColumn[i].size();j++)
            {
                double firstImagePixelColumn=firstImagesPixelsColumn[i][j];
                double firstImagePixelRow=firstImagesPixelsRow[i][j];
                double secondImagePixelColumn=secondImagesPixelsColumn[i][j];
                double secondImagePixelRow=secondImagesPixelsRow[i][j];

                QString strFirstImagePixelColumn=QString::number(qRound(firstImagePixelColumn*pixelCoordinatesFactorDoubleToInteger));
                QString strFirstImagePixelRow=QString::number(qRound(firstImagePixelRow*pixelCoordinatesFactorDoubleToInteger));
                QString strSecondImagePixelColumn=QString::number(qRound(secondImagePixelColumn*pixelCoordinatesFactorDoubleToInteger));
                QString strSecondImagePixelRow=QString::number(qRound(secondImagePixelRow*pixelCoordinatesFactorDoubleToInteger));

//                queryString="INSERT INTO pw_matches(id_matches_category,id_image_first,column1,row1,";
//                queryString+="id_image_second,column2,row2,enabled,accuracy)\n";
//                queryString+="VALUES ('"+strMatchesCategoriesID+"','"+strFirstImageId+"','"+strFirstImagePixelColumn+"','"+strFirstImagePixelRow;
//                queryString+="','"+strSecondImageId+"','"+strSecondImagePixelColumn+"','"+strSecondImagePixelRow;
//                queryString+="','"+strEnabled+"','"+strAccuracy+"');";

                query.bindValue(":strMatchesCategoriesID", strMatchesCategoriesID);
                query.bindValue(":strFirstImageId", strFirstImageId);
                query.bindValue(":strFirstImagePixelColumn", strFirstImagePixelColumn);
                query.bindValue(":strFirstImagePixelRow", strFirstImagePixelRow);
                query.bindValue(":strSecondImageId", strSecondImageId);
                query.bindValue(":strSecondImagePixelColumn", strSecondImagePixelColumn);
                query.bindValue(":strSecondImagePixelRow", strSecondImagePixelRow);
                query.bindValue(":strEnabled", strEnabled);
                query.bindValue(":strAccuracy",strAccuracy);

                query.exec();

//                if(query.prepare(queryString))
//                {
//                    query.exec();
//                }
//                else
//                {
//                    QString msg=tr("PersistenceManager::writeMatchesAccuracies error writting accuracies");
//                    qCritical() << msg;
//                    for(int i=0;i<firstImagesFile.size();i++)
//                    {
//                        QString firstImageFile=firstImagesFile.at(i);
//                        if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
//                        {
//                            delete(imagesPointCloudByImageFile[firstImageFile]);
//                            imagesPointCloudByImageFile[firstImageFile]=NULL;
//                        }
//                        QString secondImageFile=secondImagesFile.at(i);
//                        if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
//                        {
//                            delete(imagesPointCloudByImageFile[secondImageFile]);
//                            imagesPointCloudByImageFile[secondImageFile]=NULL;
//                        }
//                    }
//                    return PERSISTENCEMANAGER_ERROR;
//                }
            }
        }
        //        sqlFile.close();
    }
    for(int i=0;i<firstImagesFile.size();i++)
    {
        QString firstImageFile=firstImagesFile.at(i);
        if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
        {
            delete(imagesPointCloudByImageFile[firstImageFile]);
            imagesPointCloudByImageFile[firstImageFile]=NULL;
        }
        QString secondImageFile=secondImagesFile.at(i);
        if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
        {
            delete(imagesPointCloudByImageFile[secondImageFile]);
            imagesPointCloudByImageFile[secondImageFile]=NULL;
        }
    }
    if(!mDB.commit())
    {
        qCritical() << mDB.lastError();
        return PERSISTENCEMANAGER_ERROR;
    }
    return PERSISTENCEMANAGER_NO_ERROR;

    int miliseconds=t.elapsed();

    //return PERSISTENCEMANAGER_NO_ERROR;
    emit operationFinished();
}

int PW::PersistenceManager::writeMatches_20140821(Project *project,
                                                  QString &metadata,
                                                  QString &description,
                                                  QString &timeCalculation,
                                                  QString &matchesPath)
{
    QDir auxDir=QDir::currentPath();
    if(!auxDir.exists(matchesPath))
    {
        QString msg=tr("PersistenceManager::writeMatches error: not exists path:\n%1").arg(matchesPath);
        qCritical() << mDB.lastError();
        return(PERSISTENCEMANAGER_ERROR);
    }
    QDir sourceDir(matchesPath);
    QStringList dirs = sourceDir.entryList(QDir::Dirs);
    QVector<QString> firstImagesFile;
    QVector<QString> secondImagesFile;
    QVector<QVector<double> > firstImagesPixelsColumn;
    QVector<QVector<double> > firstImagesPixelsRow;
    QVector<QVector<double> > secondImagesPixelsColumn;
    QVector<QVector<double> > secondImagesPixelsRow;
    QMap<QString,int> imagesIdByImageFile;
    QMap<QString,ImagePointCloud*> imagesPointCloudByImageFile;

    //    mDB.transaction();

    QSqlQuery query(mDB);

    /* // DHL-20140721
    QString beginQueryString = "BEGIN IMMEDIATE TRANSACTION";
    if(query.prepare(beginQueryString))
    {
        query.exec();

    }
    else
    {
        qCritical() << mDB.lastError();
        return(PERSISTENCEMANAGER_ERROR);
    }
    */

    for(int i = 0; i< dirs.count(); i++)
    {
        QString srcPath = dirs[i];
        if(srcPath.compare(".")==0
                ||srcPath.compare("..")==0)
        {
            continue;
        }
        if(!srcPath.contains(TAPIOCA_PASTIS_PATH,Qt::CaseInsensitive))
        {
            QString msg=tr("PersistenceManager::writeMatches error: path is not valid:\n%1").arg(srcPath);
            qCritical() << mDB.lastError();
            return(PERSISTENCEMANAGER_ERROR);
        }
        QString completeSrcPath=matchesPath+"/"+srcPath;
        QString firstImageFile=srcPath.remove(TAPIOCA_PASTIS_PATH);
        PW::PWImage* firstImage=project->getImageByName(firstImageFile);
        if(firstImage==NULL)
        {
            QString msg=tr("PersistenceManager::writeMatches error: not exists image:\n%1").arg(firstImageFile);
            qCritical() << mDB.lastError();
            return(PERSISTENCEMANAGER_ERROR);
        }
        if(!imagesIdByImageFile.contains(firstImageFile))
        {
            int firstImageId=firstImage->getID();
            imagesIdByImageFile[firstImageFile]=firstImageId;
            QSize imageSize=firstImage->getSize();
            ImagePointCloud* ptrImagePointCloud=new ImagePointCloud(0,0,imageSize.width()-1,imageSize.height()-1,LIBPW_IMAGE_POINTCLOUD_GRIDSIZE);
            imagesPointCloudByImageFile[firstImageFile]=ptrImagePointCloud;
        }
        QDir sourceFiles(completeSrcPath);
        QStringList files = sourceFiles.entryList(QDir::Files);
        for(int j = 0; j< files.count(); j++)
        {
            QString filePath = files[j];
            if(filePath.compare(".")==0
                    ||filePath.compare("..")==0)
            {
                continue;
            }
            QString completeFilePath=completeSrcPath+"/"+filePath;
            QString secondImageFile=filePath.remove(MATCHING_TXT_FILE_EXTENSION);
            PW::PWImage* secondImage=project->getImageByName(secondImageFile);
            if(secondImage==NULL)
            {
                QString msg=tr("PersistenceManager::writeMatches error: not exists image:\n%1").arg(secondImageFile);
                qCritical() << msg;
                return(PERSISTENCEMANAGER_ERROR);
            }
            firstImagesFile.push_back(firstImageFile);
            secondImagesFile.push_back(secondImageFile);
            if(!imagesIdByImageFile.contains(secondImageFile))
            {
                int secondImageId=secondImage->getID();
                imagesIdByImageFile[secondImageFile]=secondImageId;
                QSize imageSize=secondImage->getSize();
                ImagePointCloud* ptrImagePointCloud=new ImagePointCloud(0,0,imageSize.width()-1,imageSize.height()-1,LIBPW_IMAGE_POINTCLOUD_GRIDSIZE);
                imagesPointCloudByImageFile[secondImageFile]=ptrImagePointCloud;
            }
            QVector<double> firstImagePixelsColumn,firstImagePixelsRow;
            QVector<double> secondImagePixelsColumn,secondImagePixelsRow;
            QString strError;
            if(TIEPOINTSIO_NO_ERROR!=TiePointsIO::readTiePointsFile(completeFilePath,
                                                                    firstImagePixelsColumn,
                                                                    firstImagePixelsRow,
                                                                    secondImagePixelsColumn,
                                                                    secondImagePixelsRow,
                                                                    strError))
            {
                qCritical() << strError;
                return(PERSISTENCEMANAGER_ERROR);
            }
            firstImagesPixelsColumn.push_back(firstImagePixelsColumn);
            firstImagesPixelsRow.push_back(firstImagePixelsRow);
            secondImagesPixelsColumn.push_back(secondImagePixelsColumn);
            secondImagesPixelsRow.push_back(secondImagePixelsRow);
        }
    }

    int projectID=project->getID();
    // la primera, posible error
    QString queryString;
    int idOldMatchesCategories=0;
    int existsMatchesCategories;
    queryString="select count(*) as count, id from pw_matches_categories where metadata='"+metadata;
    queryString+="' AND id_project="+QString::number(projectID);
    if(query.prepare(queryString))
    {
        query.exec();
        query.next();
        existsMatchesCategories = query.record().value("count").toInt();
        idOldMatchesCategories = query.record().value("id").toInt();
    }
    else
    {
        qCritical() << mDB.lastError();
        for(int i=0;i<firstImagesFile.size();i++)
        {
            QString firstImageFile=firstImagesFile.at(i);
            if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[firstImageFile]);
                imagesPointCloudByImageFile[firstImageFile]=NULL;
            }
            QString secondImageFile=secondImagesFile.at(i);
            if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[secondImageFile]);
                imagesPointCloudByImageFile[secondImageFile]=NULL;
            }
        }
        return PERSISTENCEMANAGER_ERROR;
    }
    if(existsMatchesCategories>0) // Elimina los keypoints y los matches
    {
        int numberOfKeypointsToDelete=0;
        queryString="select count(*) as count from pw_matches where id_matches_category='"+QString::number(idOldMatchesCategories);
        queryString+="'";
        if(query.prepare(queryString))
        {
            query.exec();
            query.next();
            numberOfKeypointsToDelete = query.record().value("count").toInt();
        }
        else
        {
            qCritical() << mDB.lastError();
            for(int i=0;i<firstImagesFile.size();i++)
            {
                QString firstImageFile=firstImagesFile.at(i);
                if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                {
                    delete(imagesPointCloudByImageFile[firstImageFile]);
                    imagesPointCloudByImageFile[firstImageFile]=NULL;
                }
                QString secondImageFile=secondImagesFile.at(i);
                if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                {
                    delete(imagesPointCloudByImageFile[secondImageFile]);
                    imagesPointCloudByImageFile[secondImageFile]=NULL;
                }
            }
            return PERSISTENCEMANAGER_ERROR;
        }
        QVector<int> keypointsToDelete(2*numberOfKeypointsToDelete);
        queryString="select id_keypoint_first as id1,id_keypoint_second as id2 from pw_matches where id_matches_category='";
        queryString+=QString::number(idOldMatchesCategories);
        queryString+="'";
        int posToDelete=-1;
        if(query.prepare(queryString))
        {
            query.exec();
            while(query.next())
            {
                posToDelete++;
                int id1 = query.record().value("id1").toInt();
                keypointsToDelete[posToDelete]=id1;
                posToDelete++;
                int id2 = query.record().value("id2").toInt();
                keypointsToDelete[posToDelete]=id2;
            }
        }
        else
        {
            qCritical() << mDB.lastError();
            for(int i=0;i<firstImagesFile.size();i++)
            {
                QString firstImageFile=firstImagesFile.at(i);
                if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                {
                    delete(imagesPointCloudByImageFile[firstImageFile]);
                    imagesPointCloudByImageFile[firstImageFile]=NULL;
                }
                QString secondImageFile=secondImagesFile.at(i);
                if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                {
                    delete(imagesPointCloudByImageFile[secondImageFile]);
                    imagesPointCloudByImageFile[secondImageFile]=NULL;
                }
            }
            return PERSISTENCEMANAGER_ERROR;
        }
        queryString ="DELETE FROM pw_matches_categories WHERE metadata = '"+metadata+"' AND id_project=";
        queryString += QString::number(projectID);
        if(query.prepare(queryString))
        {
            query.exec();
        }
        else
        {
            qCritical() << mDB.lastError();
            for(int i=0;i<firstImagesFile.size();i++)
            {
                QString firstImageFile=firstImagesFile.at(i);
                if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                {
                    delete(imagesPointCloudByImageFile[firstImageFile]);
                    imagesPointCloudByImageFile[firstImageFile]=NULL;
                }
                QString secondImageFile=secondImagesFile.at(i);
                if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                {
                    delete(imagesPointCloudByImageFile[secondImageFile]);
                    imagesPointCloudByImageFile[secondImageFile]=NULL;
                }
            }
            return PERSISTENCEMANAGER_ERROR;
        }
        for(int mm=0;mm<keypointsToDelete.size();mm++)
        {
            queryString="delete from pw_keypoints where id='"+QString::number(keypointsToDelete[mm])+"'";
            if(query.prepare(queryString))
            {
                query.exec();
            }
            else
            {
                qCritical() << mDB.lastError();
                for(int i=0;i<firstImagesFile.size();i++)
                {
                    QString firstImageFile=firstImagesFile.at(i);
                    if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[firstImageFile]);
                        imagesPointCloudByImageFile[firstImageFile]=NULL;
                    }
                    QString secondImageFile=secondImagesFile.at(i);
                    if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[secondImageFile]);
                        imagesPointCloudByImageFile[secondImageFile]=NULL;
                    }
                }
                return PERSISTENCEMANAGER_ERROR;
            }
        }
    }

    QString strProjectID=QString::number(projectID);
    queryString = "INSERT INTO pw_matches_categories (metadata, description, time_calculation, id_project) ";
    queryString+="VALUES ('"+metadata+"','"+description+"','"+timeCalculation+"',"+strProjectID+")";
    int matchesCategoriesID;
    if(query.prepare(queryString))
    {
        query.exec();
        matchesCategoriesID = query.lastInsertId().toInt();
    }
    else
    {
        qCritical() << mDB.lastError();
        for(int i=0;i<firstImagesFile.size();i++)
        {
            QString firstImageFile=firstImagesFile.at(i);
            if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[firstImageFile]);
                imagesPointCloudByImageFile[firstImageFile]=NULL;
            }
            QString secondImageFile=secondImagesFile.at(i);
            if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[secondImageFile]);
                imagesPointCloudByImageFile[secondImageFile]=NULL;
            }
        }
        return 1;
    }
    QString strMatchesCategoriesID=QString::number(matchesCategoriesID);
    QString strImageCoordinatesCoincidenceTolerance=QString::number(LIBPW_IMAGE_COORDINATES_COINCIDENCE_TOLERANCE,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
    QString strError;

    // DHL-20140721
    QTime t;
    t.start();
    mDB.transaction();
    // DHL-20140721

    double pixelCoordinatesFactorDoubleToInteger=pow(10.0,LIBPW_IMAGE_COORDINATES_PRECISION*1.0);
    for(int i=0;i<firstImagesFile.size();i++)
    {
        QString firstImageFile=firstImagesFile.at(i);
        QString strFirstImageId=QString::number(imagesIdByImageFile[firstImageFile]);
        ImagePointCloud* ptrFirstImagePointCloud=imagesPointCloudByImageFile[firstImageFile];

        QString secondImageFile=secondImagesFile.at(i);
        QString strSecondImageId=QString::number(imagesIdByImageFile[secondImageFile]);
        ImagePointCloud* ptrSecondImagePointCloud=imagesPointCloudByImageFile[secondImageFile];

        QString strError;
        for(int j=0;j<firstImagesPixelsColumn[i].size();j++)
        {
            double firstImagePixelColumn=firstImagesPixelsColumn[i][j];
            double firstImagePixelRow=firstImagesPixelsRow[i][j];
            double secondImagePixelColumn=secondImagesPixelsColumn[i][j];
            double secondImagePixelRow=secondImagesPixelsRow[i][j];

            QString strFirstImagePixelColumn=QString::number(qRound(firstImagePixelColumn*pixelCoordinatesFactorDoubleToInteger));
            QString strFirstImagePixelRow=QString::number(qRound(firstImagePixelRow*pixelCoordinatesFactorDoubleToInteger));
            QString strSecondImagePixelColumn=QString::number(qRound(secondImagePixelColumn*pixelCoordinatesFactorDoubleToInteger));
            QString strSecondImagePixelRow=QString::number(qRound(secondImagePixelRow*pixelCoordinatesFactorDoubleToInteger));

            /* DHL-20140721
            QString strFirstImagePixelColumn=QString::number(firstImagePixelColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            QString strFirstImagePixelRow=QString::number(firstImagePixelRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            QString strSecondImagePixelColumn=QString::number(secondImagePixelColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            QString strSecondImagePixelRow=QString::number(secondImagePixelRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            /* DHL-20140721

            /* DHL-20140721
            float firstColumnClosestPoint,firstRowClosestPoint;
            float secondColumnClosestPoint,secondRowClosestPoint;
            if(ptrFirstImagePointCloud->getClosestPair(firstImagePixelColumn,
                                                       firstImagePixelRow,
                                                       imagesIdByImageFile[secondImageFile],
                                                       secondImagePixelColumn,
                                                       secondImagePixelRow,
                                                       LIBPW_IMAGE_COORDINATES_COINCIDENCE_TOLERANCE,
                                                       firstColumnClosestPoint,
                                                       firstRowClosestPoint)
                    ||ptrSecondImagePointCloud->getClosestPair(secondImagePixelColumn,
                                                               secondImagePixelRow,
                                                               imagesIdByImageFile[firstImageFile],
                                                               firstImagePixelColumn,
                                                               firstImagePixelRow,
                                                               LIBPW_IMAGE_COORDINATES_COINCIDENCE_TOLERANCE,
                                                               secondColumnClosestPoint,
                                                               secondRowClosestPoint))
            {
                continue;
            }
            else
            {
                if(!ptrFirstImagePointCloud->insertPair(firstImagePixelColumn,
                                                        firstImagePixelRow,
                                                        imagesIdByImageFile[secondImageFile],
                                                        secondImagePixelColumn,
                                                        secondImagePixelRow,
                                                        strError))
                {
                    qCritical() << strError;
                    for(int i=0;i<firstImagesFile.size();i++)
                    {
                        QString firstImageFile=firstImagesFile.at(i);
                        if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                        {
                            delete(imagesPointCloudByImageFile[firstImageFile]);
                            imagesPointCloudByImageFile[firstImageFile]=NULL;
                        }
                        QString secondImageFile=secondImagesFile.at(i);
                        if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                        {
                            delete(imagesPointCloudByImageFile[secondImageFile]);
                            imagesPointCloudByImageFile[secondImageFile]=NULL;
                        }
                    }
                    return 1;
                }
            }
            */

            //            QString strWktFirstPoint="GeomFromText('POINT(";
            //            strWktFirstPoint+=QString::number(firstImagePixelColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            //            strWktFirstPoint+=" ";
            //            strWktFirstPoint+=QString::number(firstImagePixelRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            //            strWktFirstPoint+=")',";
            //            strWktFirstPoint+=QString::number(LIBPW_IMAGE_COORDINATES_CRS);
            //            strWktFirstPoint+=")";
            //            QString strWktSecondPoint="GeomFromText('POINT(";
            //            strWktSecondPoint+=QString::number(secondImagePixelColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            //            strWktSecondPoint+=" ";
            //            strWktSecondPoint+=QString::number(secondImagePixelRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            //            strWktSecondPoint+=")',";
            //            strWktSecondPoint+=QString::number(LIBPW_IMAGE_COORDINATES_CRS);
            //            strWktSecondPoint+=")";
            QString strEnabled=QString::number(1);
            QString strAccuracy="-1.0";

            //            queryString = "INSERT INTO pw_keypoints (id_image, pixel_point_geom) ";
            //            queryString+="VALUES ("+strFirstImageId+","+strWktFirstPoint+")";
            queryString = "INSERT INTO pw_keypoints (id_image,x,y) ";
            //            queryString+="VALUES ("+strFirstImageId+","+QString::number(firstImagePixelColumn)+
            //                    ","+QString::number(firstImagePixelRow)+")";
            queryString+="VALUES ("+strFirstImageId+","+strFirstImagePixelColumn+
                    ","+strFirstImagePixelRow+")";
            int firstKeyPointID;
            if(query.prepare(queryString))
            {
                query.exec();
                firstKeyPointID = query.lastInsertId().toInt();
            }
            else
            {
                qCritical() << mDB.lastError();
                for(int i=0;i<firstImagesFile.size();i++)
                {
                    QString firstImageFile=firstImagesFile.at(i);
                    if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[firstImageFile]);
                        imagesPointCloudByImageFile[firstImageFile]=NULL;
                    }
                    QString secondImageFile=secondImagesFile.at(i);
                    if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[secondImageFile]);
                        imagesPointCloudByImageFile[secondImageFile]=NULL;
                    }
                }
                return 1;
            }
            QString strFirstKeyPointID=QString::number(firstKeyPointID);

            //            queryString = "INSERT INTO pw_keypoints (id_image, pixel_point_geom) ";
            //            queryString+="VALUES ("+strSecondImageId+","+strWktSecondPoint+")";
            queryString = "INSERT INTO pw_keypoints (id_image,x,y) ";
            //            queryString+="VALUES ("+strSecondImageId+","+QString::number(secondImagePixelColumn)+
            //                    ","+QString::number(secondImagePixelRow)+")";
            queryString+="VALUES ("+strSecondImageId+","+strSecondImagePixelColumn+
                    ","+strSecondImagePixelRow+")";
            int secondKeyPointID;
            if(query.prepare(queryString))
            {
                query.exec();
                secondKeyPointID = query.lastInsertId().toInt();
            }
            else
            {
                qCritical() << mDB.lastError();
                for(int i=0;i<firstImagesFile.size();i++)
                {
                    QString firstImageFile=firstImagesFile.at(i);
                    if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[firstImageFile]);
                        imagesPointCloudByImageFile[firstImageFile]=NULL;
                    }
                    QString secondImageFile=secondImagesFile.at(i);
                    if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[secondImageFile]);
                        imagesPointCloudByImageFile[secondImageFile]=NULL;
                    }
                }
                return 1;
            }
            QString strSecondKeyPointID=QString::number(secondKeyPointID);

            queryString = "INSERT INTO pw_matches (id_matches_category,id_keypoint_first,id_keypoint_second,enabled, accuracy) ";
            queryString+="VALUES ("+strMatchesCategoriesID+","+strFirstKeyPointID+","+strSecondKeyPointID+","+strEnabled+","+strAccuracy+")";
            int matchesID;
            if(query.prepare(queryString))
            {
                query.exec();
                matchesID = query.lastInsertId().toInt();
            }
            else
            {
                qCritical() << mDB.lastError();
                for(int i=0;i<firstImagesFile.size();i++)
                {
                    QString firstImageFile=firstImagesFile.at(i);
                    if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[firstImageFile]);
                        imagesPointCloudByImageFile[firstImageFile]=NULL;
                    }
                    QString secondImageFile=secondImagesFile.at(i);
                    if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[secondImageFile]);
                        imagesPointCloudByImageFile[secondImageFile]=NULL;
                    }
                }
                return 1;
            }
            QString strMatchesID=QString::number(matchesID);
        }
    }
    for(int i=0;i<firstImagesFile.size();i++)
    {
        QString firstImageFile=firstImagesFile.at(i);
        if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
        {
            delete(imagesPointCloudByImageFile[firstImageFile]);
            imagesPointCloudByImageFile[firstImageFile]=NULL;
        }
        QString secondImageFile=secondImagesFile.at(i);
        if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
        {
            delete(imagesPointCloudByImageFile[secondImageFile]);
            imagesPointCloudByImageFile[secondImageFile]=NULL;
        }
    }

    if(!mDB.commit()){
        qCritical() << mDB.lastError();
        return PERSISTENCEMANAGER_ERROR;
    }
    // DHL-20140721
    int miliseconds=t.elapsed();
    // DHL-20140721

    return PERSISTENCEMANAGER_NO_ERROR;
    emit operationFinished();
}

/* Para mantener lo anterior a la modificación de 20140506

int PW::PersistenceManager::writeMatches(Project *project,
                                         QString &metadata,
                                         QString &description,
                                         QString &timeCalculation,
                                         QString &matchesPath)
{
    QDir auxDir=QDir::currentPath();
    if(!auxDir.exists(matchesPath))
    {
        QString msg=tr("PersistenceManager::writeMatches error: not exists path:\n%1").arg(matchesPath);
        qCritical() << mDB.lastError();
        return(PERSISTENCEMANAGER_ERROR);
    }
    QDir sourceDir(matchesPath);
    QStringList dirs = sourceDir.entryList(QDir::Dirs);
    QVector<QString> firstImagesFile;
    QVector<QString> secondImagesFile;
    QVector<QVector<double> > firstImagesPixelsColumn;
    QVector<QVector<double> > firstImagesPixelsRow;
    QVector<QVector<double> > secondImagesPixelsColumn;
    QVector<QVector<double> > secondImagesPixelsRow;
    QMap<QString,int> imagesIdByImageFile;
    QMap<QString,ImagePointCloud*> imagesPointCloudByImageFile;
    mDB.transaction();
    QSqlQuery query(mDB);
    for(int i = 0; i< dirs.count(); i++)
    {
        QString srcPath = dirs[i];
        if(srcPath.compare(".")==0
                ||srcPath.compare("..")==0)
        {
            continue;
        }
        if(!srcPath.contains(TAPIOCA_PASTIS_PATH,Qt::CaseInsensitive))
        {
            QString msg=tr("PersistenceManager::writeMatches error: path is not valid:\n%1").arg(srcPath);
            qCritical() << mDB.lastError();
            return(PERSISTENCEMANAGER_ERROR);
        }
        QString completeSrcPath=matchesPath+"/"+srcPath;
        QString firstImageFile=srcPath.remove(TAPIOCA_PASTIS_PATH);
        PW::PWImage* firstImage=project->getImageByName(firstImageFile);
        if(firstImage==NULL)
        {
            QString msg=tr("PersistenceManager::writeMatches error: not exists image:\n%1").arg(firstImageFile);
            qCritical() << mDB.lastError();
            return(PERSISTENCEMANAGER_ERROR);
        }
        if(!imagesIdByImageFile.contains(firstImageFile))
        {
// ahora será una variable de PWImage -> en la clase Project getImageByUrl
            QString queryString ="SELECT id FROM pw_images WHERE url = '"+firstImageFile+"'";
            int firstImageId;
            if(query.prepare(queryString))
            {
                query.exec();
                if(query.next())
                {
                    firstImageId = query.record().value("id").toInt();
                }
                else
                {
                    QString msg=tr("PersistenceManager::writeMatches error: no image in database for url:\n%1").arg(firstImageFile);
                    qCritical() <<msg;
                    return(PERSISTENCEMANAGER_ERROR);
                }
            }
            else
            {
                qCritical() << mDB.lastError();
                return PERSISTENCEMANAGER_ERROR;
            }
            imagesIdByImageFile[firstImageFile]=firstImageId;
            QSize imageSize=firstImage->getSize();
            ImagePointCloud* ptrImagePointCloud=new ImagePointCloud(0,0,imageSize.width()-1,imageSize.height()-1,LIBPW_IMAGE_POINTCLOUD_GRIDSIZE);
            imagesPointCloudByImageFile[firstImageFile]=ptrImagePointCloud;
        }
        QDir sourceFiles(completeSrcPath);
        QStringList files = sourceFiles.entryList(QDir::Files);
        for(int j = 0; j< files.count(); j++)
        {
            QString filePath = files[j];
            if(filePath.compare(".")==0
                    ||filePath.compare("..")==0)
            {
                continue;
            }
            QString completeFilePath=completeSrcPath+"/"+filePath;
            QString secondImageFile=filePath.remove(MATCHING_TXT_FILE_EXTENSION);
            PW::PWImage* secondImage=project->getImageByName(secondImageFile);
            if(secondImage==NULL)
            {
                QString msg=tr("PersistenceManager::writeMatches error: not exists image:\n%1").arg(secondImageFile);
                qCritical() << msg;
                return(PERSISTENCEMANAGER_ERROR);
            }
            firstImagesFile.push_back(firstImageFile);
            secondImagesFile.push_back(secondImageFile);
            if(!imagesIdByImageFile.contains(secondImageFile))
            {
// ahora será una variable de PWImage
                QString queryString ="SELECT id FROM pw_images WHERE url = '"+secondImageFile+"'";
                int secondImageId;
                if(query.prepare(queryString))
                {
                    query.exec();
                    if(query.next())
                    {
                        secondImageId = query.record().value("id").toInt();
                    }
                    else
                    {
                        QString msg=tr("PersistenceManager::writeMatches error: no image in database for url:\n%1").arg(secondImageFile);
                        qCritical() <<msg;
                        return(PERSISTENCEMANAGER_ERROR);
                    }
                }
                else
                {
                    qCritical() << mDB.lastError();
                    return PERSISTENCEMANAGER_ERROR;
                }
                imagesIdByImageFile[secondImageFile]=secondImageId;
                QSize imageSize=secondImage->getSize();
                ImagePointCloud* ptrImagePointCloud=new ImagePointCloud(0,0,imageSize.width()-1,imageSize.height()-1,LIBPW_IMAGE_POINTCLOUD_GRIDSIZE);
                imagesPointCloudByImageFile[secondImageFile]=ptrImagePointCloud;
            }
            QVector<double> firstImagePixelsColumn,firstImagePixelsRow;
            QVector<double> secondImagePixelsColumn,secondImagePixelsRow;
            QString strError;
            if(TIEPOINTSIO_NO_ERROR!=TiePointsIO::readTiePointsFile(completeFilePath,
                                                                    firstImagePixelsColumn,
                                                                    firstImagePixelsRow,
                                                                    secondImagePixelsColumn,
                                                                    secondImagePixelsRow,
                                                                    strError))
            {
                qCritical() << strError;
                return(PERSISTENCEMANAGER_ERROR);
            }
            firstImagesPixelsColumn.push_back(firstImagePixelsColumn);
            firstImagesPixelsRow.push_back(firstImagePixelsRow);
            secondImagesPixelsColumn.push_back(secondImagePixelsColumn);
            secondImagesPixelsRow.push_back(secondImagePixelsRow);
        }
    }

    QString projectName=project->getName(); // es único
// ahora será una variable de Project
    QString queryString ="SELECT id FROM pw_projects WHERE name = '"+projectName+"'";
    int projectID;
    if(query.prepare(queryString))
    {
        query.exec();
        if(query.next())
        {
            projectID = query.record().value("id").toInt();
        }
        else
        {
            QString msg=tr("PersistenceManager::writeMatches error: no project in database for name:\n%1").arg(projectName);
            qCritical() << msg;
            for(int i=0;i<firstImagesFile.size();i++)
            {
                QString firstImageFile=firstImagesFile.at(i);
                if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                {
                    delete(imagesPointCloudByImageFile[firstImageFile]);
                    imagesPointCloudByImageFile[firstImageFile]=NULL;
                }
                QString secondImageFile=secondImagesFile.at(i);
                if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                {
                    delete(imagesPointCloudByImageFile[secondImageFile]);
                    imagesPointCloudByImageFile[secondImageFile]=NULL;
                }
            }
            return(PERSISTENCEMANAGER_ERROR);
        }
    }
    else
    {
        qCritical() << mDB.lastError();
        for(int i=0;i<firstImagesFile.size();i++)
        {
            QString firstImageFile=firstImagesFile.at(i);
            if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[firstImageFile]);
                imagesPointCloudByImageFile[firstImageFile]=NULL;
            }
            QString secondImageFile=secondImagesFile.at(i);
            if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[secondImageFile]);
                imagesPointCloudByImageFile[secondImageFile]=NULL;
            }
        }
        return PERSISTENCEMANAGER_ERROR;
    }
// la primera, posible error
    queryString ="DELETE FROM pw_matches_categories WHERE metadata = '"+metadata+"' AND id_project=";
    queryString += QString::number(projectID);
    if(query.prepare(queryString))
    {
        query.exec();
    }
    else
    {
        qCritical() << mDB.lastError();
        for(int i=0;i<firstImagesFile.size();i++)
        {
            QString firstImageFile=firstImagesFile.at(i);
            if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[firstImageFile]);
                imagesPointCloudByImageFile[firstImageFile]=NULL;
            }
            QString secondImageFile=secondImagesFile.at(i);
            if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[secondImageFile]);
                imagesPointCloudByImageFile[secondImageFile]=NULL;
            }
        }
        return PERSISTENCEMANAGER_ERROR;
    }

    QString strProjectID=QString::number(projectID);
    queryString = "INSERT INTO pw_matches_categories (metadata, description, time_calculation, id_project) ";
    queryString+="VALUES ('"+metadata+"','"+description+"','"+timeCalculation+"',"+strProjectID+")";
    int matchesCategoriesID;
    if(query.prepare(queryString))
    {
        query.exec();
        matchesCategoriesID = query.lastInsertId().toInt();
    }
    else
    {
        qCritical() << mDB.lastError();
        for(int i=0;i<firstImagesFile.size();i++)
        {
            QString firstImageFile=firstImagesFile.at(i);
            if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[firstImageFile]);
                imagesPointCloudByImageFile[firstImageFile]=NULL;
            }
            QString secondImageFile=secondImagesFile.at(i);
            if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[secondImageFile]);
                imagesPointCloudByImageFile[secondImageFile]=NULL;
            }
        }
        return 1;
    }
    QString strMatchesCategoriesID=QString::number(matchesCategoriesID);
    QString strImageCoordinatesCoincidenceTolerance=QString::number(LIBPW_IMAGE_COORDINATES_COINCIDENCE_TOLERANCE,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
    QString strError;

    // Leer los valores de la base de datos e insertarlos en las nubes de puntos de las imagenes
    // SELECT k.id as id,ST_X(k.the_geom) as x,ST_Y(k.the_geom) as y FROM pw_keypoints AS k,pw_images AS i WHERE i.id_project=1 AND k.id_image=i.id
    queryString="SELECT k.id as id,ST_X(k.the_geom) as x,ST_Y(k.the_geom) as y FROM pw_keypoints AS k, pw_matches_categories AS m WHERE m.id_project=";
    queryString+=QString::number(projectID);
    queryString+=" AND  k.id_matches_category=m.id";
    QMap<int,float> keyPointColumns;
    QMap<int,float> keyPointRows;
    if(query.prepare(queryString))
    {
        query.exec();
        while(query.next())
        {
            int keyPointId = query.record().value("id").toInt();
            float keyPointColumn = query.record().value("x").toFloat();
            float keyPointRow = query.record().value("y").toFloat();
            keyPointColumns[keyPointId]=keyPointColumn;
            keyPointRows[keyPointId]=keyPointRow;
        }
    }
    else
    {
        QString msg=tr("PersistenceManager::writeMatches error getting keypoints:\n%1");
        qCritical() << msg;
        for(int i=0;i<firstImagesFile.size();i++)
        {
            QString firstImageFile=firstImagesFile.at(i);
            if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[firstImageFile]);
                imagesPointCloudByImageFile[firstImageFile]=NULL;
            }
            QString secondImageFile=secondImagesFile.at(i);
            if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
            {
                delete(imagesPointCloudByImageFile[secondImageFile]);
                imagesPointCloudByImageFile[secondImageFile]=NULL;
            }
        }
        return(PERSISTENCEMANAGER_ERROR);
    }
    if(keyPointColumns.size()>0)
    {
        //select m.id_keypoint_first,m.id_keypoint_second from pw_matches as m, pw_matches_categories as mc where m.id_image_first=1 and m.id_image_second=2 and mc.id_project=1 and mc.id=m.id_matches_category;
        for(int i=0;i<firstImagesFile.size();i++)
        {
            QString firstImageFile=firstImagesFile.at(i);
            QString strFirstImageId=QString::number(imagesIdByImageFile[firstImageFile]);
            ImagePointCloud* ptrFirstImagePointCloud=imagesPointCloudByImageFile[firstImageFile];

            QString secondImageFile=secondImagesFile.at(i);
            QString strSecondImageId=QString::number(imagesIdByImageFile[secondImageFile]);
            ImagePointCloud* ptrSecondImagePointCloud=imagesPointCloudByImageFile[secondImageFile];

// SELECT m.id_keypoint_first as idKp1,m.id_keypoint_second as idKp2 FROM pw_matches AS m,pw_images AS i, pw_keypoints AS k WHERE ((k.id_image=1 AND k.id=m.id_keypoint_first) AND (k.id_image=2 AND k.id=m.id_keypoint_second))
// tema del sentido contrario

            queryString="SELECT m.id as id, m.id_keypoint_first as id1,m.id_keypoint_second as id2 FROM pw_matches as m, pw_matches_categories as mc WHERE ";
            queryString+="m.id_image_first="+strFirstImageId;
            queryString+=" AND m.id_image_second="+strSecondImageId;
            queryString+=" AND mc.id_project="+QString::number(projectID)+" AND mc.id=m.id_matches_category;";

            if(query.prepare(queryString))
            {
                query.exec();
                while(query.next())
                {
                    int pairId=query.record().value("id").toInt();
                    int firstKeyPointId = query.record().value("id1").toInt();
                    int secondKeyPointId = query.record().value("id2").toInt();
                    float firstKeyPointColumn=keyPointColumns[firstKeyPointId];
                    float firstKeyPointRow=keyPointRows[firstKeyPointId];
                    float secondKeyPointColumn=keyPointColumns[secondKeyPointId];
                    float secondKeyPointRow=keyPointRows[secondKeyPointId];
                    if(!ptrFirstImagePointCloud->insertPair(firstKeyPointColumn,
                                                            firstKeyPointRow,
                                                            pairId,
                                                            secondKeyPointColumn,
                                                            secondKeyPointRow,
                                                            strError))
                    {
                        qCritical() << strError;
                        for(int i=0;i<firstImagesFile.size();i++)
                        {
                            QString firstImageFile=firstImagesFile.at(i);
                            if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                            {
                                delete(imagesPointCloudByImageFile[firstImageFile]);
                                imagesPointCloudByImageFile[firstImageFile]=NULL;
                            }
                            QString secondImageFile=secondImagesFile.at(i);
                            if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                            {
                                delete(imagesPointCloudByImageFile[secondImageFile]);
                                imagesPointCloudByImageFile[secondImageFile]=NULL;
                            }
                        }
                        return(PERSISTENCEMANAGER_ERROR);
                    }
                }
            }
            else
            {
                QString msg=tr("PersistenceManager::writeMatches error getting keypoints:\n%1");
                qCritical() << msg;
                for(int i=0;i<firstImagesFile.size();i++)
                {
                    QString firstImageFile=firstImagesFile.at(i);
                    if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[firstImageFile]);
                        imagesPointCloudByImageFile[firstImageFile]=NULL;
                    }
                    QString secondImageFile=secondImagesFile.at(i);
                    if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[secondImageFile]);
                        imagesPointCloudByImageFile[secondImageFile]=NULL;
                    }
                }
                return(PERSISTENCEMANAGER_ERROR);
            }
        }
    }

    for(int i=0;i<firstImagesFile.size();i++)
    {
        QString firstImageFile=firstImagesFile.at(i);
        QString strFirstImageId=QString::number(imagesIdByImageFile[firstImageFile]);
        ImagePointCloud* ptrFirstImagePointCloud=imagesPointCloudByImageFile[firstImageFile];

        QString secondImageFile=secondImagesFile.at(i);
        QString strSecondImageId=QString::number(imagesIdByImageFile[secondImageFile]);
        ImagePointCloud* ptrSecondImagePointCloud=imagesPointCloudByImageFile[secondImageFile];

        QString strError;
        for(int j=0;j<firstImagesPixelsColumn[i].size();j++)
        {
            double firstImagePixelColumn=firstImagesPixelsColumn[i][j];
            QString strFirstImagePixelColumn=QString::number(firstImagePixelColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            double firstImagePixelRow=firstImagesPixelsRow[i][j];
            QString strFirstImagePixelRow=QString::number(firstImagePixelRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            double secondImagePixelColumn=secondImagesPixelsColumn[i][j];
            QString strSecondImagePixelColumn=QString::number(secondImagePixelColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            double secondImagePixelRow=secondImagesPixelsRow[i][j];
            QString strSecondImagePixelRow=QString::number(secondImagePixelRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);

            float firstColumnClosestPoint,firstRowClosestPoint;
            float secondColumnClosestPoint,secondRowClosestPoint;
            if(ptrFirstImagePointCloud->getClosestPair(firstImagePixelColumn,
                                                       firstImagePixelRow,
                                                       imagesIdByImageFile[secondImageFile],
                                                       secondImagePixelColumn,
                                                       secondImagePixelRow,
                                                       LIBPW_IMAGE_COORDINATES_COINCIDENCE_TOLERANCE,
                                                       firstColumnClosestPoint,
                                                       firstRowClosestPoint)
                    ||ptrSecondImagePointCloud->getClosestPair(secondImagePixelColumn,
                                                               secondImagePixelRow,
                                                               imagesIdByImageFile[firstImageFile],
                                                               firstImagePixelColumn,
                                                               firstImagePixelRow,
                                                               LIBPW_IMAGE_COORDINATES_COINCIDENCE_TOLERANCE,
                                                               secondColumnClosestPoint,
                                                               secondRowClosestPoint))
            {
                continue;
            }
            else
            {
                if(!ptrFirstImagePointCloud->insertPair(firstImagePixelColumn,
                                                        firstImagePixelRow,
                                                        imagesIdByImageFile[secondImageFile],
                                                        secondImagePixelColumn,
                                                        secondImagePixelRow,
                                                        strError))
                {
                    qCritical() << strError;
                    for(int i=0;i<firstImagesFile.size();i++)
                    {
                        QString firstImageFile=firstImagesFile.at(i);
                        if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                        {
                            delete(imagesPointCloudByImageFile[firstImageFile]);
                            imagesPointCloudByImageFile[firstImageFile]=NULL;
                        }
                        QString secondImageFile=secondImagesFile.at(i);
                        if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                        {
                            delete(imagesPointCloudByImageFile[secondImageFile]);
                            imagesPointCloudByImageFile[secondImageFile]=NULL;
                        }
                    }
                    return 1;
                }
            }

//            QString strWktLine="GeomFromText('LINESTRING(";
//            strWktLine+=QString::number(firstImagePixelColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//            strWktLine+=" ";
//            strWktLine+=QString::number(firstImagePixelRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//            strWktLine+=",";
//            strWktLine+=QString::number(secondImagePixelColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//            strWktLine+=" ";
//            strWktLine+=QString::number(secondImagePixelRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//            strWktLine+=")',";
//            strWktLine+=QString::number(LIBPW_IMAGE_COORDINATES_CRS);
//            strWktLine+=")";

            //select id, astext(the_geom) from pw_matches where within(the_geom,buffer(
            //GeomFromText('LINESTRING(2584.292 251.051,3793.973 336.001)',-1),0.005))
//            queryString="SELECT id FROM pw_matches WHERE Within(the_geom,buffer(";
//            queryString+=strWktLine;
//            queryString+=",";
//            queryString+=strImageCoordinatesCoincidenceTolerance;
//            queryString+="))";
//            QVector<int> coincidenceIds;
//            if(query.prepare(queryString))
//            {
//                query.exec();
//                while(query.next())
//                {
//                    int coincidenceId = query.record().value("id").toInt();
//                    coincidenceIds.push_back(coincidenceId);
//                }
//            }
//            else
//            {
//                qCritical() << mDB.lastError();
//                return 1;
//            }
//            if(coincidenceIds.size()>0) // supongo que si están al contrario no hay problema
//            {
//                // Si hay alguno con el mismo orden de ids de imágenes no lo añadiré
//                bool finalCoincidence=false;
//                for(int k=0;k<coincidenceIds.size();k++)
//                {
//                    queryString="select id_image_first,id_image_second from pw_matches where id=";
//                    queryString+=QString::number(coincidenceIds[k]);
//                    int coincidenceIdImageFirst,coincidenceIdImageSecond;
//                    if(query.prepare(queryString))
//                    {
//                        query.exec();
//                        if(query.next())
//                        {
//                            coincidenceIdImageFirst = query.record().value("id_image_first").toInt();
//                            coincidenceIdImageSecond = query.record().value("id_image_second").toInt();
//                        }
//                    }
//                    else
//                    {
//                        qCritical() << mDB.lastError();
//                        return 1;
//                    }
//                    if(imagesIdByImageFile[firstImageFile]==coincidenceIdImageFirst
//                            &&imagesIdByImageFile[secondImageFile]==coincidenceIdImageSecond)
//                    {
//                        finalCoincidence=true;
//                        break;
//                    }
//                }
//                if(finalCoincidence)
//                    continue;
//            }
//            float firstImageColumnClosestPoint=-1.0;
//            float firstImageRowClosestPoint=-1.0;
//            bool pointInFirstImage=ptrFirstImagePointCloud->getClosestPoint(firstImagePixelColumn,
//                                                                            firstImagePixelRow,
//                                                                            LIBPW_IMAGE_COORDINATES_COINCIDENCE_TOLERANCE,
//                                                                            firstImageColumnClosestPoint,
//                                                                            firstImageRowClosestPoint);

//            float secondImageColumnClosestPoint=-1.0;
//            float secondImageRowClosestPoint=-1.0;
//            bool pointInSecondImage=ptrSecondImagePointCloud->getClosestPoint(secondImagePixelColumn,
//                                                                              secondImagePixelRow,
//                                                                              LIBPW_IMAGE_COORDINATES_COINCIDENCE_TOLERANCE,
//                                                                              secondImageColumnClosestPoint,
//                                                                              secondImageRowClosestPoint);
//            if(pointInFirstImage&&pointInSecondImage)
//            {
//                continue;
//            }
//            if(!ptrFirstImagePointCloud->insertPoint(firstImagePixelColumn,firstImagePixelRow,strError))
//            {
//                qCritical() << strError;
//                for(int i=0;i<firstImagesFile.size();i++)
//                {
//                    QString firstImageFile=firstImagesFile.at(i);
//                    delete(imagesPoinCloudByImageFile[firstImageFile]);

//                    QString secondImageFile=secondImagesFile.at(i);
//                    delete(imagesPoinCloudByImageFile[secondImageFile]);
//                }
//                return 1;
//            }
//            if(!ptrSecondImagePointCloud->insertPoint(secondImagePixelColumn,secondImagePixelRow,strError))
//            {
//                qCritical() << strError;
//                for(int i=0;i<firstImagesFile.size();i++)
//                {
//                    QString firstImageFile=firstImagesFile.at(i);
//                    delete(imagesPoinCloudByImageFile[firstImageFile]);

//                    QString secondImageFile=secondImagesFile.at(i);
//                    delete(imagesPoinCloudByImageFile[secondImageFile]);
//                }
//                return 1;
//            }

            // Consulta para ver si ya existe un match entre las dos imagenes en el mismo proyecto
            // pero en cualquier categoria de matches
//            select count(*) from pw_matches as m where m.id_keypoint_first in
//            (select kp.id from pw_keypoints as kp where ST_Distance (kp.pixel_point_geom,MakePoint (3806.026123,929.570984))<0.01
//            and kp.id in (select m.id_keypoint_first from pw_matches as m, pw_matches_categories as c where c.id_project=1
//            and m.id_image_first=3 and m.id_image_second=1)) and  m.id_keypoint_second in (select kp.id from
//            pw_keypoints as kp where ST_Distance (kp.pixel_point_geom,MakePoint (980.354065,924.576111))<0.01
//            and kp.id in (select m.id_keypoint_second from pw_matches as m, pw_matches_categories as c where c.id_project=1
//            and m.id_image_first=3 and m.id_image_second=1))
//            QString queryString1="SELECT count(*) FROM pw_matches WHERE id_keypoint_first IN ";
//            queryString1+="(SELECT id FROM pw_keypoints WHERE ST_Distance (pixel_point_geom,MakePoint (";
//            queryString1+=strFirstImagePixelColumn+","+strFirstImagePixelRow+"))<"+strImageCoordinatesCoincidenceTolerance;
//            queryString1+=" and id in (select m.id_keypoint_first from pw_matches as m, pw_matches_categories as c where c.id_project=";
//            queryString1+=strProjectID;
//            queryString1+=" and m.id_image_first=";
//            queryString1+=strFirstImageId;
//            queryString1+=" and m.id_image_second=";
//            queryString1+=strSecondImageId;
//            queryString1+=")) ";
//            QString queryString2="and id_keypoint_second in (select id from pw_keypoints where ";
//            queryString2+="ST_Distance (pixel_point_geom,MakePoint (";
//            queryString2+=strSecondImagePixelColumn+","+strSecondImagePixelRow+"))<"+strImageCoordinatesCoincidenceTolerance;
//            queryString2+=" and id in (select m.id_keypoint_second from pw_matches as m, pw_matches_categories as c where c.id_project=";
//            queryString2+=strProjectID;
//            queryString2+=" and m.id_image_first=";
//            queryString2+=strFirstImageId;
//            queryString2+=" and m.id_image_second=";
//            queryString2+=strSecondImageId;
//            queryString2+="))";
//            queryString=queryString1+queryString2;
//            int numberOfCoincidences=0;
//            if(query.prepare(queryString))
//            {
//                query.exec();
//                if(query.next())
//                {
//                    numberOfCoincidences = query.record().value("id").toInt();
//                }
//            }
//            else
//            {
//                qCritical() << mDB.lastError();
//                return 1;
//            }
//            if(numberOfCoincidences>0)
//                continue;
            QString strWktFirstPoint="GeomFromText('POINT(";
            strWktFirstPoint+=QString::number(firstImagePixelColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            strWktFirstPoint+=" ";
            strWktFirstPoint+=QString::number(firstImagePixelRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            strWktFirstPoint+=")',";
            strWktFirstPoint+=QString::number(LIBPW_IMAGE_COORDINATES_CRS);
            strWktFirstPoint+=")";
            QString strWktSecondPoint="GeomFromText('POINT(";
            strWktSecondPoint+=QString::number(secondImagePixelColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            strWktSecondPoint+=" ";
            strWktSecondPoint+=QString::number(secondImagePixelRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
            strWktSecondPoint+=")',";
            strWktSecondPoint+=QString::number(LIBPW_IMAGE_COORDINATES_CRS);
            strWktSecondPoint+=")";
            QString strEnabled=QString::number(1);
            QString strAccuracy="-1.0";

// corregir primero insertar los keypoints y elimino el update
            queryString = "INSERT INTO pw_matches (id_matches_category, id_image_first, id_image_second, enabled, accuracy) ";
            queryString+="VALUES ("+strMatchesCategoriesID+","+strFirstImageId+","+strSecondImageId+","+strEnabled+","+strAccuracy+")";
            int matchesID;
            if(query.prepare(queryString))
            {
                query.exec();
                matchesID = query.lastInsertId().toInt();
            }
            else
            {
                qCritical() << mDB.lastError();
                for(int i=0;i<firstImagesFile.size();i++)
                {
                    QString firstImageFile=firstImagesFile.at(i);
                    if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[firstImageFile]);
                        imagesPointCloudByImageFile[firstImageFile]=NULL;
                    }
                    QString secondImageFile=secondImagesFile.at(i);
                    if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[secondImageFile]);
                        imagesPointCloudByImageFile[secondImageFile]=NULL;
                    }
                }
                return 1;
            }
            QString strMatchesID=QString::number(matchesID);

            queryString = "INSERT INTO pw_keypoints (id_match, enabled, id_matches_category, id_image, the_geom) ";
            queryString+="VALUES ("+strMatchesID+","+strEnabled+","+strMatchesCategoriesID+","+strFirstImageId+","+strWktFirstPoint+")";
            int firstKeyPointID;
            if(query.prepare(queryString))
            {
                query.exec();
                firstKeyPointID = query.lastInsertId().toInt();
            }
            else
            {
                qCritical() << mDB.lastError();
                for(int i=0;i<firstImagesFile.size();i++)
                {
                    QString firstImageFile=firstImagesFile.at(i);
                    if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[firstImageFile]);
                        imagesPointCloudByImageFile[firstImageFile]=NULL;
                    }
                    QString secondImageFile=secondImagesFile.at(i);
                    if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[secondImageFile]);
                        imagesPointCloudByImageFile[secondImageFile]=NULL;
                    }
                }
                return 1;
            }
            QString strFirstKeyPointID=QString::number(firstKeyPointID);

            queryString = "INSERT INTO pw_keypoints (id_match, enabled, id_matches_category, id_image, the_geom) ";
            queryString+="VALUES ("+strMatchesID+","+strEnabled+","+strMatchesCategoriesID+","+strSecondImageId+","+strWktSecondPoint+")";
            int secondKeyPointID;
            if(query.prepare(queryString))
            {
                query.exec();
                secondKeyPointID = query.lastInsertId().toInt();
            }
            else
            {
                qCritical() << mDB.lastError();
                for(int i=0;i<firstImagesFile.size();i++)
                {
                    QString firstImageFile=firstImagesFile.at(i);
                    if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[firstImageFile]);
                        imagesPointCloudByImageFile[firstImageFile]=NULL;
                    }
                    QString secondImageFile=secondImagesFile.at(i);
                    if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[secondImageFile]);
                        imagesPointCloudByImageFile[secondImageFile]=NULL;
                    }
                }
                return 1;
            }
            QString strSecondKeyPointID=QString::number(secondKeyPointID);

            queryString = "UPDATE pw_matches SET id_keypoint_first ="+strFirstKeyPointID+" WHERE id = " + strMatchesID;
            if(query.prepare(queryString))
            {
                query.exec();
            }
            else
            {
                qCritical() << mDB.lastError();
                return 1;
            }

            queryString = "UPDATE pw_matches SET id_keypoint_second ="+strSecondKeyPointID+" WHERE id = " + strMatchesID;
            if(query.prepare(queryString))
            {
                query.exec();
            }
            else
            {
                qCritical() << mDB.lastError();
                for(int i=0;i<firstImagesFile.size();i++)
                {
                    QString firstImageFile=firstImagesFile.at(i);
                    if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[firstImageFile]);
                        imagesPointCloudByImageFile[firstImageFile]=NULL;
                    }
                    QString secondImageFile=secondImagesFile.at(i);
                    if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
                    {
                        delete(imagesPointCloudByImageFile[secondImageFile]);
                        imagesPointCloudByImageFile[secondImageFile]=NULL;
                    }
                }
                return 1;
            }

            int yo=1;
        }
    }
    for(int i=0;i<firstImagesFile.size();i++)
    {
        QString firstImageFile=firstImagesFile.at(i);
        if(imagesPointCloudByImageFile[firstImageFile]!=NULL)
        {
            delete(imagesPointCloudByImageFile[firstImageFile]);
            imagesPointCloudByImageFile[firstImageFile]=NULL;
        }
        QString secondImageFile=secondImagesFile.at(i);
        if(imagesPointCloudByImageFile[secondImageFile]!=NULL)
        {
            delete(imagesPointCloudByImageFile[secondImageFile]);
            imagesPointCloudByImageFile[secondImageFile]=NULL;
        }
    }

    if(!mDB.commit()){
        qCritical() << mDB.lastError();
        return PERSISTENCEMANAGER_ERROR;
    }

    return PERSISTENCEMANAGER_NO_ERROR;
}

*/
int replace(QFile *inputFile, QFile *outputFile, QString from, QString to)
{
    if (inputFile->open(QIODevice::ReadOnly)){
        QString baseString(inputFile->readAll());
        baseString.replace(QRegExp(from),to);
        inputFile->close();

        if(outputFile->open(QIODevice::WriteOnly)){
            QTextStream outputStream(outputFile);
            outputStream << baseString << "\n";

            outputFile->close();
        }
    }

    return 0;
}

int PW::PersistenceManager::writeCamera(PW::Camera *camera)
{
    QSqlQuery query(mDB);
    int cameraID = 0;


    QString queryString = "SELECT * FROM pw_cameras WHERE name = '" + camera->getName() +
            "' AND lens = '" + camera->getLensName() + "'";
    if(query.prepare(queryString)){
        query.exec();
        if(query.next()){  // Update
            cameraID = query.record().value("id").toInt();
            queryString = "UPDATE pw_cameras SET name ='" + camera->getName() +
                    "', lens ='" + camera->getName() +
                    "', sensor_width =" + QString::number(camera->getSensorWidth()) +
                    ", sensor_height = " + QString::number(camera->getSensorHeight()) +
                    ", focal = " + QString::number(camera->getDeclaredFocal()) +
                    ", scale_factor = " + QString::number(camera->getScaleFactor35());
            if(query.prepare(queryString)){
                query.exec();
            }
        }
        else{ // Insert
            queryString = "INSERT INTO pw_cameras (name, lens, sensor_width, sensor_height, focal, scale_factor) VALUES ('" +
                    camera->getName() + "','" +
                    camera->getLensName() + "'," +
                    QString::number(camera->getSensorWidth()) + "," +
                    QString::number(camera->getSensorHeight()) + "," +
                    QString::number(camera->getDeclaredFocal()) + "," +
                    QString::number(camera->getScaleFactor35()) + ")";
            if(query.prepare(queryString)){
                query.exec();
                cameraID = query.lastInsertId().toInt();
            }
        }
    }
    //Write camera on MICMAC xlm:
    //    QString dicoCameraPath = LibPW::getMicMacBinPath() + "../include/XML_User/DicoCamera.xml";
    //    QFile dicoCamera(dicoCameraPath);
    //    if (dicoCamera.exists()){
    //        QString newFileContent = QString ("\t<CameraEntry>\n\t\t<Name>") + camera->getName() + QString("</Name>\n\t\t<SzCaptMm>") +
    //                QString::number(camera->getSensorWidth()) + " " + QString::number(camera->getSensorHeight()) +
    //                QString("</SzCaptMm>\n\t\t<ShortName>") + camera->getName() +
    //                QString("</ShortName>\n\t</CameraEntry>\n\n") + QString("</MMCameraDataBase>");

    //        replace(&dicoCamera,&dicoCamera,"</MMCameraDataBase>", newFileContent);
    //    }

    return cameraID;
}
