import subprocess as subp
import numpy as np
# from scipy.stats import variation

# output = open("../constant.csv", 'w')

np.random.seed(3)


nb_line = 8
nb_berth = 2
total_bus_flow = 72*nb_berth # buses/hr

# bus arrival flow
arr_rate_line = np.random.dirichlet(np.ones(nb_line),size=1)
cv_lmbd =  lambda x: np.std(x) / np.mean(x)
arr_cv = np.apply_along_axis(cv_lmbd, axis=1, arr=arr_rate_line)
# np.sqrt(np.var(a)) / np.mean(a)
arr_headways = 3600 / (arr_rate_line.flatten() * total_bus_flow) # seconds/bus

# bus service time
mean_serv = 25 # seconds
cv_serv = 0.4 # c.v.
serv_line = np.random.dirichlet(np.ones(nb_line),size=1)
serv_cv = np.apply_along_axis(cv_lmbd, axis=1, arr=serv_line)
serv_means = serv_line.flatten() * mean_serv * nb_line
print(serv_means)



# input to main
# args = [3, 6, "0,1,2,2,1,0", "300,300,300,300,300,300", "30.1,29.2,29.4,29.6,29.6,29.6", "0.4,0.4,0.4,0.4,0.4,0.4", 1, 1]
# args = ' '.join(map(str, args))
# a = subp.check_output("./main " + args, shell=True)
# print a


# output.write(subp.check_output("./main " + args, shell=True))
# output.close()
