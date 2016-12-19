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
#include <math.h>
#include <QDebug>
#include <QFile>
#include <QDir>
//#include <Eigen/Dense>

#include "Project.h"
#include "MicMac/AperoModelReader.h"
#include "MicMac/MicMacEOReader.h"
#include "PhotogrammetricModel.h"
#include "ExternalOrientation.h"
#include "RadialExtended.h"
#include "libPW.h"
#include "PWMatchesCategory.h"
using namespace PW;

Project::Project():
    mName(""),
    mNeedToSave(false),
    mMatchingFormat(MATCHING_FORMAT_APERO_BIN),
    mOriginImage(0),
    mScaleImageA(0),
    mScaleImageB(0),
    mOXImage(0),
    mScaleDistance(0.0),
    mCloudUUID(""),
    mID(0),
    mPreprocessingPath("")
{
}

Project::~Project() {
    for(int i = 0; i< mInputImages.count(); i++)
        delete mInputImages.at(i);

    for(int i=0; i< mModelCPoints.count(); i++)
        delete mModelCPoints.at(i);

    for(int i=0; i<mMatchesCategories.count(); i++){
       delete mMatchesCategories.values().at(i);
    }

    //    QMutableMapIterator<QString, Camera *> i(mCameras);
    //     while (i.hasNext()){
    //         Camera * cam = i.next().value();
    //         if (cam!=NULL)
    //             delete cam;
    //     }

    //    QMap<QString, Camera *>::const_iterator i = mCameras.constBegin();
    //    while (i != mCameras.constEnd()) {
    //        delete i.value();
    //        ++i;
    //    }
}

int Project::getID()
{
    return mID;
}

void PW::Project::addImputImages(QList<PWImage*> imagesList)
{
    mInputImages.append(imagesList);
    mNeedToSave = true;
}

bool Project::needToSave()
{
    return mNeedToSave;
}

void Project::setNeedToSave(bool needToSave)
{
    mNeedToSave = needToSave;
}

void Project::addCamera(Camera * camera)
{
    QString key = camera->getName();
    if (!camera->getLensName().isEmpty())
        key = key + " - " + camera->getLensName();
    mCameras.insert(key ,camera);
}

void Project::setCurrentOrientation(QString orientationName)
{
    mCurrentOrientation = orientationName;
}

void Project::setMatchingFormat(int format)
{
    mMatchingFormat = format;
}

void Project::setActiveTiePointsSet(QString name)
{
    mActiveTiePointsSet = name;
}

void Project::setOriginPoint(QPointF point)
{
    mOriginPoint = point;
}

void Project::setOX1(QPointF point)
{
    mOX1 = point;
}

void Project::setOX2(QPointF point)
{
    mOX2 = point;
}

void Project::setScalePointA1(QPointF point)
{
    mScalePointA1 = point;
}

void Project::setScalePointA2(QPointF point)
{
    mScalePointA2 = point;
}

void Project::setScalePointB1(QPointF point)
{
    mScalePointB1 = point;
}

void Project::setScalePointB2(QPointF point)
{
    mScalePointB2 = point;
}

int PW::Project::addImputImages(QStringList imagePathList)
{
    for (int i=0; i<imagePathList.count(); i++){
        mInputImages.append(new PWImage(imagePathList.at(i)));
    }
    mNeedToSave = true;
    return 0;
}

QList<PW::PWImage *> PW::Project::getImages()
{
    return mInputImages;
}

QVector<QString> Project::getImagesFileName()
{
    QVector<QString> imagesFileName;
    for(int i=0;i<mInputImages.size();i++)
    {
        imagesFileName.push_back(mInputImages.at(i)->getFileName());
    }
    return(imagesFileName);
}

PWImage *PW::Project::getImageByURL(QString url)
{
    for(int i=0; i<mInputImages.count();i++){
        if (mInputImages.at(i)->getFullPath() == url)
            return mInputImages.at(i);
    }
    return NULL;
}

PWImage *PW::Project::getImageByName(QString name)
{
    for(int i=0; i<mInputImages.count();i++){
        if (mInputImages.at(i)->getFileName() == name)
            return mInputImages.at(i);
    }
    return NULL;
}

void Project::removeImageByName(QString name)
{
    QMutableListIterator<PWImage*> i(mInputImages);
    while (i.hasNext()) {
        if (name == i.next()->getFileName()){
            i.remove();
            //Remove Image Pairs:
            foreach (QString key, mMatchesCategories.keys()){
                PWMatchesCategory *category = mMatchesCategories.value(key);
                category->removeImagePairs(name);
                if (category->getImagePairsCount()==0)
                    mMatchesCategories.remove(key);
            }
        }
    }
    mNeedToSave = true;
}

void Project::removeImagesByName(QStringList namesList)
{
    QMutableListIterator<PWImage*> i(mInputImages);
    while (i.hasNext()) {
        QString name = i.next()->getFileName();
        if (namesList.contains(name)){
            i.remove();
            //Remove Image Pairs:
            foreach (QString key, mMatchesCategories.keys()){
                PWMatchesCategory *category = mMatchesCategories.value(key);
                category->removeImagePairs(name);
                if (category->getImagePairsCount()==0)
                    mMatchesCategories.remove(key);
            }
        }
    }
    mNeedToSave = true;
}

QList<PW3dPoint *> *PW::Project::getModelCPoints()
{
    return &mModelCPoints;
}

QString Project::getActiveTiePointsSet()
{
    return mActiveTiePointsSet;
}

QPointF Project::getOriginPoint()
{
    return mOriginPoint;
}

QPointF Project::getOX1()
{
    return mOX1;
}

