/*
 * MyTimer.h
 *
 *  Created on: Sep 15, 2017
 *      Author: fermon
 */

#ifndef HW2_MYTIMER_H_
#define HW2_MYTIMER_H_

#include <ctime>

class MyTimer
{
public:
	MyTimer()
	{}
	void Start()
	{
		clock_gettime(CLOCK_REALTIME, &beg_);
	}
	void Stop()
	{
		clock_gettime(CLOCK_REALTIME, &end_);
	}

	double Elapsed()
	{
		return end_.tv_sec - beg_.tv_sec
				+ (end_.tv_nsec - beg_.tv_nsec) / 1000000000.;
	}

	void Reset()
	{
		beg_.tv_sec = 0;
		beg_.tv_nsec = 0;
		end_.tv_sec = 0;
		end_.tv_nsec = 0;
	}

private:
	timespec beg_, end_;
};
#endif /* HW2_MYTIMER_H_ */
