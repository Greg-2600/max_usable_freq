/***********************************************************************
 *                                                                     *
 * Copyright (c) David L. Mills 1994-2010                              *
 *                                                                     *
 * Permission to use, copy, modify, and distribute this software and   *
 * its documentation for any purpose and without fee is hereby         *
 * granted, provided that the above copyright notice appears in all    *
 * copies and that both the copyright notice and this permission       *
 * notice appear in supporting documentation, and that the name        *
 * University of Delaware not be used in advertising or publicity      *
 * pertaining to distribution of the software without specific,        *
 * written prior permission.  The University of Delaware makes no      *
 * representations about the suitability this software for any         *
 * purpose. It is provided "as is" without express or implied          *
 * warranty.                                                           *
 *                                                                     *
 ***********************************************************************
 */
/*
 * Program to calculate maximum usable frequency and received signal
 * strength for high-frequency radio circuits. Uses MINIMUF 3.5.
 */
/*
 * Command line:
 *
 *	minimuf [-mdhspoel] [infile] [antfile]
 * 		infile		input file
 *		antfile		antenna data file
 *
 * Command-line modifiers (Unix only):
 *
 *	If any of these are specified, they override the corresponding
 *	data in the input file.
 *
 *	-d day
 *		day of month (1-31)
 *
 *	-e angle
 *		minimum takeoff angle (deg)
 *
 *	-h hour
 *		hour of day (0-23)
 *
 *	-l
 *		use long path (default is short path)
 *
 *	-m month
 *		month of year (1-12)
 *
 *	-o format
 *		output format (see below)
 *
 *	-p dbw
 *		transmitter power (dBW)
 *
 *	-s flux
 *		10-cm solar flux (65-250)
 *		output format (1-4)
 *
 * Input file format:
 *
 *	first line contains six numbers:
 *
 *	3 1 1 194 20 5
 *
 *	1. output format
 *		1 receive power (dBm above threshold)
 *		2 elevation angle (deg)
 *		3 delay (ms)
 *		4 fot (MHz), receive power (dBm above threshold)
 *		all formats are preceded by; UT LT MUF phi
 *			UT	universal time
 *			LT	local time at receiver
 *			MUF	maximum usable frequency
 *			phi	Sun zenith angle at path midpoint
 *	2. month (1-12)
 *	3. day (1-31)
 *	4. 10-cm solar flux (65-250)
 *	5. transmitter power (dBW)
 *	6. number of frequencies to follow (0 indicates default)
 *
 *	if (6) is nonzero, second line contains frequencies (MHz)
 *
 *	2.5 5 10 15 20
 *
 *	second/third line contains transmitter coordinates and name:
 *
 *	39.70000 -75.78194 W3HCF Newark 39:42:00N 75:46:55W
 *
 *	third and subsequent lines contain receiver coordinates
 *	and name:
 *
 *	39.68005 -75.75085 Evans Hall 39:40:48.184N 75:45:3.067W
 *	40.6803 -105.0408  WWV Ft Collins 40:40:49.0N 105:02:27.0W
 *	21.9906 -159.7667  WWVH Hawaii 21:59:26.0N 159:46:00.0W
 *	45.30    -75.75    CHU Ottawa 45:18N 75:45N
 *	50.17      9.00    DCF77 Mainflingen 50:01N 9:00E
 *	52.37     -1.18    MSF Rugby 52:22N 1:11W
 *
 * Timecode transmitters, frequencies and geographic coordinates
 *
 * WWV  Ft. Collins	2.5, 5, 10, 15, 20 MHz	40:40:49.0N 105:02:27.0W
 * WWVB Ft. Collins	60 kHz			40:40:28.3N 105:02:39.5W
 * WWVL Ft. Collins	20 kHz (silent key)	40:40:51.3N 105:03:00.0W
 * WWVH Kauai		2.5, 5, 10, 15 MHz	21:59:26.0N 159:46:00.0W
 * CHU Ottawa		3330, 7335, 14670	45:18N 75:45N
 * DCF77 Mainflingen	77.5 kHz		50:01N 9:00E
 * MSF Rugby		60 kHz			52:22N 1:11W
 */
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#ifndef _WIN32
#include <sys/types.h>
#endif /* _WIN32 */

