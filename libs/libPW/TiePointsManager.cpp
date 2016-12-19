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

#include "TiePointsManager.h"

using namespace PW;

TiePointsManager::TiePointsManager(int precision,
                                   int imageColumns,
                                   int imageRows,
                                   int gridSize,
                                   QVector<QString> imageFileNames):
    mPrecision(precision)
{
    mDoubleToIntegerFactor=pow(10.0,precision);;
    mIntegerToDoubleFactor=1.0/mDoubleToIntegerFactor;
    mImageColumns=qRound(imageColumns*mDoubleToIntegerFactor);
    mImageRows=qRound(imageRows*mDoubleToIntegerFactor);
    mGridSize=qRound(gridSize*mDoubleToIntegerFactor);
    int cont=0; // Para preveer que haya imágenes repetidas
    for(int i=0;i<imageFileNames.size();i++)
    {
        if(!mImagesIds.contains(imageFileNames.at(i)))
        {
            mImagesIds[imageFileNames.at(i)]=cont;
            cont++;
        }
    }
    bool control=false;
    while(!control)
    {
        mGridColumns=ceil((double)(mImageColumns/mGridSize));
        mGridRows=ceil((double)(mImageRows/mGridSize));
        int numberOfCells=mGridColumns*mGridRows;
        if(numberOfCells>(TIEPOINTSMANAGER_GRID_MAXIMUM_NUMBER_OF_CELLS*mDoubleToIntegerFactor*mDoubleToIntegerFactor))
        {
            mGridSize*=2.0;
        }
        else
        {
            control=true;
        }
    }
    QMap<QString, int>::const_iterator iterImage=mImagesIds.begin();
    while(iterImage!=mImagesIds.end())
    {
        QString imageId=iterImage.key();
        int imagePosition=iterImage.value();        
        if(!mImagesByIds.contains(imagePosition))
            mImagesByIds[imagePosition]=imageId;
        QVector<QVector<QVector<TiePoint*> > > imageTiePoints;
        for(int nCol=0;nCol<mGridColumns;nCol++)
        {
            QVector<QVector<TiePoint*> > vectorRow;
            for(int nRow=0;nRow<mGridRows;nRow++)
            {
                QVector<TiePoint*> tiePoints;
                vectorRow.push_back(tiePoints);
            }
            imageTiePoints.push_back(vectorRow);
        }
        mTiePoints[imagePosition]=imageTiePoints;
        iterImage++;
    }
}

TiePointsManager::TiePointsManager(int precision,
                                   int imageColumns,
                                   int imageRows,
                                   int gridSize,
                                   QMap<QString, int> imagesIds):
    mPrecision(precision),
    mImagesIds(imagesIds)
{
    mDoubleToIntegerFactor=pow(10.0,precision);;
    mIntegerToDoubleFactor=1.0/mDoubleToIntegerFactor;
    mImageColumns=qRound(imageColumns*mDoubleToIntegerFactor);
    mImageRows=qRound(imageRows*mDoubleToIntegerFactor);
    mGridSize=qRound(gridSize*mDoubleToIntegerFactor);
    bool control=false;
    while(!control)
    {
        mGridColumns=ceil((double)(((double)mImageColumns)/((double)mGridSize)));
        if(mGridColumns%2==0)
            mGridColumns++;
        mGridRows=ceil((double)(((double)mImageRows)/((double)mGridSize)));
        if(mGridRows%2==0)
            mGridRows++;
        int numberOfCells=mGridColumns*mGridRows;
        if(numberOfCells>(TIEPOINTSMANAGER_GRID_MAXIMUM_NUMBER_OF_CELLS*mDoubleToIntegerFactor*mDoubleToIntegerFactor))
        {
            mGridSize*=2.0;
        }
        else
        {
            control=true;
        }
    }
    QMap<QString, int>::const_iterator iterImage=mImagesIds.begin();
    while(iterImage!=mImagesIds.end())
    {
        QString imageId=iterImage.key();
        int imagePosition=iterImage.value();
        if(!mImagesByIds.contains(imagePosition))
            mImagesByIds[imagePosition]=imageId;
        QVector<QVector<QVector<TiePoint*> > > imageTiePoints;
        for(int nCol=0;nCol<mGridColumns;nCol++)
        {
            QVector<QVector<TiePoint*> > vectorRow;
            for(int nRow=0;nRow<mGridRows;nRow++)
            {
                QVector<TiePoint*> tiePoints;
                vectorRow.push_back(tiePoints);
            }
            imageTiePoints.push_back(vectorRow);
        }
        mTiePoints[imagePosition]=imageTiePoints;
        iterImage++;
    }
}

