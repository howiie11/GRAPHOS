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
#include <QDir>
#include <QTextStream>
#include <QString>
#include <QVector>

#include "math.h"

#include "libPW.h"
#include "TiePointsIO.h"


#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <cstdint>

using namespace PW;
using namespace std;
//TODO: Documentar, ¿que formato de fichero lee?
//TODO: Juntar con MICMAC/TapiocaIO

float round(float f,int precission)
{
    return floor(f*(10*precission) + 0.5)/(10*precission);
}

TiePointsIO::TiePointsIO()
{
}

int TiePointsIO::mergeTiePointsResults(QStringList inputPaths,
                                       QString outputPath,
                                       bool removeDuplicates)
{
    QDir outputDir(outputPath);
    if(!outputDir.exists())
        return 1;
    foreach (const QString &str, inputPaths) {
        QDir input(str);
        Q_FOREACH(QFileInfo info, input.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs)) {
            if (info.isDir()) {
                QDir ImageDir(info.filePath());
                QDir destImageDir(outputPath+"/"+info.fileName());
                if(!destImageDir.exists())
                    destImageDir.mkdir(destImageDir.absolutePath());
                QFileInfoList aux = ImageDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
                Q_FOREACH(QFileInfo info, ImageDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files)) {
                    if (info.isFile() && info.suffix().toLower() == "txt") {
                        QFile destMatchingFile(destImageDir.absolutePath()+"/"+info.fileName());
                        if (!destMatchingFile.exists()){
                            QFile::copy(info.filePath(),
                                        destImageDir.absolutePath()+"/"+info.fileName());

                        }
                        else{
                            QFile matchingFile(info.filePath());
                            destMatchingFile.open(QFile::Append);
                            QTextStream out(&destMatchingFile);
                            matchingFile.open(QFile::ReadOnly);
                            while (!matchingFile.atEnd()) {
                                QString line = matchingFile.readLine();
                                out << line;
                            }
                            destMatchingFile.close();
                            matchingFile.close();    
                        }
                        if(removeDuplicates)
                            removeDuplicateLines(destImageDir.absolutePath()+"/"+info.fileName());
                    }

                }
            }
        }
    }
    return 0;
}

int TiePointsIO::readTiePointsFile(QString &url,
                                   QVector<double> &firstImagePixelsColumn,
                                   QVector<double> &firstImagePixelsRow,
                                   QVector<double> &secondImagePixelsColumn,
                                   QVector<double> &secondImagePixelsRow,
                                   QString& strError)
{
    if(!QFile::exists(url))
    {
        QString msg=QObject::tr("TiePointsIO::readTiePointsFile Error: File not exists:\n%1").arg(url);
        strError+=msg;
        return(TIEPOINTSIO_ERROR);
    }
    firstImagePixelsColumn.clear();
    firstImagePixelsRow.clear();
    secondImagePixelsColumn.clear();
    secondImagePixelsRow.clear();
    QFile file(url);
    int nLine=0;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while (!file.atEnd())
        {
            QString line = file.readLine();
            nLine++;
            QStringList splitedLine = line.simplified().split(" ");
            if(splitedLine.count() == 4)
            {
                for(int i=0;i<4;i++)
                {
                    QString strValue=splitedLine.at(i).trimmed();
                    bool okToDouble=false;
                    double value=strValue.toDouble(&okToDouble);
                    if(!okToDouble)
                    {
                        QString msg=QObject::tr("TiePointsIO::readTiePointsFile Error: In file:\n%1").arg(url);
                        msg+=QObject::tr("\in line number:%1 the value %2 is not a double").arg(QString::number(nLine)).arg(strValue);
                        strError+=msg;
                        firstImagePixelsColumn.clear();
                        firstImagePixelsRow.clear();
                        secondImagePixelsColumn.clear();
                        secondImagePixelsRow.clear();
                        return(TIEPOINTSIO_ERROR);
                    }
                    if(i==0) firstImagePixelsColumn.push_back(value);
                    if(i==1) firstImagePixelsRow.push_back(value);
                    if(i==2) secondImagePixelsColumn.push_back(value);
                    if(i==3) secondImagePixelsRow.push_back(value);
                }
            }
            else
            {
                QString msg=QObject::tr("TiePointsIO::readTiePointsFile Error: In file:\n%1").arg(url);
                msg+=QObject::tr("\in line number:%1 there are not four values").arg(QString::number(nLine));
                strError+=msg;
                firstImagePixelsColumn.clear();
                firstImagePixelsRow.clear();
                secondImagePixelsColumn.clear();
                secondImagePixelsRow.clear();
                return(TIEPOINTSIO_ERROR);
            }
        }
        file.close();
    }
    else
    {
        QString msg=QObject::tr("TiePointsIO::readTiePointsFile Error: Error opening file:\n%1").arg(url);
        strError+=msg;
        return(TIEPOINTSIO_ERROR);
    }
    return(TIEPOINTSIO_NO_ERROR);
}