#define R 6371.2		/* radius of the Earth (km) */
#define hE 110.			/* mean height of E layer (km) */
#define hF 320.			/* mean height of F layer (km) */
#define GAMMA 1.42		/* geomagnetic constant */
#define LN10 2.302585		/* natural logarithm of 10 */
#define PI 3.141592653589	/* the real thing */
#define PIH (PI / 2.)		/* the real thing / 2 */
#define PID (PI * 2.)		/* the real thing * 2 */
#define VOFL 2.9979250e8	/* velocity of light (m/s) */
#define D2R (PI / 180.)		/* degrees to radians */
#define R2D (180. / PI)		/* radians to degrees */
#define MINBETA (10. * D2R)	/* min elevation angle (rad) */
#define BOLTZ 1.380622e-23	/* Boltzmann's constant */
#define NTEMP 290.		/* receiver noise temperature (K) */
#define DELTAF 2500.		/* communication bandwidth (Hz) */
#define MPATH 3.		/* multipath threshold (dB) */
#define GLOSS 3.		/* ground-reflection loss (dB) */
#define SLOSS 10.		/* excess system loss */
#define RSENS -123.		/* receiver sensitivity (dBm) */
#define NGAIN 5			/* antenna gain frequencies */
#define FMAX 10			/* max frequencies */
#define HMAX 30			/* max hops */

/*
 * Program flags (flags)
 */
#define H_FMT	0x0001		/* output format */
#define H_MONTH	0x0002		/* month of year */
#define H_DAY	0x0004		/* day of month */
#define H_HOUR	0x0008		/* hour of day */
#define H_FLUX	0x0010		/* 10-cm solar flux */
#define H_POWER	0x0020		/* transmitter power */
#define H_BETA	0x0040		/* minimum elevation angle */
#define H_GAIN	0x0080		/* antenna gain table present */
#define H_LONG	0x0100		/* use long path (default is short) */

/*
 * Path flags (daynight)
 */
#define P_J 0x01		/* hop in daytime */
#define P_N 0x02		/* hop in nighttime */
#define P_S 0x04		/* signal below sensitivity */
#define P_E 0x08		/* E-layer cutoff */
#define P_M 0x10		/* multipath */

/*
 * Global function declarations
 */
extern FILE *fopen();
extern double minimuf(double, double, double, double, double, double,
    double, double);
extern double spots(double);

#ifndef _WIN32
extern char *optarg;		/* pointer to option string */
extern int opterr;		/* error message enable */
extern int optind;		/* argv index of next argument */
#else
int optind;			/* argv index of next argument */
#endif /* _WIN32 */

/*
 * Local function declarations
 */
static double antgain(double, double);
static void ion(int, double, double);
static int pathloss(int, double);
static double zenith(double);
static void dsx(int);

/*
 * Global data
 */
double month;			/* month of year (1 - 12) */
double day;			/* day of month */
double hour;			/* hour of day (UTC) */
double flux;			/* 10-cm solar flux */
double ssn;			/* sunspot number (derived from flux) */
double lat1, lon1;		/* transmitter coordinates (deg N/W) */
double b1;			/* transmitter bearing (rad) */
char site1[30];			/* transmitter site name */
double lat2, lon2;		/* receiver coordinates (deg N/W) */
double b2;			/* receiver bearing (rad) */
char site2[30];			/* receiver site name */
double theta;			/* path angle (rad) */
double lats, lons;		/* subsolar coordinates (rad) */
double noise;			/* thermal noise (dBm) */
FILE *fp_in, *fp_an;		/* file handles */
char antfile[25];		/* antenna file name */
int flag;			/* output format */
double dB1;			/* transmitter output power (dBW) */
int options;			/* option flags */

/*
 * Antenna gain data
 */
int nfreq;			/* number of frequencies */
double freq[FMAX];		/* working frequencies (MHz) */
double gainfreq[NGAIN];		/* antenna gain frequencies (MHz) */
double gain[46][NGAIN];		/* antenna gain (main lobe) (dB) */

