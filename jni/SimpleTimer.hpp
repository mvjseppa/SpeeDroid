/*
 * SimpleTimer.hpp
 *
 *  Created on: 10 Sep 2015
 *      Author: Mikko
 */

#include <ctime>

#ifndef SIMPLETIMER_HPP_
#define SIMPLETIMER_HPP_

#define US_PER_SEC	1000000.0

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