bool TiePointsManager::addTiePoint(int firstImagePosition,
                                   int firstColumn,
                                   int firstRow,
                                   int secondImagePosition,
                                   int secondColumn,
                                   int secondRow,
                                   bool& addedTiePoint,
                                   QString &strError,
                                   int std)
{
    addedTiePoint=false;
    if(!mTiePoints.contains(firstImagePosition))
    {
        strError=QObject::tr("TiePointsManager::addTiePoint, image position %1 not found").arg(QString::number(firstImagePosition));
        return(false);
    }
    if(!mTiePoints.contains(secondImagePosition))
    {
        strError=QObject::tr("TiePointsManager::addTiePoint, image position %1 not found").arg(QString::number(secondImagePosition));
        return(false);
    }
    int firstCellColumn=ceil((double)(firstColumn/mGridSize));
    int firstCellRow=ceil((double)(firstRow/mGridSize));
    int secondCellColumn=ceil((double)(secondColumn/mGridSize));
    int secondCellRow=ceil((double)(secondRow/mGridSize));
    TiePoint* firstImageTiePoint=NULL;
    if(!getTiePoint(firstImagePosition,firstColumn,firstRow,firstImageTiePoint,strError))
        return(false);
    TiePoint* secondImageTiePoint=NULL;
    if(!getTiePoint(secondImagePosition,secondColumn,secondRow,secondImageTiePoint,strError))
        return(false);
    // Caso A: No hay ningún tiePoint que coincida en posición ni en la primera ni en la segunda imagen
    //         Hay que crear el TiePoint y añadirlo
    if(firstImageTiePoint==NULL
            &&secondImageTiePoint==NULL)
    {
        ImagePoint* firstImagePoint=new ImagePoint(firstImagePosition,firstColumn,firstRow,std);
        ImagePoint* secondImagePoint=new ImagePoint(secondImagePosition,secondColumn,secondRow,std);
        int id=mTiePoinstById.size();
        TiePoint* tiePoint=new TiePoint(id,firstImagePoint,secondImagePoint);
        mTiePoints[firstImagePosition][firstCellColumn][firstCellRow].push_back(tiePoint);
        mTiePoints[secondImagePosition][secondCellColumn][secondCellRow].push_back(tiePoint);
        mTiePoinstById.push_back(tiePoint);
        addedTiePoint=true;
        return(true);
    }
    // Caso B: El TiePoint ya existe y coincide en coordenadas en ambos puntos
    //         No hay que añadirlo para que no esté duplicado
    if(firstImageTiePoint!=NULL
            &&secondImageTiePoint!=NULL
            &&(firstImageTiePoint->getId()==secondImageTiePoint->getId()))
    {
        return(true);
    }
    // Caso C: El TiePoint ya existe y no es el mismo, no coincide en coordenadas en ambos puntos
    if(firstImageTiePoint!=NULL
            &&secondImageTiePoint!=NULL
            &&(firstImageTiePoint->getId()!=secondImageTiePoint->getId()))
    {
        QMap<int,ImagePoint*> firstImageImagePoints=firstImageTiePoint->getImagePoints();
        QMap<int,ImagePoint*> secondImageImagePoints=secondImageTiePoint->getImagePoints();
        // Caso C1: En el TiePoint que corresponde con el primero hay punto en la segunda imagen
        //          o en el TiePoint que corresponde con el segundo hay punto en la primera imagen
        //          Como no se corresponden hay incoherencia luego todo inválido
        if(firstImageImagePoints.contains(secondImagePosition)
                ||secondImageImagePoints.contains(firstImagePosition))
        {
            firstImageTiePoint->setInvalid();
            secondImageTiePoint->setInvalid();
            return(true);
        }
        // Caso C2: En el TiePoint que corresponde con el primero no hay punto en la segunda imagen
        //          y en el TiePoint que corresponde con el segundo no hay punto en la primera image
        //          Si existe otra imagen con un punto común en ambos TiePoints cuyas coordenadas
        //          no coincidan también habrá incoherencia y se pone todo a inválido
        QMap<int,ImagePoint*>::const_iterator iter1=firstImageImagePoints.begin();
        QMap<int,ImagePoint*>::const_iterator iter2;
        while(iter1!=firstImageImagePoints.end())
        {
            int firstImageImagePointImagePosition=iter1.value()->imagePosition();
            iter2=secondImageImagePoints.begin();
            while(iter2!=secondImageImagePoints.end())
            {
                int secondImageImagePointImagePosition=iter2.value()->imagePosition();
                if(firstImageImagePointImagePosition==secondImageImagePointImagePosition)
                {
                    int firstImageImagePointColumn=iter1.value()->column();
                    int firstImageImagePointRow=iter1.value()->row();
                    int secondImageImagePointColumn=iter2.value()->column();
                    int secondImageImagePointRow=iter2.value()->row();
                    if(firstImageImagePointColumn!=secondImageImagePointColumn
                            ||firstImageImagePointRow!=secondImageImagePointRow)
                    {
                        firstImageTiePoint->setInvalid();
                        secondImageTiePoint->setInvalid();
                        return(true);
                    }
                }
                iter2++;
            }
            iter1++;
        }
        // No existe ninguna imagen común con un punto no coincidente
        // Ahora enlazamos los dos existentes a través de este luego:
        // Insertamos todo en el primero e invalidamos el segundo
        // Como insertamos todos estamos insertando ya el segundo punto, del que estamos añadiendo, el que no está
        iter2=secondImageImagePoints.begin();
        while(iter2!=secondImageImagePoints.end())
        {
            ImagePoint* secondImageImagePoint=iter2.value();
            int secondImageImagePointImagePosition=secondImageImagePoint->imagePosition();
            if(!firstImageImagePoints.contains(secondImageImagePointImagePosition))
            {
                if(!firstImageTiePoint->addImagePoint(secondImageImagePoint,strError))
                {
                    return(false);
                }
            }
            else
            {
                ImagePoint* auxImagePoint=firstImageImagePoints[secondImageImagePointImagePosition];
                if(auxImagePoint->column()!=secondImageImagePoint->column()
                        ||auxImagePoint->row()!=secondImageImagePoint->row())
                {
                    firstImageTiePoint->setInvalid();
                    secondImageTiePoint->setInvalid();
                    return(true);
                }
            }
            iter2++;
        }
        return(true);
    }
    // Caso D: El punto existe en la primera imagen, y no en la segunda
    if(firstImageTiePoint!=NULL)
    {
        // Caso D.1: El secondImagePoint no está el TiePoint pero hay otro punto en la segunda imagen
        //           Hay que poner a NULL el TiePoint
        bool firstTiePointContainsOtherImagePointInSecondImage=firstImageTiePoint->containsImage(secondImagePosition);
        if(firstTiePointContainsOtherImagePointInSecondImage)
        {
            firstImageTiePoint->setInvalid();//=NULL;
        }
        // Caso D.2: El secondImagePoint no está el TiePoint y no hay otro punto en la segunda imagen
        //           Hay que añadir al TiePoint el secondImagePoint y añadir el TiePoint a la segunda imagen
        else
        {
            ImagePoint* secondImagePoint=new ImagePoint(secondImagePosition,secondColumn,secondRow,std);
            if(!firstImageTiePoint->addImagePoint(secondImagePoint,strError))
                return(false);
            mTiePoints[secondImagePosition][secondCellColumn][secondCellRow].push_back(firstImageTiePoint);
        }
    }
    // Caso E: El punto existe en la segunda imagen, y no en la primera
    if(secondImageTiePoint!=NULL)
    {
        // Caso E.1: El firstImagePoint no está el TiePoint pero hay otro punto en la primera imagen
        //           Hay que poner a NULL el TiePoint
        bool secondTiePointContainsOtherImagePointInFirstImage=secondImageTiePoint->containsImage(firstImagePosition);
        if(secondTiePointContainsOtherImagePointInFirstImage)
        {
            secondImageTiePoint->setInvalid();//=NULL;
        }
        // Caso D.2: El secondImagePoint no está el TiePoint y no hay otro punto en la segunda imagen
        //           Hay que añadir al TiePoint el secondImagePoint y añadir el TiePoint a la segunda imagen
        else
        {
            ImagePoint* firstImagePoint=new ImagePoint(firstImagePosition,firstColumn,firstRow,std);
            if(!secondImageTiePoint->addImagePoint(firstImagePoint,strError))
                return(false);
            mTiePoints[firstImagePosition][firstCellColumn][firstCellRow].push_back(secondImageTiePoint);
        }
    }
    return(true);
}