QPointF Project::getOX2()
{
    return mOX2;
}

QPointF Project::getScalePointA1()
{
    return mScalePointA1;
}

QPointF Project::getScalePointA2()
{
    return mScalePointA2;
}

QPointF Project::getScalePointB1()
{
    return mScalePointB1;
}

QPointF Project::getScalePointB2()
{
    return mScalePointB2;
}

PWImage *Project::getOriginImage()
{
    return mOriginImage;
}

PWImage *Project::getOXImage()
{
    return mOXImage;
}

PWImage *Project::getScaleImageA()
{
    return mScaleImageA;
}

PWImage *Project::getScaleImageB()
{
    return mScaleImageB;
}

double Project::getScaleDistance()
{
    return mScaleDistance;
}

QString Project::getProcessMetadata()
{
    if (mProcessMetadata.isEmpty())
        mProcessMetadata = "<ProcessMetadata> </ProcessMetadata>";
    return mProcessMetadata;
}

void Project::setProcessMetadata(QString metadata)
{
    mProcessMetadata = metadata;
}

void Project::setProcessMetadataElement(QString process, QString tag, QString value)
{
    QDomDocument metadataDoc;
    metadataDoc.setContent(getProcessMetadata());
    QDomElement root = metadataDoc.documentElement();
    QDomElement processElement = root.firstChildElement(process);
    if (processElement.tagName() != process) {
        processElement = metadataDoc.createElement(process);
        root.appendChild(processElement);
    }

    QDomElement element = processElement.firstChildElement(tag);
    QDomElement newElement = metadataDoc.createElement(tag);
    QDomText newTextNode = metadataDoc.createTextNode(tag);
    newTextNode.setNodeValue(value);
    newElement.appendChild(newTextNode);

    if(element.tagName() == tag)
        processElement.replaceChild(element, newElement);
    else
        processElement.appendChild(newElement);

    mProcessMetadata = metadataDoc.toString();
}

void Project::removeProcessMetadata(QString process)
{
    QDomDocument metadataDoc;
    metadataDoc.setContent(getProcessMetadata());
    QDomElement root = metadataDoc.documentElement();
    QDomElement processElement = root.firstChildElement(process);
    if (processElement.tagName() == process)
        root.removeChild(processElement);
    mProcessMetadata = metadataDoc.toString();
}

void Project::setID(int id)
{
    mID = id;
}

void PW::Project::updateImagesCPoints()
{
    for(int i=0; i<mModelCPoints.count();i++){

    }
    mNeedToSave = true;
}

void PW::Project::setName(QString name)
{
    mName = name;
}

void Project::setBasePath(QString path)
{
    mBasePath = path;
    for (int i=0; i<mInputImages.count(); i++)
        mInputImages.at(i)->setBasePath(path);
}

void Project::setPreprocessingPath(QString path)
{
    mPreprocessingPath = path;
}

QString PW::Project::getName()
{
    return mName;
}

QString Project::getBasePath()
{
    return mBasePath;
}

QString Project::getPreProcessingPath()
{
    if (mPreprocessingPath.isEmpty())
        return mBasePath;
    return mPreprocessingPath;
}

int Project::readOrientations(QString basePath)
{
    if(mInputImages.count() < 1)
        return 1;
    Camera *camera = mInputImages.at(0)->getCamera();
    AperoModelReader interiorReader(camera);
    QDir baseDir(basePath);
    QStringList files = baseDir.entryList(QStringList("AutoCal*.xml"), QDir::Files | QDir::NoSymLinks);

    if (!files.isEmpty() && !QFile::exists(basePath+"/"+files.at(0)))
        return 2;
    CameraModel *cameraModel = interiorReader.read(basePath+"/"+files.at(0));
    if (cameraModel == NULL)
        return 3;

    MicMacEOReader exteriorReader;

    for (int i = 0; i<getCameras().count(); i++)
        getCameras().at(i)->setCameraModel(cameraModel);
    bool readOrientationFiles=false;
    for (int w=0 ;w<mInputImages.count();w++)
    {
        QString orientationFileName=basePath+"/Orientation-" + mInputImages.at(w)->getFileName() + ".xml";
        if(QFile::exists(orientationFileName))
        {
            ExteriorOrientation *extOri =  exteriorReader.read(orientationFileName);
            if (extOri == NULL)
                return 4;
            mInputImages.at(w)->setExteriorOrientation(extOri);
            readOrientationFiles=true;
        }
    }
    if(!readOrientationFiles)
        return(4);
    return 0;
}

int PW::Project::calculateImagePoints()
{
    for(int l = 0; l<mInputImages.count();l++){

        //Remove existing points:
        mInputImages.at(l)->getControlPoints()->clear();

        if (calculateImagePoints(&mModelCPoints, (PhotogrammetricModel *)mInputImages.at(l)->getCamera()->getCameraModel(),
                                 mInputImages.at(l)->getExteriorOrientation(),mInputImages.at(l)->getControlPoints(),
                                 mInputImages.at(l)->getSize().width(),
                                 mInputImages.at(l)->getSize().height()) == 1)
            return 1;
    }
    mNeedToSave = true;
    return 0;
}

//int Project::calculateImagePoint(double xT,
//                                 double yT,
//                                 double zT,
//                                 PhotogrammetricModel *pm,
//                                 QVector<double> * cp,
//                                 QVector< QVector<double> > * r,
//                                 int imgWidth,
//                                 int imgHeight,
//                                 float& column,
//                                 float& row)
//{
//    float xImg = pm->getFocal()*(r->at(0).at(0) * (xT - cp->at(0)) +
//                               r->at(1).at(0) * (yT - cp->at(1)) +
//                               r->at(2).at(0) * (zT - cp->at(2))) /
//                              (r->at(0).at(2) * (xT - cp->at(0)) +
//                               r->at(1).at(2) * (yT - cp->at(1)) +
//                               r->at(2).at(2) * (zT - cp->at(2)));

