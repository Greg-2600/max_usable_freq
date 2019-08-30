#!/bin/bash

api_endpoint='http://lasp.colorado.edu/lisird/latis/dap/noaa_radio_flux.csv'
solar_flux_data_file='solar_flux.csv'

#curl "$api_endpoint"|grep -v '99999'|grep -v [a-z]|sed 's/,/ /g' > "${solar_flux_data_file}"

data=$(cat ${solar_flux_data_file}|tail -1000)

months=$(echo "$data"|awk {'print $2'}|sort|uniq)
days=$(echo "$data"|awk {'print $3'}|sort|uniq)

get_solar_flux_by_date() {
	month_arg=$1
	day_arg=$2
	echo "$data"|
		grep " $month_arg $day_arg"|
		awk {'print $4'}
}


get_average() {
	local list="$@"
	local sum=$(echo "$list"|awk '{s+=$1} END {print s}')
	local count=$(echo $list|wc -w)
	local average=$(echo "$sum / $count"|bc -l)
	echo $average
}


for month in $months; do
	for day in $days; do
		result=$(get_average $(get_solar_flux_by_date $month $day))
		#echo "average solar_flux on $month/$day is $result"
		echo "$result $month $day"
	done
done|sort -n|head -10