bool TiePointsManager::getTiePoint(int imagePosition,
                                   int column,
                                   int row,
                                   TiePoint *&tiePoint,
                                   QString &strError)
{
    if(!mTiePoints.contains(imagePosition))
    {
        strError=QObject::tr("TiePointsManager::getTiePoint, image position %1 not found").arg(QString::number(imagePosition));
        return(false);
    }
    if(column>mImageColumns)
    {
        strError=QObject::tr("TiePointsManager::getTiePoint, column %1 out of domain")
                .arg(QString::number(column*mIntegerToDoubleFactor,'f',mPrecision));
        return(false);
    }
    if(row>mImageRows)
    {
        strError=QObject::tr("TiePointsManager::getTiePoint, row %1 out of domain")
                .arg(QString::number(row*mIntegerToDoubleFactor,'f',mPrecision));
        return(false);
    }
    tiePoint=NULL;
    int cellColumn=ceil((double)(column/mGridSize));
    int cellRow=ceil((double)(row/mGridSize));
    QVector<TiePoint*> imageTiePointsInCell=mTiePoints[imagePosition][cellColumn][cellRow];
    for(int nTp=0;nTp<imageTiePointsInCell.size();nTp++)
    {
        TiePoint* ptrTiePoint=imageTiePointsInCell[nTp];
        if(ptrTiePoint->containsImagePoint(imagePosition,column,row))
        {
            tiePoint=ptrTiePoint;
            break;
        }
    }
    return(true);
}

