#!/bin/bash

if [ `id -u` -ne 0 ]; then
	echo "Please execute this installer script with sudo, exiting.."
	exit 1
fi

echo "Setting system's timezone to UTC."
timedatectl set-timezone Etc/UTC
sleep 1


apt-get install -y libcurl4-openssl-dev
if [[ $> 0 ]]
then
	echo "libcurl failed to install, exiting."
else
	echo "libcurl is installed, continuing with script."
fi

echo "Installing git..."

apt install -y git
if [[ $> 0 ]]
then
	echo "git failed to install, exiting."
else
	echo "git is installed, continuing with script."
fi

read -p "Enter a new username:" user_var
echo "The current username is:" "$user_var"

adduser --force-badname "$user_var" --gecos "" --disabled-password
passwd "$user_var"
while [ $? -ne 0 ]; do
	sleep 3 #give time to ctrl-c
	passwd $user_var
done

for GROUP in $(groups pi | sed 's/.*:\spi//'); do adduser "$user_var" $GROUP; done

sleep 2
getent group sudo | grep -q "$user_var"
if [ $? -eq 0 ]; then
	echo "$user_var has root privileges, continuing..."
else
	echo "Adding using to root failed...Try a new username?" 1>&2
	exit 1
fi

autologinf=/etc/systemd/system/getty@tty1.service.d/autologin.conf

if [ -e "$autologinf" ]; then
	echo "An auto-login config file already exists"
else
	echo "Building an auto-login config file for "$user_var""
	echo "[Service]"\ >> $autologinf
	echo "ExecStart="\ >> $autologinf
	echo "ExecStart=-/sbin/agetty --autologin "$user_var" --noclear %I $TERM" >> $autologinf
fi

cronjob1="@reboot sleep 300 && /home/$user_var/ad_dev/cimel_start.sh"
cronjob2="57 23 */2 * * /home/$user_var/ad_dev/status.sh"
cronjob3="0 */3 * * * /home/$user_var/ad_dev/modem_dog.sh"
cronjob4="0 */1 * * * /home/$ser_var/ad_dev/conn_dog.sh"

(crontab -u $user_var -l; echo "$cronjob1" ) | crontab -u $user_var -
(crontab -u $user_var -l; echo "$cronjob2" ) | crontab -u $user_var -
(crontab -u $user_var -l; echo "$cronjob3" ) | crontab -u $user_var -
(crontab -u $user_var -l; echo "$cronjob4" ) | crontab -u $user_var -

mkdir /home/$user_var/logs
cp -r $PWD /home/$user_var
cd /home/$user_var/ad_dev/v5_cimel_connect
echo "Compiling cimel connect..."
cc -g -o multi_https_connect multi_connect.c my_com_port.c aero_time.c -lm -L /usr/lib/arm-linux-gnueabihf/ -lcurl


sleep 2
echo "Build complete"
