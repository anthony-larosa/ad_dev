#!/bin/bash

getrun() {
	pgrep -lf ".[ /]$1( |\$)"
}
if getrun "modem_dog.sh" >/dev/null; then
	echo "modem_dog is running"
else
	echo "modem_dog is not running, executing..."
	/home/$USER/ad_dev/modem_dog.sh
fi
echo "Checked"