bool TiePointsManager::setFromMapContainers(QMap<QString, QVector<QString> > &imagesIdsTiePoints,
                                            QMap<QString, QMap<QString, QVector<double> > > &imagesFirstColumnsTiePoints,
                                            QMap<QString, QMap<QString, QVector<double> > > &imagesFirstRowsTiePoints,
                                            QMap<QString, QMap<QString, QVector<double> > > &imagesSecondColumnsTiePoints,
                                            QMap<QString, QMap<QString, QVector<double> > > &imagesSecondRowsTiePoints,
                                            QMap<QString, QMap<QString, QVector<double> > > &matchesAccuracies,
                                            QString &strError)
{
    QMap<QString, QVector<QString> >::const_iterator iterImagesIdsTiePoints=imagesIdsTiePoints.begin();
    bool existsAccuracies=false;
    while(iterImagesIdsTiePoints!=imagesIdsTiePoints.end())
    {
        QString firstImageId=iterImagesIdsTiePoints.key();
        if(!mImagesIds.contains(firstImageId))
        {
            strError=QObject::tr("TiePointsManager::setFromMapContainers, image %1 not found").arg(firstImageId);
            return(false);
        }
        QVector<QString> secondImagesIds=iterImagesIdsTiePoints.value();
        for(int nSi=0;nSi<secondImagesIds.size();nSi++)
        {
            QString secondImageId=secondImagesIds[nSi];
            if(!mImagesIds.contains(secondImageId))
            {
                strError=QObject::tr("TiePointsManager::setFromMapContainers, image %1 not found").arg(secondImageId);
                return(false);
            }
            bool existsAccuraciesInPair=false;
            if(matchesAccuracies.contains(firstImageId))
            {
                if(matchesAccuracies[firstImageId].contains(secondImageId))
                {
                    existsAccuraciesInPair=true;
                }
            }
            else if(matchesAccuracies.contains(secondImageId))
            {
                if(matchesAccuracies[secondImageId].contains(firstImageId))
                {
                    existsAccuraciesInPair=true;
                }
            }
            if(!existsAccuracies&&existsAccuraciesInPair)
                existsAccuracies=true;
            if(existsAccuracies&&!existsAccuraciesInPair)
            {
                strError=QObject::tr("TiePointsManager::setFromMapContainers, not exists accuracies for first image %1 and second image %2")
                        .arg(firstImageId)
                        .arg(secondImageId);
                return(false);
            }
        }
        iterImagesIdsTiePoints++;
    }
    iterImagesIdsTiePoints=imagesIdsTiePoints.begin();
    while(iterImagesIdsTiePoints!=imagesIdsTiePoints.end())
    {
        QString firstImageId=iterImagesIdsTiePoints.key();
        if(!mImagesIds.contains(firstImageId))
        {
            strError=QObject::tr("TiePointsManager::setFromMapContainers, image %1 not found").arg(firstImageId);
            return(false);
        }
        int firstImagePos=mImagesIds[firstImageId];
        QVector<QString> secondImagesIds=iterImagesIdsTiePoints.value();
        for(int nSi=0;nSi<secondImagesIds.size();nSi++)
        {
            QString secondImageId=secondImagesIds[nSi];
            if(!mImagesIds.contains(secondImageId))
            {
                strError=QObject::tr("TiePointsManager::setFromMapContainers, image %1 not found").arg(secondImageId);
                return(false);
            }
            int secondImagePos=mImagesIds[secondImageId];
            bool isValidImages=false;
            QVector<double> firstColumnsTiePoints,firstRowsTiePoints,secondColumnsTiePoints,secondRowsTiePoints,accuraciesTiePoints;
            if(imagesFirstColumnsTiePoints.contains(firstImageId)
                    &&imagesFirstRowsTiePoints.contains(firstImageId)
                    &&imagesSecondColumnsTiePoints.contains(firstImageId)
                    &&imagesSecondRowsTiePoints.contains(firstImageId))
            {
                if(imagesFirstColumnsTiePoints[firstImageId].contains(secondImageId)
                        &&imagesFirstRowsTiePoints[firstImageId].contains(secondImageId)
                        &&imagesSecondColumnsTiePoints[firstImageId].contains(secondImageId)
                        &&imagesSecondRowsTiePoints[firstImageId].contains(secondImageId))
                {
                    isValidImages=true;
                    firstColumnsTiePoints=imagesFirstColumnsTiePoints[firstImageId][secondImageId];
                    firstRowsTiePoints=imagesFirstRowsTiePoints[firstImageId][secondImageId];
                    secondColumnsTiePoints=imagesSecondColumnsTiePoints[firstImageId][secondImageId];
                    secondRowsTiePoints=imagesSecondRowsTiePoints[firstImageId][secondImageId];
                    if(existsAccuracies)
                        accuraciesTiePoints=matchesAccuracies[firstImageId][secondImageId];
                }
            }
            else if(imagesFirstColumnsTiePoints.contains(secondImageId)
                    &&imagesFirstRowsTiePoints.contains(secondImageId)
                    &&imagesSecondColumnsTiePoints.contains(secondImageId)
                    &&imagesSecondRowsTiePoints.contains(secondImageId))
            {
                if(imagesFirstColumnsTiePoints[secondImageId].contains(firstImageId)
                        &&imagesFirstRowsTiePoints[secondImageId].contains(firstImageId)
                        &&imagesSecondColumnsTiePoints[secondImageId].contains(firstImageId)
                        &&imagesSecondRowsTiePoints[secondImageId].contains(firstImageId))
                {
                    isValidImages=true;
                    firstColumnsTiePoints=imagesFirstColumnsTiePoints[secondImageId][firstImageId];
                    firstRowsTiePoints=imagesFirstRowsTiePoints[secondImageId][firstImageId];
                    secondColumnsTiePoints=imagesSecondColumnsTiePoints[secondImageId][firstImageId];
                    secondRowsTiePoints=imagesSecondRowsTiePoints[secondImageId][firstImageId];
                    if(existsAccuracies)
                        accuraciesTiePoints=matchesAccuracies[secondImageId][firstImageId];
                }
            }
            if(!isValidImages)
            {
                strError=QObject::tr("TiePointsManager::setFromMapContainers, not exists tie points for first image %1 and second image %2")
                        .arg(firstImageId)
                        .arg(secondImageId);
                return(false);
            }
            if((firstColumnsTiePoints.size()!=firstRowsTiePoints.size())
                    ||(firstRowsTiePoints.size()!=secondColumnsTiePoints.size())
                    ||(secondColumnsTiePoints.size()!=secondRowsTiePoints.size()))
            {
                strError=QObject::tr("TiePointsManager::setFromMapContainers, invalid dimensions in tie points containers for image %1 and second image %2")
                        .arg(firstImageId)
                        .arg(secondImageId);
                return(false);
            }
            else
            {
                if(existsAccuracies)
                {
                    if(firstColumnsTiePoints.size()!=accuraciesTiePoints.size())
                    {
                        strError=QObject::tr("TiePointsManager::setFromMapContainers, invalid dimensions in tie points containers for image %1 and second image %2")
                                .arg(firstImageId)
                                .arg(secondImageId);
                        return(false);
                    }
                }
            }
            for(int nTp=0;nTp<firstColumnsTiePoints.size();nTp++)
            {
                double doubleFirstColumn=firstColumnsTiePoints[nTp];
                double doubleFirstRow=firstRowsTiePoints[nTp];
                double doubleSecondColumn=secondColumnsTiePoints[nTp];
                double doubleSecondRow=secondRowsTiePoints[nTp];
                int firstColumn=qRound(doubleFirstColumn*mDoubleToIntegerFactor);
                int firstRow=qRound(doubleFirstRow*mDoubleToIntegerFactor);
                int secondColumn=qRound(doubleSecondColumn*mDoubleToIntegerFactor);
                int secondtRow=qRound(doubleSecondRow*mDoubleToIntegerFactor);
                int accuracy=-1;
                if(existsAccuracies)
                {
                    double doubleAccuracy=accuraciesTiePoints[nTp];
                    accuracy=qRound(doubleAccuracy*mDoubleToIntegerFactor);
                }
                bool addedTiePoint=false;
                if(!addTiePoint(firstImagePos,firstColumn,firstRow,secondImagePos,secondColumn,secondtRow,addedTiePoint,strError,accuracy))
                    return(false);
            }
        }
        iterImagesIdsTiePoints++;
    }
    return(true);
}

