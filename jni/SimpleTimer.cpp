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

#include <ctime>
#include "SimpleTimer.hpp"
#include "debugprint.hpp"

SimpleTimer::SimpleTimer() : waitTime(0.0) {

}

void SimpleTimer::startTimer(float seconds){
	waitTime = seconds;
	gettimeofday(&(this->startTime), NULL);
}

bool SimpleTimer::isElapsed(){
	if(waitTime == 0.0) return true;

	struct timeval now;
	gettimeofday(&now, NULL);

	float elapsed = ((now.tv_sec + now.tv_usec/US_PER_SEC) -
			(startTime.tv_sec + startTime.tv_usec/US_PER_SEC));

	LOGD("elapsed: %lf", elapsed);
	LOGD("set time: %lf", waitTime);

	return elapsed >= waitTime;
}