/*
 * Path variables
 */
double mufE[HMAX];		/* maximum E-layer MUF (MHz) */
double mufF[HMAX];		/* minimum F-layer MUF (MHz) */
double absorp[HMAX];		/* ionospheric absorption coefficient */
double dB2[HMAX];		/* receive power (dBm) */
double path[HMAX];		/* path length (km) */
double beta[HMAX];		/* elevation angle (rad) */
char daynight[HMAX];		/* path flags */

/*
 * Main program
 */
int
main(
	int argc,		/* count of arguments */
	char **argv		/* argument list */
	)
{

	/*
	 * Path variables
	 */
	double delay;		/* path delay (ms) */
	double psi;		/* sun zenith angle (rad) */
	double ftemp, gtemp;	/* double temps */
	int i, j, h, n;		/* int temps */
	double offset;		/* offset for local time (hours) */
	double fcF;		/* F-layer critical frequency (MHz) */
	double phiF;		/* F-layer angle of incidence (rad) */
	int hop;		/* number of ray hops */
	double beta1;		/* elevation angle (rad) */
	double minbeta;		/* minimum elevation angle (rad) */
	double d;		/* great-circle distance (rad) */
	double dhop;		/* hop great-circle distance (rad) */
	double height;		/* height of F layer (km) */
	double time;		/* time of day (hour) */

	double hr1, hr2;	/* hour span */

#ifndef _WIN32
	double opt_month;	/* month of year (1 - 12) */
	double opt_day;		/* day of month */
	double opt_flux;	/* 10-cm solar flux */
	double opt_dB1;		/* transmitter output power (dBW) */
	int opt_flag;		/* output format */
	int temp;		/* int temp */
#endif /* _WIN32 */

	fp_in = stdin;
	hr1 = 0;
	hr2 = 23;
	optind = 1;
	options = 0;
	minbeta = MINBETA;

#ifndef _WIN32
	/*
	 * Process command-line arguments
	 */
	while ((temp = getopt(argc, argv, "d:e:h:lm:o:p:s:")) != -1) {
		switch (temp) {

		/*
		 * Day
		 */
		case 'd':
			sscanf(optarg, "%lf", &opt_day);
			options |= H_DAY;
			break;

		/*
		 * Minimum elevation angle
		 */
		case 'e':
			sscanf(optarg, "%lf", &minbeta);
			minbeta *= R2D;
			options |= H_BETA;
			break;

		/*
		/* Hour
		 */
		case 'h':
			sscanf(optarg, "%lf", &hr1);
			hr2 = hr1;
			options |= H_HOUR;
			break;

		/*
		 * Use long path (default is short path)
		 */
		case 'l':
			options |= H_LONG;
			break;

		/*
		 * Month
		 */
		case 'm':
			sscanf(optarg, "%lf", &opt_month);
			options |= H_MONTH;
			break;

		/*
		 * Output format
		 */
		case 'o':
			sscanf(optarg, "%d", &opt_flag);
			options |= H_FMT;
			break;

		/*
		 * Transmitter power output
		 */
		case 'p':
			sscanf(optarg, "%lf", &opt_dB1);
			options |= H_POWER;
			break;

		/*
		 * 10-cm solar flux
		 */
		case 's':
			sscanf(optarg, "%lf", &opt_flux);
			options |= H_FLUX;
			break;
		}
	}
#endif /* _WIN32 */

	/*
	 * Read data and frequency list.
	 */
	if (argc > optind)
		fp_in = fopen (argv[optind], "r");
	if (fp_in == NULL)
		return(1);
	fscanf(fp_in, "%i%lf%lf%lf%lf%i", &flag, &month, &day, &flux,
	    &dB1, &nfreq);
	if (nfreq > 0) {
		if (nfreq > FMAX)
			nfreq = FMAX;
		for (i = 0; i < nfreq; i++) {
			if (fscanf(fp_in, "%lf", &freq[i]) != 1)
				return (1);
		}
	}

#ifndef _WIN32
	if (options & H_MONTH)
		month = opt_month;
	if (options & H_DAY)
		day = opt_day;
	if (options & H_FLUX)
		flux = opt_flux;
	if (options & H_POWER)
		dB1 = opt_dB1;
	if (options & H_FMT)
		flag = opt_flag;
#endif /* _WIN32 */

	/*
	 * Read in optional frequency and antenna gain tables.
	 */
	optind++;
	if (argc > optind) {
		fp_an = fopen(argv[optind], "r");
		if (fp_an == NULL)
			 return (1);

		for (j = 0; j < NGAIN; j++) {
			if (fscanf(fp_an, "%lf", &gainfreq[j]) != 1)
				return (1);
		}
		for (i = 0; i < 46; i++) {
			for (j = 0; j < NGAIN; j++) {
				if (fscanf(fp_an, "%lf", &gain[i][j]) !=
				    1)
					return (1);
			}
		}
		options |= H_GAIN;
	}

	/*
	 * Get transmitter coordinates and site name.
	 */
	fscanf(fp_in, "%lf%lf%[^\n]", &lat1, &lon1, site1);
	lat1 = lat1 * D2R;
	lon1 = - lon1 * D2R;

	/*
	 * Main loop. Get receiver coordinates and site name.
	 */
	ssn = spots(flux);
L1:	if (fscanf(fp_in, "%lf%lf%[^\n]", &lat2, &lon2, site2) != 3)
		return (0);

	lat2 = lat2 * D2R;
	lon2 = -lon2 * D2R;

	/*
	 * Compute great-circle bearings, great-circle distance, min
	 * hops, F-layer angle of incidence and path delay
	 */
	theta = lon1 - lon2;
	if (theta >= PI)
		theta -= PID;
	if (theta <= -PI)
		theta += PID;
	d = acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) *
	    cos(theta));
	if (d < 0.)
		d += PI;
	b1 = acos((sin(lat2) - sin(lat1) * cos(d)) / (cos(lat1) *
	    sin(d)));
	if (b1 < 0.)
		b1 += PI;
	if (theta < 0)
		b1 = PID - b1;
	b2 = acos((sin(lat1) - sin(lat2) * cos(d)) / (cos(lat2) *
	    sin(d)));
	if (b2 < 0.)
		b2 += PI;
	if (theta >= 0.)
		b2 = PID - b2;
	if (options & H_LONG) {
		d = PID - d;
		b1 += PI;
		if (b1 >= PID)
 			b1 -= PID;
		b2 += PI;
		if (b2 >= PID)
			b2 -= PID;
	}
	hop = (int)(d / (2. * acos(R / (R + hF))));
	beta1 = 0.;
	while (beta1 < minbeta) {
		hop++;
		dhop = d / (hop * 2.);
		beta1 = atan((cos(dhop) - R / (R + hF)) / sin(dhop));
	}
	ftemp = R * cos(beta1) / (R + hF);
	phiF = atan(ftemp / sqrt(1. - ftemp * ftemp));
	delay = 2. * hop * sin(dhop) * (R + hF) / cos(beta1) / VOFL *
	    1e6;

	if (flag < 4) {
		printf("\n10-cm solar flux:%4.0lf   SN:%4.0lf   Month:%3.0lf   Day:%3.0lf\n",
		    flux, ssn, month, day);
		printf("Power:%3.0f dBW    Distance:%6.0f km    Delay:%5.1f ms\n",
		    dB1, d * R, delay);
		printf("Location                        Lat      Long    Azim\n");
		printf("%-27s %7.2fN  %7.2fW    %3.0f\n",
		    site1, lat1 * R2D, lon1 * R2D, b1 * R2D);
		printf("%-27s %7.2fN  %7.2fW    %3.0f\n",
		    site2, lat2 * R2D, lon2 * R2D, b2 * R2D);
		printf("UT LT  MUF Zen");
		for (i = 0; i < nfreq; i++)
			printf("%7.1f", freq[i]);
		printf("\n");
	}

	/*
	 * Hour loop: This loop determines the min-hop path and next two
	 * higher-hop paths. It selects the most likely path for each
	 * frequency and calculates the receive power. The F-layer
	 * critical frequency is computed directly from MINIMUF 3.5 and
	 * the secant law.
	 */
	noise = 10. * log10(BOLTZ * NTEMP * DELTAF) + 30.;
	offset = (lon2 * 24. / PID);
	for (hour = hr1; hour <= hr2; hour++) {
		time = hour - offset;
		if (time < 0.)
			time += 24.;
		if (time >= 24.)
			time -= 24.;
		printf("%2.0f %2.0f", hour, time);
		ftemp = minimuf(flux, month, day, hour, lat1, lon1,
		    lat2, lon2);
		fcF = ftemp * cos(phiF);

		/*
		 * Calculate subsolar coordinates.
		 */
		ftemp = (month - 1.) * 365.25 / 12. + day - 80.;
		lats = 23.5 * D2R * sin(ftemp / 365.25 * PID);
		lons = (hour * 15. - 180.) * D2R;

		/*
		 * Path loop: This loop determines the geometry of the
		 * min-hop path and the next two higher-hop paths. It
		 * calculates the minimum F-layer MUF, maximum E-layer
		 * MUF and ionospheric absorption factor for each
		 * geometry.
		 */
		for (h = hop; h < hop + 3; h++) {

			/*
			 * We assume the F layer height decreases during
			 * the day and increases at night, as determined
			 * at the midpoint of the path.
			 */
			height = hF;
			psi = zenith(d / 2.);
			if (90. - psi * R2D < 0)
				height += 70.;
			else
				height -= 30.;
			dhop = d / (h * 2.);
			beta[h] = atan((cos(dhop) - R / (R + height)) /
			    sin(dhop));
			path[h] = 2. * h * sin(dhop) * (R + height) /
			    cos(beta[h]);
			ion(h, d, fcF);
		}
		/*
		 * Display one line for this hour.
		 */
		printf("%5.1f%4.0f ", mufF[hop], 90. - psi * R2D);
		ftemp = noise;
		for (i = 0; i < nfreq; i++) {
			n = pathloss(hop, freq[i]);
			if (flag != 4)
				dsx(n);
			gtemp = dB2[n];
			if (gtemp > ftemp && n > 0) {
				ftemp = gtemp;
				h = i;
				j = n;
			}
		}
		if (flag == 4) {
			printf("%8.5lf", freq[h]);
			dsx(j);
 		}
		printf("\n");
	}
	goto L1;
}

