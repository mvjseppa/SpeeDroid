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



/*
 * Crops the ROI from src to dst. If ROI won't fit inside src, crop as much as possible.
 *
 * PARAMS:
 * const Mat& src		-	source image
 * Mat& dst				-	destination image
 * const Rect roi		-	region to crop
 */

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "speedroidutils.hpp"

void safeCrop(const Mat& src, Mat& dst, const Rect &roi){
	Rect roiSafe = roi & Rect(0, 0, src.cols, src.rows);
	dst = src(roiSafe);
}



/*
 * Find red pixels in src and set then white in dst
 *
 * PARAMS:
 * Mat& src 	-	source image (RGB)
 * Mat& dst		-	destination image (BIN)
 */
void findRed(Mat& src, Mat& dst){

	Mat hsv, bin1, bin2;

    cvtColor(src, hsv, COLOR_RGB2HSV);
    inRange(hsv, Scalar(0, 100, 100), Scalar(10,255,255), bin1);
    inRange(hsv, Scalar(170, 100, 100), Scalar(179,255,255), bin2);

    bitwise_or(bin1, bin2, dst);

    hsv.release();
    bin1.release();
    bin2.release();
}

/*
 * Find yellow pixels in src and set then white in dst
 *
 * PARAMS:
 * Mat& src 	-	source image (RGB)
 * Mat& dst		-	destination image (BIN)
 */
void findYellow(Mat& src, Mat& dst){
	Mat hsv;
    cvtColor(src, hsv, COLOR_RGB2HSV);
    inRange(hsv, Scalar(15, 60, 80), Scalar(35,255,255), dst);
    hsv.release();
}

/*
 * Find black (dark gray) pixels in src and set then white in dst
 *
 * PARAMS:
 * Mat& src 	-	source image (RGB)
 * Mat& dst		-	destination image (BIN)
 */
void findBlack(Mat& src, Mat& dst){
	Mat hsv;
    cvtColor(src, hsv, COLOR_RGB2HSV);
    inRange(hsv, Scalar(0,0,0), Scalar(180,255,100), dst);
    hsv.release();
}
