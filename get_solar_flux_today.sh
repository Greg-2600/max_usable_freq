#!/bin/bash
get_solar_data() {
	#Solar-terrestrial indices for 20 August follow.
	#Solar flux 67 and estimated planetary A-index 4.
	#The estimated planetary K-index at 1500 UTC on 21 August was 1.

	#No space weather storms were observed for the past 24 hours.

	#No space weather storms are predicted for the next 24 hours.

	solar_data=$(curl -s -L "http://services.swpc.noaa.gov/text/wwv.txt")
	solar_flux=$(echo "$solar_data"|grep flux|cut -f3 -d ' ')
	echo "$solar_flux"
}
get_solar_data
