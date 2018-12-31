
# output = open("/Users/samuel/Desktop/log.csv", 'w')

import matplotlib.pyplot as plt
from pylab import MaxNLocator
import numpy as np
import pandas as pd

df = pd.read_csv("/Users/samuel/Desktop/Corridor/results/tt_cv=0,cap=140,bus_flow=100,arr_cv=50.csv", sep=" ",header=None, names=['berth', 'platoon', 'line', 'tt_mean', 'bus_flow', 'pax_flow', 'bus_arrival_cv', 'tt_cv', 'dispatch_mode', 'stop', 'delay', 'service'])
df.head()
nrow = df['bus_flow'].nunique() * df['pax_flow'].nunique() * df['tt_cv'].nunique()
ncolumn = df['bus_arrival_cv'].nunique()*3
# fig, axes = plt.subplots(nrows=nrow, ncols=ncolumn, figsize=(20, 45), gridspec_kw = {'width_ratios':[1, 1, 1]})
fig, axes = plt.subplots(nrows=nrow, ncols=ncolumn, figsize=(18, nrow*5))
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
                # select the normal delay series
                delay_normal = arr_cv_df[arr_cv_df.dispatch_mode==0]['delay'].get_values()
                serv_time_normal = arr_cv_df[arr_cv_df.dispatch_mode==0]['service'].get_values()

                for s in range(3):
                    # calculate the percentage differences
                    for dip_mode, mode_df in arr_cv_df.groupby('dispatch_mode'):
                        x = mode_df['stop'].get_values()
                        delay = mode_df['delay'].get_values()
                        service_time = mode_df['service'].get_values()
                        axes[i].set_xlabel('stops')
                        if s==0:
                            # diff = np.divide(delay-delay_normal, delay_normal, out=np.zeros_like(delay), where=delay_normal!=0)
                            axes[i].plot(x, delay, lineStyle[dip_mode], color=lineColors[dip_mode], linewidth=2.5)
                            axes[i].set_ylabel('averge bus delay(min)')
                            axes[i].set_title(titleStr + ", delay")
                        elif s==1:
                            axes[i].plot(x, service_time, lineStyle[dip_mode], color=lineColors[dip_mode], linewidth=2.5)
                            axes[i].set_ylabel('averge bus service_time(min)')
                            axes[i].set_title(titleStr + ", service_time")
                        else:
                            axes[i].plot(x, delay+service_time, lineStyle[dip_mode], color=lineColors[dip_mode], linewidth=2.5)
                            axes[i].set_ylabel('averge bus delay + service_time(min)')
                            axes[i].set_title(titleStr + ", delay + service_time")

                        ya = axes[i].get_yaxis()
                        ya.set_major_locator(MaxNLocator(integer=True))
                        axes[i].xaxis.set_major_locator(MaxNLocator(integer=True))
                        axes[i].legend(legNames, fontsize=8)
                    i = i + 1

fig.tight_layout()
fig.savefig("/Users/samuel/Desktop/limited_capacity.pdf", bbox_inches='tight')
