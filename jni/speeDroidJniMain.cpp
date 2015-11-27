/*
 * SpeeDroid
 * Speed sign detector for Android
 *
 * Copyright (c) 2015, Mikko Sepp�l�
 * All rights reserved.
 *
 * See LICENSE.md file for licensing details.
 *
 */

#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cmath>

#include "debugprint.hpp"
#include "SimpleTimer.hpp"
#include "CircleRANSAC.hpp"
#include "speeDroidUtils.hpp"

using namespace cv;

void updateDetectedSigns(Mat& newSign);
void drawDetectedSigns(Mat& img);
void findFeatures(Mat& img);
bool detectFalsePositives(Mat signCandidate);

#define SIGN1_SIZE	400
#define SIGN2_SIZE	200
#define SIGN3_SIZE	100

static Mat detectedSigns[3];

extern "C" {
JNIEXPORT void JNICALL Java_mvs_speedroid_SpeeDroidActivity_ProcessImage(JNIEnv*, jobject, jlong addrRgba, jint roiWidth, jint roiHeight);
JNIEXPORT void JNICALL Java_mvs_speedroid_SpeeDroidActivity_InitJniPart(void);
JNIEXPORT void JNICALL Java_mvs_speedroid_SpeeDroidActivity_DestroyJniPart(void);


//init function to set globals
JNIEXPORT void JNICALL Java_mvs_speedroid_SpeeDroidActivity_InitJniPart(void){

	srand(time(NULL));

	detectedSigns[0] = Mat::zeros(Size(SIGN1_SIZE,SIGN1_SIZE), CV_8UC4);
	detectedSigns[1] = Mat::zeros(Size(SIGN2_SIZE,SIGN2_SIZE), CV_8UC4);
	detectedSigns[2] = Mat::zeros(Size(SIGN3_SIZE,SIGN3_SIZE), CV_8UC4);

	LOGD("SpeeDroid JNI part initialized.");
}

//"destructor" to release globals
JNIEXPORT void JNICALL Java_mvs_speedroid_SpeeDroidActivity_DestroyJniPart(void){
	detectedSigns[0].release();
	detectedSigns[1].release();
	detectedSigns[2].release();

	LOGD("SpeeDroid JNI part exit.");
}

JNIEXPORT void JNICALL Java_mvs_speedroid_SpeeDroidActivity_ProcessImage(JNIEnv*, jobject, jlong addrRgba, jint roiWidth, jint roiHeight)
{


    //Get the image data from Java side pointer
	Mat& rgb = *(Mat*)addrRgba;

	//Buffer images
    Mat thresh;
    Mat mainRoiImg;
    Mat cropped;

    //log the function execution time
    clock_t startTime = clock();
    clock_t endTime;

    //We do the processing for to regions of interest.
    //We create a ROI on both sides of the image to scan both sides of the road.
    //Both ROIs have same dimensions.
    unsigned int roiW = (unsigned int)(rgb.cols/2 * (roiWidth / 100.0));
    unsigned int roiH = (unsigned int)(rgb.rows * (roiHeight / 100.0));

    //Timer for cooldown after successful detection
    static SimpleTimer resultCooldown = SimpleTimer();

    //Circle to store RANSAC result
    CircleType c = {Point(0,0), 0};


    if(rgb.cols < 2*roiW || rgb.rows < roiH) return;

    //Copy the rois from input image
    mainRoiImg = Mat::zeros(Size(2*roiW,roiH), CV_8UC4);
	rgb(Rect(0,0,roiW,roiH)).copyTo(mainRoiImg(Rect(0,0,roiW,roiH)));
	rgb(Rect(rgb.cols-roiW, 0, roiW, roiH)).copyTo(mainRoiImg(Rect(roiW,0,roiW,roiH)));


	//median blur on the rois to remove noise but preserve edges
    //this is quite expensive operation!!
    //medianBlur(mainRoiImg, mainRoiImg, 3);

    //Find red pixels in the image
    findRed(mainRoiImg, thresh);



    //Identify red circles as traffic sign candidates
    if (CircleRANSAC(thresh, c)){

    	//bin1.copyTo(rgb(Rect(0,0,bin1.cols, bin1.rows)));

    	//correct the coordinate if the circle center is in roi 2
		if(c.center.x >= roiW){
			c.center.x = (c.center.x - roiW) + (rgb.cols - roiW);
		}

		//LOGD("%d,%d,%d", c.center.x, c.center.y, c.radius);

		Point displace(c.radius, c.radius);
		Rect roiSign(c.center-displace, c.center+displace);

		safeCrop(rgb, cropped, roiSign);


		//update results if the sign candidate is accepted and
		//2 seconds has elapsed since last detection
		if(!detectFalsePositives(cropped) && resultCooldown.isElapsed()){

			//findFeatures(cropped);
			updateDetectedSigns(cropped);
			resultCooldown.startTimer(2.0);
		}

		//circle(rgb, c.center, c.radius, Scalar(255,0,255), 4);
    }

	rectangle(rgb, Rect(0,0,roiW,roiH), Scalar(0,0,255), 2);
	rectangle(rgb, Rect(rgb.cols-roiW,0,roiW,roiH), Scalar(0,0,255), 2);

    //cvtColor(bin2, bin2, COLOR_GRAY2RGBA);
    //resize(bin2, rgb, Size(rgb.cols, rgb.rows));
	//findYellow(rgb, rgb);
	//cvtColor(rgb,rgb,COLOR_GRAY2RGBA);

	drawDetectedSigns(rgb);

    thresh.release();
    mainRoiImg.release();
    cropped.release();

    endTime = clock();
    LOGD("JNI exec time: %lf", (double(endTime - startTime) / CLOCKS_PER_SEC));

}
} //extern "C"

