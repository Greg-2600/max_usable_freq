#!/bin/bash

display_usage() {
	echo
	echo -e "Arguments from the command line must not contain spaces, sorry"
	echo -e "Seperate coordinates comma i.e.: \"-90,101\" and names with underscores i.e.: \"this_name\""
	echo

	echo  "Usage:
	$0 --month 2 --day 28 --frequency 7.185 --solar_flux 67 --transmission_coordinates 90,-102 --transmission_name this_place --receive_coordinates -101,89 --receive_name that_place"
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
			receive_coordinates=$(echo "$arg"|sed 's/,/ /') ;;

		--receive_name)
			receive_name="$arg" ;;
		*)
	esac
	x=$arg
done

max_month=12
if [ "$month" -gt "$max_month" ]; then
	echo "ERROR: invalid month range $month is larger than $max_month" 
	display_usage
fi

max_day=31
if [ "$day" -gt "$max_day" ]; then
	echo "ERROR: invalid day range $day is larger than $max_day" 
	display_usage
fi

receive_coordinates_count=$(echo $receive_coordinates|tr -d '-'|wc -w)
if [  $receive_coordinates_count -ne 2 ]; then 
	echo "ERROR: please verify --receive_coordinates: $receive_coordinates" 
	display_usage
fi 

transmission_coordinates_count=$(echo $transmission_coordinates|tr -d '-'|wc -w)
if [  $transmission_coordinates_count -ne 2 ]; then 
	echo "ERROR: please verify --transmission_coordinates: $transmission_coordinates" 
	display_usage
fi 

echo "month: $month"
echo "day: $day"
echo "frequency: $frequency" 
echo "solar_flux: $solar_flux"
echo "transmission_coordinates: $transmission_coordinates"
echo "transmission_name: $transmission_name"
echo "receive_coordinates: $receive_coordinates"
echo "receive_name: $receive_name"
#./arguments.sh --month 2 --day 28 --frequency 7.185 --solar_flux 67 --transmission_coordinates 90,-102 --transmission_name foo_bar --receive_coordinates -101,89 --receive_name baz_bro
