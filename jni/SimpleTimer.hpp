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

#ifndef SIMPLETIMER_HPP_
#define SIMPLETIMER_HPP_

#define US_PER_SEC	1000000.0

/*
 * A timer that can be polled to check if a time period has elapsed.
 */
class SimpleTimer{
private:
	struct timeval startTime;
	float waitTime;

public:
	SimpleTimer();

	void startTimer(float seconds);
	bool isElapsed();

};


#endif /* SIMPLETIMER_HPP_ */