bool TiePointsManager::setFromMapContainers(QMap<QString, QVector<QString> > &imagesIdsTiePoints,
                                            QMap<QString, QMap<QString, QVector<bool> > > &matchesEnableds,
                                            QMap<QString, QMap<QString, QVector<double> > > &imagesFirstColumnsTiePoints,
                                            QMap<QString, QMap<QString, QVector<double> > > &imagesFirstRowsTiePoints,
                                            QMap<QString, QMap<QString, QVector<double> > > &imagesSecondColumnsTiePoints,
                                            QMap<QString, QMap<QString, QVector<double> > > &imagesSecondRowsTiePoints,
                                            QMap<QString, QMap<QString, QVector<double> > > &matchesAccuracies,
                                            QString &strError)
{
    QMap<QString, QVector<QString> >::const_iterator iterImagesIdsTiePoints=imagesIdsTiePoints.begin();
    bool existsAccuracies=false;
    while(iterImagesIdsTiePoints!=imagesIdsTiePoints.end())
    {
        QString firstImageId=iterImagesIdsTiePoints.key();
        if(!mImagesIds.contains(firstImageId))
        {
            strError=QObject::tr("TiePointsManager::setFromMapContainers, image %1 not found").arg(firstImageId);
            return(false);
        }
        QVector<QString> secondImagesIds=iterImagesIdsTiePoints.value();
        for(int nSi=0;nSi<secondImagesIds.size();nSi++)
        {
            QString secondImageId=secondImagesIds[nSi];
            if(!mImagesIds.contains(secondImageId))
            {
                strError=QObject::tr("TiePointsManager::setFromMapContainers, image %1 not found").arg(secondImageId);
                return(false);
            }
            bool existsAccuraciesInPair=false;
            if(matchesAccuracies.contains(firstImageId))
            {
                if(matchesAccuracies[firstImageId].contains(secondImageId))
                {
                    existsAccuraciesInPair=true;
                }
            }
            else if(matchesAccuracies.contains(secondImageId))
            {
                if(matchesAccuracies[secondImageId].contains(firstImageId))
                {
                    existsAccuraciesInPair=true;
                }
            }
            if(!existsAccuracies&&existsAccuraciesInPair)
                existsAccuracies=true;
            if(existsAccuracies&&!existsAccuraciesInPair)
            {
                strError=QObject::tr("TiePointsManager::setFromMapContainers, not exists accuracies for first image %1 and second image %2")
                        .arg(firstImageId)
                        .arg(secondImageId);
                return(false);
            }
        }
        iterImagesIdsTiePoints++;
    }
    iterImagesIdsTiePoints=imagesIdsTiePoints.begin();
    while(iterImagesIdsTiePoints!=imagesIdsTiePoints.end())
    {
        QString firstImageId=iterImagesIdsTiePoints.key();
        if(!mImagesIds.contains(firstImageId))
        {
            strError=QObject::tr("TiePointsManager::setFromMapContainers, image %1 not found").arg(firstImageId);
            return(false);
        }
        int firstImagePos=mImagesIds[firstImageId];
        QVector<QString> secondImagesIds=iterImagesIdsTiePoints.value();
        for(int nSi=0;nSi<secondImagesIds.size();nSi++)
        {
            QString secondImageId=secondImagesIds[nSi];
            if(!mImagesIds.contains(secondImageId))
            {
                strError=QObject::tr("TiePointsManager::setFromMapContainers, image %1 not found").arg(secondImageId);
                return(false);
            }
            int secondImagePos=mImagesIds[secondImageId];
            bool isValidImages=false;
            QVector<double> firstColumnsTiePoints,firstRowsTiePoints,secondColumnsTiePoints,secondRowsTiePoints,accuraciesTiePoints;
            QVector<bool> enableds;
            if(imagesFirstColumnsTiePoints.contains(firstImageId)
                    &&imagesFirstRowsTiePoints.contains(firstImageId)
                    &&imagesSecondColumnsTiePoints.contains(firstImageId)
                    &&imagesSecondRowsTiePoints.contains(firstImageId)
                    &&matchesEnableds.contains(firstImageId))
            {
                if(imagesFirstColumnsTiePoints[firstImageId].contains(secondImageId)
                        &&imagesFirstRowsTiePoints[firstImageId].contains(secondImageId)
                        &&imagesSecondColumnsTiePoints[firstImageId].contains(secondImageId)
                        &&imagesSecondRowsTiePoints[firstImageId].contains(secondImageId)
                        &&matchesEnableds[firstImageId].contains(secondImageId))
                {
                    isValidImages=true;
                    firstColumnsTiePoints=imagesFirstColumnsTiePoints[firstImageId][secondImageId];
                    firstRowsTiePoints=imagesFirstRowsTiePoints[firstImageId][secondImageId];
                    secondColumnsTiePoints=imagesSecondColumnsTiePoints[firstImageId][secondImageId];
                    secondRowsTiePoints=imagesSecondRowsTiePoints[firstImageId][secondImageId];
                    enableds=matchesEnableds[firstImageId][secondImageId];
                    if(existsAccuracies)
                        accuraciesTiePoints=matchesAccuracies[firstImageId][secondImageId];
                }
            }
            else if(imagesFirstColumnsTiePoints.contains(secondImageId)
                    &&imagesFirstRowsTiePoints.contains(secondImageId)
                    &&imagesSecondColumnsTiePoints.contains(secondImageId)
                    &&imagesSecondRowsTiePoints.contains(secondImageId)
                    &&matchesEnableds.contains(secondImageId))
            {
                if(imagesFirstColumnsTiePoints[secondImageId].contains(firstImageId)
                        &&imagesFirstRowsTiePoints[secondImageId].contains(firstImageId)
                        &&imagesSecondColumnsTiePoints[secondImageId].contains(firstImageId)
                        &&imagesSecondRowsTiePoints[secondImageId].contains(firstImageId)
                        &&matchesEnableds[secondImageId].contains(firstImageId))
                {
                    isValidImages=true;
                    firstColumnsTiePoints=imagesFirstColumnsTiePoints[secondImageId][firstImageId];
                    firstRowsTiePoints=imagesFirstRowsTiePoints[secondImageId][firstImageId];
                    secondColumnsTiePoints=imagesSecondColumnsTiePoints[secondImageId][firstImageId];
                    secondRowsTiePoints=imagesSecondRowsTiePoints[secondImageId][firstImageId];
                    enableds=matchesEnableds[secondImageId][firstImageId];
                    if(existsAccuracies)
                        accuraciesTiePoints=matchesAccuracies[secondImageId][firstImageId];
                }
            }
            if(!isValidImages)
            {
                strError=QObject::tr("TiePointsManager::setFromMapContainers, not exists tie points for first image %1 and second image %2")
                        .arg(firstImageId)
                        .arg(secondImageId);
                return(false);
            }
            if((firstColumnsTiePoints.size()!=firstRowsTiePoints.size())
                    ||(firstRowsTiePoints.size()!=secondColumnsTiePoints.size())
                    ||(secondColumnsTiePoints.size()!=secondRowsTiePoints.size())
                    ||enableds.size()!=secondRowsTiePoints.size())
            {
                strError=QObject::tr("TiePointsManager::setFromMapContainers, invalid dimensions in tie points containers for image %1 and second image %2")
                        .arg(firstImageId)
                        .arg(secondImageId);
                return(false);
            }
            else
            {
                if(existsAccuracies)
                {
                    if(firstColumnsTiePoints.size()!=accuraciesTiePoints.size())
                    {
                        strError=QObject::tr("TiePointsManager::setFromMapContainers, invalid dimensions in tie points containers for image %1 and second image %2")
                                .arg(firstImageId)
                                .arg(secondImageId);
                        return(false);
                    }
                }
            }
            for(int nTp=0;nTp<firstColumnsTiePoints.size();nTp++)
            {
                if(enableds[nTp])
                {
                    double doubleFirstColumn=firstColumnsTiePoints[nTp];
                    double doubleFirstRow=firstRowsTiePoints[nTp];
                    double doubleSecondColumn=secondColumnsTiePoints[nTp];
                    double doubleSecondRow=secondRowsTiePoints[nTp];
                    int firstColumn=qRound(doubleFirstColumn*mDoubleToIntegerFactor);
                    int firstRow=qRound(doubleFirstRow*mDoubleToIntegerFactor);
                    int secondColumn=qRound(doubleSecondColumn*mDoubleToIntegerFactor);
                    int secondtRow=qRound(doubleSecondRow*mDoubleToIntegerFactor);
                    int accuracy=-1;
                    if(existsAccuracies)
                    {
                        double doubleAccuracy=accuraciesTiePoints[nTp];
                        accuracy=qRound(doubleAccuracy*mDoubleToIntegerFactor);
                    }
                    bool addedTiePoint=false;
                    if(!addTiePoint(firstImagePos,firstColumn,firstRow,secondImagePos,secondColumn,secondtRow,addedTiePoint,strError,accuracy))
                        return(false);
                }
            }
        }
        iterImagesIdsTiePoints++;
    }
    return(true);
}