/*
 * ion(d, h, fcF) - determine paratmeters for hop h
 *
 * This routine determines the reflection zones for each hop along the
 * path and computes the minimum F-layer MUF, maximum E-layer MUF,
 * ionospheric absorption factor and day/night flags for the entire
 * path.
 */
static void
ion(
	int h,			/* hop index */
	double d,		/* path angle (rad) */
	double fcF		/* F-layer critical frequency */
	)
{
	double beta;		/* elevation angle (rad) */
	double psi;		/* sun zenith angle (rad) */
	double dhop;		/* hop angle / 2 (rad) */
	double dist;		/* path angle (rad) */
	double phiF;		/* F-layer angle of incidence (rad) */
	double phiE;		/* E-layer angle of incidence (rad) */
	double fcE;		/* E-layer critical frequency (MHz) */
	double ftemp;		/* double temp */
	int daytime, nightime;	/* path flags */

	/*
	 * Determine the path geometry, E-layer angle of incidence and
	 * minimum F-layer MUF. The F-layer MUF is determined from the
	 * F-layer critical frequency previously calculated by MINIMUF
	 * 3.5 and the secant law and so depends only on the F-layer
	 * angle of incidence. This is somewhat of a crock; however,
	 * doing it with MINIMUF 3.5 on a hop-by-hop basis results in
	 * rather serious errors.
	 */
	daytime = 0;
	nightime = 0;
	dhop = d / (h * 2.);
	beta = atan((cos(dhop) - R / (R + hF)) / sin(dhop));
	ftemp = R * cos(beta) / (R + hE);
	phiE = atan(ftemp / sqrt(1. - ftemp * ftemp));
	ftemp = R * cos(beta) / (R + hF);
	phiF = atan(ftemp / sqrt(1. - ftemp * ftemp));
	mufE[h] = 0;
	mufF[h] = fcF / cos(phiF);;
	absorp[h] = 0.;
	daynight[h] = 0;
	for (dist = dhop; dist < d; dist += dhop * 2) {

		/*
		 * Calculate the E-layer critical frequency and MUF.
		 */
		fcE = 0.;
		psi = zenith(dist);
		ftemp = cos(psi);
		if (ftemp > 0.)
			fcE = .9 * pow((180. + 1.44 * ssn) * ftemp,
			    .25);
		if (fcE < .005 * ssn)
			fcE = .005 * ssn;
		ftemp = fcE / cos(phiE);
		if (ftemp > mufE[h])
			mufE[h] = ftemp;

		/*
		 * Calculate ionospheric absorption coefficient and
		 * day/night indicators. Note that some hops along a
		 * path can be in daytime and others in nighttime.
		 */
		ftemp = psi;
		if (ftemp > 100.8 * D2R) {
			ftemp = 100.8 * D2R;
			daynight[h] |= P_N;
		}
		else
			daynight[h] |= P_J;
		ftemp = cos(90. / 100.8 * ftemp);
		if (ftemp < 0.)
			ftemp = 0.;
		ftemp = (1. + .0037 * ssn) * pow(ftemp, 1.3);
		if (ftemp < .1)
			ftemp = .1;
		absorp[h] += ftemp;
	}
}

