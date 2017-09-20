/*
 * MyTimer.cpp
 *
 *  Created on: Sep 15, 2017
 *      Author: fermon
 */
#include "MyTimer.h"

MyTimer::MyTimer() :
		m_start(0), m_stop(0)
{

}
MyTimer::~MyTimer()
{

}
bool MyTimer::Start()
{
	m_start = high_resolution_clock::now();
}
bool MyTimer::Stop()
{
	m_stop = high_resolution_clock::now();
}
duration<double> MyTimer::GetElapsedTime()
{
	return duration_cast<duration<double>>(m_stop - m_start);
}
void MyTimer::Reset()
{
	m_start = 0;
	m_stop = 0;

}

