#!/bin/bash

echo -e "Last reboot was: $(uptime -s)\n" >> $HOME/logs/status.log
systemctl status holoConn.service >> $HOME/logs/status.log
echo -e "\nThe system has been up since: $(uptime)" >> $HOME/logs/status.log