/*
 * pathloss(freq, hop) - Compute receive power for given path.
 *
 * This routine determines which of the three ray paths determined
 * previously are usable. It returns the hop index of the best of these
 * or zero if none are found.
 */
static int
pathloss(
	int hop,		/* minimum hops */
	double freq		/* frequency */
	)
{
	int h;			/* hop number */
	double level;		/* max signal (dBm) */
	double signal;		/* receive signal (dBm) */
	double ftemp;		/* double temp */
	int j;			/* index temp */

	/*
	 * Calculate signal and noise for all hops. The noise level is
	 * -140 dBm for a receiver bandwidth of 2500 Hz and noise
	 * temperature 290 K. The receiver sensitivity is assumed -123
	 * dBm (0.15 V at 50 Ohm for 10 dB S/N). Paths where the signal
	 * is less than the noise or when the frequency exceeds the F-
	 * layer MUF are considered unusable.
	 */
	level = noise;
	j = 0;
	for (h = hop; h < hop + 3; h++) {
		daynight[h] &= ~(P_E | P_S | P_M);
		if (freq < 0.85 * mufF[h]) {

			/*
			 * Transmit power (dBm)
			 */
			signal = dB1 + antgain(freq, beta[h]) + 30.;

			/*
			 * Path loss
			 */
			signal -= 32.44 + 20. * log10(path[h] * freq) +
			    SLOSS;

			/*
			 * Ionospheric loss
			 */
			ftemp = R * cos(beta[h]) / (R + hE);
			ftemp = atan(ftemp / sqrt(1. - ftemp * ftemp));
			signal -= 677.2 * absorp[h] / cos(ftemp) /
			    (pow((freq + GAMMA), 1.98) + 10.2);

			/*
			 * Ground reflection loss
			 */
			signal -= h * GLOSS;

			dB2[h] = signal;

			/*
			 * Paths where the signal is greater than the
			 * noise, but less than the receiver sensitivity
			 * are marked 's'. Paths below the E-layer MUF
			 * are marked 'e'. When comparing for maximum
			 * signal, The signal for these paths is reduced
			 * by 3 dB so they will be used only as a last
			 * resort.
			 */
			if (signal < RSENS)
				daynight[h] |= P_S;
			if (freq < mufE[h]) {
				daynight[h] |= P_E;
				signal -= MPATH;
			}
			if (signal > level) {
				level = signal;
				j = h;
			}
		}
	}

	/*
	 * We have found the best path. If this path is less than 3 dB
	 * above the RMS sum of the other paths, the path is marked 'm'.
	 */
	if (j == 0)
		return (0);

	ftemp = 0.;
	for (h = hop; h < hop + 3; h++) {
		if (h != j)
			ftemp += exp(2. / 10. * dB2[h] * LN10);
	}
	ftemp = 10. / 2. * log10(ftemp);
	if (level < ftemp + MPATH)
		daynight[j] |= P_M;
	return (j);
}

