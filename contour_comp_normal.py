import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

df = pd.read_csv("/Users/samuel/Desktop/Corridor/results/temp.csv", sep=" ",header=None, names=['berth', 'platoon', 'line', 'tt_mean', 'bus_flow', 'pax_flow', 'bus_arrival_cv', 'tt_cv', 'mode', 'stop', 'delay', 'service'])
df.head()
fig = plt.figure()
ax = fig.add_subplot(111)
u = np.linspace(-1,1,100)
x, y = np.meshgrid(u, u)
z = x**2 + y**2
ax.contourf(x,y,z)
plt.show()
