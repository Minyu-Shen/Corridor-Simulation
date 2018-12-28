import matplotlib.pyplot as plt
from pylab import MaxNLocator
import numpy as np
import pandas as pd

df = pd.read_csv("/Users/samuel/Desktop/Cases/temp.csv", sep=" " ,header=None, names=['berth', 'platoon', 'line', 'tt_mean', 'bus_flow', 'pax_flow', 'bus_arrival_cv', 'tt_cv', 'mode', 'stop', 'delay', 'service'])
df.head()
lineStyle = ["-", "-.", "--", ":", "-."]
legNames = ["normal", "convoy", "sorting", "sortingFixHeadway", "convoyFixHeadway"]
lineColors = ['r','y','g','b','k']
ax = plt.figure().gca()
ax.xaxis.set_major_locator(MaxNLocator(integer=True))
bus_flow = df["bus_flow"][0]
pax_flow = df["pax_flow"][0]
bus_arr = df["bus_arrival_cv"][0]
for mode, mode_df in df.groupby('mode'):
    x = mode_df['stop']
    delay = mode_df['delay']
    service_time = mode_df['service']
    titleStr = "bus_flow = " + str(bus_flow) + ", " + "pax_flow = " + str(pax_flow) + ", " + \
                "bus_arrival_cv = " + str(bus_arr)
    plt.xlabel('stops')
    plt.ylabel('delay / delay+service_time(min)')
    plt.plot(x,delay,lineStyle[mode], color=lineColors[mode])
    plt.plot(x,delay+service_time,lineStyle[mode],color=lineColors[mode])
    plt.title(titleStr)

# fig.savefig("/Users/samuel/Desktop/tt_cv=0,inf_cap,const_arr.pdf", bbox_inches='tight')
