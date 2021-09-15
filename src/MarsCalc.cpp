#include "MarsCalc.h"

#define PI 3.141592653589793238463
#define AS_RAD PI/180
#define AS_DEG 180/PI

double peturbs(double j2000Days);
double utcTTDiff(time_t utcTime);
double julianDateTT(time_t utcTime);
float solarElevationJ2000(double lon, double lat, double j2000Days);
time_t timeFromJ2000(double j2000Days);

float MarsCalc::solarElevation(double lon, double lat, time_t utcTime) {
	double julianTT  = julianDateTT(utcTime);
	double j2000Days = julianTT - 2451545.0;

	double meanAnom = fmod(19.3871 + 0.52402073*j2000Days, 360.0f);
	double pbs = peturbs(j2000Days);
	double eoc = (10.691 + 3.0e-7*j2000Days)*sin(meanAnom*AS_RAD) +
		0.6230*sin(2*meanAnom*AS_RAD) +
		0.0500*sin(3*meanAnom*AS_RAD) +
		0.0050*sin(4*meanAnom*AS_RAD) +
		0.0005*sin(5*meanAnom*AS_RAD) +
		pbs;

	double fms = fmod(270.3871 + 0.524038496*j2000Days, 360.0f);
	double solarLon = fmod(fms + eoc, 360.0f);
	double sinSolarLon = sin(solarLon*AS_RAD);
	double solarDec = asin(0.42565*sinSolarLon)*AS_DEG + 0.25*sinSolarLon; // TODO: Fix inaccuracy

	double eot = 2.861*sin(2*solarLon*AS_RAD) -
		0.071*sin(4*solarLon*AS_RAD) +
		0.002*sin(6*solarLon*AS_RAD) - eoc; //
	double mst = fmod(24*(((julianTT - 2451549.5)/1.0274912517) + 44796.0 - 0.0009626), 24.0f);
	double subsolLon = fmod(mst*15 + eot + 180, 360.0f);

	double hourAng = lon - subsolLon;
	if (lat > 90 || lat < -90) {
		printf("ERROR: Invalid latitude given when getting solar elevation.\n");
		return 0;
	}
	double zenithAng = acos(
			sin(solarDec*AS_RAD)*sin(lat*AS_RAD) +
			cos(solarDec*AS_RAD)*cos(lat*AS_RAD)*cos(hourAng*AS_RAD)
		)*AS_DEG;

	// printf("SOLAR ELEVATION NUMBERS:\n");
	// printf("JDTT:    %f\n", julianTT);
	// printf("ΔtJ2000: %f\n", j2000Days);
	// printf("M:       %f\n", meanAnom);
	// printf("αFMS:    %f\n", fms);
	// printf("PBS:     %f\n", pbs);
	// printf("ν-M:     %f\n", eoc);
	// printf("Ls:      %f\n", solarLon);
	// printf("EOT:     %f\n", eot);
	// printf("MST:     %f\n", mst);
	// printf("Λs:      %f\n", subsolLon);
	// printf("δs:      %f\n", solarDec);
	// printf("Z:       %f\n\n", zenithAng);

	return 90 - zenithAng;
}

// TODO: Fix this
float MarsCalc::solarAzimuth(double lon, double lat, time_t utcTime) {
	double julianTT  = julianDateTT(utcTime);
	double j2000Days = julianTT - 2451545.0;

	double meanAnom = fmod(19.3871 + 0.52402073*j2000Days, 360.0f);
	double pbs = peturbs(j2000Days);
	double eoc = (10.691 + 3.0e-7*j2000Days)*sin(meanAnom*AS_RAD) +
		0.6230*sin(2*meanAnom*AS_RAD) +
		0.0500*sin(3*meanAnom*AS_RAD) +
		0.0050*sin(4*meanAnom*AS_RAD) +
		0.0005*sin(5*meanAnom*AS_RAD) +
		pbs;

	double fms = fmod(270.3871 + 0.524038496*j2000Days, 360.0f);
	double solarLon = fmod(fms + eoc, 360.0f);
	double sinSolarLon = sin(solarLon*AS_RAD);
	double solarDec = asin(0.42565*sinSolarLon)*AS_DEG + 0.25*sinSolarLon; // TODO: Fix inaccuracy

	double eot = 2.861*sin(2*solarLon*AS_RAD) -
		0.071*sin(4*solarLon*AS_RAD) +
		0.002*sin(6*solarLon*AS_RAD) - eoc; //
	double mst = fmod(24*(((julianTT - 2451549.5)/1.0274912517) + 44796.0 - 0.0009626), 24.0f);
	double subsolLon = fmod(mst*15 + eot + 180, 360.0f);

	double hourAng = lon - subsolLon;
	if (lat > 90 || lat < -90) {
		printf("ERROR: Invalid latitude given when getting solar elevation.\n");
		return 0;
	}

	double azimuth = atan2(
			sin(hourAng*AS_RAD),
			cos(lat*AS_RAD)*tan(solarDec*AS_RAD) -
			sin(lat*AS_RAD)*cos(hourAng*AS_RAD)
		)*AS_DEG;

	// printf("SOLAR AZIMUTH NUMBERS:\n");
	// printf("JDTT:    %f\n", julianTT);
	// printf("ΔtJ2000: %f\n", j2000Days);
	// printf("M:       %f\n", meanAnom);
	// printf("αFMS:    %f\n", fms);
	// printf("PBS:     %f\n", pbs);
	// printf("ν-M:     %f\n", eoc);
	// printf("Ls:      %f\n", solarLon);
	// printf("EOT:     %f\n", eot);
	// printf("MST:     %f\n", mst);
	// printf("Λs:      %f\n", subsolLon);
	// printf("δs:      %f\n", solarDec);
	// printf("A:       %f\n\n", azimuth);

	return azimuth;
}

