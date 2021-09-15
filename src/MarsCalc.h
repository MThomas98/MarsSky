#pragma once

#include <stdio.h>
#include <math.h>
#include <ctime>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

class MarsCalc {
public:
	static float solarElevation(double lon, double lat, time_t time);
	static float solarAzimuth(double lon, double lat, time_t time);
	static time_t lastSunrise(double lon, double lat, time_t utcTime);
	static time_t nextSunset(double lon, double lat, time_t utcTime);
};