//    float yImg = pm->getFocal()*(r->at(0).at(1) * (xT - cp->at(0)) +
//                               r->at(1).at(1) * (yT - cp->at(1)) +
//                               r->at(2).at(1) * (zT - cp->at(2))) /
//                              (r->at(0).at(2) * (xT - cp->at(0)) +
//                               r->at(1).at(2) * (yT - cp->at(1)) +
//                               r->at(2).at(2) * (zT - cp->at(2)));
//    xImg+=pm->getXp();
//    yImg+=pm->getYp();
//    pm->distort(xImg,yImg);
//    column = pm->getXp() + xImg;
//    row = pm->getYp() + yImg;
//    return(0);
//}

bool Project::insertMatchesCategory(PWMatchesCategory *matchesCategory)
{
    if(matchesCategory==NULL)
        return(false);
    QString description=matchesCategory->getDescription();

    //TODO: Si existe la machaco, no??
//    if(mMatchesCategories.contains(description))
//        return(false);
    mMatchesCategories[description]=matchesCategory;
    return(true);
}

int Project::calculateImagePoints(QList<PW3dPoint *> *points,
                                  PhotogrammetricModel *pm,
                                  ExteriorOrientation * ext,
                                  QList<PW2dPoint *> *resultPoints,
                                  int imgWidth,
                                  int imgHeight){


    QVector< QVector<double> > * r = ext->getR();
    QVector<double> * cp = ext->getCP();

    for (int j=0; j<points->count();j++){

        PW3dPoint * terraniPoint = points->at(j);

        float xImg = pm->getFocal()*(r->at(0).at(0) * (terraniPoint->x() - cp->at(0)) +
                                     r->at(1).at(0) * (terraniPoint->y() - cp->at(1)) +
                                     r->at(2).at(0) * (terraniPoint->z() - cp->at(2))) /
                (r->at(0).at(2) * (terraniPoint->x() - cp->at(0)) +
                 r->at(1).at(2) * (terraniPoint->y() - cp->at(1)) +
                 r->at(2).at(2) * (terraniPoint->z() - cp->at(2)));

        float yImg = pm->getFocal()*(r->at(0).at(1) * (terraniPoint->x() - cp->at(0)) +
                                     r->at(1).at(1) * (terraniPoint->y() - cp->at(1)) +
                                     r->at(2).at(1) * (terraniPoint->z() - cp->at(2))) /
                (r->at(0).at(2) * (terraniPoint->x() - cp->at(0)) +
                 r->at(1).at(2) * (terraniPoint->y() - cp->at(1)) +
                 r->at(2).at(2) * (terraniPoint->z() - cp->at(2)));
        xImg = pm->getXp() + xImg;
        yImg = pm->getYp() + yImg;

        DistortionModel *distortion = pm->getDistortionModel();
        RadialExtended *extended = dynamic_cast<RadialExtended*>(distortion);

        double xu;
        double yu;
        double rho2;
        double cx=0.0;
        double cy=0.0;
        QVector<double> distCoef;
        if(extended != NULL){  //Radial Extended
            xu = xImg - extended->getXcd();
            yu = yImg - extended->getYcd();
            rho2 = (xu*xu +yu*yu);
            cx=0.0;
            cy=0.0;

            distCoef << extended->getK1() <<
                        extended->getK2() <<
                        extended->getK3() <<
                        extended->getK4() <<
                        extended->getK5();

        }else{ //Radial Basic
            RadialBasic *basic = dynamic_cast<RadialBasic*>(distortion);
            xu = xImg - pm->getXp();
            yu = yImg - pm->getYp();
            rho2 = (xu*xu +yu*yu);
            cx=0.0;
            cy=0.0;

            distCoef << basic->getK1() <<
                        basic->getK2();
        }

        for (int t = 1; t <= distCoef.count(); t++){
            cx=cx+xu*(pow (rho2, t)* distCoef.at(t-1));
            cy=cy+yu*(pow (rho2, t)* distCoef.at(t-1));
        }

        xImg = xImg + cx;
        yImg = yImg + cy;

        if(xImg >=0 && xImg < imgWidth && yImg >= 0 && yImg < imgHeight){
            resultPoints->append(new PW2dPoint(xImg, yImg, terraniPoint->getName()));
            qDebug() << xImg <<", "<< yImg;
        }
    }
    return 0;

    //            pm->distort(xImg,yImg);
    //            xImg = pm->getXp() + xImg;
    //            yImg = pm->getYp() + yImg;

    //            if(xImg >=0 && xImg < imgWidth && yImg >= 0 && yImg < imgHeight){
    //                resultPoints->append(new PW2dPoint(xImg, yImg, terraniPoint->getName()));
    //                qDebug() << xImg <<", "<< yImg;
    //            }

    //        }
    //        return 0;
}