// TODO: Fix this
time_t MarsCalc::lastSunrise(double lon, double lat, time_t utcTime) {
	double julianTT  = julianDateTT(utcTime);
	double j2000Days = julianTT - 2451545.0;

	double meanAnom = fmod(19.3871 + 0.52402073*j2000Days, 360.0f);
	double pbs = peturbs(j2000Days);
	double eoc = (10.691 + 3.0e-7*j2000Days)*sin(meanAnom*AS_RAD) +
		0.6230*sin(2*meanAnom*AS_RAD) +
		0.0500*sin(3*meanAnom*AS_RAD) +
		0.0050*sin(4*meanAnom*AS_RAD) +
		0.0005*sin(5*meanAnom*AS_RAD) +
		pbs;

	double fms = fmod(270.3871 + 0.524038496*j2000Days, 360.0f);
	double solarLon = fmod(fms + eoc, 360.0f);
	double sinSolarLon = sin(solarLon*AS_RAD);
	double solarDec = asin(0.42565*sinSolarLon)*AS_DEG + 0.25*sinSolarLon; // TODO: Fix inaccuracy

	double eot = 2.861*sin(2*solarLon*AS_RAD) -
		0.071*sin(4*solarLon*AS_RAD) +
		0.002*sin(6*solarLon*AS_RAD) - eoc;
	double mst = fmod(24*(((julianTT - 2451549.5)/1.0274912517) + 44796.0 - 0.0009626), 24.0f);

	double lmst = fmod(mst - lon*(1/15), 24.0);
	double ltst = lmst + eot*(1/15);

	double lastMidnight = j2000Days - ltst/24;
	double nextMidnight = j2000Days + (24-ltst)/24;
	double noon = 0.5*(lastMidnight + nextMidnight);

	double a = lastMidnight;
	double b = noon;
	double c = a;
	while ((b-a) >= 0.000001) {
		c = (a+b)/2;
		if (solarElevationJ2000(lon, lat, c) == 0.0) {
			break;
		} else if (solarElevationJ2000(lon, lat, c)*solarElevationJ2000(lon, lat, a) < 0){
			b=c;
		} else{
			a=c;
		}
	}

	return timeFromJ2000(c);
}

// TODO: Fix this
time_t MarsCalc::nextSunset(double lon, double lat, time_t utcTime) {
	double julianTT  = julianDateTT(utcTime);
	double j2000Days = julianTT - 2451545.0;

	double meanAnom = fmod(19.3871 + 0.52402073*j2000Days, 360.0f);
	double pbs = peturbs(j2000Days);
	double eoc = (10.691 + 3.0e-7*j2000Days)*sin(meanAnom*AS_RAD) +
		0.6230*sin(2*meanAnom*AS_RAD) +
		0.0500*sin(3*meanAnom*AS_RAD) +
		0.0050*sin(4*meanAnom*AS_RAD) +
		0.0005*sin(5*meanAnom*AS_RAD) +
		pbs;

	double fms = fmod(270.3871 + 0.524038496*j2000Days, 360.0f);
	double solarLon = fmod(fms + eoc, 360.0f);
	double sinSolarLon = sin(solarLon*AS_RAD);
	double solarDec = asin(0.42565*sinSolarLon)*AS_DEG + 0.25*sinSolarLon; // TODO: Fix inaccuracy

	double eot = 2.861*sin(2*solarLon*AS_RAD) -
		0.071*sin(4*solarLon*AS_RAD) +
		0.002*sin(6*solarLon*AS_RAD) - eoc;
	double mst = fmod(24*(((julianTT - 2451549.5)/1.0274912517) + 44796.0 - 0.0009626), 24.0f);

	double lmst = fmod(mst - lon*(1/15), 24.0);
	double ltst = lmst + eot*(1/15);

	double lastMidnight = j2000Days - ltst/24;
	double nextMidnight = j2000Days + (24-ltst)/24;
	double noon = 0.5*(lastMidnight + nextMidnight);

	double a = noon;
	double b = nextMidnight;
	double c = a;
	while ((b-a) >= 0.000001) {
		c = (a+b)/2;
		if (solarElevationJ2000(lon, lat, c) == 0.0) {
			break;
		} else if (solarElevationJ2000(lon, lat, c)*solarElevationJ2000(lon, lat, a) < 0) {
			b=c;
		} else{
			a=c;
		}
	}

	return timeFromJ2000(c);
}

