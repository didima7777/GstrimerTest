#include "Counter.h"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include  "opencv2/video/background_segm.hpp"
#include <iostream>

using namespace std;

Counter::Counter(cv::Mat& detectionZone, cv::Size& frameSize,  int stripeNumber, double stripeThr, bool isBGSColor)
{
	this->detectionZone = detectionZone;
	this->stripeThr = stripeThr;
	this->isBGSColor = isBGSColor;

	if (isBGSColor)
	{
		bgSubtractors = new cv::BackgroundSubtractorMOG*[3];
		fgMasks = new cv::Mat*[3];
	}
	else {
		bgSubtractors = new cv::BackgroundSubtractorMOG*;
		fgMasks = new cv::Mat*;
	}

	int history = 100;
	int nmixtures = 5;
	double bgRatio = 0.7;
	double noiseSigma = 15.0;
	if (isBGSColor)
	{
		*bgSubtractors = new cv::BackgroundSubtractorMOG(history, nmixtures, bgRatio, noiseSigma);		
	}
	else {
		for (int i = 0; i < 3; i++)
		{
			bgSubtractors[i] = new cv::BackgroundSubtractorMOG(history, nmixtures, bgRatio, noiseSigma);		
		}
	}


	initDetectionZoneStripes(detectionZone, stripeNumber);

	if (isBGSColor)
	{
		*fgMasks = new cv::Mat(zoneRect->size(), CV_8UC1);
	}
	else {
		for (int i = 0; i < 3; i++)
		{		
			fgMasks[i] = new cv::Mat(zoneRect->size(), CV_8UC1);	
		}
	}

	fgMask = new cv::Mat((*fgMasks[0]).size(), CV_8UC1);
	fgMask1 = new cv::Mat((*fgMasks[0]).size(), CV_8UC1);


	inputFrameCh = new cv::Mat[3];

	countResult = 0;
	enteringNumber = 1;
	lastCounted = 0;
}


Counter::~Counter()
{
	if (debugVideo)
	{
		vidWriter->release();
		delete vidWriter;
	}
}

void Counter::bgSubtractionCycle(cv::Mat& inpFrame)
{
	cv::split(inpFrame, inputFrameCh);
	for (int i = 0; i < 3; i++)
	{
		//cv::imwrite("1.bmp", inputFrameCh[i]);	
		(*bgSubtractors[i])(inputFrameCh[i], *fgMasks[i], 0.00005);
		//cv::imwrite("2.bmp", *fgMasks[i]);
	}
	cv::bitwise_or(*fgMasks[0], *fgMasks[1], *fgMask1);
	cv::bitwise_or(*fgMask1, *fgMasks[2], *fgMask);
}



int Counter::processFrame(cv::Mat& inpFrame)
{
	clock_t start;
	clock_t diff1;

	start=clock();
	cv::Mat zoneMask, debugFGMask, debugZoneMask;
	
		cv::Mat inpFrame1 = inpFrame(*zoneRect);
		if (isBGSColor)
		{
			(**bgSubtractors)(inpFrame1, **fgMasks, 0.00005);
		}
		else
		{
			bgSubtractionCycle(inpFrame1);
		}
		zoneMask = *fgMask;

	diff1=clock()-start;
	//cout<<"substartor : "<<(float)diff1/CLOCKS_PER_SEC<<endl;

	start=clock();
	double zoneArea = sqrt(zoneMask.dot(zoneMask));			

	std::vector<double> stripeResults;
	std::vector<uchar> stripeDetections;	

	for (int stripeInd = 0; stripeInd < stripes.size(); stripeInd++)
	{
		double dotRes1 = stripes[stripeInd].dot(zoneMask);
		double dotRes = (dotRes1) / stripeSizes[stripeInd] / (zoneArea + 1.0);
		stripeResults.push_back(dotRes);
		if (dotRes > stripeThr)
		{			
			if (debugVideo)
				debugZoneMask = debugZoneMask + (stripes[stripeInd] / 2);
			stripeDetections.push_back(1);
		}
		else {			
			stripeDetections.push_back(0);
		}
	}	
	diff1=clock()-start;
	//cout<<"mul : "<<(float)diff1/CLOCKS_PER_SEC<<endl;

	if (currentObjects.size() == 0)
	{
		for (int stripeInd = 0; stripeInd < stripes.size(); stripeInd++)
			currentObjects.push_back(0);
	}
	std::vector<int> newObjects;
	for (int stripeInd = 0; stripeInd < stripes.size(); stripeInd++)
		newObjects.push_back(0);

	for (int stripeInd = 0; stripeInd < stripes.size(); stripeInd++)
	{
		if (stripeDetections[stripeInd] && stripeInd == 0)
		{
			if (currentObjects[stripeInd] == 0)
			{
				newObjects[stripeInd] = enteringNumber;
				enteringNumber++;
			}
			else {				
				newObjects[stripeInd] = currentObjects[stripeInd];
			}			
		}
		if (stripeDetections[stripeInd] && stripeInd > 0)
		{
			if (currentObjects[stripeInd - 1] && !currentObjects[stripeInd])
			{
				newObjects[stripeInd] = currentObjects[stripeInd - 1];
			}
			if (currentObjects[stripeInd])
			{
				newObjects[stripeInd] = currentObjects[stripeInd];
			}

		}
		if (!stripeDetections[stripeInd] && stripeInd == stripes.size()-1)
		{
			if (currentObjects[stripeInd])
			{
				bool created = false;
				int newId = -1;
				for (int befInd = stripeInd - 1; befInd >= 0; befInd--)
				{
					if (newObjects[befInd] == currentObjects[stripeInd])
					{
						if (!created)
						{
							newId = enteringNumber;
							enteringNumber++;
							created = true;
						}
						newObjects[befInd] = newId;
					}
				}
			}
		}
		if (!stripeDetections[stripeInd] && stripeInd == stripes.size() - 1)
		{
			if (currentObjects[stripeInd]  && currentObjects[stripeInd] != lastCounted)
			{
				countResult++;
				lastCounted = currentObjects[stripeInd];
			}
		}
	}
	currentObjects = newObjects;

	return countResult;

}