bool Project::networkConnectivityAnalysis(QMap<QString, QVector<QString> > &imagesIdsTiePoints,
                                          QVector<QString>& connectedImages,
                                          QVector<QString>& unconnectedImages)
{
    connectedImages.clear();
    unconnectedImages.clear();

    // Contenedor con todas las imágenes conectadas con cada imagen, independientemente del orden
    QMap<QString,QVector<QString> > connectionsImages;
    QMap<QString, QVector<QString> >:: const_iterator iter=imagesIdsTiePoints.constBegin();
    while(iter!=imagesIdsTiePoints.constEnd())
    {
        QString firstImage=iter.key();
        QVector<QString> secondImages=iter.value();
        for(int i=0;i<secondImages.size();i++)
        {
            QString secondImage=secondImages.at(i);
            if(connectionsImages.contains(firstImage))
            {
                if(!connectionsImages[firstImage].contains(secondImage))
                    connectionsImages[firstImage].append(secondImage);
            }
            else
            {
                QVector<QString> aux;
                aux.push_back(secondImage);
                connectionsImages[firstImage]=aux;
            }
            if(connectionsImages.contains(secondImage))
            {
                if(!connectionsImages[secondImage].contains(firstImage))
                    connectionsImages[secondImage].append(firstImage);
            }
            else
            {
                QVector<QString> aux;
                aux.push_back(firstImage);
                connectionsImages[secondImage]=aux;
            }
        }
        iter++;
    }

    // Grupos de conexiones de imagenes
    QVector<QVector<QString> > connectedImagesGroups;
    int numberOfConnectedImages=0;
    for(int nI=0;nI<mInputImages.size();nI++)
    {
        QString firstImage=mInputImages[nI]->getFileName();
        if(!connectionsImages.contains(firstImage))
        {
            unconnectedImages.push_back(firstImage);
            continue;
        }
        QVector<QString> secondImages=connectionsImages[firstImage];
        bool findGroup=false;
        if(connectedImagesGroups.size()>0)
        {
            for(int nG=0;nG<connectedImagesGroups.size();nG++)
            {
                QVector<QString> connectedImagesGroup=connectedImagesGroups[nG];
                int firstImagePosInGroup=connectedImagesGroup.indexOf(firstImage);
                if(firstImagePosInGroup!=-1)
                {
                    findGroup=true;
                    insertConnectionsImages(firstImage,connectionsImages,connectedImagesGroup,numberOfConnectedImages);
                    break;
                }
                else
                {
                    for(int nSI=0;nSI<secondImages.size();nSI++)
                    {
                        QString secondImage=secondImages[nSI];
                        int secondImagePosInGroup=connectedImagesGroup.indexOf(secondImage);
                        if(secondImagePosInGroup!=-1)
                        {
                            findGroup=true;
                            insertConnectionsImages(firstImage,connectionsImages,connectedImagesGroup,numberOfConnectedImages);
                            if(numberOfConnectedImages==mInputImages.size())
                                break;
                        }
                    }
                    if(numberOfConnectedImages==mInputImages.size())
                        break;
                }
                if(numberOfConnectedImages==mInputImages.size())
                    break;
            }
        }
        if(!findGroup)  // crear un nuevo grupo y meter la imagen y todas sus conectadas
        {
            QVector<QString> connectedImagesGroup;
            insertConnectionsImages(firstImage,connectionsImages,connectedImagesGroup,numberOfConnectedImages);
            connectedImagesGroups.push_back(connectedImagesGroup);
        }
        if(numberOfConnectedImages==mInputImages.size())
            break;
    }

    // buscar el grupo con mayor número de conexiones
    if(connectedImagesGroups.size()>0)
    {
        int mainGroup=-1;
        int maxNumberOfImagesInGroup=-1;
        for(int nG=0;nG<connectedImagesGroups.size();nG++)
        {
            if(connectedImagesGroups[nG].size()>maxNumberOfImagesInGroup)
            {
                maxNumberOfImagesInGroup=connectedImagesGroups[nG].size();
                mainGroup=nG;
            }
        }
        for(int nI=0;nI<connectedImagesGroups[mainGroup].size();nI++)
        {
            connectedImages.push_back(connectedImagesGroups[mainGroup][nI]);
        }
    }

    // Comprobar y resolver el problema de que alguna de las imagenes conectadas no sea origen de matches
    // Este problema se ha detectado en apero
    bool findProblem=false;
    QVector<QString> noFirstImages;
    for(int nI=0;nI<connectedImages.size();nI++)
    {
        QString image=connectedImages[nI];
        if(!imagesIdsTiePoints.contains(image))
        {
            findProblem=true;
            noFirstImages.push_back(image);
        }
    }
    if(findProblem)
    {
        bool control=true;
        while(noFirstImages.size()>0
              &&control)
        {
            QString noFirstImage=noFirstImages[0]; // porque las voy eliminando en la función que corrige
            control=convertInFirstImage(noFirstImage,noFirstImages,imagesIdsTiePoints);
        }
    }
    bool success=true;
    if(connectedImages.size()<LIBPW_MINIMUM_NUMBER_OF_CONNECTED_IMAGES_IN_PROJECT)
    {
        success=false;
    }
    if(noFirstImages.size()>0)
    {
        success=false;
    }
    return(success);
}

void Project::insertConnectionsImages(QString& image, QMap<QString,
                                      QVector<QString> >& connectionsImages,
                                      QVector<QString>& connectedImagesGroup,
                                      int &numberOfConnectedImages)
{
    if(!connectedImagesGroup.contains(image))
    {
        connectedImagesGroup.push_back(image);
        numberOfConnectedImages++;
        if(numberOfConnectedImages==mInputImages.size())
            return;
    }
    if(!connectionsImages.contains(image))
        return;
    for(int nSI=0;nSI<connectionsImages[image].size();nSI++)
    {
        QString secondImage=connectionsImages[image][nSI];
        if(connectedImagesGroup.contains(secondImage))
            continue;
        insertConnectionsImages(secondImage,connectionsImages,connectedImagesGroup,numberOfConnectedImages);
        if(numberOfConnectedImages==mInputImages.size())
            return;
    }
}

