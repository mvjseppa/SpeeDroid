/*
 * SpeeDroid
 * Speed sign detector for Android
 *
 * Copyright (c) 2015, Mikko Seppälä
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
bool detectFalsePositives(Mat& signCandidate);

#define SIGN1_SIZE	400
#define SIGN2_SIZE	200
#define SIGN3_SIZE	100

static Mat detectedSigns[3];

extern "C" {
JNIEXPORT void JNICALL Java_mvs_speedroid_SpeeDroidActivity_ProcessImage(JNIEnv*, jobject, jlong addrRgba, jint roiWidth, jint roiHeight);
JNIEXPORT void JNICALL Java_mvs_speedroid_SpeeDroidActivity_InitJniPart(JNIEnv*, jobject);
JNIEXPORT void JNICALL Java_mvs_speedroid_SpeeDroidActivity_DestroyJniPart(JNIEnv*, jobject);


/*
 * Initialize SpeeDroid JNI module. Set the global Mat objects to zeros.
 *
 * PARAMS:
 * JNIEnv*			-	JNI function access pointer (not used)
 * jobject			-	Java "this" (not used)
 */
JNIEXPORT void JNICALL Java_mvs_speedroid_SpeeDroidActivity_InitJniPart(JNIEnv*, jobject){

	srand(time(NULL));

	detectedSigns[0] = Mat::zeros(Size(SIGN1_SIZE,SIGN1_SIZE), CV_8UC4);
	detectedSigns[1] = Mat::zeros(Size(SIGN2_SIZE,SIGN2_SIZE), CV_8UC4);
	detectedSigns[2] = Mat::zeros(Size(SIGN3_SIZE,SIGN3_SIZE), CV_8UC4);

	LOGD("SpeeDroid JNI part initialized.");
}

/*
 * "Destructor" for SpeeDroid JNI module. Releases the global Mat objects.
 *
 * PARAMS:
 * JNIEnv*			-	JNI function access pointer (not used)
 * jobject			-	Java "this" (not used)
 */
JNIEXPORT void JNICALL Java_mvs_speedroid_SpeeDroidActivity_DestroyJniPart(JNIEnv*, jobject){
	detectedSigns[0].release();
	detectedSigns[1].release();
	detectedSigns[2].release();

	LOGD("SpeeDroid JNI part exit.");
}


/*
 * SpeeDroid image processing is done here.
 *
 * PARAMS:
 * JNIEnv*			-	JNI function access pointer (not used)
 * jobject			-	Java "this" (not used)
 * jlong addrRgba	-	Memory address of frame to process (OpenCV Mat)
 * jint				-	ROI width as percentage of image
 * jint				-	ROI height as percentage of
 */
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

    if(roiW == 0){
    	LOGD("ROI width set to 0!");
    	return;
    }

    if(roiH == 0){
    	LOGD("ROI height set to 0!");
    	return;
    }

    //Timer for cooldown after successful detection
    static SimpleTimer resultCooldown = SimpleTimer();

    //Circle to store RANSAC result
    CircleType c = {Point(0,0), 0};

    if(rgb.cols < 2*roiW || rgb.rows < roiH){
    	LOGD("Too small frame!");
    	return;
    }

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

    	//correct the coordinate if the circle center is in roi 2
		if(c.center.x >= roiW){
			c.center.x = (c.center.x - roiW) + (rgb.cols - roiW);
		}

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

		//Draw RANSAC result on output Mat
		circle(rgb, c.center, c.radius, Scalar(0,255,255), 4);
    }

    //Draw the ROI on the output Mat
	rectangle(rgb, Rect(0,0,roiW,roiH), Scalar(0,0,255), 2);
	rectangle(rgb, Rect(rgb.cols-roiW,0,roiW,roiH), Scalar(0,0,255), 2);

	//Draw detection results to output Mat
	drawDetectedSigns(rgb);

    thresh.release();
    mainRoiImg.release();
    cropped.release();

    endTime = clock();
    LOGD("JNI exec time: %lf", (double(endTime - startTime) / CLOCKS_PER_SEC));

}
} //extern "C"

/*
 * Some checks to filter out most common false positives in speed sign detection.
 *
 * PARAMS:
 * Mat& signCandidate	-	cropped image that potentially contains a speed sign
 *
 * RETURNS:
 * bool		-	true if a sign candidate is identified as a false positive
 * 				false otherwise
 */
bool detectFalsePositives(Mat& signCandidate){
	Mat bin, edge;
	bool retval = false;

	//a traffic sign should have a significant amount of yellow pixels
	findYellow(signCandidate, bin);
	if(countNonZero(bin) < 0.3 * bin.rows * bin.cols) retval = true;

	/*

	//TODO:
	//find non-yellow blobs in the image
	//try to exclude brick walls, bushes etc based on blob count

	//invert the pixels
	bitwise_not(bin,bin);

	int largest_area=0;
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
	//edge.release();

	return retval;
}


/*
 * Store an identified sign into global image buffer.
 *
 * PARAMS:
 * Mat& newSign		-	image to store
 */
void updateDetectedSigns(Mat& newSign){
	resize(detectedSigns[1], detectedSigns[2], Size(SIGN3_SIZE, SIGN3_SIZE));
	resize(detectedSigns[0], detectedSigns[1], Size(SIGN2_SIZE, SIGN2_SIZE));
	resize(newSign, detectedSigns[0], Size(SIGN1_SIZE, SIGN1_SIZE));
}

/*
 * Copy the detection results from global buffers to img.
 *
 * PARAMS:
 * Mat& img		-	Detection results are drawn on this image
 */
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

/*
 * Extract FAST features on the image.
 * TODO: Try to identify the detected sign using the features.
 *
 * PARAMS:
 * Mat& img		-	Detect FAST features on this image and mark them with circles.
 */
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
