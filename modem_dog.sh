#!/bin/bash
now=$(date)

sudo /usr/local/bin/hologram network disconnect
sleep 20

sudo /usr/local/bin/hologram modem reset
sleep 180

sudo /usr/local/bin/hologram network connect
sleep 10

echo "Executed modem disconnect, reset and connect at ${now}"  >> $HOME/logs/mdmlog.log
