#!/bin/bash

display_usage() {
	echo
	echo -e "Arguments from the command line must not contain spaces, sorry"
	echo -e "Seperate cordinates comma i.e.: \"-90,101\" and names with underscores i.e.: \"this_name\""
	echo

	echo  "Usage:
	$0 --month 2 --day 28 --frequency 7.185 --solar_flux 67 --transmission_cordinates 90,-102 --transmission_name this_place --receive_cordinates -101,89 --receive_name that_place"
	exit 0
}

if [  $# -le 1 ]; then 
	display_usage
	exit 1
fi 

argc=$@
x=0
for arg in $argc; do
	case $x in
		-h|--help)
			display_usage ;;

		-m|--month)
			month="$arg" ;;

		-d|--day)
			day="$arg" ;;

		-f|--frequency)
			frequency="$arg" ;;

		-s|--solar_flux)
			solar_flux="$arg" ;;

		--transmission_coordinates)
			transmission_coordinates=$(echo "$arg"|sed 's/,/ /') ;;

		--transmission_name)
			transmission_name="$arg" ;;

		--receive_coordinates)
			receive_coorinates=$(echo "$arg"|sed 's/,/ /') ;;

		--receive_name)
			receive_name="$arg" ;;
		*)
	esac
	x=$arg
done

echo "month: $month"
echo "day: $day"
echo "frequency: $frequency" 
echo "solar_flux: $solar_flux"
echo "transmission_cordinates: $transmission_cordinates"
echo "transmission_name: $transmission_name"
echo "receive_cordinates: $receive_corinates"
echo "receive_name: $receive_name"
#./arguments.sh --month 2 --day 28 --frequency 7.185 --solar_flux 67 --transmission_cordinates 90,-102 --transmission_name foo_bar --receive_cordinates -101,89 --receive_name baz_bro
