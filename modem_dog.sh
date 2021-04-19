#!/bin/bash

now=$(date)
counter=0
getscript() {
  pgrep -lf ".[ /]$1( |\$)"
}
while [ $counter -lt 4 ]; do
	if getscript "model5_connect" >/dev/null; then
		echo "Model 5 connect is running at ${now}"
		sleep 30m
	else
		echo "Model 5 connect was not running at ${now}." >> $HOME/logs/connection.log
		counter=$((counter+1))
		/home/$USER/ad_dev/v5_cimel_connect/model5_connect USB0
		sleep 30m
	fi
done
echo "Reached counter at ${now} and performed reboot" >> $HOME/logs/connection.log
sudo reboot
