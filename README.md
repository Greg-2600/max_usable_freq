# max_usable_freq
wrapping minimuf - expermental



           Program to Predict High Frequency Propagation Data

Purpose

This program predicts the most likely operating frequencies and signal
levels for high frequency (shortwave) radio propagation paths on
specified days of the year and hours of the day. It is most useful for
paths between 250 km and 6000 km, but can be used with reduced accuracy
for paths shorter or longer than this. It can be used with a companion
program "icom" and Unix shell scripts to automatically retune radio
receivers for broadcast time services such as CHU (Canada) and WWV/H
(U.S.) as propagation conditions change over the day.

High frequency radio propagation between approximately 3 MHz and 30 MHz
is dominated by two ionospheric layers, the E layer at a height of about
110 km above the Earth, and the F layer, which varies between 250 km and
350 km, depending on the time of day, season of year and solar activity.
For the medium range paths for which this program is intended, the
dominant mode is via the F-layer, although the program does consider the
E-layer in order to determine path loss. The success of each path is
determined by the transmitter power, receiver sensitivity, propagation
path loss and ambient noise. Especially at the lower frequencies during
the summer months, the ambient noise is dominated by electrical
discharges due to lightning in local and distant thunderstorms. This
program computes the thermal component of the ambient noise, but does
not include the electrical component, which must be estimated from
experience and seasonal conditions.

Radio propagation predictions require knowledge of solar activity, which
is compiled from daily observations of the Sun and usually expressed as
the 10-cm solar flux, which has largely replaced the use of sunspot
counts for this purpose. The accuracy of the program depends on the
accuracy in predicting this number, which is usually averaged over some
period of days, weeks or months. The flux for the preceding day is
broadcast by WWV/WWVH each hour and are also available by telephone
[NIS90]. Predictions for one or more months in advance are also
available in periodicals such as QST. The program uses a routine MINIMUF
3.5 developed by the U.S. Navy and used to predict the MUF given the
predicted flux, day of the year, hour of the day and geographic
coordinates of the transmitter and receiver. This routine is reasonably
accurate for the purposes here, with a claimed RMS error of 3.8 MHz, but
much smaller and less complex than the programs used by major shortwave
broadcasting organizations, such as the Voice of America.

In order to interpret the data produced by this program, it is useful to
consider how it is produced. Given the MUF as predicted, the program
constructs candidate ray geometries over the shorter of the two great-
circle paths between the transmitter and receiver. The program can also
do this for the longer of these paths, but the accuracy is questionable
at best. The program constructs first the minimum-hop path constrained
by the minimum takeoff angle; that is, the minimum angle between the
local horizon at the great-circle azimuth and transmitted ray, which is
a property of the antenna and local topography. The default minimum
angle is 10 deg, but this can be changed by an option. The program then
constructs paths for the next two higher-number hops, in order to assess
multipath conditions. For all three paths the program computes the
minimum F-layer MUF, maximum E-layer MUF and ionospheric absorption
factor.

Using these data and the transmitter power, antenna gain and frequencies
specified, the program determines for each frequency whether a geometric
ray path is possible and, if so, computes the path loss and delay. A
path is possible only when the frequency is below the F-layer MUF and
the signal level is greater than the thermal noise (2500 Hz at 290 K).
The program output is in the form of a table, with one entry for each
hour of the UTC day, as described below.
Program Input Data

The program begins by reading an input data file followed by a list of
frequencies and the geographic coordinates of the transmitter. Next, the
program reads a list of geographic coordinates for each of the receivers
and produces an output table for each in turn. The data are read in the
following order without regard to line boundaries:

                format month day sn power nfreq freq...