void Counter::initDetectionZoneStripes(cv::Mat& detectionZone, int stripeNum)
{
	int maxX = 0, maxY = 0, minX = 1e9, minY = 1e9;
	for (int i = 0; i < detectionZone.rows; i++)
	{
		float xVal = detectionZone.at<cv::Point2f>(i).x;
		if (xVal > maxX)
		{
			maxX = xVal;
		}
		float yVal = detectionZone.at<cv::Point2f>(i).y;
		if (yVal > maxY)
		{
			maxY = yVal;
		}
		if (xVal < minX)
		{
			minX = xVal;
		}		
		if (yVal < minY)
		{
			minY = yVal;
		}
	}
	zoneShift.x = minX;
	zoneShift.y = minY;
	cv::Size zoneRectSize(maxX - minX + 1, maxY - minY + 1);

	cv::Mat canonicalPoints(4, 1, CV_32FC2);
	cv::Point2f pt1(0, 0);
	canonicalPoints.at<cv::Point2f>(0) = pt1;
	cv::Point2f pt2(1, 0);
	canonicalPoints.at<cv::Point2f>(1) = pt2;
	cv::Point2f pt3(1, 1);
	canonicalPoints.at<cv::Point2f>(2) = pt3;
	cv::Point2f pt4(0, 1);
	canonicalPoints.at<cv::Point2f>(3) = pt4;

	for (int i = 0; i < 4; i++)
	{
		detectionZone.at<cv::Point2f>(i, 0).x -= zoneShift.x;
		detectionZone.at<cv::Point2f>(i, 0).y -= zoneShift.y;
	}
	cv::Mat H = findHomography(detectionZone, canonicalPoints);	
	for (int stripeInd = 0; stripeInd < stripeNum; stripeInd++)
	{
		cv::Mat stripeMask(zoneRectSize, CV_8UC1);
		stripeMask.setTo(0);
		stripes.push_back(stripeMask);
	}
	
	std::vector<cv::Point2f> stripePts;
	for (int rowInd = 0; rowInd < zoneRectSize.height; rowInd++)
	{
		for (int colInd = 0; colInd < zoneRectSize.width; colInd++)
		{

			cv::Point2f pth;
			pth.x = colInd;
			pth.y = rowInd; 
			stripePts.push_back(pth);
		}
	}
	std::vector<cv::Point2f> canonPts;

	cv::perspectiveTransform(stripePts, canonPts, H);
	int ptInd = 0;
	for (int rowInd = 0; rowInd < zoneRectSize.height; rowInd++)
	{
		for (int colInd = 0; colInd < zoneRectSize.width; colInd++)
		{
			cv::Point2f pth = canonPts[ptInd];
			int stripeInd = floor(pth.y * stripeNum);
			if (stripeInd >= 0 && stripeInd < stripeNum && pth.x >= 0 && pth.x <= 1.0)
			{
				stripes[stripeInd].at<uchar>(rowInd, colInd) = 255;
			}
			ptInd++;
		}
	}		
	for (int stripeInd = 0; stripeInd < stripeNum; stripeInd++)
	{
		double stripeNorm = sqrt(stripes[stripeInd].dot(stripes[stripeInd]));		
		stripeSizes.push_back(stripeNorm);
		if (debugVideo)
		{
			std::string maskPath = "m" + std::to_string(stripeInd) + ".bmp";
			cv::imwrite(maskPath, stripes[stripeInd]);
		}
	}
	zoneRect = new cv::Rect(zoneShift.x, zoneShift.y, stripes[0].cols, stripes[0].rows);
	
}