//int TiePointsIO::writeHomol(QString& homolPath,
//                            QMap<QString, QVector<QString> > &imagesIdsTiePoints,
//                            QMap<QString, QMap<QString, QVector<double> > > &imagesFirstColumnsTiePoints,
//                            QMap<QString, QMap<QString, QVector<double> > > &imagesFirstRowsTiePoints,
//                            QMap<QString, QMap<QString, QVector<double> > > &imagesSecondColumnsTiePoints,
//                            QMap<QString, QMap<QString, QVector<double> > > &imagesSecondRowsTiePoints,
//                            QString& strError)
//{
//    QMap<QString,QVector<QString> >::const_iterator iterPairs=imagesIdsTiePoints.constBegin();
//    while(iterPairs!=imagesIdsTiePoints.constEnd())
//    {
//        QString firstImageUrl=iterPairs.key();
////        if(!imagesFirstColumnsTiePoints.contains(firstImageUrl))
////        {
////            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstColumnsTiePoints").arg(firstImageUrl);
////            strError+=msg;
////            return(TIEPOINTSIO_ERROR);
////        }
////        if(!imagesFirstRowsTiePoints.contains(firstImageUrl))
////        {
////            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstRowsTiePoints").arg(firstImageUrl);
////            strError+=msg;
////            return(TIEPOINTSIO_ERROR);
////        }
////        if(!imagesSecondColumnsTiePoints.contains(firstImageUrl))
////        {
////            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondColumnsTiePoints").arg(firstImageUrl);
////            strError+=msg;
////            return(TIEPOINTSIO_ERROR);
////        }
////        if(!imagesSecondRowsTiePoints.contains(firstImageUrl))
////        {
////            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondRowsTiePoints").arg(firstImageUrl);
////            strError+=msg;
////            return(TIEPOINTSIO_ERROR);
////        }
//        QString firstImagePath= homolPath + "/" + TAPIOCA_PASTIS_PATH+firstImageUrl;
//        QDir firstImageDir(firstImagePath);
//        if(!firstImageDir.exists())
//        {
//            if(!firstImageDir.mkdir(firstImagePath))
//            {
//                QString msg=QObject::tr("TiePointsIO::writeHomol Error:  making homol dir for image %1").arg(firstImageUrl);
//                strError+=msg;
//                return(TIEPOINTSIO_ERROR);
//            }
//        }
//        QVector<QString> imagesUrls=iterPairs.value();
//        for(int i=0;i<imagesUrls.size();i++)
//        {
//            QString secondImageUrl=imagesUrls.at(i);
//            QVector<double> firstColumns,firstRows,secondColumns,secondRows;
//            bool findFirstImage=false;
//            bool findSecondImage=false;
//            if(imagesFirstColumnsTiePoints.contains(firstImageUrl))
//            {
//                if(!imagesFirstColumnsTiePoints[firstImageUrl].contains(secondImageUrl))
//                {
//                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstColumnsTiePoints[] for image %2").arg(secondImageUrl).arg(firstImageUrl);
//                    strError+=msg;
//                    return(TIEPOINTSIO_ERROR);
//                }
//                if(!imagesFirstRowsTiePoints[firstImageUrl].contains(secondImageUrl))
//                {
//                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstRowsTiePoints[] for image %2").arg(secondImageUrl).arg(firstImageUrl);
//                    strError+=msg;
//                    return(TIEPOINTSIO_ERROR);
//                }
//                if(!imagesSecondColumnsTiePoints[firstImageUrl].contains(secondImageUrl))
//                {
//                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondColumnsTiePoints[] for image %2").arg(secondImageUrl).arg(firstImageUrl);
//                    strError+=msg;
//                    return(TIEPOINTSIO_ERROR);
//                }
//                if(!imagesSecondRowsTiePoints[firstImageUrl].contains(secondImageUrl))
//                {
//                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondRowsTiePoints[] for image %2").arg(secondImageUrl).arg(firstImageUrl);
//                    strError+=msg;
//                    return(TIEPOINTSIO_ERROR);
//                }
//                firstColumns=imagesFirstColumnsTiePoints[firstImageUrl][secondImageUrl];
//                firstRows=imagesFirstRowsTiePoints[firstImageUrl][secondImageUrl];
//                secondColumns=imagesSecondColumnsTiePoints[firstImageUrl][secondImageUrl];
//                secondRows=imagesSecondRowsTiePoints[firstImageUrl][secondImageUrl];
//                findFirstImage=true;
//                findSecondImage=true;
//            }
//            else
//            {
//                if(!imagesIdsTiePoints.contains(secondImageUrl))
//                {
//                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesIdsTiePoints[]").arg(secondImageUrl);
//                    strError+=msg;
//                    return(TIEPOINTSIO_ERROR);
//                }
//                if(!imagesFirstColumnsTiePoints[secondImageUrl].contains(firstImageUrl))
//                {
//                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstColumnsTiePoints[] for image %2").arg(firstImageUrl).arg(secondImageUrl);
//                    strError+=msg;
//                    return(TIEPOINTSIO_ERROR);
//                }
//                if(!imagesFirstRowsTiePoints[secondImageUrl].contains(firstImageUrl))
//                {
//                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstRowsTiePoints[] for image %2").arg(firstImageUrl).arg(secondImageUrl);
//                    strError+=msg;
//                    return(TIEPOINTSIO_ERROR);
//                }
//                if(!imagesSecondColumnsTiePoints[secondImageUrl].contains(firstImageUrl))
//                {
//                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondColumnsTiePoints[] for image %2").arg(firstImageUrl).arg(secondImageUrl);
//                    strError+=msg;
//                    return(TIEPOINTSIO_ERROR);
//                }
//                if(!imagesSecondRowsTiePoints[secondImageUrl].contains(firstImageUrl))
//                {
//                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondRowsTiePoints[] for image %2").arg(firstImageUrl).arg(secondImageUrl);
//                    strError+=msg;
//                    return(TIEPOINTSIO_ERROR);
//                }
//                firstColumns=imagesFirstColumnsTiePoints[secondImageUrl][firstImageUrl];
//                firstRows=imagesFirstRowsTiePoints[secondImageUrl][firstImageUrl];
//                secondColumns=imagesSecondColumnsTiePoints[secondImageUrl][firstImageUrl];
//                secondRows=imagesSecondRowsTiePoints[secondImageUrl][firstImageUrl];
//                findFirstImage=true;
//                findSecondImage=true;
//            }
//            if(firstColumns.size()!=firstRows.size()
//                    ||secondColumns.size()!=secondRows.size()
//                    ||firstColumns.size()!=secondColumns.size())
//            {
//                QString msg=QObject::tr("TiePointsIO::writeHomol Error: not equal dimensions in coordinates containes for image %1 and image %2").arg(firstImageUrl).arg(secondImageUrl);
//                strError+=msg;
//                return(TIEPOINTSIO_ERROR);
//            }
//            QString secondImageFileName=firstImagePath+"/"+secondImageUrl+MATCHING_TXT_FILE_EXTENSION;
//            QFile file(secondImageFileName);
//            if(file.open(QIODevice::WriteOnly | QIODevice::Text))
//            {
//                QTextStream out(&file);
//                QVector<QString> strFirstColumns(firstColumns.size());
//                QVector<QString> strFirstRows(firstColumns.size());
//                QVector<QString> strSecondColumns(firstColumns.size());
//                QVector<QString> strSecondRows(firstColumns.size());
//                double incrementCoordinates=(float)(1.0/pow(10.0,LIBPW_IMAGE_COORDINATES_PRECISION));
//                for(int nP=0;nP<firstColumns.size();nP++)
//                {
//                    double firstColumn=firstColumns[nP];
//                    double firstRow=firstRows[nP];
//                    double secondColumn=secondColumns[nP];
//                    double secondRow=secondRows[nP];
//                    strFirstColumns[nP]=QString::number(firstColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//                    strFirstRows[nP]=QString::number(firstRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//                    strSecondColumns[nP]=QString::number(secondColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//                    strSecondRows[nP]=QString::number(secondRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//                    if(nP>0)
//                    {
//                        bool coincidence=false;
//                        for(int nPa=0;nPa<nP;nPa++)
//                        {
//                            if(strFirstColumns[nP]==strFirstColumns[nPa]
//                                    &&strFirstRows[nP]==strFirstRows[nPa])
//                            {
//                                strFirstColumns[nP]=QString::number(firstColumn+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//                                strFirstRows[nP]=QString::number(firstRow+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//                                coincidence=true;
//                            }
//                            if(strSecondColumns[nP]==strSecondColumns[nPa]
//                                    &&strSecondRows[nP]==strSecondRows[nPa])
//                            {
//                                strSecondColumns[nP]=QString::number(secondColumn+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//                                strSecondRows[nP]=QString::number(secondRow+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//                                coincidence=true;
//                            }
//                            // no break por si coincidiera con otro tras el cambio
//                        }
//                    }
//                    out<<strFirstColumns[nP]<<" ";
//                    out<<strFirstRows[nP]<<" ";
//                    out<<strSecondColumns[nP]<<" ";
//                    out<<strSecondRows[nP]<<"\n";
//                }
//                file.close();
//            }
//            else
//            {
//                QString msg=QObject::tr("TiePointsIO::writeHomol Error: opening file por image %1 and image %2").arg(firstImageUrl).arg(secondImageUrl);
//                strError+=msg;
//                return(TIEPOINTSIO_ERROR);
//            }
//        }
//        iterPairs++;
//    }
//    return(TIEPOINTSIO_NO_ERROR);
//}