where

     format    output format 1: signal (dB), 2: takeoff angle (deg), 3:
               path delay (ms), 4: special format - see Usage below.
               Corresponds to -o option on command line.

     month     month of year (1-12). Corresponds to -m option on command
               line.

     day       day of month (1-31). Corresponds to -d option on command
               line.

     flux      10-cm solar flux (65-250). Corresponds to -s option on
               command line.

     power     transmitter power (dBW). Corresponds to -p option on
               command line.

     nfreq     number of frequencies to follow

     freq...   frequencies (MHz)

These date are followed by a single line specifying the location and
site name of the transmitter

                            tlat tlon tsite

where

     tlat      transmitter latitude (deg N)

     tlon      transmitter longitude (deg W)

     tsite     transmitter site name (extends to end of line)

These data are followed by one or more lines specifying the location and
site name for each receiver

                            rlat rlon rsite

where

     rlat      receiver latitude (deg N)

     rlon      receiver longitude (deg W)

     rsite     receiver site name (extends to end of line)

Note that latitude is measured in degrees 0-90 and fraction from the
Equator, where northern latitudes have a positive sign and southern
latitudes have a negative sign. Longitude is measured in degrees from
the prime (Greenwich) meridian 0-360 and fraction increasing to the
east. For instance, the geographic coordinates of Evans Hall at the
University of Delaware are latitude 39.68005 and longitude -75.75085, as
determined by a GPS receiver.
In the following example, the first line specifies the output format for
signal level, the day of 5 March, 10-cm solar flux 90 and transmitter
power 20 dBW (100 W). The second line specifies five frequencies, while
the third specifies the transmitter data and the fourth the receiver
data.

     1 3 5 88 20
     5 2.5 5 10 15 20
     39.68005 -75.75085 UDel Evans Hall
     40.6803 -105.0408 WWV Fort Collins

The program assumes the transmitter and receivers use isotropic
antennas, with uniform effective radiated power over all elevation and
azimuth angles. However, the program can read an antenna data file
describing the antenna gain at various frequencies and elevation angles.
The format of this table is described in the source program. When using
this table, the program interpolates between frequencies and elevation
angles as required.

Program Output Data

In most formats the program produces a table with a header and one line
for each hour of the day. The header includes the input data, together
with calculated sunspot number (SN), great-circle distance (km), path
delay (ms) and transmitter antenna azimuth and receiver antenna azimuth
(deg clockwise from North). The following lines show the UTC time at the
transmitter, local time at the receiver, MUF of the path (MHz), Sun
zenith angle (deg) at the midpoint of the path and path descriptor for
each frequency with which communication is possible. Following is a
typical display:

     10-cm solar flux:  90   SN:  36   Month:  3   Day:  5
     Power: 20 dBW    Distance:  2479 km    Delay:  9.5 ms
     Location                        Lat      Long    Azim
      UDel Evans Hall              39.68N    75.75W    282
      WWV Fort Collins             40.68N   105.04W     83
     UT LT  MUF Zen    2.5    5.0   10.0   15.0   20.0
      0 17 18.8  -4   34j2m  34j2   33j2   30j2m
      1 18 15.0 -15   36x2   36x2   33x2m
      2 19 12.7 -26   36n2   36n2   33n2m
      3 20 11.8 -36   36n2   36n2   33n2m
      4 21 11.1 -45   36n2   36n2
      5 22 10.5 -52   36n2   36n2
      6 23 10.0 -55   36n2   36n2
      7 24  9.6 -52   36n2   36n2
      8  1  9.3 -46   36n2   36n2
      9  2  9.0 -37   36n2   36n2
     10  3  8.8 -26   36n2   36n2
     11  4  8.7 -15   36x2   36x2
     12  5 15.5  -4   34j2m  35j2   33j2
     13  6 19.0   7   22j2   29j2m  31j2   29j2
     14  7 21.0  18    3j2   19j2   27j2   28j2
     15  8 22.3  28          10j2   24j2   26j2
     16  9 23.2  36           3j2   21j2   25j2
     17 10 23.8  41          -2j2s  19j2   24j2   24j2
     18 11 24.0  43          -3j2s  19j2   23j2   24j2
     19 12 24.0  41          -2j2s  19j2   24j2   24j2
     20 13 23.7  36           3j2   21j2   25j2   25j2
     21 14 23.2  28          10j2   24j2   26j2
     22 15 22.2  18    3j2   19j2   27j2   28j2
     23 16 20.9   8   22j2   28j2m  31j2   29j2

