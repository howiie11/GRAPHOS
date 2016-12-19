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
#include <QFile>
#include <QDir>
#include <QTextStream>

#include <opencv2/calib3d/calib3d.hpp>

#include "libOrsa.h"
#include "libOrsaAsift.h"

#include "OpenCVKpMParametersDefinitions.h"
#include "OpenCVKpM.h"

using namespace PW;

OpenCVKpM::OpenCVKpM()
{
    initialize();
}

double OpenCVKpM::azimuth(double xi,
                          double yi,
                          double xj,
                          double yj)
{
    double pi=4.0*atan(1.0);
    double ax,ay;
    ax=xj-xi;
    ay=yj-yi;
    double azimuth=atan2(ax,ay);
    if(azimuth<0.0) azimuth=azimuth+2.0*pi;
    return(azimuth);
}

double OpenCVKpM::distance(double xi,
                           double yi,
                           double xj,
                           double yj)
{
    double ax,ay;
    ax=xj-xi;
    ay=yj-yi;
    double dis2d=sqrt(ax*ax+ay*ay);
    return(dis2d);
}
/*
int OpenCVKpM::detectOutliersInMatches(bool &printReport,
                                       QString &strError)
{
    QString outlierDetectorTag;
    QStringList parametersTag;
    QStringList parametersValue;
    QStringList parametersType;
    if(OPENCV_KPM_NO_ERROR!=mParameters.getOutliersDetector(outlierDetectorTag,
                                                            parametersTag,
                                                            parametersValue,
                                                            parametersType,
                                                            strError))
    {
        strError="OpenCVKpM::detectOutliersInMatches,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    bool findOutlierDetector=false;
    QString pathForMatchesFiles;
    bool saveImagesMatches=mParameters.getSaveImagesMatches(pathForMatchesFiles);
    bool bidirectionalMatches=mParameters.getBidirectionalMatches();
    //outlierDetectorTag="";
    if(outlierDetectorTag.compare(OPENCV_KPM_OUTLIERSDETECTOR_DHL_KMEANS_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHNUMBEROFCLUSTERS_TAG;
        int parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int azimuthNumberOfClusters=parametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHEPSILONDEG_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double azimuthEpsilogDeg=parametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHITERATIONS_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int azimuthIterations=parametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_DISTANCENUMBEROFCLUSTERS_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int distanceNumberOfClusters=parametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_DISTANCEEPSILON_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double distanceEpsilon=parametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_DISTANCEITERATIONS_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int distanceIterations=parametersValue[parameterPos].toInt();

        QTextStream qout(stdout);
        QMap<QString,QVector<QString> >::const_iterator iterImagePairs = mImagesPairs.constBegin();
        double pi=4.0*atan(1.0);
        while(iterImagePairs!=mImagesPairs.constEnd())
        {
            QString imageId=iterImagePairs.key();
            if(!mImagesKeyPoints.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesKeyPointsDescriptors.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesPairsMatchesPointsFirsts.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points firsts for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesPairsMatchesPointsSeconds.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points seconds for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(bidirectionalMatches)
            {
                if(!mImagesPairsMatchesPointsFirstsReverse.contains(imageId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points firsts for image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
                if(!mImagesPairsMatchesPointsSecondsReverse.contains(imageId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points seconds for image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
            }
            std::vector<cv::KeyPoint> imageKeyPoints=mImagesKeyPoints[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsFirst=mImagesPairsMatchesPointsFirsts[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsSecond=mImagesPairsMatchesPointsSeconds[imageId];
            QVector<std::vector<int> > imagePairsMatchesIndexesFirst=mImagesPairsMatchesIndexesFirsts[imageId];
            QVector<std::vector<int> > imagePairsMatchesIndexesSecond=mImagesPairsMatchesIndexesSeconds[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsFirstReverse;
            QVector<std::vector<cv::Point2f> > matchingPointsSecondReverse;
            QVector<std::vector<int> > imagePairsMatchesIndexesFirstReverse;
            QVector<std::vector<int> > imagePairsMatchesIndexesSecondReverse;
            if(bidirectionalMatches)
            {
                matchingPointsFirstReverse=mImagesPairsMatchesPointsFirstsReverse[imageId];
                matchingPointsSecondReverse=mImagesPairsMatchesPointsSecondsReverse[imageId];
                imagePairsMatchesIndexesFirstReverse=mImagesPairsMatchesIndexesFirstsReverse[imageId];
                imagePairsMatchesIndexesSecondReverse=mImagesPairsMatchesIndexesSecondsReverse[imageId];
            }
            QVector<QString> imagePairs=iterImagePairs.value();
            if(imagePairs.size()!=matchingPointsFirst.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(imagePairs.size()!=matchingPointsSecond.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(bidirectionalMatches)
            {
                if(imagePairs.size()!=matchingPointsFirstReverse.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
                if(imagePairs.size()!=matchingPointsSecondReverse.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
            }
            for(int nImageInPairs=0;nImageInPairs<imagePairs.size();nImageInPairs++)
            {
                QString imagePairId=imagePairs.at(nImageInPairs);
                if(!mImagesKeyPoints.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::KeyPoint> imagePairKeyPoints=mImagesKeyPoints[imagePairId];
                if(!mImagesKeyPointsDescriptors.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::Point2f> imageMatchingPoints=matchingPointsFirst[nImageInPairs];
                std::vector<cv::Point2f> imagePairMatchingPoints=matchingPointsSecond[nImageInPairs];
                std::vector<int> imagePairsMatchesIndexes=imagePairsMatchesIndexesFirst[nImageInPairs];
                std::vector<int> imagePairPairsMatchesIndexes=imagePairsMatchesIndexesSecond[nImageInPairs];
                std::vector<cv::Point2f> imageMatchingPointsReverse;
                std::vector<cv::Point2f> imagePairMatchingPointsReverse;
                std::vector<int> imagePairsMatchesIndexesReverse;
                std::vector<int> imagePairPairsMatchesIndexesReverse;
                if(bidirectionalMatches)
                {
                    imageMatchingPointsReverse=matchingPointsFirstReverse[nImageInPairs];
                    imagePairMatchingPointsReverse=matchingPointsSecondReverse[nImageInPairs];
                    imagePairsMatchesIndexesReverse=imagePairsMatchesIndexesFirstReverse[nImageInPairs];
                    imagePairPairsMatchesIndexesReverse=imagePairsMatchesIndexesSecondReverse[nImageInPairs];
                }
                if(imageMatchingPoints.size()!=imagePairMatchingPoints.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not equal number of matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                if(bidirectionalMatches)
                {
                    if(imageMatchingPointsReverse.size()!=imagePairMatchingPointsReverse.size())
                    {
                        strError=QObject::tr("OpenCVKpM::getOutliersDetector, not equal number of reverse matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                int numberOfMatches=(int)imageMatchingPoints.size();
                int numberOfMatchesReverse;
                int numberOfTotalMatches=numberOfMatches;
                if(bidirectionalMatches)
                {
                    numberOfMatchesReverse=(int)imageMatchingPointsReverse.size();
                    numberOfTotalMatches+=numberOfMatchesReverse;
                }
                std::vector<double> x1s,y1s,x2s,y2s;
                for( int kk = 0; kk < numberOfMatches; kk++ )
                {
                    cv::Point2f pt1 = imageMatchingPoints[kk];
                    cv::Point2f pt2 = imagePairMatchingPoints[kk];
                    double x1=pt1.x;
                    double y1=pt1.y;
                    double x2=pt2.x;
                    double y2=pt2.y;
                    x1s.push_back(x1);
                    y1s.push_back(y1);
                    x2s.push_back(x2);
                    y2s.push_back(y2);
                }
                if(bidirectionalMatches)
                {
                    for( int kk = 0; kk < numberOfMatchesReverse; kk++ )
                    {
                        cv::Point2f pt2 = imageMatchingPointsReverse[kk];
                        cv::Point2f pt1 = imagePairMatchingPointsReverse[kk];
                        double x1=pt1.x;
                        double y1=pt1.y;
                        double x2=pt2.x;
                        double y2=pt2.y;
                        x1s.push_back(x1);
                        y1s.push_back(y1);
                        x2s.push_back(x2);
                        y2s.push_back(y2);
                    }
                }
                int w1=4000;
                int h1=3000;
                int w2=4000;
                int h2=3000;
                double precision=0.0;
                std::vector<int> vec_inliers;
                if(!LibOrsa::getInliers(w1,h1,w2,h2,precision,x1s,y1s,x2s,y2s,vec_inliers,strError))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, orsa library error in image %1 and image pair: %2").arg(imageId).arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::Point2f> imageMatchingPointsWithoutOutliers;
                std::vector<cv::Point2f> imagePairMatchingPointsWithoutOutliers;
                std::vector<int> imagePairsMatchesIndexesWithoutOutliers;
                std::vector<int> imagePairPairsMatchesIndexesWithoutOutliers;
                std::vector<cv::DMatch> matches=mImagesPairsMatches[imageId][nImageInPairs];
                std::vector<cv::DMatch> matchesWithoutOutliers;

                std::vector<cv::Point2f> imageMatchingPointsWithoutOutliersReverse;
                std::vector<cv::Point2f> imagePairMatchingPointsWithoutOutliersReverse;
                std::vector<int> imagePairsMatchesIndexesWithoutOutliersReverse;
                std::vector<int> imagePairPairsMatchesIndexesWithoutOutliersReverse;
                std::vector<cv::DMatch> matchesReverse;
                std::vector<cv::DMatch> matchesWithoutOutliersReverse;

                if(bidirectionalMatches)
                {
                    matchesReverse=mImagesPairsMatchesReverse[imageId][nImageInPairs];
                }
                int numberOfInliers=0;
                int numberOfInliersReverse=0;
                for(int iterInliers=0;iterInliers<vec_inliers.size();iterInliers++)
                {
                    int nM=vec_inliers.at(iterInliers);
                    if(nM<numberOfMatches)
                    {
                        imageMatchingPointsWithoutOutliers.push_back(imageMatchingPoints[nM]);
                        imagePairMatchingPointsWithoutOutliers.push_back(imagePairMatchingPoints[nM]);
                        imagePairsMatchesIndexesWithoutOutliers.push_back(imagePairsMatchesIndexes[nM]);
                        imagePairPairsMatchesIndexesWithoutOutliers.push_back(imagePairPairsMatchesIndexes[nM]);
                        cv::DMatch match=matches[nM];
                        matchesWithoutOutliers.push_back(match);
                        numberOfInliers++;
                    }
                    else
                    {
                        if(bidirectionalMatches)// no debería ser en otro caso
                        {
                            nM=nM-numberOfMatches;
                            imageMatchingPointsWithoutOutliersReverse.push_back(imageMatchingPointsReverse[nM]);
                            imagePairMatchingPointsWithoutOutliersReverse.push_back(imagePairMatchingPointsReverse[nM]);
                            imagePairsMatchesIndexesWithoutOutliersReverse.push_back(imagePairsMatchesIndexesReverse[nM]);
                            imagePairPairsMatchesIndexesWithoutOutliersReverse.push_back(imagePairPairsMatchesIndexesReverse[nM]);
                            cv::DMatch match=matchesReverse[nM];
                            matchesWithoutOutliersReverse.push_back(match);
                            numberOfInliersReverse++;
                        }
                    }
                }
                mImagesPairsMatchesPointsFirsts[imageId][nImageInPairs]=imageMatchingPointsWithoutOutliers;
                mImagesPairsMatchesPointsSeconds[imageId][nImageInPairs]=imagePairMatchingPointsWithoutOutliers;
                mImagesPairsMatchesIndexesFirsts[imageId][nImageInPairs]=imagePairsMatchesIndexesWithoutOutliers;
                mImagesPairsMatchesIndexesSeconds[imageId][nImageInPairs]=imagePairPairsMatchesIndexesWithoutOutliers;
                mImagesPairsMatches[imageId][nImageInPairs]=matchesWithoutOutliers;
                if(numberOfInliers>0&&saveImagesMatches)
                {
                    QString imageFileName=mImageFileNames[imageId];
                    QString imagePairFileName=mImageFileNames[imagePairId];
                    QFileInfo fileInfoImage(imageFileName);
                    QString imageBaseName=fileInfoImage.baseName();
                    QFileInfo fileInfoImagePair(imagePairFileName);
                    QString imagePairBaseName=fileInfoImagePair.baseName();
                    QString absoluteFilePath=fileInfoImage.absolutePath();
                    QDir auxDir(absoluteFilePath);
                    QString imageMatchesDir=pathForMatchesFiles;
                    if(!auxDir.exists(imageMatchesDir))
                    {
                        if(!auxDir.mkdir(imageMatchesDir))
                        {
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, error creating directory: %1").arg(imageMatchesDir);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                    QString imageMatchesFileName=imageMatchesDir+"/";
                    imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                    if(QFile::exists(imageMatchesFileName))
                        QFile::remove(imageMatchesFileName);
                    std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                    cv::Mat imageMatches;
                    cv::Mat image=mImages[imageId];
                    cv::Mat imagePair=mImages[imagePairId];
                    try
                    {
                        cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matchesWithoutOutliers,imageMatches);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                    try
                    {
                        cv::imwrite(imageMatchesFileNameStdString,imageMatches);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                if(bidirectionalMatches)
                {
                    mImagesPairsMatchesPointsFirstsReverse[imageId][nImageInPairs]=imageMatchingPointsWithoutOutliersReverse;
                    mImagesPairsMatchesPointsSecondsReverse[imageId][nImageInPairs]=imagePairMatchingPointsWithoutOutliersReverse;
                    mImagesPairsMatchesIndexesFirstsReverse[imageId][nImageInPairs]=imagePairsMatchesIndexesWithoutOutliersReverse;
                    mImagesPairsMatchesIndexesSecondsReverse[imageId][nImageInPairs]=imagePairPairsMatchesIndexesWithoutOutliersReverse;
                    mImagesPairsMatchesReverse[imageId][nImageInPairs]=matchesWithoutOutliersReverse;
                    if(numberOfInliersReverse>0&&saveImagesMatches)
                    {
                        QString imageFileName=mImageFileNames[imageId];
                        QString imagePairFileName=mImageFileNames[imagePairId];
                        QFileInfo fileInfoImage(imageFileName);
                        QString imageBaseName=fileInfoImage.baseName();
                        QFileInfo fileInfoImagePair(imagePairFileName);
                        QString imagePairBaseName=fileInfoImagePair.baseName();
                        QString absoluteFilePath=fileInfoImage.absolutePath();
                        QDir auxDir(absoluteFilePath);
                        QString imageMatchesDir=pathForMatchesFiles;
                        if(!auxDir.exists(imageMatchesDir))
                        {
                            if(!auxDir.mkdir(imageMatchesDir))
                            {
                                strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, error creating directory: %1").arg(imageMatchesDir);
                                return(OPENCV_KPM_ERROR);
                            }
                        }
                        QString imageMatchesFileName=imageMatchesDir+"/";
//                        imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                        imageMatchesFileName+=(imagePairBaseName+"_"+imageBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                        if(QFile::exists(imageMatchesFileName))
                            QFile::remove(imageMatchesFileName);
                        std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                        cv::Mat imageMatchesReverse;
                        cv::Mat image=mImages[imageId];
                        cv::Mat imagePair=mImages[imagePairId];
                        try
                        {
//                            cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matchesWithoutOutliers,imageMatches);
                            cv::drawMatches( imagePair, imagePairKeyPoints,image, imageKeyPoints, matchesWithoutOutliersReverse,imageMatchesReverse);
                        }
                        catch( cv::Exception& e )
                        {
                            const char* err_msg = e.what();
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                            return(OPENCV_KPM_ERROR);
                        }
                        try
                        {
                            cv::imwrite(imageMatchesFileNameStdString,imageMatchesReverse);
                        }
                        catch( cv::Exception& e )
                        {
                            const char* err_msg = e.what();
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                }
//                meanAzimuth=meanAzimuth/maxAzimuthValues;
//                meanDistance=meanDistance/maxDistanceValues;
//                double stdMeanAzimuth=0.0;
//                double maxAzimuthDifference=0.0;
//                double stdMeanDistance=0.0;
//                double maxDistanceDifference=0.0;
//                for(int nM=0;nM<numberOfTotalMatches;nM++)
//                {
//                    int azimuthClusterIdx = azimuthLabels.at<int>(nM);
//                    if(azimuthClusterIdx==posAzimuthSolution)
//                    {
//                        double azimuthValue=azimuths.at<float>(nM,0);
//                        stdMeanAzimuth+=pow(meanAzimuth-azimuthValue,2.0);
//                        if(fabs(meanAzimuth-azimuthValue)>maxAzimuthDifference)
//                            maxAzimuthDifference=fabs(meanAzimuth-azimuthValue);
//                    }
//                    int distanceClusterIdx = distanceLabels.at<int>(nM);
//                    if(distanceClusterIdx==posDistanceSolution)
//                    {
//                        double distanceValue=distances.at<float>(nM,0);
//                        stdMeanDistance+=pow(meanDistance-distanceValue,2.0);
//                        if(fabs(meanDistance-distanceValue)>maxDistanceDifference)
//                            maxDistanceDifference=fabs(meanDistance-distanceValue);
//                    }
//                }
//                stdMeanAzimuth=sqrt(stdMeanAzimuth/maxAzimuthValues);
//                stdMeanDistance=sqrt(stdMeanDistance/maxDistanceValues);

//                if(printReport)
//                {
//                    QString prefix="          ";
//                    qout<<prefix<<"Image pair: "<<imageId<<" - "<<imagePairId<<endl;
//                    qout<<prefix<<"   Azimuth_value  Num_Values  Distance_value  Num_Values"<<endl;
//                    int maxClusterCount=azimuthNumberOfClusters;
//                    if(distanceNumberOfClusters>maxClusterCount)
//                        maxClusterCount=distanceNumberOfClusters;
//                    for(int nC=0;nC<maxClusterCount;nC++)
//                    {
//                        if(nC<numberOfAzimuthCluster)
//                        {
//                            double azimuthCenterValue=azimuthCenters.at<float>(nC);
//                            qout<<prefix<<QString::number(azimuthCenterValue,'f',4).rightJustified(16,' ');
//                            qout<<QString::number(azimuthClusterCount.at(nC)).rightJustified(12,' ');
//                        }
//                        else
//                            qout<<prefix<<"                ";
//                        if(nC<numberOfDistanceCluster)
//                        {
//                            double distanceCenterValue=distanceCenters.at<float>(nC);
//                            qout<<QString::number(distanceCenterValue,'f',2).rightJustified(16,' ');
//                            qout<<QString::number(distanceClusterCount.at(nC)).rightJustified(12,' ');
//                        }
//                        else
//                            qout<<"        --------";
//                        qout<<endl;
//                    }
//                    qout<<prefix<<"Solutions: "<<QString::number(coincidenceTotalValues.size())<<" coincidences"<<endl;
//                    qout<<prefix<<"- Azimuth (mean, std, maximum difference) (DEG): ("
//                       <<QString::number(meanAzimuth,'f',4)<<","
//                       <<QString::number(stdMeanAzimuth,'f',6)<<","<<QString::number(maxAzimuthDifference,'f',4)<<endl;
//                    qout<<prefix<<"- Distance (mean, std, maximum difference) (m): ("
//                       <<QString::number(meanDistance,'f',2)<<","
//                       <<QString::number(stdMeanDistance,'f',2)<<","<<QString::number(maxDistanceDifference,'f',2)<<endl;
//                }

            }
            iterImagePairs++;
        }
        findOutlierDetector=true;
    }

    //else //if(outlierDetectorTag.compare(OPENCV_KPM_OUTLIERSDETECTOR_DHL_KMEANS_TAG,Qt::CaseSensitive)==0)
    {
//        QString parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHNUMBEROFCLUSTERS_TAG;
//        int parameterPos=parametersTag.indexOf(parameterTag);
//        if(parameterPos==-1)
//        {
//            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
//            return(OPENCV_KPM_ERROR);
//        }
//        int azimuthNumberOfClusters=parametersValue[parameterPos].toInt();

//        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHEPSILONDEG_TAG;
//        parameterPos=parametersTag.indexOf(parameterTag);
//        if(parameterPos==-1)
//        {
//            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
//            return(OPENCV_KPM_ERROR);
//        }
//        double azimuthEpsilogDeg=parametersValue[parameterPos].toDouble();

        double maxDistanceToEpipolarLine=3.0;
        double confidenceLevel=0.99;
        int fundamentalMatrixMethod=cv::FM_RANSAC;

        QTextStream qout(stdout);
        QMap<QString,QVector<QString> >::const_iterator iterImagePairs = mImagesPairs.constBegin();
        double pi=4.0*atan(1.0);
        while(iterImagePairs!=mImagesPairs.constEnd())
        {
            QString imageId=iterImagePairs.key();
            if(!mImagesKeyPoints.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesKeyPointsDescriptors.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesPairsMatchesPointsFirsts.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points firsts for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesPairsMatchesPointsSeconds.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points seconds for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(bidirectionalMatches)
            {
                if(!mImagesPairsMatchesPointsFirstsReverse.contains(imageId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points firsts for image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
                if(!mImagesPairsMatchesPointsSecondsReverse.contains(imageId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points seconds for image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
            }
            std::vector<cv::KeyPoint> imageKeyPoints=mImagesKeyPoints[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsFirst=mImagesPairsMatchesPointsFirsts[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsSecond=mImagesPairsMatchesPointsSeconds[imageId];
            QVector<std::vector<int> > imagePairsMatchesIndexesFirst=mImagesPairsMatchesIndexesFirsts[imageId];
            QVector<std::vector<int> > imagePairsMatchesIndexesSecond=mImagesPairsMatchesIndexesSeconds[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsFirstReverse;
            QVector<std::vector<cv::Point2f> > matchingPointsSecondReverse;
            QVector<std::vector<int> > imagePairsMatchesIndexesFirstReverse;
            QVector<std::vector<int> > imagePairsMatchesIndexesSecondReverse;
            if(bidirectionalMatches)
            {
                matchingPointsFirstReverse=mImagesPairsMatchesPointsFirstsReverse[imageId];
                matchingPointsSecondReverse=mImagesPairsMatchesPointsSecondsReverse[imageId];
                imagePairsMatchesIndexesFirstReverse=mImagesPairsMatchesIndexesFirstsReverse[imageId];
                imagePairsMatchesIndexesSecondReverse=mImagesPairsMatchesIndexesSecondsReverse[imageId];
            }
            QVector<QString> imagePairs=iterImagePairs.value();
            if(imagePairs.size()!=matchingPointsFirst.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(imagePairs.size()!=matchingPointsSecond.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(bidirectionalMatches)
            {
                if(imagePairs.size()!=matchingPointsFirstReverse.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
                if(imagePairs.size()!=matchingPointsSecondReverse.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
            }
            for(int nImageInPairs=0;nImageInPairs<imagePairs.size();nImageInPairs++)
            {
                QString imagePairId=imagePairs.at(nImageInPairs);
                if(!mImagesKeyPoints.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::KeyPoint> imagePairKeyPoints=mImagesKeyPoints[imagePairId];
                if(!mImagesKeyPointsDescriptors.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::Point2f> imageMatchingPoints=matchingPointsFirst[nImageInPairs];
                std::vector<cv::Point2f> imagePairMatchingPoints=matchingPointsSecond[nImageInPairs];
                std::vector<int> imagePairsMatchesIndexes=imagePairsMatchesIndexesFirst[nImageInPairs];
                std::vector<int> imagePairPairsMatchesIndexes=imagePairsMatchesIndexesSecond[nImageInPairs];
                std::vector<cv::Point2f> imageMatchingPointsReverse;
                std::vector<cv::Point2f> imagePairMatchingPointsReverse;
                std::vector<int> imagePairsMatchesIndexesReverse;
                std::vector<int> imagePairPairsMatchesIndexesReverse;
                if(bidirectionalMatches)
                {
                    imageMatchingPointsReverse=matchingPointsFirstReverse[nImageInPairs];
                    imagePairMatchingPointsReverse=matchingPointsSecondReverse[nImageInPairs];
                    imagePairsMatchesIndexesReverse=imagePairsMatchesIndexesFirstReverse[nImageInPairs];
                    imagePairPairsMatchesIndexesReverse=imagePairsMatchesIndexesSecondReverse[nImageInPairs];
                }
                if(imageMatchingPoints.size()!=imagePairMatchingPoints.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not equal number of matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                if(bidirectionalMatches)
                {
                    if(imageMatchingPointsReverse.size()!=imagePairMatchingPointsReverse.size())
                    {
                        strError=QObject::tr("OpenCVKpM::getOutliersDetector, not equal number of reverse matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                int numberOfMatches=(int)imageMatchingPoints.size();
                int numberOfMatchesReverse;
                int numberOfTotalMatches=numberOfMatches;
                if(bidirectionalMatches)
                {
                    numberOfMatchesReverse=(int)imageMatchingPointsReverse.size();
                    numberOfTotalMatches+=numberOfMatchesReverse;
                }
                std::vector<cv::Point2f> points1,points2;
                for(int nM=0;nM<numberOfMatches;nM++)
                {
                    cv::Point2f point1=imageMatchingPoints.at(nM);
                    cv::Point2f point2=imagePairMatchingPoints.at(nM);
                    points1.push_back(point1);
                    points2.push_back(point2);
                }
                if(bidirectionalMatches)
                {
                    for(int nM=0;nM<numberOfMatchesReverse;nM++)
                    {
                        cv::Point2f point2=imageMatchingPointsReverse.at(nM);
                        cv::Point2f point1=imagePairMatchingPointsReverse.at(nM);
                        points1.push_back(point1);
                        points2.push_back(point2);
                    }
                }
                std::vector<uchar> inliers(numberOfMatches,0);
                double f11,f12,f13,f21,f22,f23,f31,f32,f33;
                try
                {
                    cv::Mat F = cv::findFundamentalMat(points1,
                                                       points2,
                                                       inliers,
                                                       fundamentalMatrixMethod,
                                                       maxDistanceToEpipolarLine,
                                                       confidenceLevel);
                    f11=F.at<double>(0,0);
                    f12=F.at<double>(0,1);
                    f13=F.at<double>(0,2);
                    f21=F.at<double>(1,0);
                    f22=F.at<double>(1,1);
                    f23=F.at<double>(1,2);
                    f31=F.at<double>(2,0);
                    f32=F.at<double>(2,1);
                    f33=F.at<double>(2,2);
                }
                catch( cv::Exception& e )
                {
                    const char* err_msg = e.what();
                    strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                    return(OPENCV_KPM_ERROR);
                }
                QVector<int> coincidenceTotalValues;
                QVector<int> coincidenceValues;
                std::vector<cv::Point2f> imageMatchingPointsWithoutOutliers;
                std::vector<cv::Point2f> imagePairMatchingPointsWithoutOutliers;
                std::vector<int> imagePairsMatchesIndexesWithoutOutliers;
                std::vector<int> imagePairPairsMatchesIndexesWithoutOutliers;
                std::vector<cv::DMatch> matches=mImagesPairsMatches[imageId][nImageInPairs];
                std::vector<cv::DMatch> matchesWithoutOutliers;
                for(int nM=0;nM<numberOfMatches;nM++)
                {
                    int inlier=inliers.at(nM);
                    cv::Point2f point1=points1.at(nM);
                    double x1=point1.x;
                    double y1=point1.y;
                    cv::Point2f point2=points2.at(nM);
                    double x2=point2.x;
                    double y2=point2.y;
                    double aux1=f11*x2+f12*y2+f13*1.0;
                    double aux2=f21*x2+f22*y2+f23*1.0;
                    double aux3=f31*x2+f32*y2+f33*1.0;
                    double value=x1*aux1+y1*aux2+1.0*aux3;
                    if(fabs(value)<maxDistanceToEpipolarLine)
                        inlier=1;
                    else
                        inlier=0;
                    if(inlier==1)
                    {
                        coincidenceValues.push_back(nM);
                        coincidenceTotalValues.push_back(nM);
                        imageMatchingPointsWithoutOutliers.push_back(imageMatchingPoints[nM]);
                        imagePairMatchingPointsWithoutOutliers.push_back(imagePairMatchingPoints[nM]);
                        imagePairsMatchesIndexesWithoutOutliers.push_back(imagePairsMatchesIndexes[nM]);
                        imagePairPairsMatchesIndexesWithoutOutliers.push_back(imagePairPairsMatchesIndexes[nM]);
                        cv::DMatch match=matches[nM];
                        matchesWithoutOutliers.push_back(match);
                    }
                }
                mImagesPairsMatchesPointsFirsts[imageId][nImageInPairs]=imageMatchingPointsWithoutOutliers;
                mImagesPairsMatchesPointsSeconds[imageId][nImageInPairs]=imagePairMatchingPointsWithoutOutliers;
                mImagesPairsMatchesIndexesFirsts[imageId][nImageInPairs]=imagePairsMatchesIndexesWithoutOutliers;
                mImagesPairsMatchesIndexesSeconds[imageId][nImageInPairs]=imagePairPairsMatchesIndexesWithoutOutliers;
                mImagesPairsMatches[imageId][nImageInPairs]=matchesWithoutOutliers;
                if(coincidenceValues.size()>0&&saveImagesMatches)
                {
                    QString imageFileName=mImageFileNames[imageId];
                    QString imagePairFileName=mImageFileNames[imagePairId];
                    QFileInfo fileInfoImage(imageFileName);
                    QString imageBaseName=fileInfoImage.baseName();
                    QFileInfo fileInfoImagePair(imagePairFileName);
                    QString imagePairBaseName=fileInfoImagePair.baseName();
                    QString absoluteFilePath=fileInfoImage.absolutePath();
                    QDir auxDir(absoluteFilePath);
                    QString imageMatchesDir=pathForMatchesFiles;
                    if(!auxDir.exists(imageMatchesDir))
                    {
                        if(!auxDir.mkdir(imageMatchesDir))
                        {
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, error creating directory: %1").arg(imageMatchesDir);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                    QString imageMatchesFileName=imageMatchesDir+"/";
                    imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                    if(QFile::exists(imageMatchesFileName))
                        QFile::remove(imageMatchesFileName);
                    std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                    cv::Mat imageMatches;
                    cv::Mat image=mImages[imageId];
                    cv::Mat imagePair=mImages[imagePairId];
                    try
                    {
                        cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matchesWithoutOutliers,imageMatches);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                    try
                    {
                        cv::imwrite(imageMatchesFileNameStdString,imageMatches);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                if(bidirectionalMatches)
                {

                    QVector<int> coincidenceValuesReverse;
                    std::vector<cv::Point2f> imageMatchingPointsWithoutOutliersReverse;
                    std::vector<cv::Point2f> imagePairMatchingPointsWithoutOutliersReverse;
                    std::vector<int> imagePairsMatchesIndexesWithoutOutliersReverse;
                    std::vector<int> imagePairPairsMatchesIndexesWithoutOutliersReverse;
                    std::vector<cv::DMatch> matchesReverse=mImagesPairsMatchesReverse[imageId][nImageInPairs];
                    std::vector<cv::DMatch> matchesWithoutOutliersReverse;
                    for(int nM=0;nM<numberOfMatchesReverse;nM++)
                    {
                        int inlier=inliers.at(nM+numberOfMatches);
                        cv::Point2f point1=points1.at(nM+numberOfMatches);
                        double x1=point1.x;
                        double y1=point1.y;
                        cv::Point2f point2=points2.at(nM+numberOfMatches);
                        double x2=point2.x;
                        double y2=point2.y;
                        double aux1=f11*x2+f12*y2+f13*1.0;
                        double aux2=f21*x2+f22*y2+f23*1.0;
                        double aux3=f31*x2+f32*y2+f33*1.0;
                        double value=x1*aux1+y1*aux2+1.0*aux3;
                        if(fabs(value)<maxDistanceToEpipolarLine)
                            inlier=1;
                        else
                            inlier=0;
                        if(inlier==1)
                        {
                            coincidenceValuesReverse.push_back(nM);
                            coincidenceTotalValues.push_back(nM+numberOfMatches);
                            imageMatchingPointsWithoutOutliersReverse.push_back(imageMatchingPointsReverse[nM]);
                            imagePairMatchingPointsWithoutOutliersReverse.push_back(imagePairMatchingPointsReverse[nM]);
                            imagePairsMatchesIndexesWithoutOutliersReverse.push_back(imagePairsMatchesIndexesReverse[nM]);
                            imagePairPairsMatchesIndexesWithoutOutliersReverse.push_back(imagePairPairsMatchesIndexesReverse[nM]);
                            cv::DMatch match=matchesReverse[nM];
                            matchesWithoutOutliersReverse.push_back(match);
                        }
                    }
                    mImagesPairsMatchesPointsFirstsReverse[imageId][nImageInPairs]=imageMatchingPointsWithoutOutliersReverse;
                    mImagesPairsMatchesPointsSecondsReverse[imageId][nImageInPairs]=imagePairMatchingPointsWithoutOutliersReverse;
                    mImagesPairsMatchesIndexesFirstsReverse[imageId][nImageInPairs]=imagePairsMatchesIndexesWithoutOutliersReverse;
                    mImagesPairsMatchesIndexesSecondsReverse[imageId][nImageInPairs]=imagePairPairsMatchesIndexesWithoutOutliersReverse;
                    mImagesPairsMatchesReverse[imageId][nImageInPairs]=matchesWithoutOutliersReverse;
                    if(coincidenceValuesReverse.size()>0&&saveImagesMatches)
                    {
                        QString imageFileName=mImageFileNames[imageId];
                        QString imagePairFileName=mImageFileNames[imagePairId];
                        QFileInfo fileInfoImage(imageFileName);
                        QString imageBaseName=fileInfoImage.baseName();
                        QFileInfo fileInfoImagePair(imagePairFileName);
                        QString imagePairBaseName=fileInfoImagePair.baseName();
                        QString absoluteFilePath=fileInfoImage.absolutePath();
                        QDir auxDir(absoluteFilePath);
                        QString imageMatchesDir=pathForMatchesFiles;
                        if(!auxDir.exists(imageMatchesDir))
                        {
                            if(!auxDir.mkdir(imageMatchesDir))
                            {
                                strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, error creating directory: %1").arg(imageMatchesDir);
                                return(OPENCV_KPM_ERROR);
                            }
                        }
                        QString imageMatchesFileName=imageMatchesDir+"/";
//                        imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                        imageMatchesFileName+=(imagePairBaseName+"_"+imageBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                        if(QFile::exists(imageMatchesFileName))
                            QFile::remove(imageMatchesFileName);
                        std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                        cv::Mat imageMatchesReverse;
                        cv::Mat image=mImages[imageId];
                        cv::Mat imagePair=mImages[imagePairId];
                        try
                        {
//                            cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matchesWithoutOutliers,imageMatches);
                            cv::drawMatches( imagePair, imagePairKeyPoints,image, imageKeyPoints, matchesWithoutOutliersReverse,imageMatchesReverse);
                        }
                        catch( cv::Exception& e )
                        {
                            const char* err_msg = e.what();
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                            return(OPENCV_KPM_ERROR);
                        }
                        try
                        {
                            cv::imwrite(imageMatchesFileNameStdString,imageMatchesReverse);
                        }
                        catch( cv::Exception& e )
                        {
                            const char* err_msg = e.what();
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                }
            }
            iterImagePairs++;
        }
        findOutlierDetector=true;
    }
    if(!findOutlierDetector)
    {
        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, outlier detector: %1 is not implemented").arg(outlierDetectorTag);
        return(OPENCV_KPM_ERROR);
    }
    return(OPENCV_KPM_NO_ERROR);
}
*/