bool Project::convertInFirstImage(QString &noFirstImage,
                                  QVector<QString> &noFirstImages,
                                  QMap<QString, QVector<QString> > &imagesIdsTiePoints)
{
    if(imagesIdsTiePoints.contains(noFirstImage)) // no tiene sentido, no hay problema a resolver
        return(true);
    // 1. Encuentro qué imagen la contiene y tiene un mayor número de conexiones
    QString containerImage;
    int maxNumberOfConnections=0;
    QMap<QString, QVector<QString> >::Iterator iter=imagesIdsTiePoints.begin();
    int noFirstImagePosition=-1;
    while(iter!=imagesIdsTiePoints.end())
    {
        QString auxImage=iter.key();
        QVector<QString> images=iter.value();
        int noFirstImagePositionInImages=images.indexOf(noFirstImage);
        if(noFirstImagePositionInImages!=-1)
        {
            int numberOfConnections=iter.value().size();
            if(numberOfConnections>maxNumberOfConnections)
            {
                maxNumberOfConnections=numberOfConnections;
                containerImage=auxImage;
                noFirstImagePosition=noFirstImagePositionInImages;
            }
        }
        iter++;
    }
    bool success=false;
    if(maxNumberOfConnections>0)
    {
        imagesIdsTiePoints[containerImage].remove(noFirstImagePosition);
        QVector<QString> images;
        images.push_back(containerImage);
        imagesIdsTiePoints[noFirstImage]=images;
        int posToRemove=noFirstImages.indexOf(noFirstImage);
        noFirstImages.remove(posToRemove);
        if(maxNumberOfConnections==1)
        {
            success=convertInFirstImage(noFirstImage,noFirstImages,imagesIdsTiePoints);
        }
        else
            success=true;
    }
    return(success);
}
QStringList Project::getTiePointsSets()
{
    return mTiePointsSets;
}

QStringList Project::getValidTiePointsSets()
{
    QMap<QString,PWMatchesCategory*>::const_iterator iter=mMatchesCategories.begin();
    QStringList validCategories;
    while(iter!=mMatchesCategories.end())
    {
        if(iter.value()->isValid())
            validCategories.append(iter.value()->getDescription());
        iter++;
    }
    return validCategories;
}

void Project::clearTiePointsSets()
{
    mTiePointsSets.clear();
    //    mMatchesCategoryTimeCalculation.clear();
    //    mMatchesCategoryGraphImages.clear();
    //    mMatchesCategoryImagesPairs.clear();
    //    mMatchesCategoryByDescription.clear();

}

void Project::appendTiePointsSet(QString tiePointsSet)
{
    mTiePointsSets.append(tiePointsSet);
}

bool Project::existsMatchesCategoryValid()
{
    QMap<QString,PWMatchesCategory*>::const_iterator iter=mMatchesCategories.begin();
    while(iter!=mMatchesCategories.end())
    {
        if(iter.value()->isValid())
            return(true);
        iter++;
    }
    return(false);
}

void Project::getActiveImagePairs(QMap<QString, QVector<QString> > &imagesIdsPairs)
{
    imagesIdsPairs.clear();
    QString matchesCategoryDescription=mActiveTiePointsSet;
    if(mMatchesCategories.contains(matchesCategoryDescription))
        mMatchesCategories[matchesCategoryDescription]->getImagePairs(imagesIdsPairs);
}

void Project::getMatchesCategoriesInformation(QStringList &metadataMatchesCategories,
                                              QStringList &descriptionMatchesCategories,
                                              QStringList &timeCalculationMatchesCategories,
                                              QVector<bool>& validityMatchesCategories)
{
    metadataMatchesCategories.clear();
    descriptionMatchesCategories.clear();
    timeCalculationMatchesCategories.clear();
    validityMatchesCategories.clear();
    QMap<QString,PWMatchesCategory*>::const_iterator iter=mMatchesCategories.begin();
    while(iter!=mMatchesCategories.end())
    {
        QString description=iter.key();
        PWMatchesCategory* matchesCategory=iter.value();
        QString metadata=matchesCategory->getMetadata();
        QString timeCalculation=matchesCategory->getTimeCalculation();
        bool validity=matchesCategory->isValid();
        metadataMatchesCategories.append(metadata);
        descriptionMatchesCategories.append(description);
        timeCalculationMatchesCategories.append(timeCalculation);
        validityMatchesCategories.push_back(validity);
        iter++;
    }
}

QList<PWMatchesCategory *> Project::getMatchesCategories()
{
    return mMatchesCategories.values();
}

bool Project::isValidMatchedCategory(QString metadata)
{
    bool success=false;
    QMap<QString,PWMatchesCategory*>::const_iterator iter=mMatchesCategories.begin();
    while(iter!=mMatchesCategories.end())
    {
        PWMatchesCategory* matchedCategory=iter.value();
        QString metadataValue=matchedCategory->getMetadata();
        if(metadata.compare(metadataValue)==0)
        {
            if(matchedCategory->isValid())
            {
                success=true;
                break;
            }
        }
        iter++;
    }
    return(success);
}

bool Project::isValidMatchedCategoryByDescription(QString description)
{
    bool success=false;
    if(mMatchesCategories.contains(description))
    {
        PWMatchesCategory* matchedCategory=mMatchesCategories[description];
        success=matchedCategory->isValid();
    }
    return(success);
}