/*
 * antgain(freq, beta) - Compute antenna gain from tables.
 *
 * The gain table gain[i][j] is indexed by elevation i in 2-degree
 * increments and frequency j as dermined from the gainfreq[j] vector.
 * If the table is not present, an isotropic radiator is assumed.
 */
static double
antgain(
	double freq,	/* frequency (MHz) */
	double beta		/* elevation angle (rad) */
	)
{
	double p, q, r, s; /* double temps */
	int i, j;		/* index temps */

	if (~options & H_GAIN)
		return (0);

	r = beta * R2D / 2.;
	i = (int)r;
	r -= i;
	s = 1. - r;
	for (j = 0; j < nfreq && gainfreq[j] < freq; j++);

	/*
	 * Handle the exceptions.
	 */
	if (j == 0) {
		if (i == 44)
			return (gain[i][j]);
		else
			return(s * gain[i][j] + r * gain[i + 1][j]);
		}
	if (j == nfreq)
		if (i == 44)
			return (gain[i][j - 1]);
		else
			return(s * gain[i][j - 1] + r * gain[i + 1][j - 1]);
	/*
	 * Interpolate the table.
	 */ 
	p = (freq - gainfreq[j - 1]) / (gainfreq[j] - gainfreq[j - 1]);
	q = 1. - p;
	return(q * (s * gain[i][j - 1] + r * gain[i + 1][j - 1]) +
	    p * (s * gain[i][j] + r * gain[i + 1][j]));
}

