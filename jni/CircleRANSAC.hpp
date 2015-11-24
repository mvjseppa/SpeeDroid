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
#include <cstdlib>
#include <cmath>
#include <vector>

#ifndef CIRCLERANSAC_HPP_
#define CIRCLERANSAC_HPP_

using namespace std;
using namespace cv;

#define POW2(x) ((x)*(x))

typedef struct CircleType_ {
	Point center;
	unsigned int radius;
} CircleType;

bool CircleRANSAC(Mat& img, CircleType &result);
bool ConstructCircle(Point p1, Point p2, Point p3, CircleType &result);
template <typename T> bool pickRandomSamples(vector<T> &v, size_t n);

#endif /* CIRCLERANSAC_HPP_ */