PWMatchesCategory *Project::getMatchesCategory(QString description)
{
    PWMatchesCategory* matchedCategory=NULL;
    if(mMatchesCategories.contains(description))
        matchedCategory=mMatchesCategories[description];
    return(matchedCategory);
}

void Project::getInvalidMatchedCategoryExplanation(QString metadata,
                                                   QString &title,
                                                   QStringList &subgraphs)
{
    QMap<QString,PWMatchesCategory*>::const_iterator iter=mMatchesCategories.begin();
    while(iter!=mMatchesCategories.end())
    {
        PWMatchesCategory* matchedCategory=iter.value();
        QString metadataValue=matchedCategory->getMetadata();
        if(metadata.compare(metadataValue)==0)
        {
            if(!matchedCategory->isValid())
                matchedCategory->getInvalidMatchedCategoryExplanation(title,subgraphs);
            return;
        }
        iter++;
    }
}

void Project::getInvalidMatchedCategoryExplanationByDescription(QString description,
                                                                QString &title,
                                                                QStringList &subgraphs)
{
    if(mMatchesCategories.contains(description))
    {
        PWMatchesCategory* matchedCategory=mMatchesCategories[description];
        if(!matchedCategory->isValid())
            matchedCategory->getInvalidMatchedCategoryExplanation(title,subgraphs);
    }
}

bool Project::getMatchedCategoryImagePairs(QString &metadata,
                                           QMap<QString, QVector<QString> > &imagePairs)
{
    bool success=false;
    QMap<QString,PWMatchesCategory*>::const_iterator iter=mMatchesCategories.begin();
    while(iter!=mMatchesCategories.end())
    {
        PWMatchesCategory* matchedCategory=iter.value();
        QString metadataValue=matchedCategory->getMetadata();
        if(metadata.compare(metadataValue)==0)
        {
            matchedCategory->getImagePairs(imagePairs);
            success=true;
            break;
        }
        iter++;
    }
    return(success);
}

bool Project::getMatchedCategoryImagePairsByDescription(QString &description,
                                                        QMap<QString, QVector<QString> > &imagePairs)
{
    bool success=false;
    if(mMatchesCategories.contains(description))
    {
        PWMatchesCategory* matchedCategory=mMatchesCategories[description];
        matchedCategory->getImagePairs(imagePairs);
        success=true;
    }
    return(success);
}

bool Project::getMatchedCategoryOrderedImages(QString &metadata,
                                              int nGraph,
                                              QVector<QString> &orderedImages)
{
    bool success=false;
    QMap<QString,PWMatchesCategory*>::const_iterator iter=mMatchesCategories.begin();
    while(iter!=mMatchesCategories.end())
    {
        PWMatchesCategory* matchedCategory=iter.value();
        QString metadataValue=matchedCategory->getMetadata();
        if(metadata.compare(metadataValue)==0)
        {
            PWGraphImages* graph=matchedCategory->getGraphImages();
            success=graph->getOrderedImages(nGraph,orderedImages);
            break;
        }
        iter++;
    }
    return(success);
}

bool Project::getMatchedCategoryOrderedImagesByDescription(QString &description,
                                                           int nGraph,
                                                           QVector<QString>& orderedImages)
{
    bool success=false;
    if(mMatchesCategories.contains(description))
    {
        PWMatchesCategory* matchedCategory=mMatchesCategories[description];
        PWGraphImages* graph=matchedCategory->getGraphImages();
        success=graph->getOrderedImages(nGraph,orderedImages);
    }
    return(success);
}