/*
 * zenith(dist) - Determine sun zenith angle at reflection zone.
 */
static double
zenith(
	double dist		/* path angle */
	)
{
	double latr, lonr;	/* reflection zone coordinates (rad) */
	double thetar;		/* reflection zone angle (rad) */
	double psi;		/* sun zenith angle (rad) */

	/*
	 * Calculate reflection zone coordinates.
	 */
	latr = acos(cos(dist) * sin(lat1) + sin(dist) *
	    cos(lat1) * cos(b1));
	if (latr < 0.)
		latr += PI;
	latr = PIH - latr;
	lonr = acos((cos(dist) - sin(latr) * sin(lat1)) /
	    (cos(latr) * cos(lat1)));
	if (lonr < 0.)
		lonr += PI;
	if (theta < 0.)
		lonr = - lonr;
	lonr = lon1 - lonr;
	if (lonr >= PI)
		lonr -= PID;
	if (lonr <= -PI)
		lonr += PID;
	thetar = lons - lonr;
	if (thetar > PI)
		thetar = PID - thetar;
	if (thetar < - PI)
		thetar -= PID;

	/*
	 * Calculate sun zenith angle.
	 */
	psi = acos(sin(latr) * sin(lats) + cos(latr) * cos(lats) *
	    cos(thetar));
	if (psi < 0.)
		psi += PI;
	return(psi);
}

/*
 * dsx(h) - Decode and display path descriptor.
 */
static void
dsx(
	int h			/* hop index */
	)
{
	char c1, c2;		/* path flags *

	/*
	 * Determine day/night flags for the path.
	 */
	if (h == 0) {
		if (flag != 4)
			printf("       ");
		return;
	}
	if (daynight[h] & P_J && daynight[h] & P_N)
		c1 = 'x';
	else if (daynight[h] & P_J)
		c1 = 'j';
	else if (daynight[h] & P_N)
		c1 = 'n';
	if (daynight[h] & P_S)
		c2 = 's';
	else if (daynight[h] & P_M)
		c2 = 'm';
	else
		c2 = ' ';
	switch (flag) {

	case 1:
	case 4:
		printf("%4.0f%c%1i%c", dB2[h] - RSENS, c1, h, c2);
		break;

	case 2:
		printf("%4.0f%c%1i%c", beta[h] * R2D, c1, h, c2);
		break;

	case 3:
		printf("%5.1f%c%1i", path[h] / VOFL * 1e6, c1, h);
		break;
	}
}
