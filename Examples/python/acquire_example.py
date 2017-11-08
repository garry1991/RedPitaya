#!/usr/bin/python

import sys
from time import sleep
import redpitaya_scpi as scpi
import matplotlib.pyplot as plot

ip = '192.168.10.144'

rp_s = scpi.scpi(ip)

rp_s.tx_txt('ACQ:RST')
rp_s.tx_txt('ACQ:SOUR1:GAIN LV')
rp_s.tx_txt('ACQ:TRIG:LEV 0')
rp_s.tx_txt('ACQ:TRIG:DLY 0')
rp_s.tx_txt('ACQ:DEC 8')
rp_s.tx_txt('ACQ:START')

sleep(0.1)

rp_s.tx_txt('ACQ:TRIG CH1_PE')

while 1:
    rp_s.tx_txt('ACQ:TRIG:STAT?')
    if rp_s.rx_txt() == 'TD':
        break

rp_s.tx_txt('ACQ:SOUR1:DATA?')

buff_string = rp_s.rx_txt()
buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
buff = list(map(float, buff_string))

plot.plot(buff)
plot.ylabel('Voltage')
plot.grid()
plot.show()
