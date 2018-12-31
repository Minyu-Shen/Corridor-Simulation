
# output = open("/Users/samuel/Desktop/log.csv", 'w')

import matplotlib.pyplot as plt
from pylab import MaxNLocator
import numpy as np
import pandas as pd

df = pd.read_csv("/Users/samuel/Desktop/Corridor/results/temp.csv", sep=" ",header=None, names=['berth', 'platoon', 'line', 'tt_mean', 'bus_flow', 'pax_flow', 'bus_arrival_cv', 'tt_cv', 'mode', 'stop', 'delay', 'service'])
df.head()
nrow = df['bus_flow'].nunique() * df['pax_flow'].nunique() * df['tt_cv'].nunique()
ncolumn = df['bus_arrival_cv'].nunique()*3
# fig, axes = plt.subplots(nrows=nrow, ncols=ncolumn, figsize=(20, 45), gridspec_kw = {'width_ratios':[1, 1, 1]})
fig, axes = plt.subplots(nrows=nrow, ncols=ncolumn, figsize=(15, 5))
axes = axes.ravel()
lineStyle = ["-", "-.", "--", ":", "-."]
legNames = ["normal", "convoy", "sorting", "sortingFixHeadway", "convoyFixHeadway"]
lineColors = ['r','y','g','b','k']
# mode: 0: normal, 1: convoy, 2: Serial, 3: SerialFixedHeadway, 4: convoyFixedHeadway,
i = 0
for bus_flow, bus_flow_df in df.groupby('bus_flow'):
    for pax_flow, pax_flow_df in bus_flow_df.groupby('pax_flow'):
        for tt_cv, tt_cv_df  in pax_flow_df.groupby('tt_cv'):
            for arr_cv, arr_cv_df in tt_cv_df.groupby('bus_arrival_cv'):
                # 5 modes in one figure
                titleStr = "bus=" + str(bus_flow) + ", " + "pax=" + str(pax_flow) + ", " + \
                    "arr_cv = " + str(arr_cv) + ", "  + "ttcv=" + str(tt_cv)
                for s in range(3):
                    for mode, mode_df in arr_cv_df.groupby('mode'):
                        x = mode_df['stop']
                        delay = mode_df['delay']
                        service_time = mode_df['service']
                        axes[i].set_xlabel('stops')
                        if s==0:
                            axes[i].plot(x, delay, lineStyle[mode], color=lineColors[mode], linewidth=2.5)
                            # axes[i].plot(x, delay, lineStyle[mode])
                            axes[i].set_ylabel('delay(min)')
                            axes[i].set_title(titleStr + ", delay")
                        elif s==1:
                            axes[i].plot(x, service_time, lineStyle[mode], color=lineColors[mode], linewidth=2.5)
                            # axes[i].plot(x, delay+service_time, lineStyle[mode])
                            axes[i].set_ylabel('service_time(min)')
                            axes[i].set_title(titleStr + ", service_time")
                        else:
                            axes[i].plot(x, delay+service_time, lineStyle[mode], color=lineColors[mode], linewidth=2.5)
                            # axes[i].plot(x, delay+service_time, lineStyle[mode])
                            axes[i].set_ylabel('delay + service_time(min)')
                            axes[i].set_title(titleStr + ", delay + service_time")

                        ya = axes[i].get_yaxis()
                        ya.set_major_locator(MaxNLocator(integer=True))
                        axes[i].xaxis.set_major_locator(MaxNLocator(integer=True))
                        axes[i].legend(legNames, fontsize=8)
                    i = i + 1

fig.tight_layout()
fig.savefig("/Users/samuel/Desktop/limited_capacity_2.pdf", bbox_inches='tight')
