#define RUNNING
#ifdef RUNNING

/* ========================================================================
 * PROJECT: windage Library
 * ========================================================================
 * This work is based on the original windage Library developed by
 *   Woonhyuk Baek
 *   Woontack Woo
 *   U-VR Lab, GIST of Gwangju in Korea.
 *   http://windage.googlecode.com/
 *   http://uvr.gist.ac.kr/
 *
 * Copyright of the derived and new portions of this work
 *     (C) 2009 GIST U-VR Lab.
 *
 * This framework is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this framework; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For further information please contact 
 *   Woonhyuk Baek
 *   <windage@live.com>
 *   GIST U-VR Lab.
 *   Department of Information and Communication
 *   Gwangju Institute of Science and Technology
 *   1, Oryong-dong, Buk-gu, Gwangju
 *   South Korea
 * ========================================================================
 ** @author   Woonhyuk Baek
 * ======================================================================== */

#include <iostream>

#include <highgui.h>
#include <windage.h>

#define FLIP
#define RECTIFICATION
#define ADAPTIVE_THRESHOLD

//#define USE_IMAGE_SEQUENCE

const int LEANING_SIZE = 400;

const int FIND_FEATURE_COUNT = 10;

const int MAX_FAST_THRESHOLD = 80;
const int MIN_FAST_THRESHOLD = 20;
const int ADAPTIVE_THRESHOLD_VALUE = 500;
const int THRESHOLD_STEP = 1;

const int WIDTH = 640;
const int HEIGHT = 480;
const double intrinsicValues[8] = {1029.400, 1028.675, 316.524, 211.395, -0.206360, 0.238378, 0.001089, -0.000769};

windage::ModifiedSURFTracker* CreateTracker(IplImage* refImage, int index=0)
{
	windage::ModifiedSURFTracker* tracker = new windage::ModifiedSURFTracker();
	tracker->Initialize(intrinsicValues[0], intrinsicValues[1], intrinsicValues[2], intrinsicValues[3], intrinsicValues[4], intrinsicValues[5], intrinsicValues[6], intrinsicValues[7], 30);
	tracker->RegistReferenceImage(refImage, (double)LEANING_SIZE, (double)LEANING_SIZE, 8.0, 8);
	tracker->SetPoseEstimationMethod(windage::PROSAC);
	tracker->SetOutlinerRemove(true);
	tracker->InitializeOpticalFlow(WIDTH, HEIGHT, 10, cvSize(8, 8), 3);
	tracker->SetOpticalFlowRunning(true);
//	tracker->GetCameraParameter()->InitUndistortionMap(WIDTH, HEIGHT);
	tracker->SetFeatureExtractThreshold(30);

	tracker->SetSetpIndex(index);
	
	return tracker;
}