The path descriptor indicates the quality of the path, which depends on
the signal level, number of hops and whether multipath (fading)
conditions exist. It is coded in the following format:
                                 nnnihf

where

     nnn  Signal level in dB above the receiver sensitivity (formats 1
          and 4), takeoff angle in degrees (format 2) or path delay in
          ms (format 3).

     i    Character indicating whether all ray hops are in daytime 'j',
          all hops are in nighttime 'n' or whether some are in daytime
          and others in nighttime 'x'. For this purpose, a hop is in
          daytime if the Sun zenith angle at the ionospheric reflection
          zone is greater than zero and nighttime otherwise.

     h    Number of ray hops. The maximum distance that can be covered
          in a single hop is less than 4000 km, depending on the takeoff
          angle calculated for the particular ray geometry. Thus, while
          it is possible for the longest great-circle path of 20,000 km
          to be covered in five hops, more like 7 or 8 hops are usually
          required.

     f    Character indicating special path conditions. The character
          'e' indicates the frequency is below the E-layer MUF, which
          indicates the possibility that propagation via the F-layer
          mode may be cut off; however, it also indicates that
          propagation via the E-layer mode or mixed modes may exist. The
          character 'm' indicates that propagation via two or more modes
          is possible and that the relative receive levels are within 3
          dB of each other. When this is the case, severe multipath
          distortion (fading) may occur. The character 's' indicates the
          signal level is below the rated receiver sensitivity and may
          be masked by noise.

The signal level is determined relative to the receiver sensitivity,
which is defined as the signal which produces a signal/noise ratio of 10
dB for CW, SSB and RTTY modes. This program assumes a sensitivity of
0.15 microvolt at 50 Ohms receiver input impedance (-123 dBm), which is
typical of modern communications-grade receivers. The signal-strength
(S) meter is usually calibrated to indicate S9 with a receiver input of
100 microvolts (-67 dBm), which corresponds to 56 dB as indicated in the
path descriptor. These meters are usually calibrated at 6 dB per S unit,
so a signal that just nudges the meter, which usually occurs at the AGC
threshold, corresponds to 2 dB as indicated.

Usage

The command format is

                  minimuf [options] [infile] [antfile]

where

     options   command-line options

     infile    name of input data file (optional)

     antfile   name of antenna data file (optional)

If both file names are omitted, the program reads the input data from
the standard input (stdin). In the case of Unix environments, a set of
command-line modifiers is available. Some of these can be used to
override data read from the input data file. This is useful when
building shell scripts to automatically tune receivers in response to
changing ionospheric conditions over the day. The options are specified
as an option letter, preceded by a minus (-) sign and followed by an
argument. Following is a list of the options currently implemented.

     -d day    day of month (1-31). Overrides day specified in the input

     -e angle  minimum takeoff angle (deg) (default is 10 deg) data
               file.

     -h hour   hour of day (0-23). When the hour is specified in a
               command-line option, the program produces only a single
               line of output for that hour. Overrides hour specified in
               the input data file.

     -l        use long path (default is short path)

     -m month  month of year (1-12). Overrides month specified in the
               input data file.

     -o format output format 1: signal (dB), 2: takeoff angle (deg), 3:
               path delay (ms), 4: special format - see below. Overrides
               format specified in the input data file.

     -p power  transmitter power (dBW). Overrides power specified in the
               input data file.

     -s flux   10-cm solar flux. Overrides flux specified in the input
               data file.