int TiePointsIO::writeHomol(QString& homolPath,
                            QMap<QString, QVector<QString> > &imagesIdsTiePoints,
                            QMap<QString, QMap<QString, QVector<double> > > &imagesFirstColumnsTiePoints,
                            QMap<QString, QMap<QString, QVector<double> > > &imagesFirstRowsTiePoints,
                            QMap<QString, QMap<QString, QVector<double> > > &imagesSecondColumnsTiePoints,
                            QMap<QString, QMap<QString, QVector<double> > > &imagesSecondRowsTiePoints,
                            QString& strError,
                            bool binaryFormat=false)
{
    QMap<QString,QVector<QString> >::const_iterator iterPairs=imagesIdsTiePoints.constBegin();
    while(iterPairs!=imagesIdsTiePoints.constEnd())
    {
        QString firstImageUrl=iterPairs.key();
//        if(!imagesFirstColumnsTiePoints.contains(firstImageUrl))
//        {
//            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstColumnsTiePoints").arg(firstImageUrl);
//            strError+=msg;
//            return(TIEPOINTSIO_ERROR);
//        }
//        if(!imagesFirstRowsTiePoints.contains(firstImageUrl))
//        {
//            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstRowsTiePoints").arg(firstImageUrl);
//            strError+=msg;
//            return(TIEPOINTSIO_ERROR);
//        }
//        if(!imagesSecondColumnsTiePoints.contains(firstImageUrl))
//        {
//            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondColumnsTiePoints").arg(firstImageUrl);
//            strError+=msg;
//            return(TIEPOINTSIO_ERROR);
//        }
//        if(!imagesSecondRowsTiePoints.contains(firstImageUrl))
//        {
//            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondRowsTiePoints").arg(firstImageUrl);
//            strError+=msg;
//            return(TIEPOINTSIO_ERROR);
//        }
        QString firstImagePath= homolPath + "/" + TAPIOCA_PASTIS_PATH+firstImageUrl;
        QDir firstImageDir(firstImagePath);
        if(!firstImageDir.exists())
        {
            if(!firstImageDir.mkdir(firstImagePath))
            {
                QString msg=QObject::tr("TiePointsIO::writeHomol Error:  making homol dir for image %1").arg(firstImageUrl);
                strError+=msg;
                return(TIEPOINTSIO_ERROR);
            }
        }
        QVector<QString> imagesUrls=iterPairs.value();
        for(int i=0;i<imagesUrls.size();i++)
        {
            QString secondImageUrl=imagesUrls.at(i);
            QVector<double> firstColumns,firstRows,secondColumns,secondRows;
            bool findFirstImage=false;
            bool findSecondImage=false;
            if(imagesFirstColumnsTiePoints.contains(firstImageUrl))
            {
                if(!imagesFirstColumnsTiePoints[firstImageUrl].contains(secondImageUrl))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstColumnsTiePoints[] for image %2").arg(secondImageUrl).arg(firstImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesFirstRowsTiePoints[firstImageUrl].contains(secondImageUrl))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstRowsTiePoints[] for image %2").arg(secondImageUrl).arg(firstImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesSecondColumnsTiePoints[firstImageUrl].contains(secondImageUrl))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondColumnsTiePoints[] for image %2").arg(secondImageUrl).arg(firstImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesSecondRowsTiePoints[firstImageUrl].contains(secondImageUrl))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondRowsTiePoints[] for image %2").arg(secondImageUrl).arg(firstImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                firstColumns=imagesFirstColumnsTiePoints[firstImageUrl][secondImageUrl];
                firstRows=imagesFirstRowsTiePoints[firstImageUrl][secondImageUrl];
                secondColumns=imagesSecondColumnsTiePoints[firstImageUrl][secondImageUrl];
                secondRows=imagesSecondRowsTiePoints[firstImageUrl][secondImageUrl];
                findFirstImage=true;
                findSecondImage=true;
            }
            else
            {
                if(!imagesIdsTiePoints.contains(secondImageUrl))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesIdsTiePoints[]").arg(secondImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesFirstColumnsTiePoints[secondImageUrl].contains(firstImageUrl))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstColumnsTiePoints[] for image %2").arg(firstImageUrl).arg(secondImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesFirstRowsTiePoints[secondImageUrl].contains(firstImageUrl))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstRowsTiePoints[] for image %2").arg(firstImageUrl).arg(secondImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesSecondColumnsTiePoints[secondImageUrl].contains(firstImageUrl))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondColumnsTiePoints[] for image %2").arg(firstImageUrl).arg(secondImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesSecondRowsTiePoints[secondImageUrl].contains(firstImageUrl))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondRowsTiePoints[] for image %2").arg(firstImageUrl).arg(secondImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                firstColumns=imagesFirstColumnsTiePoints[secondImageUrl][firstImageUrl];
                firstRows=imagesFirstRowsTiePoints[secondImageUrl][firstImageUrl];
                secondColumns=imagesSecondColumnsTiePoints[secondImageUrl][firstImageUrl];
                secondRows=imagesSecondRowsTiePoints[secondImageUrl][firstImageUrl];
                findFirstImage=true;
                findSecondImage=true;
            }
            if(firstColumns.size()!=firstRows.size()
                    ||secondColumns.size()!=secondRows.size()
                    ||firstColumns.size()!=secondColumns.size())
            {
                QString msg=QObject::tr("TiePointsIO::writeHomol Error: not equal dimensions in coordinates containes for image %1 and image %2").arg(firstImageUrl).arg(secondImageUrl);
                strError+=msg;
                return(TIEPOINTSIO_ERROR);
            }
            QString secondImageFileName=firstImagePath+"/"+secondImageUrl;
            if(!binaryFormat)
                secondImageFileName+=MATCHING_TXT_FILE_EXTENSION;
            else
                secondImageFileName+=MATCHING_BINARY_FILE_EXTENSION;
            if(!binaryFormat)
            {
                QFile file(secondImageFileName);
                if(file.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    QTextStream out(&file);
                    QVector<QString> strFirstColumns(firstColumns.size());
                    QVector<QString> strFirstRows(firstColumns.size());
                    QVector<QString> strSecondColumns(firstColumns.size());
                    QVector<QString> strSecondRows(firstColumns.size());
                    double incrementCoordinates=(float)(1.0/pow(10.0,LIBPW_IMAGE_COORDINATES_PRECISION));
                    for(int nP=0;nP<firstColumns.size();nP++)
                    {
                        double firstColumn=firstColumns[nP];
                        double firstRow=firstRows[nP];
                        double secondColumn=secondColumns[nP];
                        double secondRow=secondRows[nP];
                        strFirstColumns[nP]=QString::number(firstColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                        strFirstRows[nP]=QString::number(firstRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                        strSecondColumns[nP]=QString::number(secondColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                        strSecondRows[nP]=QString::number(secondRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                        bool coincidence=false;
                        for(int nPa=0;nPa<nP;nPa++)
                        {
                            if(strFirstColumns[nP]==strFirstColumns[nPa]
                                    &&strFirstRows[nP]==strFirstRows[nPa])
                            {
                                strFirstColumns[nP]=QString::number(firstColumn+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                                strFirstRows[nP]=QString::number(firstRow+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                                coincidence=true;
                            }
                            if(strSecondColumns[nP]==strSecondColumns[nPa]
                                    &&strSecondRows[nP]==strSecondRows[nPa])
                            {
                                strSecondColumns[nP]=QString::number(secondColumn+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                                strSecondRows[nP]=QString::number(secondRow+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                                coincidence=true;
                            }
                            // no break por si coincidiera con otro tras el cambio
                        }
                        out<<strFirstColumns[nP]<<" ";
                        out<<strFirstRows[nP]<<" ";
                        out<<strSecondColumns[nP]<<" ";
                        out<<strSecondRows[nP]<<"\n";
                    }
                    file.close();
                }
                else
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: opening file por image %1 and image %2").arg(firstImageUrl).arg(secondImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
            }
            else
            {
                QString fileName=secondImageFileName.replace("/","\\");
                string i_filename=fileName.toStdString();
                ofstream f( i_filename.c_str(), ios::binary );
                if ( !f )
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: opening binary file por image %1 and image %2").arg(firstImageUrl).arg(secondImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }

                int32_t dimension=2; // dimension est la taille des nuplets, a priori toujours = 2
                f.write( (char*)&dimension, 4 );
                int numberOfTiePoints=firstColumns.size();
                int32_t nbNuplets=numberOfTiePoints; // le nombre de couples
                f.write( (char*)&nbNuplets, 4 );
                QVector<QString> strFirstColumns(firstColumns.size());
                QVector<QString> strFirstRows(firstColumns.size());
                QVector<QString> strSecondColumns(firstColumns.size());
                QVector<QString> strSecondRows(firstColumns.size());
                double incrementCoordinates=(float)(1.0/pow(10.0,LIBPW_IMAGE_COORDINATES_PRECISION));
                for(int nP=0;nP<firstColumns.size();nP++)
                {
                    double firstColumn=firstColumns[nP];
                    double firstRow=firstRows[nP];
                    double secondColumn=secondColumns[nP];
                    double secondRow=secondRows[nP];
                    strFirstColumns[nP]=QString::number(firstColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                    strFirstRows[nP]=QString::number(firstRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                    strSecondColumns[nP]=QString::number(secondColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                    strSecondRows[nP]=QString::number(secondRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                    bool coincidence=false;
                    for(int nPa=0;nPa<nP;nPa++)
                    {
                        if(strFirstColumns[nP]==strFirstColumns[nPa]
                                &&strFirstRows[nP]==strFirstRows[nPa])
                        {
                            strFirstColumns[nP]=QString::number(firstColumn+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                            strFirstRows[nP]=QString::number(firstRow+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                            coincidence=true;
                        }
                        if(strSecondColumns[nP]==strSecondColumns[nPa]
                                &&strSecondRows[nP]==strSecondRows[nPa])
                        {
                            strSecondColumns[nP]=QString::number(secondColumn+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                            strSecondRows[nP]=QString::number(secondRow+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                            coincidence=true;
                        }
                        // no break por si coincidiera con otro tras el cambio
                    }
                    double x1=strFirstColumns[nP].toDouble();
                    double y1=strFirstRows[nP].toDouble();
                    double x2=strSecondColumns[nP].toDouble();
                    double y2=strSecondRows[nP].toDouble();
                    int32_t nbPoints=2;
                    f.write( (char*)&nbPoints, 4 );
                    double scale=1.0;
                    f.write( (char*)&scale, sizeof(double) );
                    double p[2];
                    p[0]=x1;
                    p[1]=y1;
                    f.write( (char*)p, 2*sizeof(double) );
                    p[0]=x2;
                    p[1]=y2;
                    f.write( (char*)p, 2*sizeof(double) );
                }
            }
        }
        iterPairs++;
    }
    return(TIEPOINTSIO_NO_ERROR);
}

int TiePointsIO::writeHomolOrtoSky(QString& homolPath,
                                   QMap<QString, QString> &imageFileNameWithoutPathsByName,
                                   QMap<QString, QVector<QString> > &imagesIdsTiePoints,
                                   QMap<QString, QMap<QString, QVector<double> > > &imagesFirstColumnsTiePoints,
                                   QMap<QString, QMap<QString, QVector<double> > > &imagesFirstRowsTiePoints,
                                   QMap<QString, QMap<QString, QVector<double> > > &imagesSecondColumnsTiePoints,
                                   QMap<QString, QMap<QString, QVector<double> > > &imagesSecondRowsTiePoints,
                                   QString& strError)
{
    QMap<QString,QVector<QString> >::const_iterator iterPairs=imagesIdsTiePoints.constBegin();
    while(iterPairs!=imagesIdsTiePoints.constEnd())
    {
        QString firstImageId=iterPairs.key();
        QString firstImageUrl=imageFileNameWithoutPathsByName[firstImageId]; // filenam
//        if(!imagesFirstColumnsTiePoints.contains(firstImageUrl))
//        {
//            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstColumnsTiePoints").arg(firstImageUrl);
//            strError+=msg;
//            return(TIEPOINTSIO_ERROR);
//        }
//        if(!imagesFirstRowsTiePoints.contains(firstImageUrl))
//        {
//            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstRowsTiePoints").arg(firstImageUrl);
//            strError+=msg;
//            return(TIEPOINTSIO_ERROR);
//        }
//        if(!imagesSecondColumnsTiePoints.contains(firstImageUrl))
//        {
//            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondColumnsTiePoints").arg(firstImageUrl);
//            strError+=msg;
//            return(TIEPOINTSIO_ERROR);
//        }
//        if(!imagesSecondRowsTiePoints.contains(firstImageUrl))
//        {
//            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondRowsTiePoints").arg(firstImageUrl);
//            strError+=msg;
//            return(TIEPOINTSIO_ERROR);
//        }
        QString firstImagePath= homolPath + "/" + TAPIOCA_PASTIS_PATH+firstImageUrl;
        QDir firstImageDir(firstImagePath);
        if(!firstImageDir.exists())
        {
            if(!firstImageDir.mkdir(firstImagePath))
            {
                QString msg=QObject::tr("TiePointsIO::writeHomol Error:  making homol dir for image %1").arg(firstImageUrl);
                strError+=msg;
                return(TIEPOINTSIO_ERROR);
            }
        }
        QVector<QString> imagesIds=iterPairs.value();
        for(int i=0;i<imagesIds.size();i++)
        {
            QString secondImageId=imagesIds.at(i);
            QString secondImageUrl=imageFileNameWithoutPathsByName[secondImageId];
            QVector<double> firstColumns,firstRows,secondColumns,secondRows;
            bool findFirstImage=false;
            bool findSecondImage=false;
            if(imagesFirstColumnsTiePoints.contains(firstImageId))
            {
                if(!imagesFirstColumnsTiePoints[firstImageId].contains(secondImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstColumnsTiePoints[] for image %2").arg(secondImageId).arg(firstImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesFirstRowsTiePoints[firstImageId].contains(secondImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstRowsTiePoints[] for image %2").arg(secondImageId).arg(firstImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesSecondColumnsTiePoints[firstImageId].contains(secondImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondColumnsTiePoints[] for image %2").arg(secondImageId).arg(firstImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesSecondRowsTiePoints[firstImageId].contains(secondImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondRowsTiePoints[] for image %2").arg(secondImageId).arg(firstImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                firstColumns=imagesFirstColumnsTiePoints[firstImageId][secondImageId];
                firstRows=imagesFirstRowsTiePoints[firstImageId][secondImageId];
                secondColumns=imagesSecondColumnsTiePoints[firstImageId][secondImageId];
                secondRows=imagesSecondRowsTiePoints[firstImageId][secondImageId];
                findFirstImage=true;
                findSecondImage=true;
            }
            else
            {
                if(!imagesIdsTiePoints.contains(secondImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesIdsTiePoints[]").arg(secondImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesFirstColumnsTiePoints[secondImageId].contains(firstImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstColumnsTiePoints[] for image %2").arg(firstImageId).arg(secondImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesFirstRowsTiePoints[secondImageId].contains(firstImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstRowsTiePoints[] for image %2").arg(firstImageId).arg(secondImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesSecondColumnsTiePoints[secondImageId].contains(firstImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondColumnsTiePoints[] for image %2").arg(firstImageId).arg(secondImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesSecondRowsTiePoints[secondImageId].contains(firstImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondRowsTiePoints[] for image %2").arg(firstImageId).arg(secondImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
//                firstColumns=imagesFirstColumnsTiePoints[secondImageId][firstImageId];
//                firstRows=imagesFirstRowsTiePoints[secondImageId][firstImageId];
//                secondColumns=imagesSecondColumnsTiePoints[secondImageId][firstImageId];
//                secondRows=imagesSecondRowsTiePoints[secondImageId][firstImageId];
                secondColumns=imagesFirstColumnsTiePoints[secondImageId][firstImageId];
                secondRows=imagesFirstRowsTiePoints[secondImageId][firstImageId];
                firstColumns=imagesSecondColumnsTiePoints[secondImageId][firstImageId];
                firstRows=imagesSecondRowsTiePoints[secondImageId][firstImageId];
                findFirstImage=true;
                findSecondImage=true;
            }
            if(firstColumns.size()!=firstRows.size()
                    ||secondColumns.size()!=secondRows.size()
                    ||firstColumns.size()!=secondColumns.size())
            {
                QString msg=QObject::tr("TiePointsIO::writeHomol Error: not equal dimensions in coordinates containes for image %1 and image %2").arg(firstImageId).arg(secondImageId);
                strError+=msg;
                return(TIEPOINTSIO_ERROR);
            }
            QString secondImageFileName=firstImagePath+"/"+secondImageUrl+MATCHING_TXT_FILE_EXTENSION;
            QFile file(secondImageFileName);
            if(file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&file);
                QVector<QString> strFirstColumns(firstColumns.size());
                QVector<QString> strFirstRows(firstColumns.size());
                QVector<QString> strSecondColumns(firstColumns.size());
                QVector<QString> strSecondRows(firstColumns.size());
                double incrementCoordinates=(float)(1.0/pow(10.0,LIBPW_IMAGE_COORDINATES_PRECISION));
                for(int nP=0;nP<firstColumns.size();nP++)
                {
                    double firstColumn=firstColumns[nP];
                    double firstRow=firstRows[nP];
                    double secondColumn=secondColumns[nP];
                    double secondRow=secondRows[nP];
                    strFirstColumns[nP]=QString::number(firstColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                    strFirstRows[nP]=QString::number(firstRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                    strSecondColumns[nP]=QString::number(secondColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                    strSecondRows[nP]=QString::number(secondRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                    if(nP>0)
                    {
                        bool coincidence=false;
                        for(int nPa=0;nPa<nP;nPa++)
                        {
                            if(strFirstColumns[nP]==strFirstColumns[nPa]
                                    &&strFirstRows[nP]==strFirstRows[nPa])
                            {
                                strFirstColumns[nP]=QString::number(firstColumn+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                                strFirstRows[nP]=QString::number(firstRow+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                                coincidence=true;
                            }
                            if(strSecondColumns[nP]==strSecondColumns[nPa]
                                    &&strSecondRows[nP]==strSecondRows[nPa])
                            {
                                strSecondColumns[nP]=QString::number(secondColumn+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                                strSecondRows[nP]=QString::number(secondRow+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                                coincidence=true;
                            }
                            // no break por si coincidiera con otro tras el cambio
                        }
                    }
                    out<<strFirstColumns[nP]<<" ";
                    out<<strFirstRows[nP]<<" ";
                    out<<strSecondColumns[nP]<<" ";
                    out<<strSecondRows[nP]<<"\n";
                }
                file.close();
            }
            else
            {
                QString msg=QObject::tr("TiePointsIO::writeHomol Error: opening file por image %1 and image %2").arg(firstImageUrl).arg(secondImageUrl);
                strError+=msg;
                return(TIEPOINTSIO_ERROR);
            }
        }
        iterPairs++;
    }
    return(TIEPOINTSIO_NO_ERROR);
}

int TiePointsIO::writeHomolOrtoSky(QString& homolPath,
                                   QMap<QString, QString> &imageFileNameWithoutPathsByName,
                                   QMap<QString, QVector<QString> > &imagesIdsTiePoints,
                                   QMap<QString, QMap<QString, QVector<bool> > > &imagesEnabledsTiePoints,
                                   QMap<QString, QMap<QString, QVector<double> > > &imagesFirstColumnsTiePoints,
                                   QMap<QString, QMap<QString, QVector<double> > > &imagesFirstRowsTiePoints,
                                   QMap<QString, QMap<QString, QVector<double> > > &imagesSecondColumnsTiePoints,
                                   QMap<QString, QMap<QString, QVector<double> > > &imagesSecondRowsTiePoints,
                                   QString& strError)
{
    QMap<QString,QVector<QString> >::const_iterator iterPairs=imagesIdsTiePoints.constBegin();
    while(iterPairs!=imagesIdsTiePoints.constEnd())
    {
        QString firstImageId=iterPairs.key();
        QString firstImageUrl=imageFileNameWithoutPathsByName[firstImageId]; // filenam
//        if(!imagesFirstColumnsTiePoints.contains(firstImageUrl))
//        {
//            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstColumnsTiePoints").arg(firstImageUrl);
//            strError+=msg;
//            return(TIEPOINTSIO_ERROR);
//        }
//        if(!imagesFirstRowsTiePoints.contains(firstImageUrl))
//        {
//            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstRowsTiePoints").arg(firstImageUrl);
//            strError+=msg;
//            return(TIEPOINTSIO_ERROR);
//        }
//        if(!imagesSecondColumnsTiePoints.contains(firstImageUrl))
//        {
//            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondColumnsTiePoints").arg(firstImageUrl);
//            strError+=msg;
//            return(TIEPOINTSIO_ERROR);
//        }
//        if(!imagesSecondRowsTiePoints.contains(firstImageUrl))
//        {
//            QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondRowsTiePoints").arg(firstImageUrl);
//            strError+=msg;
//            return(TIEPOINTSIO_ERROR);
//        }
        QString firstImagePath= homolPath + "/" + TAPIOCA_PASTIS_PATH+firstImageUrl;
        QDir firstImageDir(firstImagePath);
        if(!firstImageDir.exists())
        {
            if(!firstImageDir.mkdir(firstImagePath))
            {
                QString msg=QObject::tr("TiePointsIO::writeHomol Error:  making homol dir for image %1").arg(firstImageUrl);
                strError+=msg;
                return(TIEPOINTSIO_ERROR);
            }
        }
        QVector<QString> imagesIds=iterPairs.value();
        for(int i=0;i<imagesIds.size();i++)
        {
            QString secondImageId=imagesIds.at(i);
            QString secondImageUrl=imageFileNameWithoutPathsByName[secondImageId];
            QVector<double> firstColumns,firstRows,secondColumns,secondRows;
            QVector<bool> enableds;
            bool findFirstImage=false;
            bool findSecondImage=false;
            if(imagesFirstColumnsTiePoints.contains(firstImageId))
            {
                if(!imagesFirstColumnsTiePoints[firstImageId].contains(secondImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstColumnsTiePoints[] for image %2").arg(secondImageId).arg(firstImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesFirstRowsTiePoints[firstImageId].contains(secondImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstRowsTiePoints[] for image %2").arg(secondImageId).arg(firstImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesSecondColumnsTiePoints[firstImageId].contains(secondImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondColumnsTiePoints[] for image %2").arg(secondImageId).arg(firstImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesSecondRowsTiePoints[firstImageId].contains(secondImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondRowsTiePoints[] for image %2").arg(secondImageId).arg(firstImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesEnabledsTiePoints[firstImageId].contains(secondImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesEnabledsTiePoints[] for image %2").arg(secondImageId).arg(firstImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                firstColumns=imagesFirstColumnsTiePoints[firstImageId][secondImageId];
                firstRows=imagesFirstRowsTiePoints[firstImageId][secondImageId];
                secondColumns=imagesSecondColumnsTiePoints[firstImageId][secondImageId];
                secondRows=imagesSecondRowsTiePoints[firstImageId][secondImageId];
                enableds=imagesEnabledsTiePoints[firstImageId][secondImageId];
                findFirstImage=true;
                findSecondImage=true;
            }
            else
            {
                if(!imagesIdsTiePoints.contains(secondImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesIdsTiePoints[]").arg(secondImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesFirstColumnsTiePoints[secondImageId].contains(firstImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstColumnsTiePoints[] for image %2").arg(firstImageId).arg(secondImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesFirstRowsTiePoints[secondImageId].contains(firstImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesFirstRowsTiePoints[] for image %2").arg(firstImageId).arg(secondImageUrl);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesSecondColumnsTiePoints[secondImageId].contains(firstImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondColumnsTiePoints[] for image %2").arg(firstImageId).arg(secondImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesSecondRowsTiePoints[secondImageId].contains(firstImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesSecondRowsTiePoints[] for image %2").arg(firstImageId).arg(secondImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
                if(!imagesEnabledsTiePoints[secondImageId].contains(firstImageId))
                {
                    QString msg=QObject::tr("TiePointsIO::writeHomol Error: no image %1 in imagesEnabledsTiePoints[] for image %2").arg(firstImageId).arg(secondImageId);
                    strError+=msg;
                    return(TIEPOINTSIO_ERROR);
                }
//                firstColumns=imagesFirstColumnsTiePoints[secondImageId][firstImageId];
//                firstRows=imagesFirstRowsTiePoints[secondImageId][firstImageId];
//                secondColumns=imagesSecondColumnsTiePoints[secondImageId][firstImageId];
//                secondRows=imagesSecondRowsTiePoints[secondImageId][firstImageId];
                secondColumns=imagesFirstColumnsTiePoints[secondImageId][firstImageId];
                secondRows=imagesFirstRowsTiePoints[secondImageId][firstImageId];
                firstColumns=imagesSecondColumnsTiePoints[secondImageId][firstImageId];
                firstRows=imagesSecondRowsTiePoints[secondImageId][firstImageId];
                enableds=imagesEnabledsTiePoints[secondImageId][firstImageId];
                findFirstImage=true;
                findSecondImage=true;
            }
            if(firstColumns.size()!=firstRows.size()
                    ||secondColumns.size()!=secondRows.size()
                    ||firstColumns.size()!=secondColumns.size()
                    ||enableds.size()!=secondColumns.size())
            {
                QString msg=QObject::tr("TiePointsIO::writeHomol Error: not equal dimensions in coordinates containes for image %1 and image %2").arg(firstImageId).arg(secondImageId);
                strError+=msg;
                return(TIEPOINTSIO_ERROR);
            }
            QString secondImageFileName=firstImagePath+"/"+secondImageUrl+MATCHING_TXT_FILE_EXTENSION;
            QFile file(secondImageFileName);
            if(file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&file);
                QVector<QString> strFirstColumns(firstColumns.size());
                QVector<QString> strFirstRows(firstColumns.size());
                QVector<QString> strSecondColumns(firstColumns.size());
                QVector<QString> strSecondRows(firstColumns.size());
                double incrementCoordinates=(float)(1.0/pow(10.0,LIBPW_IMAGE_COORDINATES_PRECISION));
                for(int nP=0;nP<firstColumns.size();nP++)
                {
                    if(enableds[nP])
                    {
                        double firstColumn=firstColumns[nP];
                        double firstRow=firstRows[nP];
                        double secondColumn=secondColumns[nP];
                        double secondRow=secondRows[nP];
                        strFirstColumns[nP]=QString::number(firstColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                        strFirstRows[nP]=QString::number(firstRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                        strSecondColumns[nP]=QString::number(secondColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                        strSecondRows[nP]=QString::number(secondRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                        if(nP>0)
                        {
                            bool coincidence=false;
                            for(int nPa=0;nPa<nP;nPa++)
                            {
                                if(enableds[nPa])
                                {
                                    if(strFirstColumns[nP]==strFirstColumns[nPa]
                                            &&strFirstRows[nP]==strFirstRows[nPa])
                                    {
                                        strFirstColumns[nP]=QString::number(firstColumn+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                                        strFirstRows[nP]=QString::number(firstRow+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                                        coincidence=true;
                                    }
                                    if(strSecondColumns[nP]==strSecondColumns[nPa]
                                            &&strSecondRows[nP]==strSecondRows[nPa])
                                    {
                                        strSecondColumns[nP]=QString::number(secondColumn+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                                        strSecondRows[nP]=QString::number(secondRow+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
                                        coincidence=true;
                                    }
                                    // no break por si coincidiera con otro tras el cambio
                                }
                            }
                        }
                        out<<strFirstColumns[nP]<<" ";
                        out<<strFirstRows[nP]<<" ";
                        out<<strSecondColumns[nP]<<" ";
                        out<<strSecondRows[nP]<<"\n";
                    }
                }
                file.close();
            }
            else
            {
                QString msg=QObject::tr("TiePointsIO::writeHomol Error: opening file por image %1 and image %2").arg(firstImageUrl).arg(secondImageUrl);
                strError+=msg;
                return(TIEPOINTSIO_ERROR);
            }
        }
        iterPairs++;
    }
    return(TIEPOINTSIO_NO_ERROR);
}

int TiePointsIO::removeDuplicateLines(QString filePath)
{
    QFile file(filePath);
    QStringList lines;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        while (!file.atEnd()) {
            QString line = file.readLine();
            if (!lines.contains(line))
                lines.append(line);
        }
        file.close();

        QFile file(filePath);
        if(file.open(QIODevice::WriteOnly)){
            QTextStream out(&file);
            Q_FOREACH (QString line, lines){
                out << line;
            }
            file.close();
        }
        else return 1;
    }
    else return 1;

    return 0;
}


int TiePointsIO::WriteDatTiePoints(QString &url, QVector<double> &firstImagePixelsColumn,
                                   QVector<double> &firstImagePixelsRow,
                                   QVector<double> &secondImagePixelsColumn,
                                   QVector<double> &secondImagePixelsRow, QString &strError){

    ofstream f( url.toUtf8().constData(), ios::binary );
    if ( !f )
    {
        QString msg="TiePointsIO::writeHomol Error: opening binary file "+url;
        strError+=msg;
        return(TIEPOINTSIO_ERROR);
    }

    int32_t dimension=2; // dimension est la taille des nuplets, a priori toujours = 2
    f.write( (char*)&dimension, 4 );
    int numberOfTiePoints=firstImagePixelsColumn.size();
    int32_t nbNuplets=numberOfTiePoints; // le nombre de couples
    f.write( (char*)&nbNuplets, 4 );
//    QVector<QString> strFirstColumns(firstColumns.size());
//    QVector<QString> strFirstRows(firstColumns.size());
//    QVector<QString> strSecondColumns(firstColumns.size());
//    QVector<QString> strSecondRows(firstColumns.size());
//    double incrementCoordinates=(float)(1.0/pow(10.0,LIBPW_IMAGE_COORDINATES_PRECISION));
    for(int nP=0;nP<firstImagePixelsColumn.size();nP++)
    {
        double x1=round(firstImagePixelsColumn[nP],4);
        double y1=round(firstImagePixelsRow[nP],4);
        double x2=round(secondImagePixelsColumn[nP],4);
        double y2=round(secondImagePixelsRow[nP],4);
//        QString strFirstColumns=QString::number(firstColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//        QString strFirstRows=QString::number(firstRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//        QString strSecondColumns=QString::number(secondColumn,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//        QString strSecondRows=QString::number(secondRow,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//        bool coincidence=false;
//        for(int nPa=0;nPa<nP;nPa++)
//        {
//            if(strFirstColumns[nP]==strFirstColumns[nPa]
//                    &&strFirstRows[nP]==strFirstRows[nPa])
//            {
//                strFirstColumns[nP]=QString::number(firstColumn+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//                strFirstRows[nP]=QString::number(firstRow+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//                coincidence=true;
//            }
//            if(strSecondColumns[nP]==strSecondColumns[nPa]
//                    &&strSecondRows[nP]==strSecondRows[nPa])
//            {
//                strSecondColumns[nP]=QString::number(secondColumn+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//                strSecondRows[nP]=QString::number(secondRow+incrementCoordinates,'f',LIBPW_IMAGE_COORDINATES_PRECISION);
//                coincidence=true;
//            }
//            // no break por si coincidiera con otro tras el cambio
//        }
//        double x1=strFirstColumns.toDouble();
//        double y1=strFirstRows.toDouble();
//        double x2=strSecondColumns.toDouble();
//        double y2=strSecondRows.toDouble();
        int32_t nbPoints=2;
        f.write( (char*)&nbPoints, 4 );
        double scale=1.0;
        f.write( (char*)&scale, sizeof(double) );
        double p[2];
        p[0]=x1;
        p[1]=y1;
        f.write( (char*)p, 2*sizeof(double) );
        p[0]=x2;
        p[1]=y2;
        f.write( (char*)p, 2*sizeof(double) );
    }
}
