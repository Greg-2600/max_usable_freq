#!/bin/bash


get_solar_data() {
	#Solar-terrestrial indices for 20 August follow.
	#Solar flux 67 and estimated planetary A-index 4.
	#The estimated planetary K-index at 1500 UTC on 21 August was 1.

	#No space weather storms were observed for the past 24 hours.

	#No space weather storms are predicted for the next 24 hours.

	solar_data=$(curl -s -L "http://services.swpc.noaa.gov/text/wwv.txt")
	export solar_flux=$(echo "$solar_data"|grep flux|cut -f3 -d ' ')
}


#for month in $(seq 1 12); do

format=1
month=08
day=15
power=100
count_freq=1
freq='7.185'
trans_loc='39.68005 -75.75085'
trans_name='UDel Evans Hall'
recv_loc='40.6803 -105.0408'
recv_name='WWV Fort Collins'
temp_in='temp_in'
temp_out='temp_out'
utc_hour=$(date -u +%H)
solar_flux=67


echo "$format $month $day $solar_flux $power">${temp_in}
echo "$count_freq $freq">>${temp_in}
echo "$trans_loc $trans_name">>${temp_in}
echo "$recv_loc $recv_name">>${temp_in}

data=$(./minimuf temp_in dipole.dat)


distance=$(echo "$data"|grep istance|awk {'print $4, $5'})
delay=$(echo "$data"|grep elay|awk {'print $7, $8'})
snr=$(echo "$data"|grep SN|awk {'print $6'})
nnnihf_hour=$(echo "$data"|grep -A24 'MUF Zen'|grep -E "^$utc_hour")
nnnihf_hour_MUF=$(echo "$data"|grep -A24 'MUF Zen'|grep -E "^$utc_hour"|awk {'print $3'})
nnnihf_hour_zenith=$(echo "$data"|grep -A24 'MUF Zen'|grep -E "^$utc_hour"|awk {'print $4'})
nnnihf_hour_path_descriptor=$(echo "$data"|grep -A24 'MUF Zen'|grep -E "^$utc_hour"|awk {'print $5'})
hops_day=$(echo $nnnihf_hour_path_descriptor|grep j)
hops_night=$(echo $nnnihf_hour_path_descriptor|grep n)
hops_day_and_night=$(echo $nnnihf_hour_path_descriptor|grep x)
below_e_layer_MUF=$(echo $nnnihf_hour_path_descriptor|grep e)
multipath_distortion=$(echo $nnnihf_hour_path_descriptor|grep m)
high_noise=$(echo $nnnihf_hour_path_descriptor|grep s)
signal_level_db=$(echo "$nnnihf_hour_path_descriptor"|tr "[a-z]" " "|awk {'print $1'})


echo "distance: $distance"
echo "signal_to_noise: $snr"
echo "delay: $delay"
echo "UTC_hour: $utc_hour"
echo "nnnihf_hour_MUF: $nnnihf_hour_MUF"
echo "nnnihf_hour_zenith: $nnnihf_hour_zenith"
echo "nnnihf_hour_path_descriptor: $nnnihf_hour_path_descriptor"
echo "signal_level_in_db: $signal_level_db"
echo "hops_day: $hops_day"
echo "hops_night: $hops_night"
echo "hops_day_and_night: $hops_day_and_night"
echo "below_e_layer_MUF: $below_e_layer_MUF"
echo "multipath_distortion: $multipath_distortion"
echo "high_noise: $high_noise"


get_best_hour() {
	#echo "$data"|awk {'print $5'}
	nnnihf_hour_path_descriptor=$(echo "$data"|grep -A24 'MUF Zen'|awk {'print $1, $5'}|tr "[a-z]" " "|awk {'print $2,$1'}|sort -n|awk {'print "UTC:" $2, "dB:" $1'}|tail -1)
	echo best_hour: "$nnnihf_hour_path_descriptor"
}
get_best_hour

#echo $month $day

#done


#get_arguments() {

	#args=()
	#while [[ $# -gt 0 ]]
	#do
	#	
	#	case $key in
	#    		-m|--month)
	#    			month="$2"
	#    			shift shift ;;
	#
	#    		-d|--day)
	#    			day="$2"
	#    			shift shift ;;
	#
	#    		-f|--frequency)
	#    			freq="$2"
	#    			shift shift ;;
	#
	#    		-t|--transmission_cordinates)
	#    			transmission_cordinates="$2"
	#    			shift shift ;;
	#
	#    		-r|--receive_corinates)
	#    			receive_corinates="$2"
	#    			shift shift ;;
	#    		*)
	#
	#    		args+=("$1") # save it in an array for later
	#    		shift ;;
	#	esac
	#done
	#set -- "${args[@]}"
#}
display_usage() {
	echo ""
	echo "Usage: $0 [--month num] [--day num] [--frequency num] [--solar_flux num]"
	echo "          [--transmission_coordinates [value]] [--transmission_name [pattern]]"
	echo "          [--receive_coordinates [value]] [--receive_name [pattern]]"
	echo "          coordinates must be sperated by commas i.e.: \"-90,101\""
	echo "          names must be seperated by underscores i.e.: \"this_name\""
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
if [ $month -gt $max_month ]; then
	echo "ERROR: invalid month range $month is larger than $max_month" 
	display_usage
fi

max_day=31
if [ $day -gt $max_day ]; then
	echo "ERROR: invalid day range $day is larger than $max_day" 
	display_usage
fi

receive_coordinates_count=$(echo $receive_coordinates|tr -d '-'|wc -w)
if [ $receive_coordinates_count -ne 2 ]; then 
	echo "ERROR: please verify --receive_coordinates: $receive_coordinates" 
	display_usage
fi 

transmission_coordinates_count=$(echo $transmission_coordinates|tr -d '-'|wc -w)
if [ $transmission_coordinates_count -ne 2 ]; then 
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
#./.wrapper --month 2 --day 28 --frequency 7.185 --solar_flux 67 --transmission_coordinates 90,-102 --transmission_name foo_bar --receive_coordinates -101,89 --receive_name baz_bro
#	$0 --month 2 --day 28 --frequency 7.185 --solar_flux 67 --transmission_coordinates 90,-102 --transmission_name this_place --receive_coordinates -101,89 --receive_name that_place"
