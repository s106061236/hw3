import matplotlib.pyplot as plt
import numpy as np
import serial
import time

Fs = 100.0;  # sampling rate
Ts = 1.0/Fs; # sampling interval
t = np.arange(0,1,Ts) # time vector; create Fs samples between 0 and 1.0 sec.
y1 = np.arange(0,1,Ts) # signal vector; create Fs samples
y2 = np.arange(0,1,Ts)
y3 = np.arange(0,1,Ts)
y4 = np.arange(0,1,Ts)

serdev = '/dev/ttyACM0'
s = serial.Serial(serdev, 115200)
for x in range(0, int(Fs)):
    line=s.readline() # Read an echo string from K66F terminated with '\n'
    # print line
    y1[x] = float(line)

    line=s.readline()
    y2[x] = float(line)

    line=s.readline()
    y3[x] = float(line)

    line=s.readline()
    y4[x] = int(line)

fig, ax = plt.subplots(2, 1)
ax[0].plot(t,y1,label='X')
ax[0].plot(t,y2,label='Y')
ax[0].plot(t,y3,label='Z')
ax[0].legend()
ax[0].set_xlabel('Time')
ax[0].set_ylabel('Acc Vector')
ax[1].stem(t,y4)
ax[1].set_xlabel('Time')
ax[1].set_ylabel('Tilt')
plt.show()
s.close()