void main()
{
	windage::Logger* log = new windage::Logger(&std::cout);

	// connect camera
	CvCapture* capture = cvCaptureFromCAM(CV_CAP_ANY);

	char message[100];
	IplImage* inputImage = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 3);
	IplImage* tempImage = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 3);
	IplImage* grayImage = cvCreateImage(cvGetSize(inputImage), IPL_DEPTH_8U, 1);

	IplImage* drawImage = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 3);
	
	// Tracker Initialize
	IplImage* referenceImage = NULL;//cvLoadImage("reference1_320.png", 0);
	windage::ModifiedSURFTracker* tracker = NULL;//CreateTracker(referenceImage, 0);

	// for undistortion
	windage::Calibration* calibration = new windage::Calibration();
	calibration->Initialize(intrinsicValues[0], intrinsicValues[1], intrinsicValues[2], intrinsicValues[3], intrinsicValues[4], intrinsicValues[5], intrinsicValues[6], intrinsicValues[7]);
	calibration->InitUndistortionMap(WIDTH, HEIGHT);

	// adaptive threshold
	int fastThreshold = 30;
	IplImage* grabFrame = NULL;

	bool learning = false;

	bool processing = true;
	cvNamedWindow("result");
	while(processing)
	{
		// camera frame grabbing and convert to gray and undistortion
		log->updateTickCount();
		grabFrame = cvQueryFrame(capture);
		cvFlip(grabFrame, grabFrame);
		cvResize(grabFrame, tempImage);
		calibration->Undistortion(tempImage, inputImage);
		cvCvtColor(inputImage, grayImage, CV_BGRA2GRAY);
		log->log("capture", log->calculateProcessTime());

		// call tracking algorithm
		log->updateTickCount();

		int featureCount = 0;
		int matchingCount = 0;
		if(tracker)
		{			
			tracker->SetFeatureExtractThreshold(fastThreshold);
			tracker->UpdateCameraPose(grayImage);
			featureCount = tracker->GetFeatureCount();
			matchingCount = tracker->GetMatchedCount();

		// update fast threshold for Adaptive threshold
#ifdef ADAPTIVE_THRESHOLD
			if(featureCount > ADAPTIVE_THRESHOLD_VALUE )	fastThreshold = MIN(MAX_FAST_THRESHOLD, fastThreshold+THRESHOLD_STEP);
			else											fastThreshold = MAX(MIN_FAST_THRESHOLD, fastThreshold-THRESHOLD_STEP);
#endif
		}

		double trackingTime = log->calculateProcessTime();
		log->log("tracking", trackingTime);
		log->logNewLine();
			
		// draw tracking result
		if(featureCount > FIND_FEATURE_COUNT)
		{
			tracker->DrawOutLine(inputImage, true);
			tracker->DrawInfomation(inputImage, LEANING_SIZE/4.0);
//			tracker->DrawDebugInfo(inputImage);
		}

		sprintf(message, "Tracking Time : %.2f(ms)", trackingTime);
		windage::Utils::DrawTextToImage(inputImage, cvPoint(20, 30), message);
		sprintf(message, "FAST feature count : %d, threashold : %d", featureCount, fastThreshold);
		windage::Utils::DrawTextToImage(inputImage, cvPoint(20, 50), message);
		sprintf(message, "Match count : %d", matchingCount);
		windage::Utils::DrawTextToImage(inputImage, cvPoint(20, 70), message);

		char ch = cvWaitKey(1);
		switch(ch)
		{
		case 'l':
		case 'L':
			cvSetImageROI(grayImage, cvRect(grayImage->width/2 - LEANING_SIZE/2, grayImage->height/2 - LEANING_SIZE/2, LEANING_SIZE, LEANING_SIZE));
			if(tracker) delete tracker;
			tracker = CreateTracker(grayImage, 0);
			cvResetImageROI(grayImage);
			break;
		case 'q':
		case 'Q':
			processing = false;
			break;
		}

		cvZero(drawImage);
		cvLine(drawImage,	cvPoint(grayImage->width/2 - LEANING_SIZE/2, grayImage->height/2 - LEANING_SIZE/2),
							cvPoint(grayImage->width/2 + LEANING_SIZE/2, grayImage->height/2 - LEANING_SIZE/2), CV_RGB(255, 0, 0), 3); 
		cvLine(drawImage,	cvPoint(grayImage->width/2 + LEANING_SIZE/2, grayImage->height/2 - LEANING_SIZE/2),
							cvPoint(grayImage->width/2 + LEANING_SIZE/2, grayImage->height/2 + LEANING_SIZE/2), CV_RGB(255, 0, 0), 3);
		cvLine(drawImage,	cvPoint(grayImage->width/2 + LEANING_SIZE/2, grayImage->height/2 + LEANING_SIZE/2),
							cvPoint(grayImage->width/2 - LEANING_SIZE/2, grayImage->height/2 + LEANING_SIZE/2), CV_RGB(255, 0, 0), 3);
		cvLine(drawImage,	cvPoint(grayImage->width/2 - LEANING_SIZE/2, grayImage->height/2 + LEANING_SIZE/2),
							cvPoint(grayImage->width/2 - LEANING_SIZE/2, grayImage->height/2 - LEANING_SIZE/2), CV_RGB(255, 0, 0), 3);
		cvCircle(drawImage, cvPoint(grayImage->width/2, grayImage->height/2), LEANING_SIZE/6.0, CV_RGB(255, 0, 0), 3);
		windage::Utils::CompundImmersiveImage(drawImage, inputImage, CV_RGB(0, 0, 0), 0.5);


		cvShowImage("result", inputImage);
	}

	cvReleaseCapture(&capture);
	cvDestroyAllWindows();
}

#endif