bool Project::updateTiePointsAccuracy(QMap<QString, int> &orientationImagesIds,
                                      QMap<QString, QVector<QString> > &orientationImagesIdsTiePoints,
                                      QMap<QString, QMap<QString, QVector<double> > > &orientationImagesFirstColumnsTiePoints,
                                      QMap<QString, QMap<QString, QVector<double> > > &orientationImagesFirstRowsTiePoints,
                                      QMap<QString, QMap<QString, QVector<double> > > &orientationImagesSecondColumnsTiePoints,
                                      QMap<QString, QMap<QString, QVector<double> > > &orientationImagesSecondRowsTiePoints,
                                      QMap<QString, QMap<QString, QVector<int> > > &orientationMatchesIds,
                                      QMap<QString, QMap<QString, QVector<double> > >& orientationMatchesAccuracies,
                                      QMap<int, double> &accuraciesByMatchId)
{
    QMap<QString,PWImage*> imagesById;
    for(int nI=0;nI<mInputImages.size();nI++)
    {
        imagesById[mInputImages.at(nI)->getFileName()]=mInputImages.at(nI);
    }
    QString strError;
    if(!PW::LibPW::updateTiePointsAccuracy(imagesById,
                                           orientationImagesFirstColumnsTiePoints,
                                           orientationImagesFirstRowsTiePoints,
                                           orientationImagesSecondColumnsTiePoints,
                                           orientationImagesSecondRowsTiePoints,
                                           orientationMatchesIds,
                                           orientationMatchesAccuracies,
                                           accuraciesByMatchId,
                                           strError))
    {
        return(false);
    }
    /*
    orientationMatchesAccuracies.clear();
    accuraciesByMatchId.clear();
    QMap<QString, QMap<QString, QVector<int> > >::const_iterator iter=orientationMatchesIds.begin();
    while(iter!=orientationMatchesIds.end())
    {
        QString firstImageId=iter.key();
        PW::PWImage* firstImage=this->getImageByName(firstImageId);
        int firstImageWidth=firstImage->getSize().width();
        int firstImageHeight=firstImage->getSize().height();
        PhotogrammetricModel *pm1=(PhotogrammetricModel *)firstImage->getCamera()->getCameraModel();
        double focal1=pm1->getFocal();
        double xpp1=pm1->getXp();
        double ypp1=pm1->getYp();
        QVector< QVector<double> > * r1 = firstImage->getExteriorOrientation()->getR();
        QVector<double> * cp1 = firstImage->getExteriorOrientation()->getCP();
        QMap<QString, QVector<int> > matchesIdsByFirstImage=iter.value();
        QMap<QString, QVector<int> >::const_iterator iterBis=matchesIdsByFirstImage.begin();
        QMap<QString, QVector<float> > matchesAccuraciesByFirstImage;
        while(iterBis!=matchesIdsByFirstImage.end())
        {
            QString secondImageId=iterBis.key();
            PW::PWImage* secondImage=this->getImageByName(secondImageId);
            int secondImageWidth=secondImage->getSize().width();
            int secondImageHeight=secondImage->getSize().height();
            QVector< QVector<double> > * r2 = secondImage->getExteriorOrientation()->getR();
            QVector<double> * cp2 = secondImage->getExteriorOrientation()->getCP();
            QVector<int> matchesIds=iterBis.value();
            PhotogrammetricModel *pm2=(PhotogrammetricModel *)secondImage->getCamera()->getCameraModel();
            double focal2=pm2->getFocal();
            double xpp2=pm2->getXp();
            double ypp2=pm2->getYp();
            QVector<float> firstImageColumns=orientationImagesFirstColumnsTiePoints[firstImageId][secondImageId];
            QVector<float> firstImageRows=orientationImagesFirstRowsTiePoints[firstImageId][secondImageId];
            QVector<float> secondImageColumns=orientationImagesSecondColumnsTiePoints[firstImageId][secondImageId];
            QVector<float> secondImageRows=orientationImagesSecondRowsTiePoints[firstImageId][secondImageId];
            QVector<float> accuracies;
            for(int nM=0;nM<matchesIds.size();nM++)
            {
                int matchId=matchesIds[nM];
                float x1=firstImageColumns[nM];
                float y1=firstImageRows[nM];
                float x2=secondImageColumns[nM];
                float y2=secondImageRows[nM];
                float accuracy=0.0;
                pm1->undistort(x1,y1);
                pm2->undistort(x2,y2);
                double a1=r1->at(0).at(0)*x1+r1->at(0).at(1)*y1+r1->at(0).at(2)*focal1;
                double d1=r1->at(1).at(0)*x1+r1->at(1).at(1)*y1+r1->at(1).at(2)*focal1;
                double c1=r1->at(2).at(0)*x1+r1->at(2).at(1)*y1+r1->at(2).at(2)*focal1;
                double a2=r2->at(0).at(0)*x2+r2->at(0).at(1)*y2+r2->at(0).at(2)*focal2;
                double d2=r2->at(1).at(0)*x2+r2->at(1).at(1)*y2+r2->at(1).at(2)*focal2;
                double c2=r2->at(2).at(0)*x2+r2->at(2).at(1)*y2+r2->at(2).at(2)*focal2;
                Eigen::MatrixXd A(4,3);
                A(0,0) = c1;
                A(0,1) = 0.0;
                A(0,2) = -1.0*a1;
                A(1,0) = 0.0;
                A(1,1) = c1;
                A(1,2) = -1.0*d1;
                A(2,0) = c2;
                A(2,1) = 0.0;
                A(2,2) = -1.0*a2;
                A(3,0) = 0.0;
                A(3,1) = c2;
                A(3,2) = -1.0*d2;
                Eigen::VectorXd b(4);
                b(0)=c1*cp1->at(0)-a1*cp1->at(2);
                b(1)=c1*cp1->at(1)-d1*cp1->at(2);
                b(2)=c2*cp2->at(0)-a2*cp2->at(2);
                b(3)=c2*cp2->at(1)-d2*cp2->at(2);
                Eigen::VectorXd sol=(A.transpose() * A).ldlt().solve(A.transpose() * b);
                Eigen::VectorXd residuals=A*sol-b;
                double res2d=sqrt(residuals(0)*residuals(0)+residuals(1)*residuals(1)+residuals(2)*residuals(2)+residuals(3)*residuals(3));

                double xT=sol(0);
                double yT=sol(1);
                double zT=sol(2);
                float x1_backward=0.0;
                float y1_backward=0.0;
                float x2_backward=0.0;
                float y2_backward=0.0;
                if(calculateImagePoint(xT,yT,zT,pm1,cp1,r1,
                                       firstImageWidth,firstImageHeight,
                                       x1_backward,y1_backward)==1)
                {
                    return(false);
                }
                if(calculateImagePoint(xT,yT,zT,pm2,cp2,r2,
                                       secondImageWidth,secondImageHeight,
                                       x2_backward,y2_backward)==1)
                {
                    return(false);
                }
                double x1_error=firstImageColumns[nM]-x1_backward;
                double y1_error=firstImageRows[nM]-y1_backward;
                double x2_error=secondImageColumns[nM]-x2_backward;
                double y2_error=secondImageRows[nM]-y2_backward;
                accuracy=sqrt(x1_error*x1_error+y1_error*y1_error+x2_error*x2_error+y2_error*y2_error);
//                A=[c1   0  -a1
//                    0  c1  -d1
//                   c2   0  -a2
//                    0  c2  -d2];
//                b=[c1*XC1(1)-a1*XC1(3)
//                   c1*XC1(2)-d1*XC1(3)
//                   c2*XC2(1)-a2*XC2(3)
//                   c2*XC2(2)-d2*XC2(3)];
//                X=inv(A'*A)*(A'*b);
//                R=A*X-b;
//                xT=X(1);
//                yT=X(2);
//                zT=X(3);
                accuracies.push_back(accuracy);
                accuraciesByMatchId[matchId]=accuracy;
            }
            matchesAccuraciesByFirstImage[secondImageId]=accuracies;
            iterBis++;
        }
        orientationMatchesAccuracies[firstImageId]=matchesAccuraciesByFirstImage;
        iter++;
    }
    */
    return(true);
}
QString Project::getCaptureType()
{
    return mCaptureType;
}

