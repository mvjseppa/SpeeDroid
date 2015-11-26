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
#include <opencv2/imgproc/imgproc.hpp>
#include "CircleRANSAC.hpp"
#include "speedroidutils.hpp"

/*
 * RANSAC style circle detection algorithm
 *
 * PARAMS:
 * Mat& img				-	binary source image
 * CicleType& result	-	possible result circle
 *
 * RETURNS:
 * bool					-	true if cirlcle found
 * 							false if no circle or error occcurs
 *
 */
bool CircleRANSAC(Mat& img, CircleType& result){

	const static unsigned int minRadius = 15;
	const static unsigned int maxRadius = 100;

	bool retval = false;

	CircleType c = {Point(0,0), 0};
	CircleType resultCandidate;
	float candidateConfidence;

    vector<Point> pointData;
	vector<Point> inliers;
	vector<Point> outliers;

	Mat edge;

	Canny(img, edge, 80, 100, 5);

	//OpenCV bug:
	//Set the pixel 0,0 to 255, as findNonZero will crash if all pixels are 0.
	edge.at<uchar>(0,0) = 255;
	findNonZero(edge, pointData);

	//Check that there are enough points to try finding a circle
	if(pointData.size() < minRadius * 3) return false;


	for(size_t loops = 0; loops < 40; loops++){

		//pick three random points and move them to top of pointData
		//exit if there is a problem
		if(pickRandomSamples(pointData, 3) == false){
			retval = false;
			break;
		}

		//Try to construct a circle using the three random points
		if(ConstructCircle(pointData[0], pointData[1], pointData[2], c) == false){
			//get new samples if circle cannot be formed
			continue;
		}

		//Check that circle radius is within limits
		if(c.radius < minRadius || c.radius > maxRadius){
			c.center.x = 0;
			c.center.y = 0;
			c.radius = 0;
			continue;
		}

		//Check percentage of candidate circle pixels that are white in the original image
		inliers.clear();

		int cropSize = c.radius + 2;
		Rect rectToCrop(c.center.x - cropSize, c.center.y - cropSize, cropSize*2, cropSize*2);
		Mat cropped;

		safeCrop(edge, cropped, rectToCrop);

		Mat circleCandidate = Mat::zeros(cropped.rows, cropped.cols, CV_8UC1);
		circle(circleCandidate, Point(circleCandidate.cols/2, circleCandidate.rows/2), c.radius, 255, 3);

		bitwise_and(circleCandidate, cropped, circleCandidate);

		//OpenCV bug:
		//Set the pixel 0,0 to 255, as findNonZero will crash if all pixels are 0.
		circleCandidate.at<uchar>(0,0) = 255;
		findNonZero(circleCandidate, inliers);

		circleCandidate.release();
		cropped.release();

		float confidence = inliers.size()/(2 * c.radius * M_PI);

		if(confidence > 0.7){
			//this is most definitely a proper circle, return now
			result = c;
			retval = true;
			break;
		}
	}

	/*
	if(retval){
		cvtColor(edge, img, COLOR_GRAY2RGBA);
		circle(img, result.center, result.radius, Scalar(255,0,0));

		for(size_t i=0; i<inliers.size(); i++){
			Vec4b greenPixel;
			greenPixel.val[1] = 255;
			img.at<Vec4b>(inliers[i]) = greenPixel;
		}
	}
	*/

	edge.release();
	return retval;
}

/*
 * Select n random samples from vector v and place them to the beginning of v
 *
 * PARAMS:
 * vector<T>& v		-	vector to process
 * size_t n			-	number of random items to pick
 *
 * RETURNS:
 * bool				-	true on success and false on error.
 */
template <typename T> bool pickRandomSamples(vector<T> &v, size_t n){

	size_t items = v.size();
	T tmp;

	if(n > items || n==0) return false;

	while(n--){
		//select a random element
		size_t idx = rand()%items;

		//swap element n with the random element
		if(idx != n){
			tmp=v.at(idx);
			v.at(idx) == v.at(n);
			v.at(n) = tmp;
		}
	}

	return true;

}
/*
 * Construct a circle from three points
 *
 * PARAMS:
 * Point p1, p2, p3		-	Points for constructing the circle
 * CircleType& result	-	Resulting circle
 *
 * RETURNS:
 * bool 			-		true on success, false if no circle can be formed
 *
 */
bool ConstructCircle(Point p1, Point p2, Point p3, CircleType &result){

	Point tmpPoint;
	result.center = Point(0,0);
	result.radius = 0;

	//We need to calculate the slopes of lines p1p2 and p2p3
	//Slope for vertical lines is infinite so we need to avoid that.

	//If both p1p2 and p2p3 are vertical there can be no circle..
	if(p1.x == p2.x && p2.x == p3.x) return false;

	//If only p1p2 is vertical, swap points p2 and p3
	else if(p1.x == p2.x){
		tmpPoint = p2;
		p2 = p3;
		p3 = tmpPoint;
	}

	//If only p2p3 is vertical, swap points p2 and p1
	else if(p2.x == p3.x){
		tmpPoint = p2;
		p2 = p1;
		p1 = tmpPoint;
	}

	//Now calculate the slopes
	float ma = (p2.y-p1.y)/(float)(p2.x-p1.x);
	float mb = (p3.y-p2.y)/(float)(p3.x-p2.x);

	//If slopes are equal all points are on same line and there can be no circle
	if (mb == ma) return false;

	//Circle center point is the point where the perpendicular bisectors for
	//p1p2 and p2p3 intersect. Solve x coordinate from the bisector equations:
	result.center.x = (ma*mb*(p1.y-p3.y) + mb*(p1.x+p2.x) - ma*(p2.x+p3.x))/(2*(mb-ma));

	//Solve the y coordinate by substituting x coordinate into bisector equation of p1p2.
	//If this bisector is vertical use bisector of p2p3 instead.
	if(ma == 0)	result.center.y = (-1/mb)*(result.center.x-(p2.x+p3.x)/2)+(p2.y+p3.y)/2;
	//Else solve y from the bisector equation:
	else		result.center.y = (-1/ma)*(result.center.x-(p1.x+p2.x)/2)+(p1.y+p2.y)/2;

	//radius is p1 distance from center point
	result.radius = sqrt(POW2(p1.x-result.center.x) + POW2(p1.y-result.center.y));
	return true;
}


