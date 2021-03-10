#!/bin/bash

getrun() {
	pgrep -lf ".[ /]$1( |\$)"
}
if getrun "modem_dog" >/dev/null; then
	echo "Running modem dog"
else
	echo "not running modem dog"
	/home/$USER/ad_dev/modem_dog.sh
fi
echo "Checked"