//some checks to identify most common false positives.
//returns true if a sign candidate is identified as a false positive.
bool detectFalsePositives(Mat signCandidate){
	Mat bin, edge;
	bool retval = false;

	//a traffic sign should have a significant amount of yellow pixels
	findYellow(signCandidate, bin);
	if(countNonZero(bin) < 0.3 * bin.rows * bin.cols) retval = true;

	//invert the pixels
	bitwise_not(bin,bin);

	//find non-yellow blobs in the image
	//try to exclude brick walls, bushes etc based on blob count

/*	int largest_area=0;
	int largest_contour_index=0;

	vector< vector<Point> > contours; // Vector for storing contour
	vector<Vec4i> hierarchy;

	findContours( bin, contours, hierarchy,CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE ); // Find the contours in the image

	for( int i = 0; i< contours.size(); i++ ) {// iterate through each contour.
		double a=contourArea( contours[i],false);  //  Find the area of contour
		if(a>largest_area){
			largest_area=a;
			largest_contour_index=i;                //Store the index of largest contour
			//bounding_rect=boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
		}
	}
*/

	bin.release();
	edge.release();

	return retval;
}

void updateDetectedSigns(Mat& newSign){
	resize(detectedSigns[1], detectedSigns[2], Size(SIGN3_SIZE, SIGN3_SIZE));
	resize(detectedSigns[0], detectedSigns[1], Size(SIGN2_SIZE, SIGN2_SIZE));
	resize(newSign, detectedSigns[0], Size(SIGN1_SIZE, SIGN1_SIZE));
}

void drawDetectedSigns(Mat& img){
	Rect roiSign;
	Mat mask;

	//check that the image is big enough...
	if(img.cols < 1280 || img.rows < 720) return;

	//Upper left corner of last detected sign goes here
	Point p(img.cols-SIGN1_SIZE-5,img.rows-SIGN1_SIZE-5);

	//Sign 1
	mask = Mat::zeros(SIGN1_SIZE,SIGN1_SIZE,CV_8UC1);
	circle(mask,Point(SIGN1_SIZE/2,SIGN1_SIZE/2),SIGN1_SIZE/2,Scalar(255,255,255),-1);
	roiSign = Rect(p.x, p.y, SIGN1_SIZE, SIGN1_SIZE);
	detectedSigns[0].copyTo(img(roiSign), mask);

	//Sign 2
	mask = Mat::zeros(SIGN2_SIZE,SIGN2_SIZE,CV_8UC1);
	circle(mask,Point(SIGN2_SIZE/2,SIGN2_SIZE/2),SIGN2_SIZE/2,Scalar(255,255,255),-1);
	roiSign = Rect(p.x-SIGN2_SIZE, p.y+SIGN2_SIZE/2, SIGN2_SIZE, SIGN2_SIZE);
	detectedSigns[1].copyTo(img(roiSign), mask);

	//Sign 3
	mask = Mat::zeros(SIGN3_SIZE,SIGN3_SIZE,CV_8UC1);
	circle(mask,Point(SIGN3_SIZE/2,SIGN3_SIZE/2),SIGN3_SIZE/2,Scalar(255,255,255),-1);
	roiSign = Rect(p.x-SIGN2_SIZE-SIGN3_SIZE, p.y+SIGN2_SIZE/2+SIGN3_SIZE/2, SIGN3_SIZE, SIGN3_SIZE);
	detectedSigns[2].copyTo(img(roiSign),mask);

	mask.release();
}

void findFeatures(Mat& img){

	vector<KeyPoint> features;
    Mat gray;


    cvtColor(img, gray, COLOR_RGB2GRAY);

	medianBlur(gray, gray, 5);
    Canny(gray, gray, 80, 100, 5);

    FastFeatureDetector detector(50);
    detector.detect(gray, features);

    cvtColor(gray,img,COLOR_GRAY2RGBA);

    for( unsigned int i = 0; i < features.size(); i++ )
    {
        const KeyPoint& kp = features[i];
        circle(img, Point(kp.pt.x, kp.pt.y), 10, Scalar(0,0,255,255), 1);
    }

    gray.release();
}