double peturbs(double j2000Days) {
	// ** TABULAR VALUES ** //
	double A[] = {0.0071, 0.0057, 0.0039, 0.0037, 0.0021, 0.0020, 0.0018};
	double T[] = {2.2353, 2.7543, 1.1177, 15.7866, 2.1354, 2.4694, 32.8492};
	double y[] = {49.409, 168.173, 191.837, 21.736, 15.704, 95.528, 49.095};
	// ******************** //

	double pbs = 0;
	for (int i = 0; i < 7; i++)
		pbs += A[i]*cos((((0.985626 * j2000Days/T[i]) + y[i])*3.14159/180));

	return pbs;
}

double utcTTDiff(time_t utcTime) {
	double jdut = 2440587.5f + (double)(utcTime)/86400.0f;

	ifstream fileStream("taiutc.txt");
	string line;

	time_t epoch = 0;

	int taiMinUTC = 0;
	time_t curTime;
	if (utcTime >= epoch) {
		bool found = false;
		while (getline(fileStream, line)) {
			tm tmTime;
			tmTime.tm_year  = atoi(line.substr(0,4).c_str()) - 1900;
			tmTime.tm_mon   = atoi(line.substr(5,2).c_str());
			tmTime.tm_mday  = atoi(line.substr(8,2).c_str());
			tmTime.tm_hour  = 0;
			tmTime.tm_min   = 0;
			tmTime.tm_sec   = 0;
			tmTime.tm_isdst = 0;
			curTime = mktime(&tmTime);

			if (curTime > utcTime) {
				found = true;
				break;
			}

			// printf("%s%d\n\n", asctime(gmtime(&curTime)), atoi(line.substr(12,2).c_str()));
			taiMinUTC = atoi(line.substr(12,2).c_str());
		}

		if (found)
			return taiMinUTC + 32.184;
	}

	return 64.184 + 59*jdut - 51.2*jdut*jdut - 67.1*jdut*jdut*jdut - 16.4*jdut*jdut*jdut*jdut;
}

time_t timeFromJ2000(double j2000Days) {
	double julianTT  = j2000Days + 2451545.0;
	time_t ttTime = (julianTT - 2440587.5f)*86400.0f;
	return ttTime;
	// double utcTime = ttTime - ttUTCDiff();
}

double julianDateTT(time_t utcTime) {
	double jdut = 2440587.5f + (double)(utcTime)/86400.0f;
	return jdut + (utcTTDiff(utcTime) / 86400.0f);
}

// TODO: Fix this
float solarElevationJ2000(double lon, double lat, double j2000Days) {
	double julianTT  = j2000Days + 2451545.0;

	double meanAnom = fmod(19.3871 + 0.52402073*j2000Days, 360)*PI/180;
	double pbs = peturbs(j2000Days);
	double eoc = (10.691 + 3.0e-7*j2000Days)*sin(meanAnom) +
		0.6230*sin(2*meanAnom) +
		0.0500*sin(3*meanAnom) +
		0.0050*sin(4*meanAnom) +
		0.0005*sin(5*meanAnom) +
		pbs;

	double fms = fmod(270.3863 + 0.52403840*j2000Days, 360.0);
	double solarLon = fmod(fms + eoc, 360.0)*PI/180;
	double solarDec = asin(0.42565*sin(solarLon)) + 0.25*(PI/180)*sin(solarLon);

	double eot = 2.861*sin(2*solarLon) -
		0.071*sin(4*solarLon) +
		0.002*sin(6*solarLon) - eoc;
	double mtc = fmod(24*(((julianTT - 2451549.5)/1.0274912517) + 44795.999), 24.0);
	double subsolLon = fmod(mtc*15 + eot + 180, 360);

	double hourAng = lon*PI/180 - subsolLon*PI/180;
	if (lat > 90 || lat < -90) {
		printf("ERROR: Invalid latitude given when getting solar elevation.\n");
		return 0;
	}
	double zenithAng = acos(
			sin(solarDec)*sin(lat) +
			cos(solarDec)*cos(lat)*cos(hourAng)
		)*180/PI;

	return 90 - zenithAng;
}
