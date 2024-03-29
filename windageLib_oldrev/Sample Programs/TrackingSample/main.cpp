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

#define ADAPTIVE_THRESHOLD

const int FIND_FEATURE_COUNT = 10;

const int MAX_FAST_THRESHOLD = 80;
const int MIN_FAST_THRESHOLD = 20;
const int ADAPTIVE_THRESHOLD_VALUE = 500;
const int THRESHOLD_STEP = 1;

const int WIDTH = 640;
const int HEIGHT = 480;
const double intrinsicValues[8] = {1029.400, 1028.675, 316.524, 211.395, -0.206360, 0.238378, 0.001089, -0.000769};

windage::ModifiedSURFTracker* CreateTracker(IplImage* refImage, int index)
{
	windage::ModifiedSURFTracker* tracker = new windage::ModifiedSURFTracker();
	tracker->Initialize(intrinsicValues[0], intrinsicValues[1], intrinsicValues[2], intrinsicValues[3], intrinsicValues[4], intrinsicValues[5], intrinsicValues[6], intrinsicValues[7], 30);
	tracker->RegistReferenceImage(refImage, 267.0, 200.0, 4.0, 8);
	tracker->SetPoseEstimationMethod(windage::LMEDS);
	tracker->SetOutlinerRemove(true);
	tracker->InitializeOpticalFlow(WIDTH, HEIGHT, 10, cvSize(8, 8), 3);
	tracker->SetOpticalFlowRunning(true);
	tracker->SetRefinement(false);
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
	
	// Tracker Initialize
	IplImage* trainingImage = cvLoadImage("reference1_320.png", 0);
	IplImage* referenceImage = cvLoadImage("reference1.png");
	IplImage* resultImage = cvCreateImage(cvSize(WIDTH, HEIGHT*2), IPL_DEPTH_8U, 3);
	IplImage* resultImage2 = cvCreateImage(cvSize(resultImage->width/2+10, resultImage->height/2+10), IPL_DEPTH_8U, 3);
	windage::ModifiedSURFTracker* tracker = CreateTracker(trainingImage, 0);

	// for undistortion
	windage::Calibration* calibration = new windage::Calibration();
	calibration->Initialize(intrinsicValues[0], intrinsicValues[1], intrinsicValues[2], intrinsicValues[3], intrinsicValues[4], intrinsicValues[5], intrinsicValues[6], intrinsicValues[7]);
	calibration->InitUndistortionMap(WIDTH, HEIGHT);

	// adaptive threshold
	int fastThreshold = 70;
	IplImage* grabFrame = NULL;

	bool saving = false;
	bool processing = true;
//	cvNamedWindow("result");
	cvNamedWindow("matching");

	while(processing)
	{
		// camera frame grabbing and convert to gray and undistortion
		log->updateTickCount();
		grabFrame = cvQueryFrame(capture);
//		cvFlip(grabFrame, grabFrame);
		cvResize(grabFrame, tempImage);
		calibration->Undistortion(tempImage, inputImage);
		cvCvtColor(inputImage, grayImage, CV_BGRA2GRAY);
		log->log("capture", log->calculateProcessTime());

		// call tracking algorithm
		log->updateTickCount();

		tracker->SetFeatureExtractThreshold(fastThreshold);
		tracker->UpdateCameraPose(grayImage);
		int featureCount = tracker->GetFeatureCount();
		int matchingCount = tracker->GetMatchedCount();

		double trackingTime = log->calculateProcessTime();
		log->log("tracking", trackingTime);
		log->logNewLine();
			
		// update fast threshold for Adaptive threshold
#ifdef ADAPTIVE_THRESHOLD
		if(featureCount > ADAPTIVE_THRESHOLD_VALUE )	fastThreshold = MIN(MAX_FAST_THRESHOLD, fastThreshold+THRESHOLD_STEP);
		else											fastThreshold = MAX(MIN_FAST_THRESHOLD, fastThreshold-THRESHOLD_STEP);
#endif
		
		// draw tracking result
		if(featureCount > FIND_FEATURE_COUNT)
		{
			tracker->DrawOutLine(inputImage, true);
			tracker->DrawInfomation(inputImage, 100.0);
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
		case 's':
		case 'S':
			saving = true;
			break;
		case 'q':
		case 'Q':
			processing = false;
			break;
		}

		cvZero(resultImage);
		cvSetImageROI(resultImage, cvRect(0, 0, WIDTH, HEIGHT));
		cvCopy(referenceImage, resultImage);
		cvSetImageROI(resultImage, cvRect(0, HEIGHT, WIDTH, HEIGHT));
		cvCopy(inputImage, resultImage);
		cvResetImageROI(resultImage);
		tracker->DrawDebugInfo2(resultImage);

		cvRectangle(resultImage2, cvPoint(0, 0), cvPoint(resultImage2->width, resultImage2->height), CV_RGB(255, 255, 255), CV_FILLED);
		cvSetImageROI(resultImage2, cvRect(5, 5, resultImage2->width-10, resultImage2->height-10));
		cvResize(resultImage, resultImage2);
		cvResetImageROI(resultImage2);

		cvShowImage("result", inputImage);
		cvShowImage("matching", resultImage2);

		if(saving)
		{
			cvSaveImage("matching.png", resultImage);
			cvSaveImage("result.png", inputImage);
			saving = false;
		}
	}

	cvReleaseCapture(&capture);
	cvDestroyAllWindows();
}
