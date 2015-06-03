#ifndef COUNTER_H_
#define COUNTER_H_

#include <vector>
#include "opencv2/core/core.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/highgui/highgui.hpp"

class Counter
{
public:
	//driving top down
	//detectionZone points
	//1 - 2
	//|   |
	//4 - 3
	Counter(cv::Mat& detectionZone, cv::Size& frameSize,  int stripeNumber, double stripeThr, bool isBGSColor);
	~Counter();
	int processFrame(cv::Mat& inpFrame);		

private:
	void initDetectionZoneStripes(cv::Mat& detectionZone, int stripeNumber);
	void bgSubtractionCycle(cv::Mat& inpFrame);

	const bool debugVideo = true;

	cv::BackgroundSubtractorMOG** bgSubtractors;
	std::vector<cv::Point2f> detectionZone;
	cv::VideoWriter* vidWriter;
	cv::Mat** fgMasks;
	std::vector<cv::Mat> stripes;
	cv::Point2f zoneShift;
	std::vector<double> stripeSizes;

	int countResult;
	int enteringNumber;
	int lastCounted;
	std::vector<int> currentObjects;
	double stripeThr;
	
	cv::Rect* zoneRect;
	cv::Mat *fgMask, *fgMask1;
	cv::Mat* inputFrameCh;
	bool isBGSColor;

};


#endif
