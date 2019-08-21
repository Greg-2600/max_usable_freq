/***********************************************************************
 *                                                                     *
 * Copyright (c) David L. Mills 1994-1998                              *
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
 * MINIMUF 3.5 from QST December 1982
 * (originally in BASIC)
 */
#include <ctype.h>
#include <math.h>

#define PI 3.141592653589	/* the real thing */
#define PIH (PI / 2.)		/* the real thing / 2 */
#define PID (PI * 2.)		/* the real thing * 2 */
#define SGN(x) ((x==0.)?0.:((x>0.)?1.:-1.)) /* BASIC SGN function */

double spots(double);

/*
 * MINIMUF 3.5 (From QST December 1982, originally in BASIC)
 */
double
minimuf(
	double flux,		/* 10-cm solar flux */
	double month,		/* month of year (1 - 12) */
	double day,		/* day of month (1 - 31) */
	double hour,		/* hour of day (utc) (0 - 23) */
	double lat1,		/* transmitter latitude (deg n) */
	double lon1,		/* transmitter longitude (deg w) */
	double lat2,		/* receiver latitude (deg n) */
	double lon2		/* receiver longitude (deg w) */
	)

{
	double ssn;		/* sunspot number dervived from flux */
	double muf;		/* maximum usable frequency */
	double dist;		/* path angle (rad) */
	double a, p, q;		/* unfathomable local variables */
	double y1, y2, y3;
	double t, t4, t9;
	double g0, g8;
	double k1, k6, k8, k9;
	double m9, c0;
	double ftemp, gtemp;	/* volatile temps */

	/*
	 * Determine geometry and invariant coefficients
	 */
	ssn = spots(flux);
	ftemp = sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) *
	    cos(lon2 - lon1);
	if (ftemp < -1.)
		ftemp = -1.;
	if (ftemp > 1.)
		ftemp = 1.;
	dist = acos(ftemp);
	k6 = 1.59 * dist;
	if (k6 < 1.)
		k6 = 1.;
	p = sin(lat2);
	q = cos(lat2);
	a = (sin(lat1) - p * cos(dist)) / (q * sin(dist));
	y1 = .0172 * (10. + (month - 1.) * 30.4 + day);
	y2 = .409 * cos(y1);
	ftemp = 2.5 * dist / k6;
	if (ftemp > PIH)
		ftemp = PIH;
	ftemp = sin(ftemp);
	m9 = 1. + 2.5 * ftemp * sqrt(ftemp);
	muf = 100.;

	/*
	 * Loop along path
	 */
	for (k1 = 1. / (2. * k6); k1 <= 1. - 1. / (2. * k6);
	    k1 += fabs(.9999 - 1. / k6)) {
		gtemp = dist * k1;
		ftemp = p * cos(gtemp) + q * sin(gtemp) * a;
		if (ftemp < -1.)
			ftemp = -1.;
		if (ftemp > 1.)
			ftemp = 1.;
		y3 = PIH - acos(ftemp);
		ftemp = (cos(gtemp) - ftemp * p) / (q * sqrt(1. - ftemp
		    * ftemp));
		if (ftemp < -1.)
			ftemp = -1.;
		if (ftemp > 1.)
			ftemp = 1.;
		ftemp = lon2 + SGN(sin(lon1 - lon2)) * acos(ftemp);
		if (ftemp < 0.)
			ftemp += PID;
		if (ftemp >= PID)
			ftemp -= PID;
		ftemp = 3.82 * ftemp + 12. + .13 * (sin(y1) + 1.2 *
		    sin(2. * y1));
		k8 = ftemp - 12. * (1. + SGN(ftemp - 24.)) *
		    SGN(fabs(ftemp - 24.));
		if (cos(y3 + y2) <= -.26) {
			k9 = 0.;
			g0 = 0.;
		} else {
			ftemp = (-.26 + sin(y2) * sin(y3)) / (cos(y2) *
			    cos(y3) + .001);
			k9 = 12. - atan(ftemp / sqrt(fabs(1. - ftemp *
			    ftemp))) * 7.639437;
			t = k8 - k9 / 2. + 12. * (1. - SGN(k8 - k9 /
			    2.)) * SGN(fabs(k8 - k9 / 2.));
			t4 = k8 + k9 / 2. - 12. * (1. + SGN(k8 + k9 /
			    2. - 24.)) * SGN(fabs(k8 + k9 / 2. - 24.));
			c0 = fabs(cos(y3 + y2));
			t9 = 9.7 * pow(c0, 9.6);
			if (t9 < .1)
				t9 = .1;
			g8 = PI * t9 / k9;
			if ((t4 < t && (hour - t4) * (t - hour) > 0.) ||
			    (t4 >= t && (hour - t) * (t4 - hour) <= 0.))
			    {
				ftemp = hour + 12. * (1. + SGN(t4 -
				    hour)) * SGN(fabs(t4 - hour));
				ftemp = (t4 - ftemp) / 2.;
				g0 = c0 * (g8 * (exp(-k9 / t9) + 1.)) *
				    exp(ftemp) / (1. + g8 * g8);
			} else {
				ftemp = hour + 12. * (1. + SGN(t -
				    hour)) * SGN(fabs(t - hour));
				gtemp = PI * (ftemp - t) / k9;
				ftemp = (t - ftemp) / t9;
				g0 = c0 * (sin(gtemp) + g8 * (exp(ftemp)
				    - cos(gtemp))) / (1. + g8 * g8);
				ftemp = c0 * (g8 * (exp(-k9 / t9) + 1.))
				    * exp((k9 - 24.) / 2.) / (1. + g8 *
				    g8);
				if (g0 < ftemp)
					g0 = ftemp;
			}
		}
		ftemp = (1. + ssn / 250.) * m9 * sqrt(6. + 58. *
		    sqrt(g0));
		ftemp *= 1. - .1 * exp((k9 - 24.) / 3.);
		ftemp *= 1. + .1 * (1. - SGN(lat1) * SGN(lat2));
		ftemp *= 1. - .1 * (1. + SGN(fabs(sin(y3)) - cos(y3)));
		if (ftemp < muf)
			muf = ftemp;
	}
	return (muf);
}

/*
 * spots(flux) - Routine to map solar flux to sunspot number.
 *
 * THis routine was done by eyeball and graph on p. 22-6 of the 1991
 * ARRL Handbook. The nice curve fitting was done using Mathematica.
 */
double
spots(
	double flux			/* 10-cm solar flux */
	)
{
	double ftemp;			/* double temp */

	if (flux < 65.)
		return (0.);
	else if (flux < 110.) {
		ftemp = flux - 200.6;
		ftemp = 108.36 - .005896 * ftemp * ftemp;
	} else if (flux < 213.) {
		ftemp = 60. + 1.0680 * (flux - 110.);
	} else {
		ftemp = flux - 652.9;
		ftemp = 384.0 - .0011059 * ftemp * ftemp;
	}
	return (ftemp);
}
