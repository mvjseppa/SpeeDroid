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

#include <opencv2/core/core.hpp>

#ifndef SPEEDROIDUTILS_H_
#define SPEEDROIDUTILS_H_

using namespace std;
using namespace cv;

void safeCrop(const Mat& src, Mat& dst, const Rect &roi);
void findRed(Mat& src, Mat& dst);
void findYellow(Mat& src, Mat& dst);
void findBlack(Mat& src, Mat& dst);


#endif /* SPEEDROIDUTILS_H_ */
