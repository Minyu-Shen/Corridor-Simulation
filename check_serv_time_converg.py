import matplotlib.pyplot as plt
from pylab import MaxNLocator
import numpy as np
import pandas as pd

df = pd.read_csv("/Users/samuel/Desktop/Cases/temp.csv", sep=" " ,header=None, names=['time', 'delay'])
df.head()
# ax = plt.figure().gca()
# ax.xaxis.set_major_locator(MaxNLocator(integer=True))
# plt.xlabel('stops')
# plt.ylabel('delay(min)')
# plt.plot(x,y,lineStyle[mode])
# plt.legend(legNames, fontsize=8)
# plt.title(titleStr)

times = df["time"].tolist()
delays = df["delay"].tolist()
time_avgs = []
delay_avgs = []
for i in range(len(times)-30):
    time_avgs.append((sum(times[0:i+1]) / 60.0) / len(times[0:i+1]))
    delay_avgs.append((sum(delays[0:i+1]) / 60.0) / len(delays[0:i+1]))
plt.xlabel('bus #')
plt.ylabel('average service time / delays of first n buses (min)')
plt.plot(time_avgs,'b')
plt.plot(delay_avgs,'r')
plt.legend(['average service time in the stop', 'delays'])
plt.savefig("/Users/samuel/Desktop/convoy_holding.pdf")
