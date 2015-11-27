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


void safeCrop(const cv::Mat& src, cv::Mat& dst, const cv::Rect &roi);
void findRed(cv::Mat& src, cv::Mat& dst);
void findYellow(cv::Mat& src, cv::Mat& dst);
void findBlack(cv::Mat& src, cv::Mat& dst);


#endif /* SPEEDROIDUTILS_H_ */