Output format 4 is designed for shell scripts and other Unix utilities.
In this format no header is produced. The program selects the best path
for each frequency in the usual way, then selects the best from among
all frequencies. As in the other modes, it outputs the UTC time at the
transmitter, local time at the receiver, MUF of the path (MHz) and Sun
zenith angle (deg) at the midpoint of the path. This is followed by the
selected frequency and path descriptor. Note that the frequency is
represented with a precision of 10 Hz, which is consistent with the
tuning precision of typical communications-grade receivers. Following is
an example display derived from the same data as the above example.

      0 17 22.2  -4  5.00000  30j2
      1 18 17.7 -15  2.50000  33x2
      2 19 15.0 -26  2.50000  33n2
      3 20 14.0 -36  2.50000  33n2
      4 21 13.1 -45  2.50000  36n2
      5 22 12.4 -52  2.50000  36n2
      6 23 11.8 -55  2.50000  36n2
      7 24 11.3 -52  2.50000  36n2
      8  1 11.0 -46  2.50000  36n2
      9  2 10.7 -37  2.50000  36n2
     10  3 10.4 -26  2.50000  36n2
     11  4 10.2 -15  2.50000  36x2
     12  5 18.4  -4  5.00000  33j2
     13  6 22.5   7 10.00000  29j2
     14  7 24.8  18 15.00000  28j2
     15  8 26.4  28 15.00000  26j2
     16  9 27.4  36 15.00000  25j2
     17 10 28.1  41 20.00000  24j2
     18 11 28.4  43 20.00000  24j2
     19 12 28.4  41 20.00000  24j2
     20 13 28.0  36 20.00000  25j2
     21 14 27.4  28 15.00000  26j2
     22 15 26.3  18 15.00000  28j2
     23 16 24.7   8 10.00000  29j2

Often only a single line of the above is required, as in shell scripts
activated once per hour, for example. In this case the -h command-line
option can be used to select the hour.

Installation Notes

The distribution includes the following files in a compressed tar
archive minimuf.tar.Z:

     Makefile       control file for make utility
     README         this file
     antenna.dat    sample antenna data file (dipole)
     dipole.dat     sample antenna data file (dipole/Yagi-Uda)
     minimuf.c      minimuf routine to compute F-layer MUF
     qth.dat        validation input data file
     shell.c        main program
     test.dat       test input data file

There are two antenna data files, one (dipole.dat) using a half-wave
dipole and the other (antenna.dat) using two antennas, a half-wave
dipole at 50 feet for frequencies below 14 MHz and a three-element Yagi-
Uda antenna at 50 feet for frequencies above 14 MHz. These tables were
constructed by another program not described here. While the particular
data were compiled for discrete Amateur frequencies from 80 meters (3.5
MHz) to 10 meters (28 MHz), data interpolated between these frequencies
are generally useful in view of other uncertainties inherent in the
algorithms used.

The program can be compiled, installed and run on most Unix systems
using the make utility. The default target is to compile and link, the
install target to install the executable and the clean target to remove
residual files produced during the make operation. The program can also
be compiled and run on most PC compatibles using Microsoft Visual C++
for Windows 95 or later.

The program can be tested using the test.dat input data file and
isotropic antennas (no antenna data file). It should produce the results
given earlier in this document. For validation, the qth.dat file
containing 156 receivers scattered all over the world can be used.
Running times depend on the CPU, of course; on a 486/50 the validation
suite completes in about 3.5 minutes, on a Sun IPC it takes about 55
seconds and on a DEC 3000/400 Alpha it takes a little over five seconds.

Reference

[NIS90] NIST Time and Frequency Dissemination Services. NBS Special
Publication 432 (Revised 1990), National Institute of Science and
Technology, U.S. Department of Commerce, 1990.

David L. Mills, W3HCF
43 The Horseshoe
Newark, DE 19711
mills@udel.edu

14 March 1994
last update 14 July 1998
