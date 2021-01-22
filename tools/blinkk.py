#!/usr/bin/python3
import board
import busio
import time
import adafruit_ads1x15.ads1115 as ADS
from adafruit_ads1x15.analog_in import AnalogIn
from datetime import datetime
import datetime

i2c = busio.I2C(board.SCL, board.SDA)
ads = ADS.ADS1115(i2c)

while True:
	f=open('/home/pi/battery.txt','a')
	now = datetime.datetime.now().isoformat()
	chan = AnalogIn(ads, ADS.P0)
	bat_volts = chan.voltage * 5.532
	data = now + ',' + "{0:.3f}".format(bat_volts) + '\n'
	f.write(data)
	time.sleep(600)
	f.close()
