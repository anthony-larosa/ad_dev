#!/bin/bash

now=$(date)

getscript() {
  pgrep -lf ".[ /]$1( |\$)"
}
if getscript "model5_connect" >/dev/null; then
	echo "Model 5 connect is running at ${now}" >> $HOME/logs/connection.log
else
	echo "Model 5 connect was not running at ${now}." >> $HOME/logs/connection.log
	/home/RPiz_2/ad_dev/v5_cimel_connect/model5_connect USB0
fi
echo "Finished checking if Cimel Connect was running"