int OpenCVKpM::detectOutliersInMatches(bool &printReport,
                                       QString &strError)
{
#ifdef USE_OPENCV
    QString outlierDetectorTag;
    QStringList parametersTag;
    QStringList parametersValue;
    QStringList parametersType;
    if(OPENCV_KPM_NO_ERROR!=mParameters.getOutliersDetector(outlierDetectorTag,
                                                            parametersTag,
                                                            parametersValue,
                                                            parametersType,
                                                            strError))
    {
        strError="OpenCVKpM::detectOutliersInMatches,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    bool findOutlierDetector=false;
    QString pathForMatchesFiles;
    bool saveImagesMatches=mParameters.getSaveImagesMatches(pathForMatchesFiles);
    bool bidirectionalMatches=mParameters.getBidirectionalMatches();
    //outlierDetectorTag="";

    if(outlierDetectorTag.compare(OPENCV_KPM_OUTLIERSDETECTOR_DHL_KMEANS_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHNUMBEROFCLUSTERS_TAG;
        int parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int azimuthNumberOfClusters=parametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHEPSILONDEG_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double azimuthEpsilogDeg=parametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHITERATIONS_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int azimuthIterations=parametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_DISTANCENUMBEROFCLUSTERS_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int distanceNumberOfClusters=parametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_DISTANCEEPSILON_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double distanceEpsilon=parametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_DISTANCEITERATIONS_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int distanceIterations=parametersValue[parameterPos].toInt();

        QTextStream qout(stdout);
        QMap<QString,QVector<QString> >::const_iterator iterImagePairs = mImagesPairs.constBegin();
        double pi=4.0*atan(1.0);
        while(iterImagePairs!=mImagesPairs.constEnd())
        {
            QString imageId=iterImagePairs.key();
            if(!mImagesKeyPoints.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesKeyPointsDescriptors.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesPairsMatchesPointsFirsts.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points firsts for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesPairsMatchesPointsSeconds.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points seconds for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(bidirectionalMatches)
            {
                if(!mImagesPairsMatchesPointsFirstsReverse.contains(imageId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points firsts for image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
                if(!mImagesPairsMatchesPointsSecondsReverse.contains(imageId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points seconds for image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
            }
            std::vector<cv::KeyPoint> imageKeyPoints=mImagesKeyPoints[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsFirst=mImagesPairsMatchesPointsFirsts[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsSecond=mImagesPairsMatchesPointsSeconds[imageId];
            QVector<std::vector<int> > imagePairsMatchesIndexesFirst=mImagesPairsMatchesIndexesFirsts[imageId];
            QVector<std::vector<int> > imagePairsMatchesIndexesSecond=mImagesPairsMatchesIndexesSeconds[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsFirstReverse;
            QVector<std::vector<cv::Point2f> > matchingPointsSecondReverse;
            QVector<std::vector<int> > imagePairsMatchesIndexesFirstReverse;
            QVector<std::vector<int> > imagePairsMatchesIndexesSecondReverse;
            if(bidirectionalMatches)
            {
                matchingPointsFirstReverse=mImagesPairsMatchesPointsFirstsReverse[imageId];
                matchingPointsSecondReverse=mImagesPairsMatchesPointsSecondsReverse[imageId];
                imagePairsMatchesIndexesFirstReverse=mImagesPairsMatchesIndexesFirstsReverse[imageId];
                imagePairsMatchesIndexesSecondReverse=mImagesPairsMatchesIndexesSecondsReverse[imageId];
            }
            QVector<QString> imagePairs=iterImagePairs.value();
            if(imagePairs.size()!=matchingPointsFirst.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(imagePairs.size()!=matchingPointsSecond.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(bidirectionalMatches)
            {
                if(imagePairs.size()!=matchingPointsFirstReverse.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
                if(imagePairs.size()!=matchingPointsSecondReverse.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
            }
            for(int nImageInPairs=0;nImageInPairs<imagePairs.size();nImageInPairs++)
            {
                QString imagePairId=imagePairs.at(nImageInPairs);
                if(!mImagesKeyPoints.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::KeyPoint> imagePairKeyPoints=mImagesKeyPoints[imagePairId];
                if(!mImagesKeyPointsDescriptors.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::Point2f> imageMatchingPoints=matchingPointsFirst[nImageInPairs];
                std::vector<cv::Point2f> imagePairMatchingPoints=matchingPointsSecond[nImageInPairs];
                std::vector<int> imagePairsMatchesIndexes=imagePairsMatchesIndexesFirst[nImageInPairs];
                std::vector<int> imagePairPairsMatchesIndexes=imagePairsMatchesIndexesSecond[nImageInPairs];
                std::vector<cv::Point2f> imageMatchingPointsReverse;
                std::vector<cv::Point2f> imagePairMatchingPointsReverse;
                std::vector<int> imagePairsMatchesIndexesReverse;
                std::vector<int> imagePairPairsMatchesIndexesReverse;
                if(bidirectionalMatches)
                {
                    imageMatchingPointsReverse=matchingPointsFirstReverse[nImageInPairs];
                    imagePairMatchingPointsReverse=matchingPointsSecondReverse[nImageInPairs];
                    imagePairsMatchesIndexesReverse=imagePairsMatchesIndexesFirstReverse[nImageInPairs];
                    imagePairPairsMatchesIndexesReverse=imagePairsMatchesIndexesSecondReverse[nImageInPairs];
                }
                if(imageMatchingPoints.size()!=imagePairMatchingPoints.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not equal number of matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                if(bidirectionalMatches)
                {
                    if(imageMatchingPointsReverse.size()!=imagePairMatchingPointsReverse.size())
                    {
                        strError=QObject::tr("OpenCVKpM::getOutliersDetector, not equal number of reverse matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                int numberOfMatches=(int)imageMatchingPoints.size();
                int numberOfMatchesReverse;
                int numberOfTotalMatches=numberOfMatches;
                if(bidirectionalMatches)
                {
                    numberOfMatchesReverse=(int)imageMatchingPointsReverse.size();
                    numberOfTotalMatches+=numberOfMatchesReverse;
                }
                cv::Mat azimuths=cv::Mat::zeros(numberOfTotalMatches,1, CV_32F);
                cv::Mat distances=cv::Mat::zeros(numberOfTotalMatches,1, CV_32F);
                cv::Mat azimuthLabels,distanceLabels;
                for( int kk = 0; kk < numberOfMatches; kk++ )
                {
                    cv::Point2f pt1 = imageMatchingPoints[kk];
                    cv::Point2f pt2 = imagePairMatchingPoints[kk];
                    double x1=pt1.x;
                    double y1=pt1.y;
                    double x2=pt2.x;
                    double y2=pt2.y;
                    double azi=azimuth(x1,y1,x2,y2)*180.0/pi;
                    double dis2d=distance(x1,y1,x2,y2);
                    azimuths.at<float>(kk,0)=azi;
                    distances.at<float>(kk,0)=dis2d;
                }
                if(bidirectionalMatches)
                {
                    for( int kk = 0; kk < numberOfMatchesReverse; kk++ )
                    {
                        cv::Point2f pt2 = imageMatchingPointsReverse[kk];
                        cv::Point2f pt1 = imagePairMatchingPointsReverse[kk];
                        double x1=pt1.x;
                        double y1=pt1.y;
                        double x2=pt2.x;
                        double y2=pt2.y;
                        double azi=azimuth(x1,y1,x2,y2)*180.0/pi;
                        double dis2d=distance(x1,y1,x2,y2);
                        azimuths.at<float>(numberOfMatches+kk,0)=azi;
                        distances.at<float>(numberOfMatches+kk,0)=dis2d;
                    }
                }
                cv::Mat azimuthCenters(azimuthNumberOfClusters, 1, CV_32F);
                try
                {
                    kmeans(azimuths, azimuthNumberOfClusters, azimuthLabels,
                           //TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 100, 5.0),
                           cv::TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_EPS,azimuthIterations,azimuthEpsilogDeg),
                           3, cv::KMEANS_PP_CENTERS, azimuthCenters);
                }
                catch( cv::Exception& e )
                {
                    const char* err_msg = e.what();
                    strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                    return(OPENCV_KPM_ERROR);
                }
                int numberOfAzimuthCluster=azimuthCenters.rows;

                cv::Mat distanceCenters(distanceNumberOfClusters, 1, CV_32F);
                try
                {
                    kmeans(distances, distanceNumberOfClusters,distanceLabels,
                           //TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 100, 20.0),
                           cv::TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_EPS, distanceIterations, distanceEpsilon),
                           3, cv::KMEANS_PP_CENTERS, distanceCenters);
                }
                catch( cv::Exception& e )
                {
                    const char* err_msg = e.what();
                    strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                    return(OPENCV_KPM_ERROR);
                }
                int numberOfDistanceCluster=distanceCenters.rows;
                QVector<int> azimuthClusterCount;
                for(int nca=0;nca<numberOfAzimuthCluster;nca++)
                    azimuthClusterCount.push_back(0);

                QVector<int> distanceClusterCount;
                for(int ncd=0;ncd<numberOfDistanceCluster;ncd++)
                    distanceClusterCount.push_back(0);

                for(int nM=0;nM<numberOfTotalMatches;nM++)
                {
                    int azimuthClusterIdx = azimuthLabels.at<int>(nM);
                    azimuthClusterCount[azimuthClusterIdx]++;
                    int distanceClusterIdx = distanceLabels.at<int>(nM);
                    distanceClusterCount[distanceClusterIdx]++;
                }
                int posAzimuthSolution=-1;
                int maxAzimuthValues=0;
                for(int nca=0;nca<numberOfAzimuthCluster;nca++)
                {
                    int numberOfValues=azimuthClusterCount.at(nca);
                    if(numberOfValues>maxAzimuthValues)
                    {
                        posAzimuthSolution=nca;
                        maxAzimuthValues=numberOfValues;
                    }
                }
                int posDistanceSolution=-1;
                int maxDistanceValues=0;
                for(int ncd=0;ncd<numberOfDistanceCluster;ncd++)
                {
                    int numberOfValues=distanceClusterCount.at(ncd);
                    if(numberOfValues>maxDistanceValues)
                    {
                        posDistanceSolution=ncd;
                        maxDistanceValues=numberOfValues;
                    }
                }
                double meanAzimuth=0.0;
                double meanDistance=0.0;
                double previousMeanAzimuth=0.0;
                double previousMeanDistance=0.0;
                bool isFirstMeanAzimuth=true;
                bool isFirstMeanDistance=true;
                std::vector<cv::Point2f> imageMatchingPointsWithoutOutliers;
                std::vector<cv::Point2f> imagePairMatchingPointsWithoutOutliers;
                std::vector<int> imagePairsMatchesIndexesWithoutOutliers;
                std::vector<int> imagePairPairsMatchesIndexesWithoutOutliers;
                std::vector<cv::DMatch> matches=mImagesPairsMatches[imageId][nImageInPairs];
                std::vector<cv::DMatch> matchesWithoutOutliers;
                bool meanControl=0;
                double stdMeanAzimuth=360.0;
                double maxAzimuthDifference=0.0;
                double stdMeanDistance=10000.0;
                double maxDistanceDifference=0.0;
                while(meanControl==0)
                {
                    meanAzimuth=0.0;
                    meanDistance=0.0;
                    int numberOfAzimuthsInMean=0;
                    int numberOfDistanceInMean=0;
                    for(int nM=0;nM<numberOfMatches;nM++)
                    {
                        int azimuthClusterIdx = azimuthLabels.at<int>(nM);
                        if(azimuthClusterIdx==posAzimuthSolution)
                        {
                            double azimuth=azimuths.at<float>(nM,0);
                            double diffAzimuth=fabs(azimuth-previousMeanAzimuth);
                            if(diffAzimuth<azimuthEpsilogDeg
                                    ||isFirstMeanAzimuth)
                            {
                                meanAzimuth+=azimuth;
                                numberOfAzimuthsInMean++;
                            }
                        }
                        int distanceClusterIdx = distanceLabels.at<int>(nM);
                        if(distanceClusterIdx==posDistanceSolution)
                        {
                            double distance=distances.at<float>(nM,0);
                            double diffDistance=fabs(distance-previousMeanDistance);
                            if(diffDistance<distanceEpsilon
                                    ||isFirstMeanDistance)
                            {
                                meanDistance+=distance;
                                numberOfDistanceInMean++;
                            }
                        }
                    }
                    if(bidirectionalMatches)
                    {
                        for(int nM=0;nM<numberOfMatchesReverse;nM++)
                        {
                            int azimuthClusterIdx = azimuthLabels.at<int>(nM+numberOfMatches);
                            if(azimuthClusterIdx==posAzimuthSolution)
                            {
                                double azimuth=azimuths.at<float>(nM+numberOfMatches,0);
                                double diffAzimuth=fabs(azimuth-previousMeanAzimuth);
                                if(diffAzimuth<azimuthEpsilogDeg
                                        ||isFirstMeanAzimuth)
                                {
                                    meanAzimuth+=azimuth;
                                    numberOfAzimuthsInMean++;
                                }
                            }
                            int distanceClusterIdx = distanceLabels.at<int>(nM+numberOfMatches);
                            if(distanceClusterIdx==posDistanceSolution)
                            {
                                double distance=distances.at<float>(nM+numberOfMatches,0);
                                double diffDistance=fabs(distance-previousMeanDistance);
                                if(diffDistance<distanceEpsilon
                                        ||isFirstMeanDistance)
                                {
                                    meanDistance+=distance;
                                    numberOfDistanceInMean++;
                                }
                            }
                        }
                    }
                    meanAzimuth=meanAzimuth/numberOfAzimuthsInMean;
                    meanDistance=meanDistance/numberOfDistanceInMean;
                    stdMeanAzimuth=0.0;
                    maxAzimuthDifference=0.0;
                    stdMeanDistance=0.0;
                    maxDistanceDifference=0.0;
                    for(int nM=0;nM<numberOfTotalMatches;nM++)
                    {
                        int azimuthClusterIdx = azimuthLabels.at<int>(nM);
                        if(azimuthClusterIdx==posAzimuthSolution)
                        {
                            double azimuthValue=azimuths.at<float>(nM,0);
                            double diffAzimuth=fabs(azimuthValue-previousMeanAzimuth);
                            if(diffAzimuth<azimuthEpsilogDeg
                                    ||isFirstMeanAzimuth)
                            {
                                stdMeanAzimuth+=pow(meanAzimuth-azimuthValue,2.0);
                                if(fabs(meanAzimuth-azimuthValue)>maxAzimuthDifference)
                                    maxAzimuthDifference=fabs(meanAzimuth-azimuthValue);
                            }
                        }
                        int distanceClusterIdx = distanceLabels.at<int>(nM);
                        if(distanceClusterIdx==posDistanceSolution)
                        {
                            double distanceValue=distances.at<float>(nM,0);
                            double diffDistance=fabs(distanceValue-previousMeanDistance);
                            if(diffDistance<distanceEpsilon
                                    ||isFirstMeanDistance)
                            {
                                stdMeanDistance+=pow(meanDistance-distanceValue,2.0);
                                if(fabs(meanDistance-distanceValue)>maxDistanceDifference)
                                    maxDistanceDifference=fabs(meanDistance-distanceValue);
                            }
                        }
                    }
                    stdMeanAzimuth=sqrt(stdMeanAzimuth/numberOfAzimuthsInMean);
                    stdMeanDistance=sqrt(stdMeanDistance/numberOfDistanceInMean);
                    if(stdMeanAzimuth<azimuthEpsilogDeg
                            &&stdMeanDistance<distanceEpsilon)
                    {
                        meanControl=1;
                    }
                    else
                    {
                        previousMeanAzimuth=meanAzimuth;
                        previousMeanDistance=meanDistance;
                        isFirstMeanAzimuth=false;
                        isFirstMeanDistance=false;
                    }
                }

                QVector<int> coincidenceTotalValues;
                QVector<int> coincidenceValues;
                for(int nM=0;nM<numberOfMatches;nM++)
                {
                    int azimuthClusterIdx = azimuthLabels.at<int>(nM);
                    double azimuthValue=azimuths.at<float>(nM,0);
                    double diffAzimuth=azimuthValue-meanAzimuth;
                    int distanceClusterIdx = distanceLabels.at<int>(nM);
                    double distanceValue=distances.at<float>(nM,0);
                    double diffDistance=distanceValue-meanDistance;
                    if(azimuthClusterIdx==posAzimuthSolution
                            &&distanceClusterIdx==posDistanceSolution)
                    {
                        if(diffAzimuth<azimuthEpsilogDeg
                                &&diffDistance<distanceEpsilon)
                        {
                            coincidenceValues.push_back(nM);
                            coincidenceTotalValues.push_back(nM);
                            imageMatchingPointsWithoutOutliers.push_back(imageMatchingPoints[nM]);
                            imagePairMatchingPointsWithoutOutliers.push_back(imagePairMatchingPoints[nM]);
                            imagePairsMatchesIndexesWithoutOutliers.push_back(imagePairsMatchesIndexes[nM]);
                            imagePairPairsMatchesIndexesWithoutOutliers.push_back(imagePairPairsMatchesIndexes[nM]);
                            cv::DMatch match=matches[nM];
                            matchesWithoutOutliers.push_back(match);
                        }
                    }
                }

                mImagesPairsMatchesPointsFirsts[imageId][nImageInPairs]=imageMatchingPointsWithoutOutliers;
                mImagesPairsMatchesPointsSeconds[imageId][nImageInPairs]=imagePairMatchingPointsWithoutOutliers;
                mImagesPairsMatchesIndexesFirsts[imageId][nImageInPairs]=imagePairsMatchesIndexesWithoutOutliers;
                mImagesPairsMatchesIndexesSeconds[imageId][nImageInPairs]=imagePairPairsMatchesIndexesWithoutOutliers;
                mImagesPairsMatches[imageId][nImageInPairs]=matchesWithoutOutliers;
//                if(coincidenceValues.size()>0&&saveImagesMatches)
//                {
//                    QString imageFileName=mImageFileNames[imageId];
//                    QString imagePairFileName=mImageFileNames[imagePairId];
//                    QFileInfo fileInfoImage(imageFileName);
//                    QString imageBaseName=fileInfoImage.baseName();
//                    QFileInfo fileInfoImagePair(imagePairFileName);
//                    QString imagePairBaseName=fileInfoImagePair.baseName();
//                    QString absoluteFilePath=fileInfoImage.absolutePath();
//                    QDir auxDir(absoluteFilePath);
//                    QString imageMatchesDir=pathForMatchesFiles;
//                    if(!auxDir.exists(imageMatchesDir))
//                    {
//                        if(!auxDir.mkdir(imageMatchesDir))
//                        {
//                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, error creating directory: %1").arg(imageMatchesDir);
//                            return(OPENCV_KPM_ERROR);
//                        }
//                    }
//                    QString imageMatchesFileName=imageMatchesDir+"/";
//                    imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
//                    if(QFile::exists(imageMatchesFileName))
//                        QFile::remove(imageMatchesFileName);
//                    std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
//                    cv::Mat imageMatches;
//                    cv::Mat image=mImages[imageId];
//                    cv::Mat imagePair=mImages[imagePairId];
//                    try
//                    {
//                        cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matchesWithoutOutliers,imageMatches);
//                    }
//                    catch( cv::Exception& e )
//                    {
//                        const char* err_msg = e.what();
//                        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
//                        return(OPENCV_KPM_ERROR);
//                    }
//                    try
//                    {
//                        cv::imwrite(imageMatchesFileNameStdString,imageMatches);
//                    }
//                    catch( cv::Exception& e )
//                    {
//                        const char* err_msg = e.what();
//                        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
//                        return(OPENCV_KPM_ERROR);
//                    }
//                }
                if(bidirectionalMatches)
                {
                    QVector<int> coincidenceValuesReverse;
                    std::vector<cv::Point2f> imageMatchingPointsWithoutOutliersReverse;
                    std::vector<cv::Point2f> imagePairMatchingPointsWithoutOutliersReverse;
                    std::vector<int> imagePairsMatchesIndexesWithoutOutliersReverse;
                    std::vector<int> imagePairPairsMatchesIndexesWithoutOutliersReverse;
                    std::vector<cv::DMatch> matchesReverse=mImagesPairsMatchesReverse[imageId][nImageInPairs];
                    std::vector<cv::DMatch> matchesWithoutOutliersReverse;
                    for(int nM=0;nM<numberOfMatchesReverse;nM++)
                    {
                        int azimuthClusterIdx = azimuthLabels.at<int>(nM+numberOfMatches);
                        double azimuthValue=azimuths.at<float>(nM+numberOfMatches,0);
                        double diffAzimuth=azimuthValue-meanAzimuth;
                        int distanceClusterIdx = distanceLabels.at<int>(nM+numberOfMatches);
                        double distanceValue=distances.at<float>(nM+numberOfMatches,0);
                        double diffDistance=distanceValue-meanDistance;
                        if(azimuthClusterIdx==posAzimuthSolution
                                &&distanceClusterIdx==posDistanceSolution)
                        {
                            if(diffAzimuth<azimuthEpsilogDeg
                                    &&diffDistance<distanceEpsilon)
                            {
                                coincidenceValuesReverse.push_back(nM);
                                coincidenceTotalValues.push_back(nM+numberOfMatches);
                                imageMatchingPointsWithoutOutliersReverse.push_back(imageMatchingPointsReverse[nM]);
                                imagePairMatchingPointsWithoutOutliersReverse.push_back(imagePairMatchingPointsReverse[nM]);
                                imagePairsMatchesIndexesWithoutOutliersReverse.push_back(imagePairsMatchesIndexesReverse[nM]);
                                imagePairPairsMatchesIndexesWithoutOutliersReverse.push_back(imagePairPairsMatchesIndexesReverse[nM]);
                                cv::DMatch match=matchesReverse[nM];
                                matchesWithoutOutliersReverse.push_back(match);
                            }
                        }
                    }
                    mImagesPairsMatchesPointsFirstsReverse[imageId][nImageInPairs]=imageMatchingPointsWithoutOutliersReverse;
                    mImagesPairsMatchesPointsSecondsReverse[imageId][nImageInPairs]=imagePairMatchingPointsWithoutOutliersReverse;
                    mImagesPairsMatchesIndexesFirstsReverse[imageId][nImageInPairs]=imagePairsMatchesIndexesWithoutOutliersReverse;
                    mImagesPairsMatchesIndexesSecondsReverse[imageId][nImageInPairs]=imagePairPairsMatchesIndexesWithoutOutliersReverse;
                    mImagesPairsMatchesReverse[imageId][nImageInPairs]=matchesWithoutOutliersReverse;
//                    if(coincidenceValuesReverse.size()>0&&saveImagesMatches)
//                    {
//                        QString imageFileName=mImageFileNames[imageId];
//                        QString imagePairFileName=mImageFileNames[imagePairId];
//                        QFileInfo fileInfoImage(imageFileName);
//                        QString imageBaseName=fileInfoImage.baseName();
//                        QFileInfo fileInfoImagePair(imagePairFileName);
//                        QString imagePairBaseName=fileInfoImagePair.baseName();
//                        QString absoluteFilePath=fileInfoImage.absolutePath();
//                        QDir auxDir(absoluteFilePath);
//                        QString imageMatchesDir=pathForMatchesFiles;
//                        if(!auxDir.exists(imageMatchesDir))
//                        {
//                            if(!auxDir.mkdir(imageMatchesDir))
//                            {
//                                strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, error creating directory: %1").arg(imageMatchesDir);
//                                return(OPENCV_KPM_ERROR);
//                            }
//                        }
//                        QString imageMatchesFileName=imageMatchesDir+"/";
////                        imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
//                        imageMatchesFileName+=(imagePairBaseName+"_"+imageBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
//                        if(QFile::exists(imageMatchesFileName))
//                            QFile::remove(imageMatchesFileName);
//                        std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
//                        cv::Mat imageMatchesReverse;
//                        cv::Mat image=mImages[imageId];
//                        cv::Mat imagePair=mImages[imagePairId];
//                        try
//                        {
////                            cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matchesWithoutOutliers,imageMatches);
//                            cv::drawMatches( imagePair, imagePairKeyPoints,image, imageKeyPoints, matchesWithoutOutliersReverse,imageMatchesReverse);
//                        }
//                        catch( cv::Exception& e )
//                        {
//                            const char* err_msg = e.what();
//                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
//                            return(OPENCV_KPM_ERROR);
//                        }
//                        try
//                        {
//                            cv::imwrite(imageMatchesFileNameStdString,imageMatchesReverse);
//                        }
//                        catch( cv::Exception& e )
//                        {
//                            const char* err_msg = e.what();
//                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
//                            return(OPENCV_KPM_ERROR);
//                        }
//                    }
                }
//                meanAzimuth=meanAzimuth/maxAzimuthValues;
//                meanDistance=meanDistance/maxDistanceValues;
//                double stdMeanAzimuth=0.0;
//                double maxAzimuthDifference=0.0;
//                double stdMeanDistance=0.0;
//                double maxDistanceDifference=0.0;
//                for(int nM=0;nM<numberOfTotalMatches;nM++)
//                {
//                    int azimuthClusterIdx = azimuthLabels.at<int>(nM);
//                    if(azimuthClusterIdx==posAzimuthSolution)
//                    {
//                        double azimuthValue=azimuths.at<float>(nM,0);
//                        stdMeanAzimuth+=pow(meanAzimuth-azimuthValue,2.0);
//                        if(fabs(meanAzimuth-azimuthValue)>maxAzimuthDifference)
//                            maxAzimuthDifference=fabs(meanAzimuth-azimuthValue);
//                    }
//                    int distanceClusterIdx = distanceLabels.at<int>(nM);
//                    if(distanceClusterIdx==posDistanceSolution)
//                    {
//                        double distanceValue=distances.at<float>(nM,0);
//                        stdMeanDistance+=pow(meanDistance-distanceValue,2.0);
//                        if(fabs(meanDistance-distanceValue)>maxDistanceDifference)
//                            maxDistanceDifference=fabs(meanDistance-distanceValue);
//                    }
//                }
//                stdMeanAzimuth=sqrt(stdMeanAzimuth/maxAzimuthValues);
//                stdMeanDistance=sqrt(stdMeanDistance/maxDistanceValues);

                if(printReport)
                {
                    QString prefix="          ";
                    qout<<prefix<<"Image pair: "<<imageId<<" - "<<imagePairId<<endl;
                    qout<<prefix<<"   Azimuth_value  Num_Values  Distance_value  Num_Values"<<endl;
                    int maxClusterCount=azimuthNumberOfClusters;
                    if(distanceNumberOfClusters>maxClusterCount)
                        maxClusterCount=distanceNumberOfClusters;
                    for(int nC=0;nC<maxClusterCount;nC++)
                    {
                        if(nC<numberOfAzimuthCluster)
                        {
                            double azimuthCenterValue=azimuthCenters.at<float>(nC);
                            qout<<prefix<<QString::number(azimuthCenterValue,'f',4).rightJustified(16,' ');
                            qout<<QString::number(azimuthClusterCount.at(nC)).rightJustified(12,' ');
                        }
                        else
                            qout<<prefix<<"                ";
                        if(nC<numberOfDistanceCluster)
                        {
                            double distanceCenterValue=distanceCenters.at<float>(nC);
                            qout<<QString::number(distanceCenterValue,'f',2).rightJustified(16,' ');
                            qout<<QString::number(distanceClusterCount.at(nC)).rightJustified(12,' ');
                        }
                        else
                            qout<<"        --------";
                        qout<<endl;
                    }
                    qout<<prefix<<"Solutions: "<<QString::number(coincidenceTotalValues.size())<<" coincidences"<<endl;
                    qout<<prefix<<"- Azimuth (mean, std, maximum difference) (DEG): ("
                       <<QString::number(meanAzimuth,'f',4)<<","
                       <<QString::number(stdMeanAzimuth,'f',6)<<","<<QString::number(maxAzimuthDifference,'f',4)<<endl;
                    qout<<prefix<<"- Distance (mean, std, maximum difference) (m): ("
                       <<QString::number(meanDistance,'f',2)<<","
                       <<QString::number(stdMeanDistance,'f',2)<<","<<QString::number(maxDistanceDifference,'f',2)<<endl;
                }

            }
            iterImagePairs++;
        }
        findOutlierDetector=true;
    }


    //else //if(outlierDetectorTag.compare(OPENCV_KPM_OUTLIERSDETECTOR_DHL_KMEANS_TAG,Qt::CaseSensitive)==0)
    {
//        QString parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHNUMBEROFCLUSTERS_TAG;
//        int parameterPos=parametersTag.indexOf(parameterTag);
//        if(parameterPos==-1)
//        {
//            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
//            return(OPENCV_KPM_ERROR);
//        }
//        int azimuthNumberOfClusters=parametersValue[parameterPos].toInt();

//        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHEPSILONDEG_TAG;
//        parameterPos=parametersTag.indexOf(parameterTag);
//        if(parameterPos==-1)
//        {
//            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
//            return(OPENCV_KPM_ERROR);
//        }
//        double azimuthEpsilogDeg=parametersValue[parameterPos].toDouble();

        double maxDistanceToEpipolarLine=3.0;
        double confidenceLevel=0.99;
        int fundamentalMatrixMethod=cv::FM_RANSAC;

        QTextStream qout(stdout);
        QMap<QString,QVector<QString> >::const_iterator iterImagePairs = mImagesPairs.constBegin();
        double pi=4.0*atan(1.0);
        while(iterImagePairs!=mImagesPairs.constEnd())
        {
            QString imageId=iterImagePairs.key();
            if(!mImagesKeyPoints.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesKeyPointsDescriptors.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesPairsMatchesPointsFirsts.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points firsts for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesPairsMatchesPointsSeconds.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points seconds for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(bidirectionalMatches)
            {
                if(!mImagesPairsMatchesPointsFirstsReverse.contains(imageId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points firsts for image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
                if(!mImagesPairsMatchesPointsSecondsReverse.contains(imageId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points seconds for image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
            }
            std::vector<cv::KeyPoint> imageKeyPoints=mImagesKeyPoints[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsFirst=mImagesPairsMatchesPointsFirsts[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsSecond=mImagesPairsMatchesPointsSeconds[imageId];
            QVector<std::vector<int> > imagePairsMatchesIndexesFirst=mImagesPairsMatchesIndexesFirsts[imageId];
            QVector<std::vector<int> > imagePairsMatchesIndexesSecond=mImagesPairsMatchesIndexesSeconds[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsFirstReverse;
            QVector<std::vector<cv::Point2f> > matchingPointsSecondReverse;
            QVector<std::vector<int> > imagePairsMatchesIndexesFirstReverse;
            QVector<std::vector<int> > imagePairsMatchesIndexesSecondReverse;
            if(bidirectionalMatches)
            {
                matchingPointsFirstReverse=mImagesPairsMatchesPointsFirstsReverse[imageId];
                matchingPointsSecondReverse=mImagesPairsMatchesPointsSecondsReverse[imageId];
                imagePairsMatchesIndexesFirstReverse=mImagesPairsMatchesIndexesFirstsReverse[imageId];
                imagePairsMatchesIndexesSecondReverse=mImagesPairsMatchesIndexesSecondsReverse[imageId];
            }
            QVector<QString> imagePairs=iterImagePairs.value();
            if(imagePairs.size()!=matchingPointsFirst.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(imagePairs.size()!=matchingPointsSecond.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(bidirectionalMatches)
            {
                if(imagePairs.size()!=matchingPointsFirstReverse.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
                if(imagePairs.size()!=matchingPointsSecondReverse.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
            }
            for(int nImageInPairs=0;nImageInPairs<imagePairs.size();nImageInPairs++)
            {
                QString imagePairId=imagePairs.at(nImageInPairs);
                if(!mImagesKeyPoints.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::KeyPoint> imagePairKeyPoints=mImagesKeyPoints[imagePairId];
                if(!mImagesKeyPointsDescriptors.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::Point2f> imageMatchingPoints=matchingPointsFirst[nImageInPairs];
                std::vector<cv::Point2f> imagePairMatchingPoints=matchingPointsSecond[nImageInPairs];
                std::vector<int> imagePairsMatchesIndexes=imagePairsMatchesIndexesFirst[nImageInPairs];
                std::vector<int> imagePairPairsMatchesIndexes=imagePairsMatchesIndexesSecond[nImageInPairs];
                std::vector<cv::Point2f> imageMatchingPointsReverse;
                std::vector<cv::Point2f> imagePairMatchingPointsReverse;
                std::vector<int> imagePairsMatchesIndexesReverse;
                std::vector<int> imagePairPairsMatchesIndexesReverse;
                if(bidirectionalMatches)
                {
                    imageMatchingPointsReverse=matchingPointsFirstReverse[nImageInPairs];
                    imagePairMatchingPointsReverse=matchingPointsSecondReverse[nImageInPairs];
                    imagePairsMatchesIndexesReverse=imagePairsMatchesIndexesFirstReverse[nImageInPairs];
                    imagePairPairsMatchesIndexesReverse=imagePairsMatchesIndexesSecondReverse[nImageInPairs];
                }
                if(imageMatchingPoints.size()!=imagePairMatchingPoints.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not equal number of matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                if(bidirectionalMatches)
                {
                    if(imageMatchingPointsReverse.size()!=imagePairMatchingPointsReverse.size())
                    {
                        strError=QObject::tr("OpenCVKpM::getOutliersDetector, not equal number of reverse matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                int numberOfMatches=(int)imageMatchingPoints.size();
                int numberOfMatchesReverse;
                int numberOfTotalMatches=numberOfMatches;
                if(bidirectionalMatches)
                {
                    numberOfMatchesReverse=(int)imageMatchingPointsReverse.size();
                    numberOfTotalMatches+=numberOfMatchesReverse;
                }

                std::vector<double> x1s,y1s,x2s,y2s;
                for( int kk = 0; kk < numberOfMatches; kk++ )
                {
                    cv::Point2f pt1 = imageMatchingPoints[kk];
                    cv::Point2f pt2 = imagePairMatchingPoints[kk];
                    double x1=pt1.x;
                    double y1=pt1.y;
                    double x2=pt2.x;
                    double y2=pt2.y;
                    x1s.push_back(x1);
                    y1s.push_back(y1);
                    x2s.push_back(x2);
                    y2s.push_back(y2);
                }
                if(bidirectionalMatches)
                {
                    for( int kk = 0; kk < numberOfMatchesReverse; kk++ )
                    {
                        cv::Point2f pt2 = imageMatchingPointsReverse[kk];
                        cv::Point2f pt1 = imagePairMatchingPointsReverse[kk];
                        double x1=pt1.x;
                        double y1=pt1.y;
                        double x2=pt2.x;
                        double y2=pt2.y;
                        x1s.push_back(x1);
                        y1s.push_back(y1);
                        x2s.push_back(x2);
                        y2s.push_back(y2);
                    }
                }
                int w1=mImageWidths[imageId];
                int h1=mImageHeights[imageId];;
                int w2=mImageWidths[imagePairId];;
                int h2=mImageHeights[imagePairId];
                double precision=0.0;
                std::vector<int> vec_inliers;
//                if(!LibOrsa::getInliers(w1,h1,w2,h2,precision,x1s,y1s,x2s,y2s,vec_inliers,strError))
//                {
//                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, orsa library error in image %1 and image pair: %2").arg(imageId).arg(imagePairId);
//                    return(OPENCV_KPM_ERROR);
//                }
                if(!libOrsaAsift::getInliers(w1,h1,w2,h2,precision,x1s,y1s,x2s,y2s,vec_inliers,strError))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, orsa library error in image %1 and image pair: %2").arg(imageId).arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }

                std::vector<cv::Point2f> imageMatchingPointsWithoutOutliers;
                std::vector<cv::Point2f> imagePairMatchingPointsWithoutOutliers;
                std::vector<int> imagePairsMatchesIndexesWithoutOutliers;
                std::vector<int> imagePairPairsMatchesIndexesWithoutOutliers;
                std::vector<cv::DMatch> matches=mImagesPairsMatches[imageId][nImageInPairs];
                std::vector<cv::DMatch> matchesWithoutOutliers;

                std::vector<cv::Point2f> imageMatchingPointsWithoutOutliersReverse;
                std::vector<cv::Point2f> imagePairMatchingPointsWithoutOutliersReverse;
                std::vector<int> imagePairsMatchesIndexesWithoutOutliersReverse;
                std::vector<int> imagePairPairsMatchesIndexesWithoutOutliersReverse;
                std::vector<cv::DMatch> matchesReverse;
                std::vector<cv::DMatch> matchesWithoutOutliersReverse;

                if(bidirectionalMatches)
                {
                    matchesReverse=mImagesPairsMatchesReverse[imageId][nImageInPairs];
                }
                int numberOfInliers=0;
                int numberOfInliersReverse=0;
                for(int iterInliers=0;iterInliers<vec_inliers.size();iterInliers++)
                {
                    int nM=vec_inliers.at(iterInliers);
                    if(nM<numberOfMatches)
                    {
                        imageMatchingPointsWithoutOutliers.push_back(imageMatchingPoints[nM]);
                        imagePairMatchingPointsWithoutOutliers.push_back(imagePairMatchingPoints[nM]);
                        imagePairsMatchesIndexesWithoutOutliers.push_back(imagePairsMatchesIndexes[nM]);
                        imagePairPairsMatchesIndexesWithoutOutliers.push_back(imagePairPairsMatchesIndexes[nM]);
                        cv::DMatch match=matches[nM];
                        matchesWithoutOutliers.push_back(match);
                        numberOfInliers++;
                    }
                    else
                    {
                        if(bidirectionalMatches)// no debería ser en otro caso
                        {
                            nM=nM-numberOfMatches;
                            imageMatchingPointsWithoutOutliersReverse.push_back(imageMatchingPointsReverse[nM]);
                            imagePairMatchingPointsWithoutOutliersReverse.push_back(imagePairMatchingPointsReverse[nM]);
                            imagePairsMatchesIndexesWithoutOutliersReverse.push_back(imagePairsMatchesIndexesReverse[nM]);
                            imagePairPairsMatchesIndexesWithoutOutliersReverse.push_back(imagePairPairsMatchesIndexesReverse[nM]);
                            cv::DMatch match=matchesReverse[nM];
                            matchesWithoutOutliersReverse.push_back(match);
                            numberOfInliersReverse++;
                        }
                    }
                }
                mImagesPairsMatchesPointsFirsts[imageId][nImageInPairs]=imageMatchingPointsWithoutOutliers;
                mImagesPairsMatchesPointsSeconds[imageId][nImageInPairs]=imagePairMatchingPointsWithoutOutliers;
                mImagesPairsMatchesIndexesFirsts[imageId][nImageInPairs]=imagePairsMatchesIndexesWithoutOutliers;
                mImagesPairsMatchesIndexesSeconds[imageId][nImageInPairs]=imagePairPairsMatchesIndexesWithoutOutliers;
                mImagesPairsMatches[imageId][nImageInPairs]=matchesWithoutOutliers;
                if(numberOfInliers>0&&saveImagesMatches)
                {
                    QString imageFileName=mImageFileNames[imageId];
                    QString imagePairFileName=mImageFileNames[imagePairId];
                    QFileInfo fileInfoImage(imageFileName);
                    QString imageBaseName=fileInfoImage.baseName();
                    QFileInfo fileInfoImagePair(imagePairFileName);
                    QString imagePairBaseName=fileInfoImagePair.baseName();
                    QString absoluteFilePath=fileInfoImage.absolutePath();
                    QDir auxDir(absoluteFilePath);
                    QString imageMatchesDir=pathForMatchesFiles;
                    if(!auxDir.exists(imageMatchesDir))
                    {
                        if(!auxDir.mkdir(imageMatchesDir))
                        {
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, error creating directory: %1").arg(imageMatchesDir);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                    QString imageMatchesFileName=imageMatchesDir+"/";
                    imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                    if(QFile::exists(imageMatchesFileName))
                        QFile::remove(imageMatchesFileName);
                    std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                    cv::Mat imageMatches;
                    cv::Mat image=mImages[imageId];
                    cv::Mat imagePair=mImages[imagePairId];
                    try
                    {
                        cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matchesWithoutOutliers,imageMatches);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                    try
                    {
                        cv::imwrite(imageMatchesFileNameStdString,imageMatches);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                if(bidirectionalMatches)
                {
                    mImagesPairsMatchesPointsFirstsReverse[imageId][nImageInPairs]=imageMatchingPointsWithoutOutliersReverse;
                    mImagesPairsMatchesPointsSecondsReverse[imageId][nImageInPairs]=imagePairMatchingPointsWithoutOutliersReverse;
                    mImagesPairsMatchesIndexesFirstsReverse[imageId][nImageInPairs]=imagePairsMatchesIndexesWithoutOutliersReverse;
                    mImagesPairsMatchesIndexesSecondsReverse[imageId][nImageInPairs]=imagePairPairsMatchesIndexesWithoutOutliersReverse;
                    mImagesPairsMatchesReverse[imageId][nImageInPairs]=matchesWithoutOutliersReverse;
                    if(numberOfInliersReverse>0&&saveImagesMatches)
                    {
                        QString imageFileName=mImageFileNames[imageId];
                        QString imagePairFileName=mImageFileNames[imagePairId];
                        QFileInfo fileInfoImage(imageFileName);
                        QString imageBaseName=fileInfoImage.baseName();
                        QFileInfo fileInfoImagePair(imagePairFileName);
                        QString imagePairBaseName=fileInfoImagePair.baseName();
                        QString absoluteFilePath=fileInfoImage.absolutePath();
                        QDir auxDir(absoluteFilePath);
                        QString imageMatchesDir=pathForMatchesFiles;
                        if(!auxDir.exists(imageMatchesDir))
                        {
                            if(!auxDir.mkdir(imageMatchesDir))
                            {
                                strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, error creating directory: %1").arg(imageMatchesDir);
                                return(OPENCV_KPM_ERROR);
                            }
                        }
                        QString imageMatchesFileName=imageMatchesDir+"/";
//                        imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                        imageMatchesFileName+=(imagePairBaseName+"_"+imageBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                        if(QFile::exists(imageMatchesFileName))
                            QFile::remove(imageMatchesFileName);
                        std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                        cv::Mat imageMatchesReverse;
                        cv::Mat image=mImages[imageId];
                        cv::Mat imagePair=mImages[imagePairId];
                        try
                        {
//                            cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matchesWithoutOutliers,imageMatches);
                            cv::drawMatches( imagePair, imagePairKeyPoints,image, imageKeyPoints, matchesWithoutOutliersReverse,imageMatchesReverse);
                        }
                        catch( cv::Exception& e )
                        {
                            const char* err_msg = e.what();
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                            return(OPENCV_KPM_ERROR);
                        }
                        try
                        {
                            cv::imwrite(imageMatchesFileNameStdString,imageMatchesReverse);
                        }
                        catch( cv::Exception& e )
                        {
                            const char* err_msg = e.what();
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                }
            }
            iterImagePairs++;
        }
        findOutlierDetector=true;
    }

    if(!findOutlierDetector)
    {
        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, outlier detector: %1 is not implemented").arg(outlierDetectorTag);
        return(OPENCV_KPM_ERROR);
    }
#endif USE_OPENCV
    return(OPENCV_KPM_NO_ERROR);
}

/*

int OpenCVKpM::detectOutliersInMatches(bool &printReport,
                                       QString &strError)
{
    QString outlierDetectorTag;
    QStringList parametersTag;
    QStringList parametersValue;
    QStringList parametersType;
    if(OPENCV_KPM_NO_ERROR!=mParameters.getOutliersDetector(outlierDetectorTag,
                                                            parametersTag,
                                                            parametersValue,
                                                            parametersType,
                                                            strError))
    {
        strError="OpenCVKpM::detectOutliersInMatches,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    bool findOutlierDetector=false;
    QString pathForMatchesFiles;
    bool saveImagesMatches=mParameters.getSaveImagesMatches(pathForMatchesFiles);
    bool bidirectionalMatches=mParameters.getBidirectionalMatches();
    //outlierDetectorTag="";
    if(outlierDetectorTag.compare(OPENCV_KPM_OUTLIERSDETECTOR_DHL_KMEANS_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHNUMBEROFCLUSTERS_TAG;
        int parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int azimuthNumberOfClusters=parametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHEPSILONDEG_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double azimuthEpsilogDeg=parametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHITERATIONS_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int azimuthIterations=parametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_DISTANCENUMBEROFCLUSTERS_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int distanceNumberOfClusters=parametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_DISTANCEEPSILON_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double distanceEpsilon=parametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_DISTANCEITERATIONS_TAG;
        parameterPos=parametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int distanceIterations=parametersValue[parameterPos].toInt();

        QTextStream qout(stdout);
        QMap<QString,QVector<QString> >::const_iterator iterImagePairs = mImagesPairs.constBegin();
        double pi=4.0*atan(1.0);
        while(iterImagePairs!=mImagesPairs.constEnd())
        {
            QString imageId=iterImagePairs.key();
            if(!mImagesKeyPoints.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesKeyPointsDescriptors.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesPairsMatchesPointsFirsts.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points firsts for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesPairsMatchesPointsSeconds.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points seconds for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(bidirectionalMatches)
            {
                if(!mImagesPairsMatchesPointsFirstsReverse.contains(imageId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points firsts for image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
                if(!mImagesPairsMatchesPointsSecondsReverse.contains(imageId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points seconds for image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
            }
            std::vector<cv::KeyPoint> imageKeyPoints=mImagesKeyPoints[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsFirst=mImagesPairsMatchesPointsFirsts[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsSecond=mImagesPairsMatchesPointsSeconds[imageId];
            QVector<std::vector<int> > imagePairsMatchesIndexesFirst=mImagesPairsMatchesIndexesFirsts[imageId];
            QVector<std::vector<int> > imagePairsMatchesIndexesSecond=mImagesPairsMatchesIndexesSeconds[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsFirstReverse;
            QVector<std::vector<cv::Point2f> > matchingPointsSecondReverse;
            QVector<std::vector<int> > imagePairsMatchesIndexesFirstReverse;
            QVector<std::vector<int> > imagePairsMatchesIndexesSecondReverse;
            if(bidirectionalMatches)
            {
                matchingPointsFirstReverse=mImagesPairsMatchesPointsFirstsReverse[imageId];
                matchingPointsSecondReverse=mImagesPairsMatchesPointsSecondsReverse[imageId];
                imagePairsMatchesIndexesFirstReverse=mImagesPairsMatchesIndexesFirstsReverse[imageId];
                imagePairsMatchesIndexesSecondReverse=mImagesPairsMatchesIndexesSecondsReverse[imageId];
            }
            QVector<QString> imagePairs=iterImagePairs.value();
            if(imagePairs.size()!=matchingPointsFirst.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(imagePairs.size()!=matchingPointsSecond.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(bidirectionalMatches)
            {
                if(imagePairs.size()!=matchingPointsFirstReverse.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
                if(imagePairs.size()!=matchingPointsSecondReverse.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
            }
            for(int nImageInPairs=0;nImageInPairs<imagePairs.size();nImageInPairs++)
            {
                QString imagePairId=imagePairs.at(nImageInPairs);
                if(!mImagesKeyPoints.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::KeyPoint> imagePairKeyPoints=mImagesKeyPoints[imagePairId];
                if(!mImagesKeyPointsDescriptors.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::Point2f> imageMatchingPoints=matchingPointsFirst[nImageInPairs];
                std::vector<cv::Point2f> imagePairMatchingPoints=matchingPointsSecond[nImageInPairs];
                std::vector<int> imagePairsMatchesIndexes=imagePairsMatchesIndexesFirst[nImageInPairs];
                std::vector<int> imagePairPairsMatchesIndexes=imagePairsMatchesIndexesSecond[nImageInPairs];
                std::vector<cv::Point2f> imageMatchingPointsReverse;
                std::vector<cv::Point2f> imagePairMatchingPointsReverse;
                std::vector<int> imagePairsMatchesIndexesReverse;
                std::vector<int> imagePairPairsMatchesIndexesReverse;
                if(bidirectionalMatches)
                {
                    imageMatchingPointsReverse=matchingPointsFirstReverse[nImageInPairs];
                    imagePairMatchingPointsReverse=matchingPointsSecondReverse[nImageInPairs];
                    imagePairsMatchesIndexesReverse=imagePairsMatchesIndexesFirstReverse[nImageInPairs];
                    imagePairPairsMatchesIndexesReverse=imagePairsMatchesIndexesSecondReverse[nImageInPairs];
                }
                if(imageMatchingPoints.size()!=imagePairMatchingPoints.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not equal number of matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                if(bidirectionalMatches)
                {
                    if(imageMatchingPointsReverse.size()!=imagePairMatchingPointsReverse.size())
                    {
                        strError=QObject::tr("OpenCVKpM::getOutliersDetector, not equal number of reverse matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                int numberOfMatches=(int)imageMatchingPoints.size();
                int numberOfMatchesReverse;
                int numberOfTotalMatches=numberOfMatches;
                if(bidirectionalMatches)
                {
                    numberOfMatchesReverse=(int)imageMatchingPointsReverse.size();
                    numberOfTotalMatches+=numberOfMatchesReverse;
                }
                cv::Mat azimuths=cv::Mat::zeros(numberOfTotalMatches,1, CV_32F);
                cv::Mat distances=cv::Mat::zeros(numberOfTotalMatches,1, CV_32F);
                cv::Mat azimuthLabels,distanceLabels;
                for( int kk = 0; kk < numberOfMatches; kk++ )
                {
                    cv::Point2f pt1 = imageMatchingPoints[kk];
                    cv::Point2f pt2 = imagePairMatchingPoints[kk];
                    double x1=pt1.x;
                    double y1=pt1.y;
                    double x2=pt2.x;
                    double y2=pt2.y;
                    double azi=azimuth(x1,y1,x2,y2)*180.0/pi;
                    double dis2d=distance(x1,y1,x2,y2);
                    azimuths.at<float>(kk,0)=azi;
                    distances.at<float>(kk,0)=dis2d;
                }
                if(bidirectionalMatches)
                {
                    for( int kk = 0; kk < numberOfMatchesReverse; kk++ )
                    {
                        cv::Point2f pt2 = imageMatchingPointsReverse[kk];
                        cv::Point2f pt1 = imagePairMatchingPointsReverse[kk];
                        double x1=pt1.x;
                        double y1=pt1.y;
                        double x2=pt2.x;
                        double y2=pt2.y;
                        double azi=azimuth(x1,y1,x2,y2)*180.0/pi;
                        double dis2d=distance(x1,y1,x2,y2);
                        azimuths.at<float>(numberOfMatches+kk,0)=azi;
                        distances.at<float>(numberOfMatches+kk,0)=dis2d;
                    }
                }
                cv::Mat azimuthCenters(azimuthNumberOfClusters, 1, CV_32F);
                try
                {
                    kmeans(azimuths, azimuthNumberOfClusters, azimuthLabels,
                           //TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 100, 5.0),
                           cv::TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_EPS,azimuthIterations,azimuthEpsilogDeg),
                           3, cv::KMEANS_PP_CENTERS, azimuthCenters);
                }
                catch( cv::Exception& e )
                {
                    const char* err_msg = e.what();
                    strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                    return(OPENCV_KPM_ERROR);
                }
                int numberOfAzimuthCluster=azimuthCenters.rows;

                cv::Mat distanceCenters(distanceNumberOfClusters, 1, CV_32F);
                try
                {
                    kmeans(distances, distanceNumberOfClusters,distanceLabels,
                           //TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 100, 20.0),
                           cv::TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_EPS, distanceIterations, distanceEpsilon),
                           3, cv::KMEANS_PP_CENTERS, distanceCenters);
                }
                catch( cv::Exception& e )
                {
                    const char* err_msg = e.what();
                    strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                    return(OPENCV_KPM_ERROR);
                }
                int numberOfDistanceCluster=distanceCenters.rows;
                QVector<int> azimuthClusterCount;
                for(int nca=0;nca<numberOfAzimuthCluster;nca++)
                    azimuthClusterCount.push_back(0);

                QVector<int> distanceClusterCount;
                for(int ncd=0;ncd<numberOfDistanceCluster;ncd++)
                    distanceClusterCount.push_back(0);

                for(int nM=0;nM<numberOfTotalMatches;nM++)
                {
                    int azimuthClusterIdx = azimuthLabels.at<int>(nM);
                    azimuthClusterCount[azimuthClusterIdx]++;
                    int distanceClusterIdx = distanceLabels.at<int>(nM);
                    distanceClusterCount[distanceClusterIdx]++;
                }
                int posAzimuthSolution=-1;
                int maxAzimuthValues=0;
                for(int nca=0;nca<numberOfAzimuthCluster;nca++)
                {
                    int numberOfValues=azimuthClusterCount.at(nca);
                    if(numberOfValues>maxAzimuthValues)
                    {
                        posAzimuthSolution=nca;
                        maxAzimuthValues=numberOfValues;
                    }
                }
                int posDistanceSolution=-1;
                int maxDistanceValues=0;
                for(int ncd=0;ncd<numberOfDistanceCluster;ncd++)
                {
                    int numberOfValues=distanceClusterCount.at(ncd);
                    if(numberOfValues>maxDistanceValues)
                    {
                        posDistanceSolution=ncd;
                        maxDistanceValues=numberOfValues;
                    }
                }
                double meanAzimuth=0.0;
                double meanDistance=0.0;
                double previousMeanAzimuth=0.0;
                double previousMeanDistance=0.0;
                bool isFirstMeanAzimuth=true;
                bool isFirstMeanDistance=true;
                std::vector<cv::Point2f> imageMatchingPointsWithoutOutliers;
                std::vector<cv::Point2f> imagePairMatchingPointsWithoutOutliers;
                std::vector<int> imagePairsMatchesIndexesWithoutOutliers;
                std::vector<int> imagePairPairsMatchesIndexesWithoutOutliers;
                std::vector<cv::DMatch> matches=mImagesPairsMatches[imageId][nImageInPairs];
                std::vector<cv::DMatch> matchesWithoutOutliers;
                bool meanControl=0;
                double stdMeanAzimuth=360.0;
                double maxAzimuthDifference=0.0;
                double stdMeanDistance=10000.0;
                double maxDistanceDifference=0.0;
                while(meanControl==0)
                {
                    meanAzimuth=0.0;
                    meanDistance=0.0;
                    int numberOfAzimuthsInMean=0;
                    int numberOfDistanceInMean=0;
                    for(int nM=0;nM<numberOfMatches;nM++)
                    {
                        int azimuthClusterIdx = azimuthLabels.at<int>(nM);
                        if(azimuthClusterIdx==posAzimuthSolution)
                        {
                            double azimuth=azimuths.at<float>(nM,0);
                            double diffAzimuth=fabs(azimuth-previousMeanAzimuth);
                            if(diffAzimuth<azimuthEpsilogDeg
                                    ||isFirstMeanAzimuth)
                            {
                                meanAzimuth+=azimuth;
                                numberOfAzimuthsInMean++;
                            }
                        }
                        int distanceClusterIdx = distanceLabels.at<int>(nM);
                        if(distanceClusterIdx==posDistanceSolution)
                        {
                            double distance=distances.at<float>(nM,0);
                            double diffDistance=fabs(distance-previousMeanDistance);
                            if(diffDistance<distanceEpsilon
                                    ||isFirstMeanDistance)
                            {
                                meanDistance+=distance;
                                numberOfDistanceInMean++;
                            }
                        }
                    }
                    if(bidirectionalMatches)
                    {
                        for(int nM=0;nM<numberOfMatchesReverse;nM++)
                        {
                            int azimuthClusterIdx = azimuthLabels.at<int>(nM+numberOfMatches);
                            if(azimuthClusterIdx==posAzimuthSolution)
                            {
                                double azimuth=azimuths.at<float>(nM+numberOfMatches,0);
                                double diffAzimuth=fabs(azimuth-previousMeanAzimuth);
                                if(diffAzimuth<azimuthEpsilogDeg
                                        ||isFirstMeanAzimuth)
                                {
                                    meanAzimuth+=azimuth;
                                    numberOfAzimuthsInMean++;
                                }
                            }
                            int distanceClusterIdx = distanceLabels.at<int>(nM+numberOfMatches);
                            if(distanceClusterIdx==posDistanceSolution)
                            {
                                double distance=distances.at<float>(nM+numberOfMatches,0);
                                double diffDistance=fabs(distance-previousMeanDistance);
                                if(diffDistance<distanceEpsilon
                                        ||isFirstMeanDistance)
                                {
                                    meanDistance+=distance;
                                    numberOfDistanceInMean++;
                                }
                            }
                        }
                    }
                    meanAzimuth=meanAzimuth/numberOfAzimuthsInMean;
                    meanDistance=meanDistance/numberOfDistanceInMean;
                    stdMeanAzimuth=0.0;
                    maxAzimuthDifference=0.0;
                    stdMeanDistance=0.0;
                    maxDistanceDifference=0.0;
                    for(int nM=0;nM<numberOfTotalMatches;nM++)
                    {
                        int azimuthClusterIdx = azimuthLabels.at<int>(nM);
                        if(azimuthClusterIdx==posAzimuthSolution)
                        {
                            double azimuthValue=azimuths.at<float>(nM,0);
                            double diffAzimuth=fabs(azimuthValue-previousMeanAzimuth);
                            if(diffAzimuth<azimuthEpsilogDeg
                                    ||isFirstMeanAzimuth)
                            {
                                stdMeanAzimuth+=pow(meanAzimuth-azimuthValue,2.0);
                                if(fabs(meanAzimuth-azimuthValue)>maxAzimuthDifference)
                                    maxAzimuthDifference=fabs(meanAzimuth-azimuthValue);
                            }
                        }
                        int distanceClusterIdx = distanceLabels.at<int>(nM);
                        if(distanceClusterIdx==posDistanceSolution)
                        {
                            double distanceValue=distances.at<float>(nM,0);
                            double diffDistance=fabs(distanceValue-previousMeanDistance);
                            if(diffDistance<distanceEpsilon
                                    ||isFirstMeanDistance)
                            {
                                stdMeanDistance+=pow(meanDistance-distanceValue,2.0);
                                if(fabs(meanDistance-distanceValue)>maxDistanceDifference)
                                    maxDistanceDifference=fabs(meanDistance-distanceValue);
                            }
                        }
                    }
                    stdMeanAzimuth=sqrt(stdMeanAzimuth/numberOfAzimuthsInMean);
                    stdMeanDistance=sqrt(stdMeanDistance/numberOfDistanceInMean);
                    if(stdMeanAzimuth<azimuthEpsilogDeg
                            &&stdMeanDistance<distanceEpsilon)
                    {
                        meanControl=1;
                    }
                    else
                    {
                        previousMeanAzimuth=meanAzimuth;
                        previousMeanDistance=meanDistance;
                        isFirstMeanAzimuth=false;
                        isFirstMeanDistance=false;
                    }
                }

                QVector<int> coincidenceTotalValues;
                QVector<int> coincidenceValues;
                for(int nM=0;nM<numberOfMatches;nM++)
                {
                    int azimuthClusterIdx = azimuthLabels.at<int>(nM);
                    double azimuthValue=azimuths.at<float>(nM,0);
                    double diffAzimuth=azimuthValue-meanAzimuth;
                    int distanceClusterIdx = distanceLabels.at<int>(nM);
                    double distanceValue=distances.at<float>(nM,0);
                    double diffDistance=distanceValue-meanDistance;
                    if(azimuthClusterIdx==posAzimuthSolution
                            &&distanceClusterIdx==posDistanceSolution)
                    {
                        if(diffAzimuth<azimuthEpsilogDeg
                                &&diffDistance<distanceEpsilon)
                        {
                            coincidenceValues.push_back(nM);
                            coincidenceTotalValues.push_back(nM);
                            imageMatchingPointsWithoutOutliers.push_back(imageMatchingPoints[nM]);
                            imagePairMatchingPointsWithoutOutliers.push_back(imagePairMatchingPoints[nM]);
                            imagePairsMatchesIndexesWithoutOutliers.push_back(imagePairsMatchesIndexes[nM]);
                            imagePairPairsMatchesIndexesWithoutOutliers.push_back(imagePairPairsMatchesIndexes[nM]);
                            cv::DMatch match=matches[nM];
                            matchesWithoutOutliers.push_back(match);
                        }
                    }
                }

                mImagesPairsMatchesPointsFirsts[imageId][nImageInPairs]=imageMatchingPointsWithoutOutliers;
                mImagesPairsMatchesPointsSeconds[imageId][nImageInPairs]=imagePairMatchingPointsWithoutOutliers;
                mImagesPairsMatchesIndexesFirsts[imageId][nImageInPairs]=imagePairsMatchesIndexesWithoutOutliers;
                mImagesPairsMatchesIndexesSeconds[imageId][nImageInPairs]=imagePairPairsMatchesIndexesWithoutOutliers;
                mImagesPairsMatches[imageId][nImageInPairs]=matchesWithoutOutliers;
                if(coincidenceValues.size()>0&&saveImagesMatches)
                {
                    QString imageFileName=mImageFileNames[imageId];
                    QString imagePairFileName=mImageFileNames[imagePairId];
                    QFileInfo fileInfoImage(imageFileName);
                    QString imageBaseName=fileInfoImage.baseName();
                    QFileInfo fileInfoImagePair(imagePairFileName);
                    QString imagePairBaseName=fileInfoImagePair.baseName();
                    QString absoluteFilePath=fileInfoImage.absolutePath();
                    QDir auxDir(absoluteFilePath);
                    QString imageMatchesDir=pathForMatchesFiles;
                    if(!auxDir.exists(imageMatchesDir))
                    {
                        if(!auxDir.mkdir(imageMatchesDir))
                        {
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, error creating directory: %1").arg(imageMatchesDir);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                    QString imageMatchesFileName=imageMatchesDir+"/";
                    imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                    if(QFile::exists(imageMatchesFileName))
                        QFile::remove(imageMatchesFileName);
                    std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                    cv::Mat imageMatches;
                    cv::Mat image=mImages[imageId];
                    cv::Mat imagePair=mImages[imagePairId];
                    try
                    {
                        cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matchesWithoutOutliers,imageMatches);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                    try
                    {
                        cv::imwrite(imageMatchesFileNameStdString,imageMatches);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                if(bidirectionalMatches)
                {
                    QVector<int> coincidenceValuesReverse;
                    std::vector<cv::Point2f> imageMatchingPointsWithoutOutliersReverse;
                    std::vector<cv::Point2f> imagePairMatchingPointsWithoutOutliersReverse;
                    std::vector<int> imagePairsMatchesIndexesWithoutOutliersReverse;
                    std::vector<int> imagePairPairsMatchesIndexesWithoutOutliersReverse;
                    std::vector<cv::DMatch> matchesReverse=mImagesPairsMatchesReverse[imageId][nImageInPairs];
                    std::vector<cv::DMatch> matchesWithoutOutliersReverse;
                    for(int nM=0;nM<numberOfMatchesReverse;nM++)
                    {
                        int azimuthClusterIdx = azimuthLabels.at<int>(nM+numberOfMatches);
                        double azimuthValue=azimuths.at<float>(nM+numberOfMatches,0);
                        double diffAzimuth=azimuthValue-meanAzimuth;
                        int distanceClusterIdx = distanceLabels.at<int>(nM+numberOfMatches);
                        double distanceValue=distances.at<float>(nM+numberOfMatches,0);
                        double diffDistance=distanceValue-meanDistance;
                        if(azimuthClusterIdx==posAzimuthSolution
                                &&distanceClusterIdx==posDistanceSolution)
                        {
                            if(diffAzimuth<azimuthEpsilogDeg
                                    &&diffDistance<distanceEpsilon)
                            {
                                coincidenceValuesReverse.push_back(nM);
                                coincidenceTotalValues.push_back(nM+numberOfMatches);
                                imageMatchingPointsWithoutOutliersReverse.push_back(imageMatchingPointsReverse[nM]);
                                imagePairMatchingPointsWithoutOutliersReverse.push_back(imagePairMatchingPointsReverse[nM]);
                                imagePairsMatchesIndexesWithoutOutliersReverse.push_back(imagePairsMatchesIndexesReverse[nM]);
                                imagePairPairsMatchesIndexesWithoutOutliersReverse.push_back(imagePairPairsMatchesIndexesReverse[nM]);
                                cv::DMatch match=matchesReverse[nM];
                                matchesWithoutOutliersReverse.push_back(match);
                            }
                        }
                    }
                    mImagesPairsMatchesPointsFirstsReverse[imageId][nImageInPairs]=imageMatchingPointsWithoutOutliersReverse;
                    mImagesPairsMatchesPointsSecondsReverse[imageId][nImageInPairs]=imagePairMatchingPointsWithoutOutliersReverse;
                    mImagesPairsMatchesIndexesFirstsReverse[imageId][nImageInPairs]=imagePairsMatchesIndexesWithoutOutliersReverse;
                    mImagesPairsMatchesIndexesSecondsReverse[imageId][nImageInPairs]=imagePairPairsMatchesIndexesWithoutOutliersReverse;
                    mImagesPairsMatchesReverse[imageId][nImageInPairs]=matchesWithoutOutliersReverse;
                    if(coincidenceValuesReverse.size()>0&&saveImagesMatches)
                    {
                        QString imageFileName=mImageFileNames[imageId];
                        QString imagePairFileName=mImageFileNames[imagePairId];
                        QFileInfo fileInfoImage(imageFileName);
                        QString imageBaseName=fileInfoImage.baseName();
                        QFileInfo fileInfoImagePair(imagePairFileName);
                        QString imagePairBaseName=fileInfoImagePair.baseName();
                        QString absoluteFilePath=fileInfoImage.absolutePath();
                        QDir auxDir(absoluteFilePath);
                        QString imageMatchesDir=pathForMatchesFiles;
                        if(!auxDir.exists(imageMatchesDir))
                        {
                            if(!auxDir.mkdir(imageMatchesDir))
                            {
                                strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, error creating directory: %1").arg(imageMatchesDir);
                                return(OPENCV_KPM_ERROR);
                            }
                        }
                        QString imageMatchesFileName=imageMatchesDir+"/";
//                        imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                        imageMatchesFileName+=(imagePairBaseName+"_"+imageBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                        if(QFile::exists(imageMatchesFileName))
                            QFile::remove(imageMatchesFileName);
                        std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                        cv::Mat imageMatchesReverse;
                        cv::Mat image=mImages[imageId];
                        cv::Mat imagePair=mImages[imagePairId];
                        try
                        {
//                            cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matchesWithoutOutliers,imageMatches);
                            cv::drawMatches( imagePair, imagePairKeyPoints,image, imageKeyPoints, matchesWithoutOutliersReverse,imageMatchesReverse);
                        }
                        catch( cv::Exception& e )
                        {
                            const char* err_msg = e.what();
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                            return(OPENCV_KPM_ERROR);
                        }
                        try
                        {
                            cv::imwrite(imageMatchesFileNameStdString,imageMatchesReverse);
                        }
                        catch( cv::Exception& e )
                        {
                            const char* err_msg = e.what();
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                }
//                meanAzimuth=meanAzimuth/maxAzimuthValues;
//                meanDistance=meanDistance/maxDistanceValues;
//                double stdMeanAzimuth=0.0;
//                double maxAzimuthDifference=0.0;
//                double stdMeanDistance=0.0;
//                double maxDistanceDifference=0.0;
//                for(int nM=0;nM<numberOfTotalMatches;nM++)
//                {
//                    int azimuthClusterIdx = azimuthLabels.at<int>(nM);
//                    if(azimuthClusterIdx==posAzimuthSolution)
//                    {
//                        double azimuthValue=azimuths.at<float>(nM,0);
//                        stdMeanAzimuth+=pow(meanAzimuth-azimuthValue,2.0);
//                        if(fabs(meanAzimuth-azimuthValue)>maxAzimuthDifference)
//                            maxAzimuthDifference=fabs(meanAzimuth-azimuthValue);
//                    }
//                    int distanceClusterIdx = distanceLabels.at<int>(nM);
//                    if(distanceClusterIdx==posDistanceSolution)
//                    {
//                        double distanceValue=distances.at<float>(nM,0);
//                        stdMeanDistance+=pow(meanDistance-distanceValue,2.0);
//                        if(fabs(meanDistance-distanceValue)>maxDistanceDifference)
//                            maxDistanceDifference=fabs(meanDistance-distanceValue);
//                    }
//                }
//                stdMeanAzimuth=sqrt(stdMeanAzimuth/maxAzimuthValues);
//                stdMeanDistance=sqrt(stdMeanDistance/maxDistanceValues);

                if(printReport)
                {
                    QString prefix="          ";
                    qout<<prefix<<"Image pair: "<<imageId<<" - "<<imagePairId<<endl;
                    qout<<prefix<<"   Azimuth_value  Num_Values  Distance_value  Num_Values"<<endl;
                    int maxClusterCount=azimuthNumberOfClusters;
                    if(distanceNumberOfClusters>maxClusterCount)
                        maxClusterCount=distanceNumberOfClusters;
                    for(int nC=0;nC<maxClusterCount;nC++)
                    {
                        if(nC<numberOfAzimuthCluster)
                        {
                            double azimuthCenterValue=azimuthCenters.at<float>(nC);
                            qout<<prefix<<QString::number(azimuthCenterValue,'f',4).rightJustified(16,' ');
                            qout<<QString::number(azimuthClusterCount.at(nC)).rightJustified(12,' ');
                        }
                        else
                            qout<<prefix<<"                ";
                        if(nC<numberOfDistanceCluster)
                        {
                            double distanceCenterValue=distanceCenters.at<float>(nC);
                            qout<<QString::number(distanceCenterValue,'f',2).rightJustified(16,' ');
                            qout<<QString::number(distanceClusterCount.at(nC)).rightJustified(12,' ');
                        }
                        else
                            qout<<"        --------";
                        qout<<endl;
                    }
                    qout<<prefix<<"Solutions: "<<QString::number(coincidenceTotalValues.size())<<" coincidences"<<endl;
                    qout<<prefix<<"- Azimuth (mean, std, maximum difference) (DEG): ("
                       <<QString::number(meanAzimuth,'f',4)<<","
                       <<QString::number(stdMeanAzimuth,'f',6)<<","<<QString::number(maxAzimuthDifference,'f',4)<<endl;
                    qout<<prefix<<"- Distance (mean, std, maximum difference) (m): ("
                       <<QString::number(meanDistance,'f',2)<<","
                       <<QString::number(stdMeanDistance,'f',2)<<","<<QString::number(maxDistanceDifference,'f',2)<<endl;
                }

            }
            iterImagePairs++;
        }
        findOutlierDetector=true;
    }

    //else //if(outlierDetectorTag.compare(OPENCV_KPM_OUTLIERSDETECTOR_DHL_KMEANS_TAG,Qt::CaseSensitive)==0)
    {
//        QString parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHNUMBEROFCLUSTERS_TAG;
//        int parameterPos=parametersTag.indexOf(parameterTag);
//        if(parameterPos==-1)
//        {
//            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
//            return(OPENCV_KPM_ERROR);
//        }
//        int azimuthNumberOfClusters=parametersValue[parameterPos].toInt();

//        parameterTag=OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHEPSILONDEG_TAG;
//        parameterPos=parametersTag.indexOf(parameterTag);
//        if(parameterPos==-1)
//        {
//            strError=QObject::tr("OpenCVKpM::getOutliersDetector, outliser detector %1 parameter % 2 not found").arg(outlierDetectorTag).arg(parameterTag);
//            return(OPENCV_KPM_ERROR);
//        }
//        double azimuthEpsilogDeg=parametersValue[parameterPos].toDouble();

        double maxDistanceToEpipolarLine=3.0;
        double confidenceLevel=0.99;
        int fundamentalMatrixMethod=cv::FM_RANSAC;

        QTextStream qout(stdout);
        QMap<QString,QVector<QString> >::const_iterator iterImagePairs = mImagesPairs.constBegin();
        double pi=4.0*atan(1.0);
        while(iterImagePairs!=mImagesPairs.constEnd())
        {
            QString imageId=iterImagePairs.key();
            if(!mImagesKeyPoints.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesKeyPointsDescriptors.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesPairsMatchesPointsFirsts.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points firsts for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesPairsMatchesPointsSeconds.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points seconds for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(bidirectionalMatches)
            {
                if(!mImagesPairsMatchesPointsFirstsReverse.contains(imageId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points firsts for image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
                if(!mImagesPairsMatchesPointsSecondsReverse.contains(imageId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points seconds for image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
            }
            std::vector<cv::KeyPoint> imageKeyPoints=mImagesKeyPoints[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsFirst=mImagesPairsMatchesPointsFirsts[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsSecond=mImagesPairsMatchesPointsSeconds[imageId];
            QVector<std::vector<int> > imagePairsMatchesIndexesFirst=mImagesPairsMatchesIndexesFirsts[imageId];
            QVector<std::vector<int> > imagePairsMatchesIndexesSecond=mImagesPairsMatchesIndexesSeconds[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsFirstReverse;
            QVector<std::vector<cv::Point2f> > matchingPointsSecondReverse;
            QVector<std::vector<int> > imagePairsMatchesIndexesFirstReverse;
            QVector<std::vector<int> > imagePairsMatchesIndexesSecondReverse;
            if(bidirectionalMatches)
            {
                matchingPointsFirstReverse=mImagesPairsMatchesPointsFirstsReverse[imageId];
                matchingPointsSecondReverse=mImagesPairsMatchesPointsSecondsReverse[imageId];
                imagePairsMatchesIndexesFirstReverse=mImagesPairsMatchesIndexesFirstsReverse[imageId];
                imagePairsMatchesIndexesSecondReverse=mImagesPairsMatchesIndexesSecondsReverse[imageId];
            }
            QVector<QString> imagePairs=iterImagePairs.value();
            if(imagePairs.size()!=matchingPointsFirst.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(imagePairs.size()!=matchingPointsSecond.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(bidirectionalMatches)
            {
                if(imagePairs.size()!=matchingPointsFirstReverse.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
                if(imagePairs.size()!=matchingPointsSecondReverse.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                    return(OPENCV_KPM_ERROR);
                }
            }
            for(int nImageInPairs=0;nImageInPairs<imagePairs.size();nImageInPairs++)
            {
                QString imagePairId=imagePairs.at(nImageInPairs);
                if(!mImagesKeyPoints.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::KeyPoint> imagePairKeyPoints=mImagesKeyPoints[imagePairId];
                if(!mImagesKeyPointsDescriptors.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::Point2f> imageMatchingPoints=matchingPointsFirst[nImageInPairs];
                std::vector<cv::Point2f> imagePairMatchingPoints=matchingPointsSecond[nImageInPairs];
                std::vector<int> imagePairsMatchesIndexes=imagePairsMatchesIndexesFirst[nImageInPairs];
                std::vector<int> imagePairPairsMatchesIndexes=imagePairsMatchesIndexesSecond[nImageInPairs];
                std::vector<cv::Point2f> imageMatchingPointsReverse;
                std::vector<cv::Point2f> imagePairMatchingPointsReverse;
                std::vector<int> imagePairsMatchesIndexesReverse;
                std::vector<int> imagePairPairsMatchesIndexesReverse;
                if(bidirectionalMatches)
                {
                    imageMatchingPointsReverse=matchingPointsFirstReverse[nImageInPairs];
                    imagePairMatchingPointsReverse=matchingPointsSecondReverse[nImageInPairs];
                    imagePairsMatchesIndexesReverse=imagePairsMatchesIndexesFirstReverse[nImageInPairs];
                    imagePairPairsMatchesIndexesReverse=imagePairsMatchesIndexesSecondReverse[nImageInPairs];
                }
                if(imageMatchingPoints.size()!=imagePairMatchingPoints.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not equal number of matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                if(bidirectionalMatches)
                {
                    if(imageMatchingPointsReverse.size()!=imagePairMatchingPointsReverse.size())
                    {
                        strError=QObject::tr("OpenCVKpM::getOutliersDetector, not equal number of reverse matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                int numberOfMatches=(int)imageMatchingPoints.size();
                int numberOfMatchesReverse;
                int numberOfTotalMatches=numberOfMatches;
                if(bidirectionalMatches)
                {
                    numberOfMatchesReverse=(int)imageMatchingPointsReverse.size();
                    numberOfTotalMatches+=numberOfMatchesReverse;
                }
                std::vector<cv::Point2f> points1,points2;
                for(int nM=0;nM<numberOfMatches;nM++)
                {
                    cv::Point2f point1=imageMatchingPoints.at(nM);
                    cv::Point2f point2=imagePairMatchingPoints.at(nM);
                    points1.push_back(point1);
                    points2.push_back(point2);
                }
                if(bidirectionalMatches)
                {
                    for(int nM=0;nM<numberOfMatchesReverse;nM++)
                    {
                        cv::Point2f point2=imageMatchingPointsReverse.at(nM);
                        cv::Point2f point1=imagePairMatchingPointsReverse.at(nM);
                        points1.push_back(point1);
                        points2.push_back(point2);
                    }
                }
                std::vector<uchar> inliers(numberOfMatches,0);
                double f11,f12,f13,f21,f22,f23,f31,f32,f33;
                try
                {
                    cv::Mat F = cv::findFundamentalMat(points1,
                                                       points2,
                                                       inliers,
                                                       fundamentalMatrixMethod,
                                                       maxDistanceToEpipolarLine,
                                                       confidenceLevel);
                    f11=F.at<double>(0,0);
                    f12=F.at<double>(0,1);
                    f13=F.at<double>(0,2);
                    f21=F.at<double>(1,0);
                    f22=F.at<double>(1,1);
                    f23=F.at<double>(1,2);
                    f31=F.at<double>(2,0);
                    f32=F.at<double>(2,1);
                    f33=F.at<double>(2,2);
                }
                catch( cv::Exception& e )
                {
                    const char* err_msg = e.what();
                    strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                    return(OPENCV_KPM_ERROR);
                }
                QVector<int> coincidenceTotalValues;
                QVector<int> coincidenceValues;
                std::vector<cv::Point2f> imageMatchingPointsWithoutOutliers;
                std::vector<cv::Point2f> imagePairMatchingPointsWithoutOutliers;
                std::vector<int> imagePairsMatchesIndexesWithoutOutliers;
                std::vector<int> imagePairPairsMatchesIndexesWithoutOutliers;
                std::vector<cv::DMatch> matches=mImagesPairsMatches[imageId][nImageInPairs];
                std::vector<cv::DMatch> matchesWithoutOutliers;
                for(int nM=0;nM<numberOfMatches;nM++)
                {
                    int inlier=inliers.at(nM);
                    cv::Point2f point1=points1.at(nM);
                    double x1=point1.x;
                    double y1=point1.y;
                    cv::Point2f point2=points2.at(nM);
                    double x2=point2.x;
                    double y2=point2.y;
                    double aux1=f11*x2+f12*y2+f13*1.0;
                    double aux2=f21*x2+f22*y2+f23*1.0;
                    double aux3=f31*x2+f32*y2+f33*1.0;
                    double value=x1*aux1+y1*aux2+1.0*aux3;
                    if(fabs(value)<maxDistanceToEpipolarLine)
                        inlier=1;
                    else
                        inlier=0;
                    if(inlier==1)
                    {
                        coincidenceValues.push_back(nM);
                        coincidenceTotalValues.push_back(nM);
                        imageMatchingPointsWithoutOutliers.push_back(imageMatchingPoints[nM]);
                        imagePairMatchingPointsWithoutOutliers.push_back(imagePairMatchingPoints[nM]);
                        imagePairsMatchesIndexesWithoutOutliers.push_back(imagePairsMatchesIndexes[nM]);
                        imagePairPairsMatchesIndexesWithoutOutliers.push_back(imagePairPairsMatchesIndexes[nM]);
                        cv::DMatch match=matches[nM];
                        matchesWithoutOutliers.push_back(match);
                    }
                }
                mImagesPairsMatchesPointsFirsts[imageId][nImageInPairs]=imageMatchingPointsWithoutOutliers;
                mImagesPairsMatchesPointsSeconds[imageId][nImageInPairs]=imagePairMatchingPointsWithoutOutliers;
                mImagesPairsMatchesIndexesFirsts[imageId][nImageInPairs]=imagePairsMatchesIndexesWithoutOutliers;
                mImagesPairsMatchesIndexesSeconds[imageId][nImageInPairs]=imagePairPairsMatchesIndexesWithoutOutliers;
                mImagesPairsMatches[imageId][nImageInPairs]=matchesWithoutOutliers;
                if(coincidenceValues.size()>0&&saveImagesMatches)
                {
                    QString imageFileName=mImageFileNames[imageId];
                    QString imagePairFileName=mImageFileNames[imagePairId];
                    QFileInfo fileInfoImage(imageFileName);
                    QString imageBaseName=fileInfoImage.baseName();
                    QFileInfo fileInfoImagePair(imagePairFileName);
                    QString imagePairBaseName=fileInfoImagePair.baseName();
                    QString absoluteFilePath=fileInfoImage.absolutePath();
                    QDir auxDir(absoluteFilePath);
                    QString imageMatchesDir=pathForMatchesFiles;
                    if(!auxDir.exists(imageMatchesDir))
                    {
                        if(!auxDir.mkdir(imageMatchesDir))
                        {
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, error creating directory: %1").arg(imageMatchesDir);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                    QString imageMatchesFileName=imageMatchesDir+"/";
                    imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                    if(QFile::exists(imageMatchesFileName))
                        QFile::remove(imageMatchesFileName);
                    std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                    cv::Mat imageMatches;
                    cv::Mat image=mImages[imageId];
                    cv::Mat imagePair=mImages[imagePairId];
                    try
                    {
                        cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matchesWithoutOutliers,imageMatches);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                    try
                    {
                        cv::imwrite(imageMatchesFileNameStdString,imageMatches);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                if(bidirectionalMatches)
                {

                    QVector<int> coincidenceValuesReverse;
                    std::vector<cv::Point2f> imageMatchingPointsWithoutOutliersReverse;
                    std::vector<cv::Point2f> imagePairMatchingPointsWithoutOutliersReverse;
                    std::vector<int> imagePairsMatchesIndexesWithoutOutliersReverse;
                    std::vector<int> imagePairPairsMatchesIndexesWithoutOutliersReverse;
                    std::vector<cv::DMatch> matchesReverse=mImagesPairsMatchesReverse[imageId][nImageInPairs];
                    std::vector<cv::DMatch> matchesWithoutOutliersReverse;
                    for(int nM=0;nM<numberOfMatchesReverse;nM++)
                    {
                        int inlier=inliers.at(nM+numberOfMatches);
                        cv::Point2f point1=points1.at(nM+numberOfMatches);
                        double x1=point1.x;
                        double y1=point1.y;
                        cv::Point2f point2=points2.at(nM+numberOfMatches);
                        double x2=point2.x;
                        double y2=point2.y;
                        double aux1=f11*x2+f12*y2+f13*1.0;
                        double aux2=f21*x2+f22*y2+f23*1.0;
                        double aux3=f31*x2+f32*y2+f33*1.0;
                        double value=x1*aux1+y1*aux2+1.0*aux3;
                        if(fabs(value)<maxDistanceToEpipolarLine)
                            inlier=1;
                        else
                            inlier=0;
                        if(inlier==1)
                        {
                            coincidenceValuesReverse.push_back(nM);
                            coincidenceTotalValues.push_back(nM+numberOfMatches);
                            imageMatchingPointsWithoutOutliersReverse.push_back(imageMatchingPointsReverse[nM]);
                            imagePairMatchingPointsWithoutOutliersReverse.push_back(imagePairMatchingPointsReverse[nM]);
                            imagePairsMatchesIndexesWithoutOutliersReverse.push_back(imagePairsMatchesIndexesReverse[nM]);
                            imagePairPairsMatchesIndexesWithoutOutliersReverse.push_back(imagePairPairsMatchesIndexesReverse[nM]);
                            cv::DMatch match=matchesReverse[nM];
                            matchesWithoutOutliersReverse.push_back(match);
                        }
                    }
                    mImagesPairsMatchesPointsFirstsReverse[imageId][nImageInPairs]=imageMatchingPointsWithoutOutliersReverse;
                    mImagesPairsMatchesPointsSecondsReverse[imageId][nImageInPairs]=imagePairMatchingPointsWithoutOutliersReverse;
                    mImagesPairsMatchesIndexesFirstsReverse[imageId][nImageInPairs]=imagePairsMatchesIndexesWithoutOutliersReverse;
                    mImagesPairsMatchesIndexesSecondsReverse[imageId][nImageInPairs]=imagePairPairsMatchesIndexesWithoutOutliersReverse;
                    mImagesPairsMatchesReverse[imageId][nImageInPairs]=matchesWithoutOutliersReverse;
                    if(coincidenceValuesReverse.size()>0&&saveImagesMatches)
                    {
                        QString imageFileName=mImageFileNames[imageId];
                        QString imagePairFileName=mImageFileNames[imagePairId];
                        QFileInfo fileInfoImage(imageFileName);
                        QString imageBaseName=fileInfoImage.baseName();
                        QFileInfo fileInfoImagePair(imagePairFileName);
                        QString imagePairBaseName=fileInfoImagePair.baseName();
                        QString absoluteFilePath=fileInfoImage.absolutePath();
                        QDir auxDir(absoluteFilePath);
                        QString imageMatchesDir=pathForMatchesFiles;
                        if(!auxDir.exists(imageMatchesDir))
                        {
                            if(!auxDir.mkdir(imageMatchesDir))
                            {
                                strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, error creating directory: %1").arg(imageMatchesDir);
                                return(OPENCV_KPM_ERROR);
                            }
                        }
                        QString imageMatchesFileName=imageMatchesDir+"/";
//                        imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                        imageMatchesFileName+=(imagePairBaseName+"_"+imageBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                        if(QFile::exists(imageMatchesFileName))
                            QFile::remove(imageMatchesFileName);
                        std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                        cv::Mat imageMatchesReverse;
                        cv::Mat image=mImages[imageId];
                        cv::Mat imagePair=mImages[imagePairId];
                        try
                        {
//                            cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matchesWithoutOutliers,imageMatches);
                            cv::drawMatches( imagePair, imagePairKeyPoints,image, imageKeyPoints, matchesWithoutOutliersReverse,imageMatchesReverse);
                        }
                        catch( cv::Exception& e )
                        {
                            const char* err_msg = e.what();
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                            return(OPENCV_KPM_ERROR);
                        }
                        try
                        {
                            cv::imwrite(imageMatchesFileNameStdString,imageMatchesReverse);
                        }
                        catch( cv::Exception& e )
                        {
                            const char* err_msg = e.what();
                            strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                }
            }
            iterImagePairs++;
        }
        findOutlierDetector=true;
    }

    if(!findOutlierDetector)
    {
        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, outlier detector: %1 is not implemented").arg(outlierDetectorTag);
        return(OPENCV_KPM_ERROR);
    }
    return(OPENCV_KPM_NO_ERROR);
}
*/
int OpenCVKpM::getApplyOutliersDetection(bool &applyOutliersDetection,
                                         QString &strError)
{
    if(OPENCV_KPM_NO_ERROR!=mParameters.getApplyOutliersDetection(applyOutliersDetection,strError))
    {
        strError="OpenCVKpM::getApplyOutliersDetection,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getDetector(cv::FeatureDetector **ptrDetector, //cv::Feature2D** ptrDetector,
                           QString& strError)
{
#ifdef USE_OPENCV
    if((*ptrDetector)!=NULL)
        delete((*ptrDetector));
    (*ptrDetector)=NULL;
    QString detectorTag;
    QStringList detectorParametersTag;
    //QStringList detectorParametersTagQParams;
    QStringList detectorParametersValue;
    QStringList detectorParametersType;
    if(OPENCV_KPM_NO_ERROR!=mParameters.getDetector(detectorTag,
                                                    detectorParametersTag,
                                                    //detectorParametersTagQParams,
                                                    detectorParametersValue,
                                                    detectorParametersType,
                                                    strError))
    {
        strError="OpenCVKpM::getDetector,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    bool findDetector=false;
    if(detectorTag.compare(OPENCV_KPM_DETECTOR_FAST_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_FAST_NONMAXSUPPRESSION_TAG;
        int parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int intNonmaxSuppression=detectorParametersValue[parameterPos].toInt();
        bool nonmaxSuppression=true;
        if(intNonmaxSuppression==0)
            nonmaxSuppression=false;

        parameterTag=OPENCV_KPM_FAST_THRESHOLD_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int threshold=detectorParametersValue[parameterPos].toInt();

        (*ptrDetector)=new cv::FastFeatureDetector(threshold,nonmaxSuppression);
        findDetector=true;
    }
    if(detectorTag.compare(OPENCV_KPM_DETECTOR_ORB_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_ORB_WTA_K_TAG;
        int parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int wTA_K=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_ORB_EDGETHRESHOLD_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int edgeThreshold=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_ORB_FIRSTLEVEL_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int firstLevel=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_ORB_NFEATURES_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int nFeatures=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_ORB_NLEVELS_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int nLevels=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_ORB_PATCHSIZE_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int patchSize=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_ORB_SCALEFACTOR_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double scaleFactor=detectorParametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_ORB_SCORETYPE_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int scoreType=detectorParametersValue[parameterPos].toInt();

        (*ptrDetector)=new cv::ORB(nFeatures,scaleFactor,nLevels,edgeThreshold,firstLevel,wTA_K,scoreType,patchSize);
        findDetector=true;
    }
    if(detectorTag.compare(OPENCV_KPM_DETECTOR_SIFT_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_SIFT_CONTRASTTHRESHOLD_TAG;
        int parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double contrastThreshold=detectorParametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_SIFT_EDGETHRESHOLD_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double edgeThreshold=detectorParametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_SIFT_NOCTAVELAYERS_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int nOctaveLayers=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_SIFT_NFEATURES_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int nFeatures=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_SIFT_SIGMA_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double sigma=detectorParametersValue[parameterPos].toDouble();

        (*ptrDetector)=new cv::SIFT(nFeatures,nOctaveLayers,contrastThreshold,edgeThreshold,sigma);
        findDetector=true;
    }
    if(detectorTag.compare(OPENCV_KPM_DETECTOR_STAR_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_STAR_LINETHRESHOLDBINARIZED_TAG;
        int parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int lineThresholdBinarized=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_STAR_LINETHRESHOLDPROJECTED_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int lineThresholdProjected=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_STAR_MAXSIZE_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int maxSize=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_STAR_RESPONSETHRESHOLD_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int responseThreshold=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_STAR_SUPPRESSNONMAXSIZE_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int suppressNonmaxSize=detectorParametersValue[parameterPos].toInt();

        (*ptrDetector)=new cv::StarFeatureDetector(maxSize,responseThreshold,lineThresholdProjected,lineThresholdBinarized,suppressNonmaxSize);
        findDetector=true;
    }
    if(detectorTag.compare(OPENCV_KPM_DETECTOR_SURF_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_SURF_EXTENDED_TAG;
        int parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int intExtended=detectorParametersValue[parameterPos].toInt();
        bool extended=true;
        if(intExtended==0)
            extended=false;

        parameterTag=OPENCV_KPM_SURF_HESSIANTHRESHOLD_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double hessianThreshold=detectorParametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_SURF_NOCTAVELAYERS_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int nOctaveLayers=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_SURF_NOCTAVES_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int nOctaves=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_SURF_UPRIGHT_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int intUpRight=detectorParametersValue[parameterPos].toInt();
        bool upRight=true;
        if(intUpRight==0)
            upRight=false;

        (*ptrDetector)=new cv::SURF(hessianThreshold,nOctaves,nOctaveLayers,extended,upRight);
        findDetector=true;
    }
    if(detectorTag.compare(OPENCV_KPM_DETECTOR_BRISK_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_BRISK_OCTAVES_TAG;
        int parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int octaves=detectorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_BRISK_PATTERSCALE_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double patternScale=detectorParametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_BRISK_THRESH_TAG;
        parameterPos=detectorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDetector, keypoint detector %1 parameter % 2 not found").arg(detectorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int thresh=detectorParametersValue[parameterPos].toInt();

        (*ptrDetector)=new cv::BRISK(thresh,octaves,patternScale);
        findDetector=true;
    }
    if(!findDetector)
    {
        strError=QObject::tr("OpenCVKpM::getDetector, detector: %1 is not implemented").arg(detectorTag);
        return(OPENCV_KPM_ERROR);
    }
    #endif //USE_OPENCV
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getDescriptorParametersTagQParams(QString descriptorTag,
                                                 QStringList &parametersTagQParams,
                                                 QString &strError)
{
    if(OPENCV_KPM_NO_ERROR!=mParameters.getDescriptorParametersTagQParams(descriptorTag,
                                                                          parametersTagQParams,
                                                                          strError))
    {
        strError+=QObject::tr("OpenCVKpM::getDescriptorParametersTagQParams");
        return(OPENCV_KPM_ERROR);
    }
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getDetectorParametersTagQParams(QString detectorTag,
                                               QStringList &parametersTagQParams,
                                               QString& strError)
{
    if(OPENCV_KPM_NO_ERROR!=mParameters.getDetectorParametersTagQParams(detectorTag,
                                                                        parametersTagQParams,
                                                                        strError))
    {
        strError+=QObject::tr("OpenCVKpM::getDetectorParametersTagQParams");
        return(OPENCV_KPM_ERROR);
    }
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getDetectorsTag(QStringList &detectorsTag,
                               QString &strError)
{
    if(OPENCV_KPM_NO_ERROR!=mParameters.getDetectorsTag(detectorsTag,
                                                        strError))
    {
        strError+=QObject::tr("OpenCVKpM::getDetectorTags");
        return(OPENCV_KPM_ERROR);
    }
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getDescriptor(cv::DescriptorExtractor** ptrDescriptor,
                             QString& strError)
{
#ifdef USE_OPENCV
    if(*ptrDescriptor!=NULL)
        delete(*ptrDescriptor);
    *ptrDescriptor=NULL;
    QString descriptorTag;
    QStringList descriptorParametersTag;
    //QStringList detectorParametersTagQParams;
    QStringList descriptorParametersValue;
    QStringList descriptorParametersType;
    if(OPENCV_KPM_NO_ERROR!=mParameters.getDescriptor(descriptorTag,
                                                      descriptorParametersTag,
                                                      //detectorParametersTagQParams,
                                                      descriptorParametersValue,
                                                      descriptorParametersType,
                                                      strError))
    {
        strError="OpenCVKpM::getDescriptor,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    bool findDescritor=false;
    if(descriptorTag.compare(OPENCV_KPM_DETECTOR_FAST_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_BRIEF_BYTES_TAG;
        int parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int bytes=descriptorParametersValue[parameterPos].toInt();

        (*ptrDescriptor)=new cv::BriefDescriptorExtractor(bytes);
        findDescritor=true;
    }
    if(descriptorTag.compare(OPENCV_KPM_DETECTOR_ORB_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_ORB_WTA_K_TAG;
        int parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int wTA_K=descriptorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_ORB_EDGETHRESHOLD_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int edgeThreshold=descriptorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_ORB_FIRSTLEVEL_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int firstLevel=descriptorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_ORB_NFEATURES_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int nFeatures=descriptorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_ORB_NLEVELS_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int nLevels=descriptorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_ORB_PATCHSIZE_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int patchSize=descriptorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_ORB_SCALEFACTOR_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double scaleFactor=descriptorParametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_ORB_SCORETYPE_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int scoreType=descriptorParametersValue[parameterPos].toInt();

        (*ptrDescriptor)=new cv::ORB(nFeatures,scaleFactor,nLevels,edgeThreshold,firstLevel,wTA_K,scoreType,patchSize);
        findDescritor=true;
    }
    if(descriptorTag.compare(OPENCV_KPM_DETECTOR_SIFT_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_SIFT_CONTRASTTHRESHOLD_TAG;
        int parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double contrastThreshold=descriptorParametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_SIFT_EDGETHRESHOLD_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double edgeThreshold=descriptorParametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_SIFT_NOCTAVELAYERS_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int nOctaveLayers=descriptorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_SIFT_NFEATURES_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int nFeatures=descriptorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_SIFT_SIGMA_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double sigma=descriptorParametersValue[parameterPos].toDouble();

        (*ptrDescriptor)=new cv::SIFT(nFeatures,nOctaveLayers,contrastThreshold,edgeThreshold,sigma);
        findDescritor=true;
    }
    if(descriptorTag.compare(OPENCV_KPM_DETECTOR_SURF_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_SURF_EXTENDED_TAG;
        int parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int intExtended=descriptorParametersValue[parameterPos].toInt();
        bool extended=true;
        if(intExtended==0)
            extended=false;

        parameterTag=OPENCV_KPM_SURF_HESSIANTHRESHOLD_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double hessianThreshold=descriptorParametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_SURF_NOCTAVELAYERS_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int nOctaveLayers=descriptorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_SURF_NOCTAVES_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int nOctaves=descriptorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_SURF_UPRIGHT_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int intUpRight=descriptorParametersValue[parameterPos].toInt();
        bool upRight=true;
        if(intUpRight==0)
            upRight=false;

        (*ptrDescriptor)=new cv::SURF(hessianThreshold,nOctaves,nOctaveLayers,extended,upRight);
        findDescritor=true;
    }
    if(descriptorTag.compare(OPENCV_KPM_DETECTOR_BRISK_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_BRISK_OCTAVES_TAG;
        int parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int octaves=descriptorParametersValue[parameterPos].toInt();

        parameterTag=OPENCV_KPM_BRISK_PATTERSCALE_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        double patternScale=descriptorParametersValue[parameterPos].toDouble();

        parameterTag=OPENCV_KPM_BRISK_THRESH_TAG;
        parameterPos=descriptorParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getDescriptor, keypoint descriptor %1 parameter % 2 not found").arg(descriptorTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        int thresh=descriptorParametersValue[parameterPos].toInt();

        (*ptrDescriptor)=new cv::BRISK(thresh,octaves,patternScale);
        findDescritor=true;
    }

    if(!findDescritor)
    {
        strError=QObject::tr("OpenCVKpM::getDescriptor, descriptor: %1 is not implemented").arg(descriptorTag);
        return(OPENCV_KPM_ERROR);
    }
    #endif //USE_OPENCV
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getDescriptorsByDetector(QString &detectorTag,
                                        QString &defaultDescriptor,
                                        QStringList &descriptors,
                                        QString &strError)
{
    if(OPENCV_KPM_NO_ERROR!=mParameters.getDescriptorsByDetector(detectorTag,
                                                                 defaultDescriptor,
                                                                 descriptors,
                                                                 strError))
    {
        strError="OpenCVKpM::getDescriptorsByDetector,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getEngines(QStringList &engines,
                          QString &strError)
{
    if(mEngines.isEmpty())
    {
        strError=QObject::tr("OpenCVKpM::getEngines, there is not any engine loaded");
        return(OPENCV_KPM_ERROR);
    }
    engines=mEngines;
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getMatcher(cv::DescriptorMatcher **ptrMatcher,
                          int& kBestMatches,
                          double& ratioTestDistance,
                          QString &strError)
{
#ifdef USE_OPENCV
    if(*ptrMatcher!=NULL)
        delete(*ptrMatcher);
    *ptrMatcher=NULL;
    QString matcherTag;
    QStringList matcherParametersTag;
    QStringList matcherParametersValue;
    QStringList matcherParametersType;
    if(OPENCV_KPM_NO_ERROR!=mParameters.getMatcher(matcherTag,
                                                   matcherParametersTag,
                                                   matcherParametersValue,
                                                   matcherParametersType,
                                                   strError))
    {
        strError="OpenCVKpM::getMatcher,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    int normType=-1;
    if(OPENCV_KPM_NO_ERROR!=mParameters.getMatchingDistance(normType,
                                                            strError))
    {
        strError="OpenCVKpM::getMatcher,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    kBestMatches=0;
    ratioTestDistance=0.0;
    bool findMatcher=false;
    if(matcherTag.compare(OPENCV_KPM_MATCHER_BRUTEFORCE_TAG,Qt::CaseSensitive)==0)
    {
        QString parameterTag=OPENCV_KPM_BRUTEFORCE_CROSSCHECK_TAG;
        int parameterPos=matcherParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::getMacher, matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        bool crossCheck=false;
        int intValue=matcherParametersValue[parameterPos].toInt();
        if(intValue==1)
        {
            crossCheck=true;
        }

//        parameterTag=OPENCV_KPM_BRUTEFORCE_KBESTMATCHES_TAG;
//        parameterPos=matcherParametersTag.indexOf(parameterTag);
//        if(parameterPos==-1)
//        {
//            strError=QObject::tr("OpenCVKpM::getMacher, matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
//            return(OPENCV_KPM_ERROR);
//        }
//        kBestMatches=matcherParametersValue[parameterPos].toInt();

//        parameterTag=OPENCV_KPM_BRUTEFORCE_RATIOTESTDISTANCE_TAG;
//        parameterPos=matcherParametersTag.indexOf(parameterTag);
//        if(parameterPos==-1)
//        {
//            strError=QObject::tr("OpenCVKpM::getMacher, matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
//            return(OPENCV_KPM_ERROR);
//        }
//        ratioTestDistance=matcherParametersValue[parameterPos].toDouble();
        *ptrMatcher=new cv:: BFMatcher(normType,crossCheck);
        //new cv::SiftDescriptorExtractor(nFeatures,nOctaveLayers,contrastThreshold,edgeThreshold,sigma);
        findMatcher=true;
    }
    if(!findMatcher)
    {
        strError=QObject::tr("OpenCVKpM::getMacher, descriptor: %1 is not implemented").arg(matcherTag);
        return(OPENCV_KPM_ERROR);
    }
#endif
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getMatchersByDetectorAndDescriptor(QString &detectorTag,
                                                  QString &descriptorTag,
                                                  QString &defaultMatcher,
                                                  QStringList &matchers,
                                                  QString &strError)
{
    if(OPENCV_KPM_NO_ERROR!=mParameters.getMatchersByDetectorAndDescriptor(detectorTag,
                                                                           descriptorTag,
                                                                           defaultMatcher,
                                                                           matchers,
                                                                           strError))
    {
        strError="OpenCVKpM::getMatchersByDetectorAndDescriptor,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getMatcherParametersTagQParams(QString matcherTag,
                                              QStringList &matchersTagQParams,
                                              QString &strError)
{
    if(OPENCV_KPM_NO_ERROR!=mParameters.getMatcherParametersTagQParams(matcherTag,
                                                                       matchersTagQParams,
                                                                       strError))
    {
        strError+=QObject::tr("OpenCVKpM::getMatcherParametersTagQParams");
        return(OPENCV_KPM_ERROR);
    }
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getNumberOfImages(int& numberOfImages,
                                 QString& strError)
{
    numberOfImages=mImageFileNames.size();
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getNumberOfImagePairs(int& numberOfImagePairs,
                                     QString& strError)
{
    if(mNumberOfImagePairs==0&&mImagesPairs.size()>0)
    {
        QMap<QString,QVector<QString> >::const_iterator iter=mImagesPairs.constBegin();
        while(iter!=mImagesPairs.constEnd())
        {
            mNumberOfImagePairs+=iter.value().size();
            iter++;
        }
    }
    numberOfImagePairs=mNumberOfImagePairs;
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getNumberOfImagePairsProcessed(int& numberOfImagePairsProcessed,
                                              QString& strError)
{
    if(mNumberOfImagePairsProcessed==0&&mImagesPairs.size()>0)
    {
        QMap<QString,QVector<std::vector<cv::Point2f> > >::const_iterator iter=mImagesPairsMatchesPointsFirsts.constBegin();
        while(iter!=mImagesPairsMatchesPointsFirsts.constEnd())
        {
            mNumberOfImagePairsProcessed+=iter.value().size();
            iter++;
        }
    }
    numberOfImagePairsProcessed=mNumberOfImagePairsProcessed;
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getOutliersDetectionAzimuthParametersTag(QStringList &parametersTag,
                                                        QString &strError)
{
    parametersTag.clear();
    parametersTag<<OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHNUMBEROFCLUSTERS_QPARAMS_TAG;
    parametersTag<<OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHEPSILONDEG_QPARAMS_TAG;
    parametersTag<<OPENCV_KPM_OUTLIERS_DHL_KMEANS_AZIMUTHITERATIONS_QPARAMS_TAG;
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::getOutliersDetectionDistanceParametersTag(QStringList &parametersTag,
                                                         QString &strError)
{
    parametersTag.clear();
    parametersTag<<OPENCV_KPM_OUTLIERS_DHL_KMEANS_DISTANCENUMBEROFCLUSTERS_QPARAMS_TAG;
    parametersTag<<OPENCV_KPM_OUTLIERS_DHL_KMEANS_DISTANCEEPSILON_QPARAMS_TAG;
    parametersTag<<OPENCV_KPM_OUTLIERS_DHL_KMEANS_DISTANCEITERATIONS_QPARAMS_TAG;
    return(OPENCV_KPM_NO_ERROR);
}

void OpenCVKpM::initialize()
{
    mImageFileNames.clear();
    mImageWidths.clear();
    mImageHeights.clear();
    mImagesPairs.clear();
    mImagesKeyPoints.clear();
    mImagesKeyPointsDescriptors.clear();
    mImages.clear();
    mEngines.clear();
    mEngines<<OPENCV_KPM_SIFT_SIFT_BRUTEFORCE_ENGINE_TAG;
}

int OpenCVKpM::keyPointsDetections(QString& strError)
{
    mImages.clear();
    mImagesKeyPoints.clear();
    mImagesKeyPointsDescriptors.clear();
    QTextStream qout(stdout);
    cv::FeatureDetector* ptrDetector=NULL;
    if(OPENCV_KPM_NO_ERROR!=getDetector(&ptrDetector,strError))
    {
        strError="OpenCVKpM::keyPointsDetections,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    if(ptrDetector==NULL)
    {
        strError="OpenCVKpM::keyPointsDetections, detector null";
        return(OPENCV_KPM_ERROR);
    }
    cv::DescriptorExtractor *ptrDescriptor=NULL;
//    cv::Feature2D* ptrDescriptor=NULL;
    if(OPENCV_KPM_NO_ERROR!=getDescriptor(&ptrDescriptor,strError))
    {
        strError="OpenCVKpM::keyPointsDetections,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    if(ptrDescriptor==NULL)
    {
        strError="OpenCVKpM::keyPointsDetections, descriptor null";
        return(OPENCV_KPM_ERROR);
    }
    QMap<QString,QString>::const_iterator iterImages = mImageFileNames.constBegin();
    while (iterImages != mImageFileNames.constEnd())
    {
        QString imageId=iterImages.key();
        QString imageFileName=iterImages.value();
        if(!QFile::exists(imageFileName))
        {
            strError=QObject::tr("OpenCVKpM::keyPointsDetections, not exists image file: \n %1").arg(imageFileName);
            return(OPENCV_KPM_ERROR);
        }
        QFileInfo imageFileInfo(imageFileName);
        QString imageFileNamePng=imageFileInfo.absolutePath()+"/"+imageFileInfo.completeBaseName()+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG;
        if(!QFile::exists(imageFileNamePng))
        {
            strError=QObject::tr("OpenCVKpM::keyPointsDetections, not exists format png for image file: \n %1").arg(imageFileName);
            return(OPENCV_KPM_ERROR);
        }
        std::string imageFileNamePngStdString=imageFileNamePng.toStdString();
        cv::Mat openCVImg;
        QString msg=QObject::tr("          ... Image id: %1\n").arg(imageId);
        msg+=QObject::tr("                        ... Reading");
        qout<<msg<<endl;
        try
        {
            openCVImg = cv::imread(imageFileNamePngStdString, CV_LOAD_IMAGE_GRAYSCALE);
            if(!mImageWidths.contains(imageId))
            {
                mImageWidths[imageId]=openCVImg.cols;
                mImageHeights[imageId]=openCVImg.rows;
            }
        }
        catch( cv::Exception& e )
        {
            const char* err_msg = e.what();
            strError=QObject::tr("OpenCVKpM::keyPointsDetections, OpenCV exception caught: %1").arg(err_msg);
            return(OPENCV_KPM_ERROR);
        }
        mImages[imageId]=openCVImg;
        std::vector<cv::KeyPoint> imageKeyPoints;
        msg=QObject::tr("                        ... Detecting key points,  ");
        qout<<msg;
        try
        {
            ptrDetector->detect(openCVImg,imageKeyPoints);
        }
        catch( cv::Exception& e )
        {
            const char* err_msg = e.what();
            strError=QObject::tr("OpenCVKpM::keyPointsDetections, OpenCV exception caught: %1").arg(err_msg);
            return(OPENCV_KPM_ERROR);
        }
        int numberOfImageKeyPoints=imageKeyPoints.size();
        msg=QObject::tr("resulting %1 ").arg(numberOfImageKeyPoints);
        qout<<msg<<endl;
        if(numberOfImageKeyPoints==0)
        {
            strError=QObject::tr("OpenCVKpM::keyPointsDetections, not key points for image file: \n %1").arg(imageFileName);
            return(OPENCV_KPM_ERROR);
        }

        cv::Mat imageKeyPointsDescriptors;
        msg=QObject::tr("                        ... Getting descriptors");
        qout<<msg<<endl;
        try
        {
            ptrDescriptor->compute(openCVImg,imageKeyPoints,imageKeyPointsDescriptors);
        }
        catch( cv::Exception& e )
        {
            const char* err_msg = e.what();
            strError=QObject::tr("OpenCVKpM::keyPointsDetections, OpenCV exception caught: %1").arg(err_msg);
            return(OPENCV_KPM_ERROR);
        }
        if(imageKeyPointsDescriptors.type()==CV_8U)
            mDescriptorIsBinary=true;
        else
            mDescriptorIsBinary=false;
        mImagesKeyPoints[imageId]=imageKeyPoints;
        mImagesKeyPointsDescriptors[imageId]=imageKeyPointsDescriptors;
        ++iterImages;
    }
    if(ptrDetector!=NULL)
        delete(ptrDetector);
    if(ptrDescriptor!=NULL)
        delete(ptrDescriptor);
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::matching(QString &strError)
{
#ifdef USE_OPENCV
    mImagesPairsMatches.clear();
    mImagesPairsMatchesPointsFirsts.clear();
    mImagesPairsMatchesPointsSeconds.clear();
    mImagesPairsMatchesIndexesFirsts.clear();
    mImagesPairsMatchesIndexesSeconds.clear();
    mImagesPairsMatchesReverse.clear();
    mImagesPairsMatchesPointsFirstsReverse.clear();
    mImagesPairsMatchesPointsSecondsReverse.clear();
    mImagesPairsMatchesIndexesFirstsReverse.clear();
    mImagesPairsMatchesIndexesSecondsReverse.clear();
//    mImagesPairsMatchesIndexesFirsts.clear();
//    mImagesPairsMatchesIndexesSeconds.clear();
    bool isMatcherBruteForce=false;
    QString matcherTag;
    QStringList matcherParametersTag;
    QStringList matcherParametersValue;
    QStringList matcherParametersType;
    if(OPENCV_KPM_NO_ERROR!=mParameters.getMatcher(matcherTag,
                                                   matcherParametersTag,
                                                   matcherParametersValue,
                                                   matcherParametersType,
                                                   strError))
    {
        strError="OpenCVKpM::matching,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    int normType=-1;
    cvflann::flann_distance_t flannDistance;
    if(matcherTag.compare(OPENCV_KPM_MATCHER_BRUTEFORCE_TAG,Qt::CaseSensitive)==0)
    {
        isMatcherBruteForce=true;
        if(mDescriptorIsBinary)
            normType=cv::NORM_HAMMING;
        else
            normType=cv::NORM_L2;
    }
    else
    {
        if(mDescriptorIsBinary)
        {
            normType=cvflann::FLANN_DIST_HAMMING;
            flannDistance=cvflann::FLANN_DIST_HAMMING;
        }
        else
        {
            normType=cvflann::FLANN_DIST_EUCLIDEAN;
            flannDistance=cvflann::FLANN_DIST_EUCLIDEAN;
        }
    }
    int kBestMatches=-1;
    bool applyRatioTestDistance=false;
    double ratioTestDistance=-1.0;
    bool applyMinimumTestDistance=false;
    double minimumTestDistance=-1.0;
    {
        QString parameterGroup=OPENCV_KPM_TAG_MATCHER;
        QString parameterTag=OPENCV_KPM_MATCHER_KBESTMATCHES_TAG;
        QString strParameterValue;
        if(OPENCV_KPM_NO_ERROR!=mParameters.getParameterByGroupAndTag(parameterGroup,parameterTag,strParameterValue,strError))
        {
            strError="OpenCVKpM::matching,"+strError;
            return(OPENCV_KPM_ERROR);
        }
        bool okToInt=false;
        int intValue=strParameterValue.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("parameter %1 is not an integer").arg(strParameterValue);
            strError="OpenCVKpM::matching,"+strError;
            return(OPENCV_KPM_ERROR);
        }
        kBestMatches=intValue;

        parameterTag=OPENCV_KPM_MATCHER_APPLYRATIOTESTDISTANCE_TAG;
        strParameterValue="";
        if(OPENCV_KPM_NO_ERROR!=mParameters.getParameterByGroupAndTag(parameterGroup,parameterTag,strParameterValue,strError))
        {
            strError="OpenCVKpM::matching,"+strError;
            return(OPENCV_KPM_ERROR);
        }
        okToInt=false;
        intValue=strParameterValue.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("parameter %1 is not an integer").arg(strParameterValue);
            strError="OpenCVKpM::matching,"+strError;
            return(OPENCV_KPM_ERROR);
        }
        if(intValue<0||intValue>1)
        {
            strError=QObject::tr("parameter %1 is not a bool").arg(strParameterValue);
            strError="OpenCVKpM::matching,"+strError;
            return(OPENCV_KPM_ERROR);
        }
        applyRatioTestDistance=false;
        if(intValue==1)
            applyRatioTestDistance=true;

        parameterTag=OPENCV_KPM_MATCHER_RATIOTESTDISTANCE_TAG;
        strParameterValue="";
        if(OPENCV_KPM_NO_ERROR!=mParameters.getParameterByGroupAndTag(parameterGroup,parameterTag,strParameterValue,strError))
        {
            strError="OpenCVKpM::matching,"+strError;
            return(OPENCV_KPM_ERROR);
        }
        bool okToDouble=false;
        double dblValue=strParameterValue.toDouble(&okToDouble);
        if(!okToDouble)
        {
            strError=QObject::tr("parameter %1 is not a double").arg(strParameterValue);
            strError="OpenCVKpM::matching,"+strError;
            return(OPENCV_KPM_ERROR);
        }
        ratioTestDistance=dblValue;

        parameterTag=OPENCV_KPM_MATCHER_APPLYMINIMUMTESTDISTANCE_TAG;
        strParameterValue="";
        if(OPENCV_KPM_NO_ERROR!=mParameters.getParameterByGroupAndTag(parameterGroup,parameterTag,strParameterValue,strError))
        {
            strError="OpenCVKpM::matching,"+strError;
            return(OPENCV_KPM_ERROR);
        }
        okToInt=false;
        intValue=strParameterValue.toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("parameter %1 is not an integer").arg(strParameterValue);
            strError="OpenCVKpM::matching,"+strError;
            return(OPENCV_KPM_ERROR);
        }
        if(intValue<0||intValue>1)
        {
            strError=QObject::tr("parameter %1 is not a bool").arg(strParameterValue);
            strError="OpenCVKpM::matching,"+strError;
            return(OPENCV_KPM_ERROR);
        }
        applyMinimumTestDistance=false;
        if(intValue==1)
            applyMinimumTestDistance=true;

        parameterTag=OPENCV_KPM_MATCHER_MINIMUMTESTDISTANCE_TAG;
        strParameterValue="";
        if(OPENCV_KPM_NO_ERROR!=mParameters.getParameterByGroupAndTag(parameterGroup,parameterTag,strParameterValue,strError))
        {
            strError="OpenCVKpM::matching,"+strError;
            return(OPENCV_KPM_ERROR);
        }
        okToDouble=false;
        dblValue=strParameterValue.toDouble(&okToDouble);
        if(!okToDouble)
        {
            strError=QObject::tr("parameter %1 is not a double").arg(strParameterValue);
            strError="OpenCVKpM::matching,"+strError;
            return(OPENCV_KPM_ERROR);
        }
        minimumTestDistance=dblValue;
    }
    QTextStream qout(stdout);
    QMap<QString,QVector<QString> >::const_iterator iterImagePairs = mImagesPairs.constBegin();
    QString pathForMatchesFiles;
    bool saveImagesMatches=mParameters.getSaveImagesMatches(pathForMatchesFiles);
    bool bidirectionalMatches=mParameters.getBidirectionalMatches();
    if(isMatcherBruteForce)
    {
        QString parameterTag=OPENCV_KPM_BRUTEFORCE_CROSSCHECK_TAG;
        int parameterPos=matcherParametersTag.indexOf(parameterTag);
        if(parameterPos==-1)
        {
            strError=QObject::tr("OpenCVKpM::matching, matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
            return(OPENCV_KPM_ERROR);
        }
        bool crossCheck=false;
        bool okToInt=false;
        int intValue=matcherParametersValue[parameterPos].toInt(&okToInt);
        if(!okToInt)
        {
            strError=QObject::tr("parameter %1 is not an integer").arg(matcherParametersValue[parameterPos]);
            strError="OpenCVKpM::matching,"+strError;
            return(OPENCV_KPM_ERROR);
        }
        if(intValue==1)
        {
            crossCheck=true;
        }
        cv::BFMatcher matcher(normType,crossCheck);
        while(iterImagePairs!=mImagesPairs.constEnd())
        {
            QString imageId=iterImagePairs.key();
            if(!mImagesKeyPoints.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::matching, not key points for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            std::vector<cv::KeyPoint> imageKeyPoints=mImagesKeyPoints[imageId];
            if(!mImagesKeyPointsDescriptors.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::matching, not key points descriptors for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            cv::Mat imageKeyPointsDescriptors=mImagesKeyPointsDescriptors[imageId];
            QVector<QString> imagePairs=iterImagePairs.value();
            for(int nImageInPairs=0;nImageInPairs<imagePairs.size();nImageInPairs++)
            {
                QString imagePairId=imagePairs.at(nImageInPairs);
                if(!mImagesKeyPoints.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::matching, not key points for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::KeyPoint> imagePairKeyPoints=mImagesKeyPoints[imagePairId];
                if(!mImagesKeyPointsDescriptors.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::matching, not key points descriptors for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                cv::Mat imagePairKeyPointsDescriptors=mImagesKeyPointsDescriptors[imagePairId];
                std::vector<cv::DMatch> matches;
                std::vector<std::vector<cv::DMatch> > multiMatches;
                for(int kk=0;kk<imageKeyPointsDescriptors.rows;kk++)
                {
                    std::vector<cv::DMatch> auxMatches;
                    multiMatches.push_back(auxMatches);
                }
                QString msg=QObject::tr("          ... Matching pair: %1 - %2, ").arg(imageId).arg(imagePairId);
                qout<<msg;
                try
                {
                    if(kBestMatches>1)
                    {
                        matcher.knnMatch(imageKeyPointsDescriptors, imagePairKeyPointsDescriptors, multiMatches,kBestMatches);
                    }
                    else
                    {
                        matcher.match(imageKeyPointsDescriptors, imagePairKeyPointsDescriptors, matches);
                    }
                }
                catch( cv::Exception& e )
                {
                    const char* err_msg = e.what();
                    strError=QObject::tr("OpenCVKpM::matching, OpenCV exception caught: %1").arg(err_msg);
                    return(OPENCV_KPM_ERROR);
                }
                if(kBestMatches>1)  // solo puede ser 1 o 2
                                    // si applyRatioTestDistance, valido cada uno con el siguiente, no añado el último
                                    // si no applyRatioTestDistance los añado todos
                {
                    int size1=multiMatches.size();
                    for(int ii=0;ii<multiMatches.size();ii++)
                    {
                        std::vector<cv::DMatch> auxMatches=multiMatches[ii];
                        if(applyRatioTestDistance)
                        {
                            if(auxMatches.at(0).distance <= ratioTestDistance *auxMatches.at(1).distance)
                                matches.push_back(auxMatches.at(0));
                        }
                        else
                            matches.push_back(auxMatches.at(0));
                    }
                }
                std::vector<cv::Point2f> imageMatchingPoints, imagePairMatchingPoints; // Used for homography
                std::vector<int> imageMatchingIndexes,imagePairMatchingIndexes; // Used for homography
                for(unsigned int ii=0; ii<matches.size(); ++ii) //
                {
                    if(applyMinimumTestDistance)
                    {
                        if(matches.at(ii).distance > minimumTestDistance)
                            continue;
                    }
                    cv::DMatch match=matches.at(ii);
                    imageMatchingPoints.push_back(imageKeyPoints.at(matches.at(ii).queryIdx).pt);
                    imageMatchingIndexes.push_back(matches.at(ii).queryIdx);
                    imagePairMatchingPoints.push_back(imagePairKeyPoints.at(matches.at(ii).trainIdx).pt);
                    imagePairMatchingIndexes.push_back(matches.at(ii).trainIdx);
                }
                if(!mImagesPairsMatches.contains(imageId))
                {
                    QVector<std::vector<cv::DMatch> > imageMatches;
                    imageMatches.push_back(matches);
                    mImagesPairsMatches[imageId]=imageMatches;
                }
                else
                {
                    mImagesPairsMatches[imageId].push_back(matches);
                }
                if(!mImagesPairsMatchesPointsFirsts.contains(imageId))
                {
                    QVector<std::vector<cv::Point2f> > imageMatchesPointsFirsts;
                    imageMatchesPointsFirsts.push_back(imageMatchingPoints);
                    mImagesPairsMatchesPointsFirsts[imageId]=imageMatchesPointsFirsts;
                    QVector<std::vector<cv::Point2f> > imageMatchesPointsSeconds;
                    imageMatchesPointsSeconds.push_back(imagePairMatchingPoints);
                    mImagesPairsMatchesPointsSeconds[imageId]=imageMatchesPointsSeconds;
                    QVector<std::vector<int> > imageMatchesIndexesFirsts;
                    imageMatchesIndexesFirsts.push_back(imageMatchingIndexes);
                    mImagesPairsMatchesIndexesFirsts[imageId]=imageMatchesIndexesFirsts;
                    QVector<std::vector<int> > imageMatchesIndexesSeconds;
                    imageMatchesIndexesSeconds.push_back(imagePairMatchingIndexes);
                    mImagesPairsMatchesIndexesSeconds[imageId]=imageMatchesIndexesSeconds;
                }
                else
                {
                    mImagesPairsMatchesPointsFirsts[imageId].push_back(imageMatchingPoints);
                    mImagesPairsMatchesPointsSeconds[imageId].push_back(imagePairMatchingPoints);
                    mImagesPairsMatchesIndexesFirsts[imageId].push_back(imageMatchingIndexes);
                    mImagesPairsMatchesIndexesSeconds[imageId].push_back(imagePairMatchingIndexes);
                }
                int numberOfMatches=imageMatchingPoints.size();
                if(numberOfMatches>0&&saveImagesMatches)
                {
                    QString imageFileName=mImageFileNames[imageId];
                    QString imagePairFileName=mImageFileNames[imagePairId];
                    QFileInfo fileInfoImage(imageFileName);
                    QString imageBaseName=fileInfoImage.baseName();
                    QFileInfo fileInfoImagePair(imagePairFileName);
                    QString imagePairBaseName=fileInfoImagePair.baseName();
                    QString absoluteFilePath=fileInfoImage.absolutePath();
                    QDir auxDir(absoluteFilePath);
                    QString imageMatchesDir=pathForMatchesFiles;
                    if(!auxDir.exists(imageMatchesDir))
                    {
                        if(!auxDir.mkdir(imageMatchesDir))
                        {
                            strError=QObject::tr("OpenCVKpM::matching, error creating directory: %1").arg(imageMatchesDir);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                    QString imageMatchesFileName=imageMatchesDir+"/";
                    imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                    if(QFile::exists(imageMatchesFileName))
                        QFile::remove(imageMatchesFileName);
                    std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                    cv::Mat imageMatches;
                    cv::Mat image=mImages[imageId];
                    cv::Mat imagePair=mImages[imagePairId];
                    try
                    {
                        cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matches, imageMatches);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::matching, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                    try
                    {
                        cv::imwrite(imageMatchesFileNameStdString,imageMatches);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::matching, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                msg=QObject::tr("resulting %1 ").arg(numberOfMatches);
                qout<<msg<<endl;
                if(bidirectionalMatches)
                {
                    std::vector<cv::DMatch> matchesReverse;
                    std::vector<std::vector<cv::DMatch> > multiMatchesReverse;
                    for(int kk=0;kk<imagePairKeyPointsDescriptors.rows;kk++)
                    {
                        std::vector<cv::DMatch> auxMatches;
                        multiMatchesReverse.push_back(auxMatches);
                    }
                    QString msg=QObject::tr("          ... Matching pair: %1 - %2, ").arg(imagePairId).arg(imageId);
                    qout<<msg;
                    try
                    {
                        if(kBestMatches>1)
                        {
                            matcher.knnMatch(imagePairKeyPointsDescriptors, imageKeyPointsDescriptors, multiMatchesReverse,kBestMatches);
                        }
                        else
                        {
                            matcher.match(imagePairKeyPointsDescriptors, imageKeyPointsDescriptors, matchesReverse);
                        }
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::matching, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                    if(kBestMatches>1)  // solo puede ser 1 o 2
                                        // si applyRatioTestDistance, valido cada uno con el siguiente, no añado el último
                                        // si no applyRatioTestDistance los añado todos
                    {
                        int size1=multiMatchesReverse.size();
                        for(int ii=0;ii<multiMatchesReverse.size();ii++)
                        {
                            std::vector<cv::DMatch> auxMatches=multiMatchesReverse[ii];
                            if(applyRatioTestDistance)
                            {
                                if(auxMatches.at(0).distance <= ratioTestDistance *auxMatches.at(1).distance)
                                    matchesReverse.push_back(auxMatches.at(0));
                            }
                            else
                                matchesReverse.push_back(auxMatches.at(0));
                        }
                    }
                    std::vector<cv::Point2f> imageMatchingPoints, imagePairMatchingPoints; // Used for homography
                    std::vector<int> imageMatchingIndexes,imagePairMatchingIndexes; // Used for homography
                    for(unsigned int ii=0; ii<matchesReverse.size(); ++ii) //
                    {
                        if(applyMinimumTestDistance)
                        {
                            if(matchesReverse.at(ii).distance > minimumTestDistance)
                                continue;
                        }
                        cv::DMatch match=matchesReverse.at(ii);
                        imageMatchingPoints.push_back(imageKeyPoints.at(matchesReverse.at(ii).trainIdx).pt);
                        imageMatchingIndexes.push_back(matchesReverse.at(ii).trainIdx);
                        imagePairMatchingPoints.push_back(imagePairKeyPoints.at(matchesReverse.at(ii).queryIdx).pt);
                        imagePairMatchingIndexes.push_back(matchesReverse.at(ii).queryIdx);
                    }
                    if(!mImagesPairsMatchesReverse.contains(imageId))
                    {
                        QVector<std::vector<cv::DMatch> > imageMatches;
                        imageMatches.push_back(matchesReverse);
                        mImagesPairsMatchesReverse[imageId]=imageMatches;
                    }
                    else
                    {
                        mImagesPairsMatchesReverse[imageId].push_back(matchesReverse);
                    }
                    if(!mImagesPairsMatchesPointsFirstsReverse.contains(imageId))
                    {
//                        QVector<std::vector<cv::Point2f> > imageMatchesPointsFirsts;
//                        imageMatchesPointsFirsts.push_back(imageMatchingPoints);
//                        mImagesPairsMatchesPointsFirstsReverse[imageId]=imageMatchesPointsFirsts;
//                        QVector<std::vector<cv::Point2f> > imageMatchesPointsSeconds;
//                        imageMatchesPointsSeconds.push_back(imagePairMatchingPoints);
//                        mImagesPairsMatchesPointsSecondsReverse[imageId]=imageMatchesPointsSeconds;
//                        QVector<std::vector<int> > imageMatchesIndexesFirsts;
//                        imageMatchesIndexesFirsts.push_back(imageMatchingIndexes);
//                        mImagesPairsMatchesIndexesFirstsReverse[imageId]=imageMatchesIndexesFirsts;
//                        QVector<std::vector<int> > imageMatchesIndexesSeconds;
//                        imageMatchesIndexesSeconds.push_back(imagePairMatchingIndexes);
//                        mImagesPairsMatchesIndexesSecondsReverse[imageId]=imageMatchesIndexesSeconds;
                        QVector<std::vector<cv::Point2f> > imageMatchesPointsFirsts;
                        imageMatchesPointsFirsts.push_back(imagePairMatchingPoints);
                        mImagesPairsMatchesPointsFirstsReverse[imageId]=imageMatchesPointsFirsts;
                        QVector<std::vector<cv::Point2f> > imageMatchesPointsSeconds;
                        imageMatchesPointsSeconds.push_back(imageMatchingPoints);
                        mImagesPairsMatchesPointsSecondsReverse[imageId]=imageMatchesPointsSeconds;
                        QVector<std::vector<int> > imageMatchesIndexesFirsts;
                        imageMatchesIndexesFirsts.push_back(imagePairMatchingIndexes);
                        mImagesPairsMatchesIndexesFirstsReverse[imageId]=imageMatchesIndexesFirsts;
                        QVector<std::vector<int> > imageMatchesIndexesSeconds;
                        imageMatchesIndexesSeconds.push_back(imageMatchingIndexes);
                        mImagesPairsMatchesIndexesSecondsReverse[imageId]=imageMatchesIndexesSeconds;
                    }
                    else
                    {
//                        mImagesPairsMatchesPointsFirstsReverse[imageId].push_back(imageMatchingPoints);
//                        mImagesPairsMatchesPointsSecondsReverse[imageId].push_back(imagePairMatchingPoints);
//                        mImagesPairsMatchesIndexesFirstsReverse[imageId].push_back(imageMatchingIndexes);
//                        mImagesPairsMatchesIndexesSecondsReverse[imageId].push_back(imagePairMatchingIndexes);
                        mImagesPairsMatchesPointsFirstsReverse[imageId].push_back(imagePairMatchingPoints);
                        mImagesPairsMatchesPointsSecondsReverse[imageId].push_back(imageMatchingPoints);
                        mImagesPairsMatchesIndexesFirstsReverse[imageId].push_back(imagePairMatchingIndexes);
                        mImagesPairsMatchesIndexesSecondsReverse[imageId].push_back(imageMatchingIndexes);
                    }
                    int numberOfMatches=imagePairMatchingPoints.size();
                    if(numberOfMatches>0&&saveImagesMatches)
                    {
                        QString imageFileName=mImageFileNames[imageId];
                        QString imagePairFileName=mImageFileNames[imagePairId];
                        QFileInfo fileInfoImage(imageFileName);
                        QString imageBaseName=fileInfoImage.baseName();
                        QFileInfo fileInfoImagePair(imagePairFileName);
                        QString imagePairBaseName=fileInfoImagePair.baseName();
                        QString absoluteFilePath=fileInfoImage.absolutePath();
                        QDir auxDir(absoluteFilePath);
                        QString imageMatchesDir=pathForMatchesFiles;
                        if(!auxDir.exists(imageMatchesDir))
                        {
                            if(!auxDir.mkdir(imageMatchesDir))
                            {
                                strError=QObject::tr("OpenCVKpM::matching, error creating directory: %1").arg(imageMatchesDir);
                                return(OPENCV_KPM_ERROR);
                            }
                        }
                        QString imageMatchesFileName=imageMatchesDir+"/";
//                        imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                        imageMatchesFileName+=(imagePairBaseName+"_"+imageBaseName+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                        if(QFile::exists(imageMatchesFileName))
                            QFile::remove(imageMatchesFileName);
                        std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                        cv::Mat imageMatchesReverse;
                        cv::Mat image=mImages[imageId];
                        cv::Mat imagePair=mImages[imagePairId];
                        try
                        {
                            cv::drawMatches( imagePair, imagePairKeyPoints,image, imageKeyPoints, matchesReverse, imageMatchesReverse);
                        }
                        catch( cv::Exception& e )
                        {
                            const char* err_msg = e.what();
                            strError=QObject::tr("OpenCVKpM::matching, OpenCV exception caught: %1").arg(err_msg);
                            return(OPENCV_KPM_ERROR);
                        }
                        try
                        {
                            cv::imwrite(imageMatchesFileNameStdString,imageMatchesReverse);
                        }
                        catch( cv::Exception& e )
                        {
                            const char* err_msg = e.what();
                            strError=QObject::tr("OpenCVKpM::matching, OpenCV exception caught: %1").arg(err_msg);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                    msg=QObject::tr("resulting %1 ").arg(numberOfMatches);
                    qout<<msg<<endl;
                }
            }
            iterImagePairs++;
        }
    }
    else
    {
        cv::flann::IndexParams * ptrFlannIndexParams = NULL;
        bool findFlannMatcher=false;
        if(matcherTag.compare(OPENCV_KPM_MATCHER_FLANN_LINEAR_TAG,Qt::CaseSensitive)==0)
        {
            ptrFlannIndexParams=new cv::flann::LinearIndexParams();
            findFlannMatcher=true;
        }
        if(matcherTag.compare(OPENCV_KPM_MATCHER_FLANN_KDTREE_TAG,Qt::CaseSensitive)==0)
        {
            QString parameterTag=OPENCV_KPM_FLANN_KDTREE_TREES_TAG;
            int parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            int trees=matcherParametersValue[parameterPos].toInt();

            ptrFlannIndexParams=new cv::flann::KDTreeIndexParams(trees);
            findFlannMatcher=true;
        }
        if(matcherTag.compare(OPENCV_KPM_MATCHER_FLANN_KMEANS_TAG,Qt::CaseSensitive)==0)
        {
            QString parameterTag=OPENCV_KPM_FLANN_KMEANS_BRANCHING_TAG;
            int parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            int branching=matcherParametersValue[parameterPos].toInt();

            parameterTag=OPENCV_KPM_FLANN_KMEANS_CBINDEX_TAG;
            parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            double cbIndex=matcherParametersValue[parameterPos].toInt();

            parameterTag=OPENCV_KPM_FLANN_KMEANS_CENTERSINIT_TAG;
            parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            int centersInit=matcherParametersValue[parameterPos].toInt();
            cvflann::flann_centers_init_t centers_init;
            if(centersInit==1)
            {
                centers_init=cvflann::FLANN_CENTERS_RANDOM;
            }
            else if(centersInit==2)
            {
                centers_init=cvflann::FLANN_CENTERS_GONZALES;
            }
            else if(centersInit==3)
            {
                centers_init=cvflann::FLANN_CENTERS_KMEANSPP;
            }

            parameterTag=OPENCV_KPM_FLANN_KMEANS_ITERATIONS_TAG;
            parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            int iterations=matcherParametersValue[parameterPos].toInt();

            ptrFlannIndexParams=new cv::flann::KMeansIndexParams(branching,iterations,centers_init,cbIndex);
            findFlannMatcher=true;
        }
        if(matcherTag.compare(OPENCV_KPM_MATCHER_FLANN_COMPOSITE_TAG,Qt::CaseSensitive)==0)
        {
            QString parameterTag=OPENCV_KPM_FLANN_COMPOSITE_BRANCHING_TAG;
            int parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            int branching=matcherParametersValue[parameterPos].toInt();

            parameterTag=OPENCV_KPM_FLANN_COMPOSITE_CBINDEX_TAG;
            parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            double cbIndex=matcherParametersValue[parameterPos].toInt();

            parameterTag=OPENCV_KPM_FLANN_COMPOSITE_CENTERSINIT_TAG;
            parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            int centersInit=matcherParametersValue[parameterPos].toInt();
            cvflann::flann_centers_init_t centers_init;
            if(centersInit==1)
            {
                centers_init=cvflann::FLANN_CENTERS_RANDOM;
            }
            else if(centersInit==2)
            {
                centers_init=cvflann::FLANN_CENTERS_GONZALES;
            }
            else if(centersInit==3)
            {
                centers_init=cvflann::FLANN_CENTERS_KMEANSPP;
            }

            parameterTag=OPENCV_KPM_FLANN_COMPOSITE_ITERATIONS_TAG;
            parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            int iterations=matcherParametersValue[parameterPos].toInt();

            parameterTag=OPENCV_KPM_FLANN_COMPOSITE_TREES_TAG;
            parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            int trees=matcherParametersValue[parameterPos].toInt();

            ptrFlannIndexParams=new cv::flann::CompositeIndexParams(trees,branching,iterations,centers_init,cbIndex);
            findFlannMatcher=true;
        }
        if(matcherTag.compare(OPENCV_KPM_MATCHER_FLANN_AUTOTUNED_TAG,Qt::CaseSensitive)==0)
        {
            QString parameterTag=OPENCV_KPM_FLANN_AUTOTUNED_BUILDWEIGHT_TAG;
            int parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            double buildWeight=matcherParametersValue[parameterPos].toInt();

            parameterTag=OPENCV_KPM_FLANN_AUTOTUNED_MEMORYWEIGHT_TAG;
            parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            double memoryWeight=matcherParametersValue[parameterPos].toInt();

            parameterTag=OPENCV_KPM_FLANN_AUTOTUNED_SAMPLEFRACTION_TAG;
            parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            double sampleFraction=matcherParametersValue[parameterPos].toInt();

            parameterTag=OPENCV_KPM_FLANN_AUTOTUNED_TARGETPRECISION_TAG;
            parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            double targetPrecision=matcherParametersValue[parameterPos].toInt();

            ptrFlannIndexParams=new cv::flann::AutotunedIndexParams(targetPrecision,buildWeight,memoryWeight,sampleFraction);
            findFlannMatcher=true;
        }
        if(matcherTag.compare(OPENCV_KPM_MATCHER_FLANN_LSH_TAG,Qt::CaseSensitive)==0)
        {
            QString parameterTag=OPENCV_KPM_FLANN_LSH_KEYSIZE_TAG;
            int parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            int keySize=matcherParametersValue[parameterPos].toInt();

            parameterTag=OPENCV_KPM_FLANN_LSH_MULTIPROBELEVEL_TAG;
            parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            int multiProbeLevel=matcherParametersValue[parameterPos].toInt();

            parameterTag=OPENCV_KPM_FLANN_LSH_TABLENUMBER_TAG;
            parameterPos=matcherParametersTag.indexOf(parameterTag);
            if(parameterPos==-1)
            {
                strError=QObject::tr("OpenCVKpM::matching, flann matcher %1 parameter % 2 not found").arg(matcherTag).arg(parameterTag);
                return(OPENCV_KPM_ERROR);
            }
            int tableNumber=matcherParametersValue[parameterPos].toInt();

            ptrFlannIndexParams=new cv::flann::LshIndexParams(tableNumber,keySize,multiProbeLevel);
            findFlannMatcher=true;
        }
        if(!findFlannMatcher)
        {
            strError=QObject::tr("OpenCVKpM::matching, flann matcher: %1 is not implemented").arg(matcherTag);
            return(OPENCV_KPM_ERROR);
        }
        QMap<QString,cv::flann::Index*> ptrFlannIndexes;
        while(iterImagePairs!=mImagesPairs.constEnd())
        {
            QString imageId=iterImagePairs.key();
            if(!mImagesKeyPoints.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::matching, not key points for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            std::vector<cv::KeyPoint> imageKeyPoints=mImagesKeyPoints[imageId];
            if(!mImagesKeyPointsDescriptors.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::matching, not key points descriptors for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            cv::Mat imageKeyPointsDescriptors=mImagesKeyPointsDescriptors[imageId];
//            if(!Settings::getGeneral_invertedSearch())
            QVector<QString> imagePairs=iterImagePairs.value();
            for(int nImageInPairs=0;nImageInPairs<imagePairs.size();nImageInPairs++)
            {
                QString imagePairId=imagePairs.at(nImageInPairs);
                if(!mImagesKeyPoints.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::matching, not key points for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::KeyPoint> imagePairKeyPoints=mImagesKeyPoints[imagePairId];
                if(!mImagesKeyPointsDescriptors.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::matching, not key points descriptors for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                cv::Mat imagePairKeyPointsDescriptors=mImagesKeyPointsDescriptors[imagePairId];
                cv::flann::Index* ptrFlannIndex;
                if(ptrFlannIndexes.contains(imagePairId))
                    ptrFlannIndex=ptrFlannIndexes[imagePairId];
                else
                {
                    ptrFlannIndex=new cv::flann::Index(imagePairKeyPointsDescriptors, *ptrFlannIndexParams, flannDistance);
                    ptrFlannIndexes[imagePairId]=ptrFlannIndex;
                }
                cv::Mat results = cv::Mat(imageKeyPointsDescriptors.rows, kBestMatches, CV_32SC1); // Results index
                cv::Mat dists = cv::Mat(imageKeyPointsDescriptors.rows, kBestMatches, CV_32FC1); // Distance results are CV_32FC1
                QString msg=QObject::tr("          ... Matching pair: %1 - %2, ").arg(imageId).arg(imagePairId);
                qout<<msg;
                try
                {
                    // search (nearest neighbor)
                    ptrFlannIndex->knnSearch(imageKeyPointsDescriptors, results, dists, kBestMatches, cv::flann::SearchParams() );
                }
                catch( cv::Exception& e )
                {
                    const char* err_msg = e.what();
                    strError=QObject::tr("OpenCVKpM::matching, OpenCV exception caught: %1").arg(err_msg);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::Point2f> imageMatchingPoints, imagePairMatchingPoints; // Used for homography
                std::vector<int> imageMatchingIndexes,imagePairMatchingIndexes; // Used for homography
                std::vector<cv::DMatch> matches;
                for(int ii=0; ii<imageKeyPointsDescriptors.rows; ++ii)
                {
                    // Check if this descriptor matches with those of the objects
                    bool matched = true;
                    if(kBestMatches>1)
                    {
                        if(dists.at<float>(ii,0) > ratioTestDistance * dists.at<float>(ii,1))
                        {
                            matched=false;
                        }
                    }
                    if(applyMinimumTestDistance)
                    {
                        if(dists.at<float>(ii,0) > minimumTestDistance)
                        {
                            matched=false;
                        }
                    }
                    if(matched)
                    {
                        int queryIdx=ii;
                        int trainIdx=results.at<int>(ii,0);
                        float distance=dists.at<float>(ii,0);
                        cv::DMatch match(queryIdx,trainIdx,distance );
                        matches.push_back(match);
//                        imageMatchingPoints.push_back(imageKeyPoints.at(matches.at(ii).queryIdx).pt);
//                        imageMatchingIndexes.push_back(matches.at(ii).queryIdx);
//                        imagePairMatchingPoints.push_back(imagePairKeyPoints.at(matches.at(ii).trainIdx).pt);
//                        imagePairMatchingIndexes.push_back(matches.at(ii).trainIdx);

                        imageMatchingPoints.push_back(imageKeyPoints.at(ii).pt);
                        imageMatchingIndexes.push_back(ii);

                        imagePairMatchingPoints.push_back(imagePairKeyPoints.at(results.at<int>(ii,0)).pt);
                        imagePairMatchingIndexes.push_back(results.at<int>(ii,0));
                    }
                }
                if(!mImagesPairsMatches.contains(imageId))
                {
                    QVector<std::vector<cv::DMatch> > imageMatches;
                    imageMatches.push_back(matches);
                    mImagesPairsMatches[imageId]=imageMatches;
                }
                else
                {
                    mImagesPairsMatches[imageId].push_back(matches);
                }
                if(!mImagesPairsMatchesPointsFirsts.contains(imageId))
                {
                    QVector<std::vector<cv::Point2f> > imageMatchesPointsFirsts;
                    imageMatchesPointsFirsts.push_back(imageMatchingPoints);
                    mImagesPairsMatchesPointsFirsts[imageId]=imageMatchesPointsFirsts;
                    QVector<std::vector<cv::Point2f> > imageMatchesPointsSeconds;
                    imageMatchesPointsSeconds.push_back(imagePairMatchingPoints);
                    mImagesPairsMatchesPointsSeconds[imageId]=imageMatchesPointsSeconds;
                    QVector<std::vector<int> > imageMatchesIndexesFirsts;
                    imageMatchesIndexesFirsts.push_back(imageMatchingIndexes);
                    mImagesPairsMatchesIndexesFirsts[imageId]=imageMatchesIndexesFirsts;
                    QVector<std::vector<int> > imageMatchesIndexesSeconds;
                    imageMatchesIndexesSeconds.push_back(imagePairMatchingIndexes);
                    mImagesPairsMatchesIndexesSeconds[imageId]=imageMatchesIndexesSeconds;
                }
                else
                {
                    mImagesPairsMatchesPointsFirsts[imageId].push_back(imageMatchingPoints);
                    mImagesPairsMatchesPointsSeconds[imageId].push_back(imagePairMatchingPoints);
                    mImagesPairsMatchesIndexesFirsts[imageId].push_back(imageMatchingIndexes);
                    mImagesPairsMatchesIndexesSeconds[imageId].push_back(imagePairMatchingIndexes);
                }
                int numberOfMatches=imageMatchingPoints.size();
                if(numberOfMatches>0&&saveImagesMatches)
                {
                    QString imageFileName=mImageFileNames[imageId];
                    QString imagePairFileName=mImageFileNames[imagePairId];
                    QFileInfo fileInfoImage(imageFileName);
                    QString imageBaseName=fileInfoImage.baseName();
                    QFileInfo fileInfoImagePair(imagePairFileName);
                    QString imagePairBaseName=fileInfoImagePair.baseName();
                    QString absoluteFilePath=fileInfoImage.absolutePath();
                    QDir auxDir(absoluteFilePath);
                    QString imageMatchesDir=pathForMatchesFiles;
                    if(!auxDir.exists(imageMatchesDir))
                    {
                        if(!auxDir.mkdir(imageMatchesDir))
                        {
                            strError=QObject::tr("OpenCVKpM::matching, error creating directory: %1").arg(imageMatchesDir);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                    QString imageMatchesFileName=imageMatchesDir+"/";
                    imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                    if(QFile::exists(imageMatchesFileName))
                        QFile::remove(imageMatchesFileName);
                    std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                    cv::Mat imageMatches;
                    cv::Mat image=mImages[imageId];
                    cv::Mat imagePair=mImages[imagePairId];
                    try
                    {
                        cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matches, imageMatches);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::matching, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                    try
                    {
                        cv::imwrite(imageMatchesFileNameStdString,imageMatches);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::matching, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                msg=QObject::tr("resulting %1 ").arg(numberOfMatches);
                qout<<msg<<endl;
                if(bidirectionalMatches)
                {
                    cv::flann::Index* ptrFlannIndexReverse;
                    if(ptrFlannIndexes.contains(imageId))
                        ptrFlannIndexReverse=ptrFlannIndexes[imageId];
                    else
                    {
                        ptrFlannIndexReverse=new cv::flann::Index(imageKeyPointsDescriptors, *ptrFlannIndexParams, flannDistance);
                        ptrFlannIndexes[imageId]=ptrFlannIndexReverse;
                    }
                    cv::Mat resultsReverse = cv::Mat(imagePairKeyPointsDescriptors.rows, kBestMatches, CV_32SC1); // Results index
                    cv::Mat distsReverse = cv::Mat(imagePairKeyPointsDescriptors.rows, kBestMatches, CV_32FC1); // Distance results are CV_32FC1
                    QString msg=QObject::tr("          ... Matching pair: %1 - %2, ").arg(imagePairId).arg(imageId);
                    qout<<msg;
                    try
                    {
                        // search (nearest neighbor)
                        ptrFlannIndexReverse->knnSearch(imagePairKeyPointsDescriptors, resultsReverse, distsReverse, kBestMatches, cv::flann::SearchParams() );
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::matching, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                    std::vector<cv::Point2f> imageMatchingPoints, imagePairMatchingPoints; // Used for homography
                    std::vector<int> imageMatchingIndexes,imagePairMatchingIndexes; // Used for homography
                    std::vector<cv::DMatch> matchesReverse;
                    for(int ii=0; ii<imagePairKeyPointsDescriptors.rows; ++ii)
                    {
                        // Check if this descriptor matches with those of the objects
                        bool matched = true;
                        if(kBestMatches>1)
                        {
                            if(distsReverse.at<float>(ii,0) > ratioTestDistance * distsReverse.at<float>(ii,1))
                            {
                                matched=false;
                            }
                        }
                        if(applyMinimumTestDistance)
                        {
                            if(distsReverse.at<float>(ii,0) > minimumTestDistance)
                            {
                                matched=false;
                            }
                        }
                        if(matched)
                        {
                            int queryIdx=ii;
                            int trainIdx=resultsReverse.at<int>(ii,0);
                            float distance=distsReverse.at<float>(ii,0);
                            cv::DMatch match(queryIdx,trainIdx,distance );
                            matchesReverse.push_back(match);
    //                        imageMatchingPoints.push_back(imageKeyPoints.at(matches.at(ii).queryIdx).pt);
    //                        imageMatchingIndexes.push_back(matches.at(ii).queryIdx);
    //                        imagePairMatchingPoints.push_back(imagePairKeyPoints.at(matches.at(ii).trainIdx).pt);
    //                        imagePairMatchingIndexes.push_back(matches.at(ii).trainIdx);

                            imagePairMatchingPoints.push_back(imagePairKeyPoints.at(ii).pt);
                            imagePairMatchingIndexes.push_back(ii);

                            imageMatchingPoints.push_back(imageKeyPoints.at(resultsReverse.at<int>(ii,0)).pt);
                            imageMatchingIndexes.push_back(resultsReverse.at<int>(ii,0));
                        }
                    }
                    if(!mImagesPairsMatches.contains(imageId))
                    {
                        QVector<std::vector<cv::DMatch> > imageMatches;
                        imageMatches.push_back(matchesReverse);
                        mImagesPairsMatchesReverse[imageId]=imageMatches;
                    }
                    else
                    {
                        mImagesPairsMatchesReverse[imageId].push_back(matchesReverse);
                    }
                    if(!mImagesPairsMatchesPointsFirstsReverse.contains(imageId))
                    {
//                        QVector<std::vector<cv::Point2f> > imageMatchesPointsFirsts;
//                        imageMatchesPointsFirsts.push_back(imageMatchingPoints);
//                        mImagesPairsMatchesPointsFirsts[imageId]=imageMatchesPointsFirsts;
//                        QVector<std::vector<cv::Point2f> > imageMatchesPointsSeconds;
//                        imageMatchesPointsSeconds.push_back(imagePairMatchingPoints);
//                        mImagesPairsMatchesPointsSeconds[imageId]=imageMatchesPointsSeconds;
//                        QVector<std::vector<int> > imageMatchesIndexesFirsts;
//                        imageMatchesIndexesFirsts.push_back(imageMatchingIndexes);
//                        mImagesPairsMatchesIndexesFirsts[imageId]=imageMatchesIndexesFirsts;
//                        QVector<std::vector<int> > imageMatchesIndexesSeconds;
//                        imageMatchesIndexesSeconds.push_back(imagePairMatchingIndexes);
//                        mImagesPairsMatchesIndexesSeconds[imageId]=imageMatchesIndexesSeconds;
                        QVector<std::vector<cv::Point2f> > imageMatchesPointsFirsts;
                        imageMatchesPointsFirsts.push_back(imagePairMatchingPoints);
                        mImagesPairsMatchesPointsFirstsReverse[imageId]=imageMatchesPointsFirsts;
                        QVector<std::vector<cv::Point2f> > imageMatchesPointsSeconds;
                        imageMatchesPointsSeconds.push_back(imageMatchingPoints);
                        mImagesPairsMatchesPointsSecondsReverse[imageId]=imageMatchesPointsSeconds;
                        QVector<std::vector<int> > imageMatchesIndexesFirsts;
                        imageMatchesIndexesFirsts.push_back(imagePairMatchingIndexes);
                        mImagesPairsMatchesIndexesFirstsReverse[imageId]=imageMatchesIndexesFirsts;
                        QVector<std::vector<int> > imageMatchesIndexesSeconds;
                        imageMatchesIndexesSeconds.push_back(imageMatchingIndexes);
                        mImagesPairsMatchesIndexesSecondsReverse[imageId]=imageMatchesIndexesSeconds;
                    }
                    else
                    {
//                        mImagesPairsMatchesPointsFirsts[imageId].push_back(imageMatchingPoints);
//                        mImagesPairsMatchesPointsSeconds[imageId].push_back(imagePairMatchingPoints);
//                        mImagesPairsMatchesIndexesFirsts[imageId].push_back(imageMatchingIndexes);
//                        mImagesPairsMatchesIndexesSeconds[imageId].push_back(imagePairMatchingIndexes);
                        mImagesPairsMatchesPointsFirstsReverse[imageId].push_back(imagePairMatchingPoints);
                        mImagesPairsMatchesPointsSecondsReverse[imageId].push_back(imageMatchingPoints);
                        mImagesPairsMatchesIndexesFirstsReverse[imageId].push_back(imagePairMatchingIndexes);
                        mImagesPairsMatchesIndexesSecondsReverse[imageId].push_back(imageMatchingIndexes);
                    }
                    int numberOfMatches=imagePairMatchingPoints.size();
                    if(numberOfMatches>0&&saveImagesMatches)
                    {
                        QString imageFileName=mImageFileNames[imageId];
                        QString imagePairFileName=mImageFileNames[imagePairId];
                        QFileInfo fileInfoImage(imageFileName);
                        QString imageBaseName=fileInfoImage.baseName();
                        QFileInfo fileInfoImagePair(imagePairFileName);
                        QString imagePairBaseName=fileInfoImagePair.baseName();
                        QString absoluteFilePath=fileInfoImage.absolutePath();
                        QDir auxDir(absoluteFilePath);
                        QString imageMatchesDir=pathForMatchesFiles;
                        if(!auxDir.exists(imageMatchesDir))
                        {
                            if(!auxDir.mkdir(imageMatchesDir))
                            {
                                strError=QObject::tr("OpenCVKpM::matching, error creating directory: %1").arg(imageMatchesDir);
                                return(OPENCV_KPM_ERROR);
                            }
                        }
                        QString imageMatchesFileName=imageMatchesDir+"/";
//                        imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                        imageMatchesFileName+=(imagePairBaseName+"_"+imageBaseName+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                        if(QFile::exists(imageMatchesFileName))
                            QFile::remove(imageMatchesFileName);
                        std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                        cv::Mat imageMatchesReverse;
                        cv::Mat image=mImages[imageId];
                        cv::Mat imagePair=mImages[imagePairId];
                        try
                        {
//                            cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matches, imageMatches);
                            cv::drawMatches( imagePair, imagePairKeyPoints,image, imageKeyPoints, matchesReverse, imageMatchesReverse);
                        }
                        catch( cv::Exception& e )
                        {
                            const char* err_msg = e.what();
                            strError=QObject::tr("OpenCVKpM::matching, OpenCV exception caught: %1").arg(err_msg);
                            return(OPENCV_KPM_ERROR);
                        }
                        try
                        {
                            cv::imwrite(imageMatchesFileNameStdString,imageMatchesReverse);
                        }
                        catch( cv::Exception& e )
                        {
                            const char* err_msg = e.what();
                            strError=QObject::tr("OpenCVKpM::matching, OpenCV exception caught: %1").arg(err_msg);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                    msg=QObject::tr("resulting %1 ").arg(numberOfMatches);
                    qout<<msg<<endl;
                }
            }
            iterImagePairs++;
        }
        QMap<QString,cv::flann::Index*>::iterator iterFlannIndexes=ptrFlannIndexes.begin();
        while(iterFlannIndexes!=ptrFlannIndexes.end())
        {
            delete(iterFlannIndexes.value());
            iterFlannIndexes++;
        }
    }
#endif //USE_OPENCV
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::readImagePairsFileName(QString& strError)
{
    mNumberOfImagePairs=0;
    QString imagesPath,imagePairsFileName;
    if(OPENCV_KPM_NO_ERROR!=mParameters.getImagesPath(imagesPath,strError))
    {
        strError="OpenCVKpM::readImagePairsFileName,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    if(OPENCV_KPM_NO_ERROR!=mParameters.getImagePairsFileName(imagePairsFileName,strError))
    {
        strError="OpenCVKpM::readImagePairsFileName,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    QDir imagesDir(imagesPath);
    if(!imagesDir.exists())
    {
        strError=QObject::tr("OpenCVKpM::readImagePairsFileName, imagesPath not exists:\n%1").arg(imagesPath);
        return(OPENCV_KPM_ERROR);
    }
    if(!QFile::exists(imagePairsFileName))
    {
        strError=QObject::tr("OpenCVKpM::readImagePairsFileName, imagePairsFileName not exists:\n%1").arg(imagePairsFileName);
        return(OPENCV_KPM_ERROR);
    }
    QFile file(imagePairsFileName);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        int nLine=0;
        while (!file.atEnd())
        {
            nLine++;
            QString line = file.readLine().trimmed();
            if(line.isEmpty())
                continue;
            QStringList splitedLine = line.simplified().split(" ");
            if(splitedLine.count() == 2)
            {
                QString imageIdFirst=splitedLine.at(0).trimmed();
                QString imageIdSecond=splitedLine.at(1).trimmed();
                if(!mImageFileNames.contains(imageIdFirst))
                {
                    QString imageCompleteFileName=imagesPath+"/"+imageIdFirst;
                    if(!QFile::exists(imageCompleteFileName))
                    {
                        strError=QObject::tr("OpenCVKpM::readImagePairsFileName, in imagePairsFile\n%1 \n in line number: %2 not exists image file: \n %3").arg(imagePairsFileName).arg(nLine).arg(imageCompleteFileName);
                        return(OPENCV_KPM_ERROR);
                    }
                    mImageFileNames[imageIdFirst]=imageCompleteFileName;
                }
                if(!mImageFileNames.contains(imageIdSecond))
                {
                    QString imageCompleteFileName=imagesPath+"/"+imageIdSecond;
                    if(!QFile::exists(imageCompleteFileName))
                    {
                        strError=QObject::tr("OpenCVKpM::readImagePairsFileName, in imagePairsFile\n%1 \n in line number: %2 not exists image file: \n %3").arg(imagePairsFileName).arg(nLine).arg(imageCompleteFileName);
                        return(OPENCV_KPM_ERROR);
                    }
                    mImageFileNames[imageIdSecond]=imageCompleteFileName;
                }
                if(mImagesPairs.contains(imageIdFirst))
                {
                    mImagesPairs[imageIdFirst].push_back(imageIdSecond);
                }
                else
                {
                    QVector<QString> imageIds;
                    imageIds.push_back(imageIdSecond);
                    mImagesPairs[imageIdFirst]=imageIds;
                }
            }
            else
            {
                strError=QObject::tr("OpenCVKpM::readImagePairsFileName, in imagePairsFile\n%1 \n in line number: %2 there are not two strings").arg(imagePairsFileName).arg(nLine);
                return(OPENCV_KPM_ERROR);
            }
        }
    }
    else
    {
        strError=QObject::tr("OpenCVKpM::readImagePairsFileName, opening imagePairsFile:\n").arg(imagePairsFileName);
        return(OPENCV_KPM_NO_ERROR);
    }
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::readParameterFile(QString& fileName,
                             QString& strError)
{
    initialize();
    return(mParameters.setFromFile(fileName,strError));
}

int OpenCVKpM::removeDuplicatedMatches(bool toUniqueMatch,
                                       bool removeMultipleToOne,
                                       bool removeOneToMultiple,
                                       QString &strError)
{
#ifdef USE_OPENCV
    if(!toUniqueMatch&&!removeMultipleToOne&&!removeOneToMultiple)
        return(OPENCV_KPM_NO_ERROR);
    bool bidirectionalMatches=mParameters.getBidirectionalMatches();
    QString pathForMatchesFiles;
    bool saveImagesMatches=mParameters.getSaveImagesMatches(pathForMatchesFiles);
    QMap<QString,QVector<QString> >::const_iterator iterImagePairs = mImagesPairs.constBegin();
    while(iterImagePairs!=mImagesPairs.constEnd())
    {
        QString imageId=iterImagePairs.key();
        if(!mImagesKeyPoints.contains(imageId))
        {
            strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, not key points for image id: \n %1").arg(imageId);
            return(OPENCV_KPM_ERROR);
        }
        if(!mImagesKeyPointsDescriptors.contains(imageId))
        {
            strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, not key points descriptors for image id: \n %1").arg(imageId);
            return(OPENCV_KPM_ERROR);
        }
        if(!mImagesPairsMatchesPointsFirsts.contains(imageId))
        {
            strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, not matches points firsts for image id: \n %1").arg(imageId);
            return(OPENCV_KPM_ERROR);
        }
        if(!mImagesPairsMatchesPointsSeconds.contains(imageId))
        {
            strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, not matches points seconds for image id: \n %1").arg(imageId);
            return(OPENCV_KPM_ERROR);
        }
        if(bidirectionalMatches)
        {
            if(!mImagesPairsMatchesPointsFirstsReverse.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, not matches points firsts for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesPairsMatchesPointsSecondsReverse.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, not matches points seconds for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
        }
        std::vector<cv::KeyPoint> imageKeyPoints=mImagesKeyPoints[imageId];
        QVector<std::vector<cv::Point2f> > matchingPointsFirst=mImagesPairsMatchesPointsFirsts[imageId];
        QVector<std::vector<cv::Point2f> > matchingPointsSecond=mImagesPairsMatchesPointsSeconds[imageId];
        QVector<std::vector<int> > imagePairsMatchesIndexesFirst=mImagesPairsMatchesIndexesFirsts[imageId];
        QVector<std::vector<int> > imagePairsMatchesIndexesSecond=mImagesPairsMatchesIndexesSeconds[imageId];
        QVector<std::vector<cv::Point2f> > matchingPointsFirstReverse;
        QVector<std::vector<cv::Point2f> > matchingPointsSecondReverse;
        QVector<std::vector<int> > imagePairsMatchesIndexesFirstReverse;
        QVector<std::vector<int> > imagePairsMatchesIndexesSecondReverse;
        if(bidirectionalMatches)
        {
            matchingPointsFirstReverse=mImagesPairsMatchesPointsFirstsReverse[imageId];
            matchingPointsSecondReverse=mImagesPairsMatchesPointsSecondsReverse[imageId];
            imagePairsMatchesIndexesFirstReverse=mImagesPairsMatchesIndexesFirstsReverse[imageId];
            imagePairsMatchesIndexesSecondReverse=mImagesPairsMatchesIndexesSecondsReverse[imageId];
        }
        QVector<QString> imagePairs=iterImagePairs.value();
        if(imagePairs.size()!=matchingPointsFirst.size())
        {
            strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, not matches points first for all pairs of image id: \n %1").arg(imageId);
            return(OPENCV_KPM_ERROR);
        }
        if(imagePairs.size()!=matchingPointsSecond.size())
        {
            strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, not matches points first for all pairs of image id: \n %1").arg(imageId);
            return(OPENCV_KPM_ERROR);
        }
        if(bidirectionalMatches)
        {
            if(imagePairs.size()!=matchingPointsFirstReverse.size())
            {
                strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(imagePairs.size()!=matchingPointsSecondReverse.size())
            {
                strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
        }
        for(int nImageInPairs=0;nImageInPairs<imagePairs.size();nImageInPairs++)
        {
            QString imagePairId=imagePairs.at(nImageInPairs);
            if(!mImagesKeyPoints.contains(imagePairId))
            {
                strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, not key points for image id: \n %1").arg(imagePairId);
                return(OPENCV_KPM_ERROR);
            }
            std::vector<cv::KeyPoint> imagePairKeyPoints=mImagesKeyPoints[imagePairId];
            if(!mImagesKeyPointsDescriptors.contains(imagePairId))
            {
                strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, not key points descriptors for image id: \n %1").arg(imagePairId);
                return(OPENCV_KPM_ERROR);
            }
            std::vector<cv::Point2f> imageMatchingPoints=matchingPointsFirst[nImageInPairs];
            std::vector<cv::Point2f> imagePairMatchingPoints=matchingPointsSecond[nImageInPairs];
            std::vector<int> imagePairsMatchesIndexes=imagePairsMatchesIndexesFirst[nImageInPairs];
            std::vector<int> imagePairPairsMatchesIndexes=imagePairsMatchesIndexesSecond[nImageInPairs];
            std::vector<cv::Point2f> imageMatchingPointsReverse;
            std::vector<cv::Point2f> imagePairMatchingPointsReverse;
            std::vector<int> imagePairsMatchesIndexesReverse;
            std::vector<int> imagePairPairsMatchesIndexesReverse;
            if(bidirectionalMatches)
            {
                imageMatchingPointsReverse=matchingPointsFirstReverse[nImageInPairs];
                imagePairMatchingPointsReverse=matchingPointsSecondReverse[nImageInPairs];
                imagePairsMatchesIndexesReverse=imagePairsMatchesIndexesFirstReverse[nImageInPairs];
                imagePairPairsMatchesIndexesReverse=imagePairsMatchesIndexesSecondReverse[nImageInPairs];
            }
            if(imageMatchingPoints.size()!=imagePairMatchingPoints.size())
            {
                strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, not equal number of matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                return(OPENCV_KPM_ERROR);
            }
            if(bidirectionalMatches)
            {
                if(imageMatchingPointsReverse.size()!=imagePairMatchingPointsReverse.size())
                {
                    strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, not equal number of reverse matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
            }
            int numberOfMatches=(int)imageMatchingPoints.size();
            int numberOfMatchesReverse;
            int numberOfTotalMatches=numberOfMatches;
            if(bidirectionalMatches)
            {
                numberOfMatchesReverse=(int)imageMatchingPointsReverse.size();
                numberOfTotalMatches+=numberOfMatchesReverse;
            }
            std::vector<cv::Point2f> imageMatchingPointsWithoutDuplicated;
            std::vector<cv::Point2f> imagePairMatchingPointsWithoutDuplicated;
            std::vector<int> imagePairsMatchesIndexesWithoutDuplicated;
            std::vector<int> imagePairPairsMatchesIndexesWithoutDuplicated;
            std::vector<cv::DMatch> matches=mImagesPairsMatches[imageId][nImageInPairs];
            std::vector<cv::DMatch> matchesWithoutDuplicated;

            std::vector<cv::Point2f> imageMatchingPointsWithoutDuplicatedReverse;
            std::vector<cv::Point2f> imagePairMatchingPointsWithoutDuplicatedReverse;
            std::vector<int> imagePairsMatchesIndexesWithoutDuplicatedReverse;
            std::vector<int> imagePairPairsMatchesIndexesWithoutDuplicatedReverse;
            std::vector<cv::DMatch> matchesReverse;
            std::vector<cv::DMatch> matchesWithoutDuplicatedReverse;

            if(toUniqueMatch)
            {
                imageMatchingPointsWithoutDuplicated.push_back(imageMatchingPoints[0]);
                imagePairMatchingPointsWithoutDuplicated.push_back(imagePairMatchingPoints[0]);
                imagePairsMatchesIndexesWithoutDuplicated.push_back(imagePairsMatchesIndexes[0]);
                imagePairPairsMatchesIndexesWithoutDuplicated.push_back(imagePairPairsMatchesIndexes[0]);
                matchesWithoutDuplicated.push_back(matches[0]);
                for(int nMIn=1;nMIn<imageMatchingPoints.size();nMIn++)
                {
                    cv::Point2f imagePtoIn=imageMatchingPoints[nMIn];
                    cv::Point2f imagePairPtoIn=imagePairMatchingPoints[nMIn];
                    float x1In=imagePtoIn.x;
                    float y1In=imagePtoIn.y;
                    float x2In=imagePairPtoIn.x;
                    float y2In=imagePairPtoIn.y;
                    int flag_unique=1;
                    for(int nMOut=0;nMOut<imageMatchingPointsWithoutDuplicated.size();nMOut++)
                    {
                        cv::Point2f imagePtoOut=imageMatchingPointsWithoutDuplicated[nMOut];
                        cv::Point2f imagePairPtoOut=imagePairMatchingPointsWithoutDuplicated[nMOut];
                        float x1Out=imagePtoOut.x;
                        float y1Out=imagePtoOut.y;
                        float x2Out=imagePairPtoOut.x;
                        float y2Out=imagePairPtoOut.y;
                        float d1=(x1In-x1Out)*(x1In-x1Out)+(y1In-y1Out)*(y1In-y1Out);
                        float d2=(x2In-x2Out)*(x2In-x2Out)+(y2In-y2Out)*(y2In-y2Out);
                        if ((d1 <= OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_UNIQUE_TH2)
                                && ( d2 <= OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_UNIQUE_TH2) )
                         {
                           flag_unique = 0;
                           continue;
                         }
                    }
                    if(flag_unique==1)
                    {
                        imageMatchingPointsWithoutDuplicated.push_back(imageMatchingPoints[nMIn]);
                        imagePairMatchingPointsWithoutDuplicated.push_back(imagePairMatchingPoints[nMIn]);
                        imagePairsMatchesIndexesWithoutDuplicated.push_back(imagePairsMatchesIndexes[nMIn]);
                        imagePairPairsMatchesIndexesWithoutDuplicated.push_back(imagePairPairsMatchesIndexes[nMIn]);
                        matchesWithoutDuplicated.push_back(matches[nMIn]);
                    }
                }
                if(bidirectionalMatches)
                {
                    matchesReverse=mImagesPairsMatchesReverse[imageId][nImageInPairs];
                    imageMatchingPointsWithoutDuplicatedReverse.push_back(imageMatchingPointsReverse[0]);
                    imagePairMatchingPointsWithoutDuplicatedReverse.push_back(imagePairMatchingPointsReverse[0]);
                    imagePairsMatchesIndexesWithoutDuplicatedReverse.push_back(imagePairsMatchesIndexesReverse[0]);
                    imagePairPairsMatchesIndexesWithoutDuplicatedReverse.push_back(imagePairPairsMatchesIndexesReverse[0]);
                    matchesWithoutDuplicatedReverse.push_back(matchesReverse[0]);
                    for(int nMIn=1;nMIn<imageMatchingPointsReverse.size();nMIn++)
                    {
                        cv::Point2f imagePtoIn=imageMatchingPointsReverse[nMIn];
                        cv::Point2f imagePairPtoIn=imagePairMatchingPointsReverse[nMIn];
                        float x1In=imagePtoIn.x;
                        float y1In=imagePtoIn.y;
                        float x2In=imagePairPtoIn.x;
                        float y2In=imagePairPtoIn.y;
                        int flag_unique=1;
                        for(int nMOut=0;nMOut<imageMatchingPointsWithoutDuplicatedReverse.size();nMOut++)
                        {
                            cv::Point2f imagePtoOut=imageMatchingPointsWithoutDuplicatedReverse[nMOut];
                            cv::Point2f imagePairPtoOut=imagePairMatchingPointsWithoutDuplicatedReverse[nMOut];
                            float x1Out=imagePtoOut.x;
                            float y1Out=imagePtoOut.y;
                            float x2Out=imagePairPtoOut.x;
                            float y2Out=imagePairPtoOut.y;
                            float d1=(x1In-x1Out)*(x1In-x1Out)+(y1In-y1Out)*(y1In-y1Out);
                            float d2=(x2In-x2Out)*(x2In-x2Out)+(y2In-y2Out)*(y2In-y2Out);
                            if ((d1 <= OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_UNIQUE_TH2)
                                    && ( d2 <= OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_UNIQUE_TH2) )
                             {
                               flag_unique = 0;
                               continue;
                             }
                        }
                        if(flag_unique==1)
                        {
                            imageMatchingPointsWithoutDuplicatedReverse.push_back(imageMatchingPointsReverse[nMIn]);
                            imagePairMatchingPointsWithoutDuplicatedReverse.push_back(imagePairMatchingPointsReverse[nMIn]);
                            imagePairsMatchesIndexesWithoutDuplicatedReverse.push_back(imagePairsMatchesIndexesReverse[nMIn]);
                            imagePairPairsMatchesIndexesWithoutDuplicatedReverse.push_back(imagePairPairsMatchesIndexesReverse[nMIn]);
                            matchesWithoutDuplicatedReverse.push_back(matchesReverse[nMIn]);
                        }
                    }
                }
                toUniqueMatch=false;
            }
            else if(removeMultipleToOne)
            {
                QVector<int> flag_unique(imageMatchingPoints.size());
                for ( int i1 = 0; i1 < imageMatchingPoints.size(); i1++ )
                {
                    flag_unique[i1] = 1;
                }
                for(int nMIn=0;nMIn<(imageMatchingPoints.size()-1);nMIn++)
                {
                    cv::Point2f imagePtoIn=imageMatchingPoints[nMIn];
                    cv::Point2f imagePairPtoIn=imagePairMatchingPoints[nMIn];
                    float x1In=imagePtoIn.x;
                    float y1In=imagePtoIn.y;
                    float x2In=imagePairPtoIn.x;
                    float y2In=imagePairPtoIn.y;
                    for(int nMInPlus=nMIn+1;nMInPlus<imageMatchingPoints.size();nMInPlus++)
                    {
                        cv::Point2f imagePtoInPlus=imageMatchingPoints[nMInPlus];
                        cv::Point2f imagePairPtoInPlus=imagePairMatchingPoints[nMInPlus];
                        float x1InPlus=imagePtoInPlus.x;
                        float y1InPlus=imagePtoInPlus.y;
                        float x2InPlus=imagePairPtoInPlus.x;
                        float y2InPlus=imagePairPtoInPlus.y;
                        float d1=(x1In-x1InPlus)*(x1In-x1InPlus)+(y1In-y1InPlus)*(y1In-y1InPlus);
                        float d2=(x2In-x2InPlus)*(x2In-x2InPlus)+(y2In-y2InPlus)*(y2In-y2InPlus);
                        if ( ( d1 > OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_MULTIPLE_TO_ONE_TH2)
                             && ( d2 <= OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_MULTIPLE_TO_ONE_TH1) )
                        {
                            flag_unique[nMIn] = 0;
                            flag_unique[nMInPlus] = 0;
                        }
                    }
                }
                for ( int i1 = 0; i1 < imageMatchingPoints.size(); i1++ )
                {
                    if(flag_unique[i1]==1)
                    {
                        imageMatchingPointsWithoutDuplicated.push_back(imageMatchingPoints[i1]);
                        imagePairMatchingPointsWithoutDuplicated.push_back(imagePairMatchingPoints[i1]);
                        imagePairsMatchesIndexesWithoutDuplicated.push_back(imagePairsMatchesIndexes[i1]);
                        imagePairPairsMatchesIndexesWithoutDuplicated.push_back(imagePairPairsMatchesIndexes[i1]);
                        matchesWithoutDuplicated.push_back(matches[i1]);
                    }
                }
                if(bidirectionalMatches)
                {
                    matchesReverse=mImagesPairsMatchesReverse[imageId][nImageInPairs];
                    QVector<int> flag_unique(imageMatchingPointsReverse.size());
                    for ( int i1 = 0; i1 < imageMatchingPointsReverse.size(); i1++ )
                    {
                        flag_unique[i1] = 1;
                    }
                    for(int nMIn=0;nMIn<(imageMatchingPointsReverse.size()-1);nMIn++)
                    {
                        cv::Point2f imagePtoIn=imageMatchingPointsReverse[nMIn];
                        cv::Point2f imagePairPtoIn=imagePairMatchingPointsReverse[nMIn];
                        float x1In=imagePtoIn.x;
                        float y1In=imagePtoIn.y;
                        float x2In=imagePairPtoIn.x;
                        float y2In=imagePairPtoIn.y;
                        for(int nMInPlus=nMIn+1;nMInPlus<imageMatchingPointsReverse.size();nMInPlus++)
                        {
                            cv::Point2f imagePtoInPlus=imageMatchingPointsReverse[nMInPlus];
                            cv::Point2f imagePairPtoInPlus=imagePairMatchingPointsReverse[nMInPlus];
                            float x1InPlus=imagePtoInPlus.x;
                            float y1InPlus=imagePtoInPlus.y;
                            float x2InPlus=imagePairPtoInPlus.x;
                            float y2InPlus=imagePairPtoInPlus.y;
                            float d1=(x1In-x1InPlus)*(x1In-x1InPlus)+(y1In-y1InPlus)*(y1In-y1InPlus);
                            float d2=(x2In-x2InPlus)*(x2In-x2InPlus)+(y2In-y2InPlus)*(y2In-y2InPlus);
                            if ( ( d1 > OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_MULTIPLE_TO_ONE_TH2)
                                 && ( d2 <= OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_MULTIPLE_TO_ONE_TH1) )
                            {
                                flag_unique[nMIn] = 0;
                                flag_unique[nMInPlus] = 0;
                            }
                        }
                    }
                    for ( int i1 = 0; i1 < imageMatchingPointsReverse.size(); i1++ )
                    {
                        if(flag_unique[i1]==1)
                        {
                            imageMatchingPointsWithoutDuplicatedReverse.push_back(imageMatchingPointsReverse[i1]);
                            imagePairMatchingPointsWithoutDuplicatedReverse.push_back(imagePairMatchingPointsReverse[i1]);
                            imagePairsMatchesIndexesWithoutDuplicatedReverse.push_back(imagePairsMatchesIndexesReverse[i1]);
                            imagePairPairsMatchesIndexesWithoutDuplicatedReverse.push_back(imagePairPairsMatchesIndexesReverse[i1]);
                            matchesWithoutDuplicatedReverse.push_back(matchesReverse[i1]);
                        }
                    }
                }
                removeMultipleToOne=false;
            }
            else if(removeOneToMultiple)
            {
                QVector<int> flag_unique(imageMatchingPoints.size());
                for ( int i1 = 0; i1 < imageMatchingPoints.size(); i1++ )
                {
                    flag_unique[i1] = 1;
                }
                for(int nMIn=0;nMIn<(imageMatchingPoints.size()-1);nMIn++)
                {
                    cv::Point2f imagePtoIn=imageMatchingPoints[nMIn];
                    cv::Point2f imagePairPtoIn=imagePairMatchingPoints[nMIn];
                    float x1In=imagePtoIn.x;
                    float y1In=imagePtoIn.y;
                    float x2In=imagePairPtoIn.x;
                    float y2In=imagePairPtoIn.y;
                    for(int nMInPlus=nMIn+1;nMInPlus<imageMatchingPoints.size();nMInPlus++)
                    {
                        cv::Point2f imagePtoInPlus=imageMatchingPoints[nMInPlus];
                        cv::Point2f imagePairPtoInPlus=imagePairMatchingPoints[nMInPlus];
                        float x1InPlus=imagePtoInPlus.x;
                        float y1InPlus=imagePtoInPlus.y;
                        float x2InPlus=imagePairPtoInPlus.x;
                        float y2InPlus=imagePairPtoInPlus.y;
                        float d1=(x1In-x1InPlus)*(x1In-x1InPlus)+(y1In-y1InPlus)*(y1In-y1InPlus);
                        float d2=(x2In-x2InPlus)*(x2In-x2InPlus)+(y2In-y2InPlus)*(y2In-y2InPlus);
                        if ( ( ( d1 <= OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_MULTIPLE_TO_ONE_TH1)
                             && ( d2 > OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_MULTIPLE_TO_ONE_TH2) )
                             ||( ( d1 > OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_MULTIPLE_TO_ONE_TH2)
                                 && ( d2 <= OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_MULTIPLE_TO_ONE_TH1) ))
                        {
                            flag_unique[nMIn] = 0;
                            flag_unique[nMInPlus] = 0;
                        }
                    }
                }
                for ( int i1 = 0; i1 < imageMatchingPoints.size(); i1++ )
                {
                    if(flag_unique[i1]==1)
                    {
                        imageMatchingPointsWithoutDuplicated.push_back(imageMatchingPoints[i1]);
                        imagePairMatchingPointsWithoutDuplicated.push_back(imagePairMatchingPoints[i1]);
                        imagePairsMatchesIndexesWithoutDuplicated.push_back(imagePairsMatchesIndexes[i1]);
                        imagePairPairsMatchesIndexesWithoutDuplicated.push_back(imagePairPairsMatchesIndexes[i1]);
                        matchesWithoutDuplicated.push_back(matches[i1]);
                    }
                }
                if(bidirectionalMatches)
                {
                    matchesReverse=mImagesPairsMatchesReverse[imageId][nImageInPairs];
                    QVector<int> flag_unique(imageMatchingPointsReverse.size());
                    for ( int i1 = 0; i1 < imageMatchingPointsReverse.size(); i1++ )
                    {
                        flag_unique[i1] = 1;
                    }
                    for(int nMIn=0;nMIn<(imageMatchingPointsReverse.size()-1);nMIn++)
                    {
                        cv::Point2f imagePtoIn=imageMatchingPointsReverse[nMIn];
                        cv::Point2f imagePairPtoIn=imagePairMatchingPointsReverse[nMIn];
                        float x1In=imagePtoIn.x;
                        float y1In=imagePtoIn.y;
                        float x2In=imagePairPtoIn.x;
                        float y2In=imagePairPtoIn.y;
                        for(int nMInPlus=nMIn+1;nMInPlus<imageMatchingPointsReverse.size();nMInPlus++)
                        {
                            cv::Point2f imagePtoInPlus=imageMatchingPointsReverse[nMInPlus];
                            cv::Point2f imagePairPtoInPlus=imagePairMatchingPointsReverse[nMInPlus];
                            float x1InPlus=imagePtoInPlus.x;
                            float y1InPlus=imagePtoInPlus.y;
                            float x2InPlus=imagePairPtoInPlus.x;
                            float y2InPlus=imagePairPtoInPlus.y;
                            float d1=(x1In-x1InPlus)*(x1In-x1InPlus)+(y1In-y1InPlus)*(y1In-y1InPlus);
                            float d2=(x2In-x2InPlus)*(x2In-x2InPlus)+(y2In-y2InPlus)*(y2In-y2InPlus);
                            if ( ( ( d1 <= OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_MULTIPLE_TO_ONE_TH1)
                                 && ( d2 > OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_MULTIPLE_TO_ONE_TH2) )
                                 ||( ( d1 > OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_MULTIPLE_TO_ONE_TH2)
                                     && ( d2 <= OPENCV_KPM_DUPLICATED_PIXEL_TOLERANCE_MULTIPLE_TO_ONE_TH1) ))
                            {
                                flag_unique[nMIn] = 0;
                                flag_unique[nMInPlus] = 0;
                            }
                        }
                    }
                    for ( int i1 = 0; i1 < imageMatchingPointsReverse.size(); i1++ )
                    {
                        if(flag_unique[i1]==1)
                        {
                            imageMatchingPointsWithoutDuplicatedReverse.push_back(imageMatchingPointsReverse[i1]);
                            imagePairMatchingPointsWithoutDuplicatedReverse.push_back(imagePairMatchingPointsReverse[i1]);
                            imagePairsMatchesIndexesWithoutDuplicatedReverse.push_back(imagePairsMatchesIndexesReverse[i1]);
                            imagePairPairsMatchesIndexesWithoutDuplicatedReverse.push_back(imagePairPairsMatchesIndexesReverse[i1]);
                            matchesWithoutDuplicatedReverse.push_back(matchesReverse[i1]);
                        }
                    }
                }
                removeOneToMultiple=false;
            }
            mImagesPairsMatchesPointsFirsts[imageId][nImageInPairs]=imageMatchingPointsWithoutDuplicated;
            mImagesPairsMatchesPointsSeconds[imageId][nImageInPairs]=imagePairMatchingPointsWithoutDuplicated;
            mImagesPairsMatchesIndexesFirsts[imageId][nImageInPairs]=imagePairsMatchesIndexesWithoutDuplicated;
            mImagesPairsMatchesIndexesSeconds[imageId][nImageInPairs]=imagePairPairsMatchesIndexesWithoutDuplicated;
            mImagesPairsMatches[imageId][nImageInPairs]=matchesWithoutDuplicated;
            if(mImagesPairsMatches.size()>0&&saveImagesMatches)
            {
                QString imageFileName=mImageFileNames[imageId];
                QString imagePairFileName=mImageFileNames[imagePairId];
                QFileInfo fileInfoImage(imageFileName);
                QString imageBaseName=fileInfoImage.baseName();
                QFileInfo fileInfoImagePair(imagePairFileName);
                QString imagePairBaseName=fileInfoImagePair.baseName();
                QString absoluteFilePath=fileInfoImage.absolutePath();
                QDir auxDir(absoluteFilePath);
                QString imageMatchesDir=pathForMatchesFiles;
                if(!auxDir.exists(imageMatchesDir))
                {
                    if(!auxDir.mkdir(imageMatchesDir))
                    {
                        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, error creating directory: %1").arg(imageMatchesDir);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                QString imageMatchesFileName=imageMatchesDir+"/";
                imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_DUPLICATES_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                if(QFile::exists(imageMatchesFileName))
                    QFile::remove(imageMatchesFileName);
                std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                cv::Mat imageMatches;
                cv::Mat image=mImages[imageId];
                cv::Mat imagePair=mImages[imagePairId];
                try
                {
                    cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matchesWithoutDuplicated,imageMatches);
                }
                catch( cv::Exception& e )
                {
                    const char* err_msg = e.what();
                    strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, OpenCV exception caught: %1").arg(err_msg);
                    return(OPENCV_KPM_ERROR);
                }
                try
                {
                    cv::imwrite(imageMatchesFileNameStdString,imageMatches);
                }
                catch( cv::Exception& e )
                {
                    const char* err_msg = e.what();
                    strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, OpenCV exception caught: %1").arg(err_msg);
                    return(OPENCV_KPM_ERROR);
                }
            }
            if(bidirectionalMatches)
            {
                mImagesPairsMatchesPointsFirstsReverse[imageId][nImageInPairs]=imageMatchingPointsWithoutDuplicatedReverse;
                mImagesPairsMatchesPointsSecondsReverse[imageId][nImageInPairs]=imagePairMatchingPointsWithoutDuplicatedReverse;
                mImagesPairsMatchesIndexesFirstsReverse[imageId][nImageInPairs]=imagePairsMatchesIndexesWithoutDuplicatedReverse;
                mImagesPairsMatchesIndexesSecondsReverse[imageId][nImageInPairs]=imagePairPairsMatchesIndexesWithoutDuplicatedReverse;
                mImagesPairsMatchesReverse[imageId][nImageInPairs]=matchesWithoutDuplicatedReverse;
                if(mImagesPairsMatchesReverse[imageId][nImageInPairs].size()>0&&saveImagesMatches)
                {
                    QString imageFileName=mImageFileNames[imageId];
                    QString imagePairFileName=mImageFileNames[imagePairId];
                    QFileInfo fileInfoImage(imageFileName);
                    QString imageBaseName=fileInfoImage.baseName();
                    QFileInfo fileInfoImagePair(imagePairFileName);
                    QString imagePairBaseName=fileInfoImagePair.baseName();
                    QString absoluteFilePath=fileInfoImage.absolutePath();
                    QDir auxDir(absoluteFilePath);
                    QString imageMatchesDir=pathForMatchesFiles;
                    if(!auxDir.exists(imageMatchesDir))
                    {
                        if(!auxDir.mkdir(imageMatchesDir))
                        {
                            strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, error creating directory: %1").arg(imageMatchesDir);
                            return(OPENCV_KPM_ERROR);
                        }
                    }
                    QString imageMatchesFileName=imageMatchesDir+"/";
//                        imageMatchesFileName+=(imageBaseName+"_"+imagePairBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_OURLIERS_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                    imageMatchesFileName+=(imagePairBaseName+"_"+imageBaseName+OPENCV_KPM_IMAGE_FILE_WITHOUT_DUPLICATES_SUFFIX+OPENCV_KPM_IMAGE_FILE_EXTENSION_FORMAT_PNG);
                    if(QFile::exists(imageMatchesFileName))
                        QFile::remove(imageMatchesFileName);
                    std::string imageMatchesFileNameStdString=imageMatchesFileName.toStdString();
                    cv::Mat imageMatchesReverse;
                    cv::Mat image=mImages[imageId];
                    cv::Mat imagePair=mImages[imagePairId];
                    try
                    {
//                            cv::drawMatches(image, imageKeyPoints, imagePair, imagePairKeyPoints, matchesWithoutOutliers,imageMatches);
                        cv::drawMatches( imagePair, imagePairKeyPoints,image, imageKeyPoints, matchesWithoutDuplicatedReverse,imageMatchesReverse);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::removeDuplicatedMatches, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                    try
                    {
                        cv::imwrite(imageMatchesFileNameStdString,imageMatchesReverse);
                    }
                    catch( cv::Exception& e )
                    {
                        const char* err_msg = e.what();
                        strError=QObject::tr("OpenCVKpM::detectOutliersInMatches, OpenCV exception caught: %1").arg(err_msg);
                        return(OPENCV_KPM_ERROR);
                    }
                }
            }
        }
        iterImagePairs++;
    }
#endif USE_OPENCV
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::writeParametersFileFromEngine(QString& detector,
                                             QString& descriptor,
                                             QString& matcher,
                                             QVector<QParams::Param*>& detectorParameters,
                                             QVector<QParams::Param*>& descriptorParameters,
                                             QVector<QParams::Param*>& matcherParameters,
                                             int& numberOfBestMatches,
                                             bool& applyTestRatio,
                                             double& testRatioValue,
                                             bool& applyTestDistance,
                                             double& testDistanceValue,
                                             bool& applyOutliersDeteccion,
                                             bool& applyOutliersAzimuth,
                                             QVector<QParams::Param*>& outliersAzimuthParameters,
                                             bool& applyOutliersDistance,
                                             QVector<QParams::Param*>& outliersDistanceParameters,
                                             QString& imagesBasePath,
                                             QString &imagePairsFileName,
                                             QString& parametersFileName,
                                             QString& strError)
{
    if(OPENCV_KPM_NO_ERROR!=mParameters.writeParametersFileFromEngine(detector,
                                                                      descriptor,
                                                                      matcher,
                                                                      detectorParameters,
                                                                      descriptorParameters,
                                                                      matcherParameters,
                                                                      numberOfBestMatches,
                                                                      applyTestRatio,
                                                                      testRatioValue,
                                                                      applyTestDistance,
                                                                      testDistanceValue,
                                                                      applyOutliersDeteccion,
                                                                      applyOutliersAzimuth,
                                                                      outliersAzimuthParameters,
                                                                      applyOutliersDistance,
                                                                      outliersDistanceParameters,
                                                                      imagesBasePath,
                                                                      imagePairsFileName,
                                                                      parametersFileName,
                                                                      strError))
    {
        strError="OpenCVKpM::writeParametersFileFromEngine,"+strError;
        return(OPENCV_KPM_ERROR);
    }
    return(OPENCV_KPM_NO_ERROR);
}

int OpenCVKpM::writeMatchesFiles(QString &strError)
{
    QString pathForMatchesFiles;
    bool saveImagesMatches=mParameters.getSaveImagesMatches(pathForMatchesFiles);
    bool bidirectionalMatches=mParameters.getBidirectionalMatches();
    QMap<QString,QVector<QString> >::const_iterator iterImagePairs = mImagesPairs.constBegin();
    while(iterImagePairs!=mImagesPairs.constEnd())
    {
        QString imageId=iterImagePairs.key();
        if(!mImagesKeyPoints.contains(imageId))
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imageId);
            return(OPENCV_KPM_ERROR);
        }
        if(!mImagesKeyPointsDescriptors.contains(imageId))
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imageId);
            return(OPENCV_KPM_ERROR);
        }
        if(!mImagesPairsMatchesPointsFirsts.contains(imageId))
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points firsts for image id: \n %1").arg(imageId);
            return(OPENCV_KPM_ERROR);
        }
        if(!mImagesPairsMatchesPointsSeconds.contains(imageId))
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points seconds for image id: \n %1").arg(imageId);
            return(OPENCV_KPM_ERROR);
        }
        if(bidirectionalMatches)
        {
            if(!mImagesPairsMatchesPointsFirstsReverse.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points firsts for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(!mImagesPairsMatchesPointsSecondsReverse.contains(imageId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points seconds for image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
        }
        //std::vector<cv::KeyPoint> imageKeyPoints=mImagesKeyPoints[imageId];
        QVector<std::vector<cv::Point2f> > matchingPointsFirst=mImagesPairsMatchesPointsFirsts[imageId];
        QVector<std::vector<cv::Point2f> > matchingPointsSecond=mImagesPairsMatchesPointsSeconds[imageId];
        QVector<QString> imagePairs=iterImagePairs.value();
        if(imagePairs.size()!=matchingPointsFirst.size())
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points first for all pairs of image id: \n %1").arg(imageId);
            return(OPENCV_KPM_ERROR);
        }
        if(imagePairs.size()!=matchingPointsSecond.size())
        {
            strError=QObject::tr("OpenCVKpM::getOutliersDetector, not matches points first for all pairs of image id: \n %1").arg(imageId);
            return(OPENCV_KPM_ERROR);
        }
        QString imageFileName=mImageFileNames[imageId];
        QFileInfo fileInfoImage(imageFileName);
        QString imageBaseName=fileInfoImage.baseName();
        QString absoluteFilePath=fileInfoImage.absolutePath();
        QString imageMatchesDir=pathForMatchesFiles;
        QString pathForImageMatchesFiles=imageMatchesDir+"/"+OPENCV_KPM_PREFIX_FOLDER_IMAGE_MATCHES_FILES+imageId;
        QDir auxDir(QDir::currentPath());
        if(!auxDir.exists(pathForImageMatchesFiles))
        {
            if(!auxDir.mkdir(pathForImageMatchesFiles))
            {
                strError=QObject::tr("OpenCVKpM::writeMatchesFiles, error creating folder for image matches files: \n %1").arg(pathForImageMatchesFiles);
                return(OPENCV_KPM_ERROR);
            }
        }
        for(int nImageInPairs=0;nImageInPairs<imagePairs.size();nImageInPairs++)
        {
            QString imagePairId=imagePairs.at(nImageInPairs);
            if(!mImagesKeyPoints.contains(imagePairId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imagePairId);
                return(OPENCV_KPM_ERROR);
            }
            std::vector<cv::KeyPoint> imagePairKeyPoints=mImagesKeyPoints[imagePairId];
            if(!mImagesKeyPointsDescriptors.contains(imagePairId))
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imagePairId);
                return(OPENCV_KPM_ERROR);
            }
            std::vector<cv::Point2f> imageMatchingPoints=matchingPointsFirst[nImageInPairs];
            std::vector<cv::Point2f> imagePairMatchingPoints=matchingPointsSecond[nImageInPairs];
            if(imageMatchingPoints.size()!=imagePairMatchingPoints.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not equal number of matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                return(OPENCV_KPM_ERROR);
            }
            int numberOfMatches=(int)imageMatchingPoints.size();
            QString matchesFileName=pathForImageMatchesFiles+"/"+imagePairId+OPENCV_KPM_PREFIX_IMAGE_MATCHES_FILE_EXTENSION;
            QFile file(matchesFileName);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                strError=QObject::tr("OpenCVKpM::writeMatchesFiles, error creating file for image matches: \n %1").arg(matchesFileName);
                return(OPENCV_KPM_ERROR);
            }
            QTextStream out(&file);
            for( int kk = 0; kk < numberOfMatches; kk++ )
            {
                cv::Point2f pt1 = imageMatchingPoints[kk];
                cv::Point2f pt2 = imagePairMatchingPoints[kk];
                double x1=pt1.x;
                double y1=pt1.y;
                double x2=pt2.x;
                double y2=pt2.y;
                out<<QString::number(x1,'f',OPENCV_KPM_IMAGE_MATCHES_PRECISION)<<" ";
                out<<QString::number(y1,'f',OPENCV_KPM_IMAGE_MATCHES_PRECISION)<<" ";
                out<<QString::number(x2,'f',OPENCV_KPM_IMAGE_MATCHES_PRECISION)<<" ";
                out<<QString::number(y2,'f',OPENCV_KPM_IMAGE_MATCHES_PRECISION)<<endl;
            }
        }
        if(bidirectionalMatches)
        {
            QVector<std::vector<cv::Point2f> > matchingPointsFirstReverse=mImagesPairsMatchesPointsFirstsReverse[imageId];
            QVector<std::vector<cv::Point2f> > matchingPointsSecondReverse=mImagesPairsMatchesPointsSecondsReverse[imageId];
            QVector<QString> imagePairs=iterImagePairs.value();
            if(imagePairs.size()!=matchingPointsFirstReverse.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            if(imagePairs.size()!=matchingPointsSecondReverse.size())
            {
                strError=QObject::tr("OpenCVKpM::getOutliersDetector, not reverse matches points first for all pairs of image id: \n %1").arg(imageId);
                return(OPENCV_KPM_ERROR);
            }
            QString imageFileName=mImageFileNames[imageId];
            QFileInfo fileInfoImage(imageFileName);
            QString imageBaseName=fileInfoImage.baseName();
            QString absoluteFilePath=fileInfoImage.absolutePath();
//            QString pathForImageMatchesFiles=imageMatchesDir+"/"+OPENCV_KPM_PREFIX_FOLDER_IMAGE_MATCHES_FILES+imageId;
            for(int nImageInPairs=0;nImageInPairs<imagePairs.size();nImageInPairs++)
            {
                QString imageMatchesDir=pathForMatchesFiles;
                QString imagePairId=imagePairs.at(nImageInPairs);
                if(!mImagesKeyPoints.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::KeyPoint> imagePairKeyPoints=mImagesKeyPoints[imagePairId];
                if(!mImagesKeyPointsDescriptors.contains(imagePairId))
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not key points descriptors for image id: \n %1").arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                std::vector<cv::Point2f> imageMatchingPointsReverse=matchingPointsFirstReverse[nImageInPairs];
                std::vector<cv::Point2f> imagePairMatchingPointsReverse=matchingPointsSecondReverse[nImageInPairs];
                if(imageMatchingPointsReverse.size()!=imagePairMatchingPointsReverse.size())
                {
                    strError=QObject::tr("OpenCVKpM::getOutliersDetector, not equal number of reverse matches points for image id: %1 and image pair: %2").arg(imageId).arg(imagePairId);
                    return(OPENCV_KPM_ERROR);
                }
                int numberOfMatches=(int)imageMatchingPointsReverse.size();
                QString pathForImageMatchesFiles=imageMatchesDir+"/"+OPENCV_KPM_PREFIX_FOLDER_IMAGE_MATCHES_FILES+imagePairId;
                QDir auxDir(QDir::currentPath());
                if(!auxDir.exists(pathForImageMatchesFiles))
                {
                    if(!auxDir.mkdir(pathForImageMatchesFiles))
                    {
                        strError=QObject::tr("OpenCVKpM::writeMatchesFiles, error creating folder for image matches files: \n %1").arg(pathForImageMatchesFiles);
                        return(OPENCV_KPM_ERROR);
                    }
                }
                QString matchesFileName=pathForImageMatchesFiles+"/"+imageId+OPENCV_KPM_PREFIX_IMAGE_MATCHES_FILE_EXTENSION;
                QFile file(matchesFileName);
                if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    strError=QObject::tr("OpenCVKpM::writeMatchesFiles, error creating file for image matches: \n %1").arg(matchesFileName);
                    return(OPENCV_KPM_ERROR);
                }
                QTextStream out(&file);
                for( int kk = 0; kk < numberOfMatches; kk++ )
                {
                    cv::Point2f pt1 = imageMatchingPointsReverse[kk];
                    cv::Point2f pt2 = imagePairMatchingPointsReverse[kk];
                    double x1=pt1.x;
                    double y1=pt1.y;
                    double x2=pt2.x;
                    double y2=pt2.y;
                    out<<QString::number(x1,'f',OPENCV_KPM_IMAGE_MATCHES_PRECISION)<<" ";
                    out<<QString::number(y1,'f',OPENCV_KPM_IMAGE_MATCHES_PRECISION)<<" ";
                    out<<QString::number(x2,'f',OPENCV_KPM_IMAGE_MATCHES_PRECISION)<<" ";
                    out<<QString::number(y2,'f',OPENCV_KPM_IMAGE_MATCHES_PRECISION)<<endl;
                }
            }
        }
        iterImagePairs++;
    }
    return(OPENCV_KPM_NO_ERROR);
}