void Project::setCaptureType(QString type)
{
    mCaptureType = type;
}



QString Project::getCloudUUID()
{
    return mCloudUUID;
}

void Project::setCloudUUID(QString value)
{
    mCloudUUID = value;
}


//int Project::calculateImagePoints(QList<PW3dPoint *> *points,
//                                  PhotogrammetricModel *pm,
//                                  ExteriorOrientation * ext,
//                                  QList<PW2dPoint *> *resultPoints,
//                                  int imgWidth,
//                                  int imgHeight){

//        RadialExtended *distortion =  (RadialExtended *)pm->getDistortionModel();
//        QVector< QVector<double> > * r = ext->getR();
//        QVector<double> * cp = ext->getCP();

//        for (int j=0; j<points->count();j++){

//            PW3dPoint * terraniPoint = points->at(j);

//            double xImg = pm->getFocal()*(r->at(0).at(0) * (terraniPoint->x() - cp->at(0)) +
//                                       r->at(1).at(0) * (terraniPoint->y() - cp->at(1)) +
//                                       r->at(2).at(0) * (terraniPoint->z() - cp->at(2))) /
//                                      (r->at(0).at(2) * (terraniPoint->x() - cp->at(0)) +
//                                       r->at(1).at(2) * (terraniPoint->y() - cp->at(1)) +
//                                       r->at(2).at(2) * (terraniPoint->z() - cp->at(2)));

//            double yImg = pm->getFocal()*(r->at(0).at(1) * (terraniPoint->x() - cp->at(0)) +
//                                       r->at(1).at(1) * (terraniPoint->y() - cp->at(1)) +
//                                       r->at(2).at(1) * (terraniPoint->z() - cp->at(2))) /
//                                      (r->at(0).at(2) * (terraniPoint->x() - cp->at(0)) +
//                                       r->at(1).at(2) * (terraniPoint->y() - cp->at(1)) +
//                                       r->at(2).at(2) * (terraniPoint->z() - cp->at(2)));
//            xImg = pm->getXp() + xImg;
//            yImg = pm->getYp() + yImg;

//            double xu = xImg - distortion->getXcd();
//            double yu = yImg - distortion->getYcd();
//            double rho2 = (xu*xu +yu*yu);
//            double cx=0.0;
//            double cy=0.0;
//            QVector<double> distCoef;
//            distCoef << distortion->getK1() <<
//                        distortion->getK2() <<
//                        distortion->getK3() <<
//                        distortion->getK4() <<
//                        distortion->getK5();


//            for (int t = 1; t <= distCoef.count(); t++){
//                cx=cx+xu*(pow (rho2, t)* distCoef.at(t-1));
//                cy=cy+yu*(pow (rho2, t)* distCoef.at(t-1));
//            }

//            xImg = xImg + cx;
//            yImg = yImg + cy;

//            if(xImg >=0 && xImg < imgWidth && yImg >= 0 && imgHeight){
//                resultPoints->append(new PW2dPoint(xImg, yImg, terraniPoint->getName()));
//                qDebug() << xImg <<", "<< yImg;
//            }
//        }
//        return 0;
//}

QString PW::Project::getSparseModelFullPath()
{
    return mBasePath + "/" + mSparseModelPath;
}

QString PW::Project::getDenseModelFullPath()
{
    return mBasePath + "/" +  mDenseModelPath;
}

QString PW::Project::getSparseModelRelativePath()
{
    return mSparseModelPath;
}

QString PW::Project::getDenseModelRelativePath()
{
    return mDenseModelPath;
}


void PW::Project::setSparseModelRelativePath(QString path)
{
    mSparseModelPath = path;
    mNeedToSave = true;
}

void PW::Project::setDenseModelRelativePath(QString path)
{
    mDenseModelPath = path;
    mNeedToSave = true;
}

QString PW::Project::getDescription()
{
    return mDescription;
}

Camera *Project::getCamera(QString exiff_id)
{
    return mCameras.value(exiff_id);
}

QList<Camera *> Project::getCameras()
{
    return mCameras.values();
}

QString Project::getCurrentOrientation()
{
    return mCurrentOrientation;
}

int Project::getMatchingFormat()
{
    return mMatchingFormat;
}

void PW::Project::setDescription(QString description)
{
    mDescription = description;
    mNeedToSave = true;
}

void Project::setOriginImage(PWImage * image)
{
    mOriginImage = image;
}

void Project::setOXImage(PWImage *image)
{
    mOXImage = image;
}

void Project::setScaleImageA(PWImage *image)
{
    mScaleImageA = image;
}

void Project::setScaleImageB(PWImage *image)
{
    mScaleImageB = image;
}

void Project::setScaleDistance(double distance)
{
    mScaleDistance = distance;
}

QMap<QString,QList<PW2dPoint *>*>* Project::getImagePointsMap(){
    imagePointsMap = new QMap<QString,QList<PW2dPoint *>*>();

    foreach (QString imageName, getImagesFileName()) {
       imagePointsMap->insert(imageName,getImageByName(imageName)->getControlPoints());
    }

    return imagePointsMap;
}
