import subprocess as subp
import timeit
output = open("../results/result.csv", 'wb')

# The below three features were implemented in the development of this paper. 
# Just set them to be the following default values.
capacity = 10000.0 # 80.0
alight_prob  = 0
cp_ratio_all = 0

# The below parameter are used in the paper
kRoute = 3
kStop = 12
bus_demand = 100
tt_mean = 60.0 * 3 # This value does not affect the performance metrics in the paper.
serial_group_size = 3
bus_arrival_cv = 0.4
queue_rule = 1 # 0-> parallel, 1->FIFO
pax_demand = 600
test_runs = 20
std_tt = 10.0 # std of link travel time in seconds
tt_cv = std_tt / tt_mean # c.v. of link travel time
mode = 0 # 0->do-nothing, 1->convoy, 2->holding, 3->convoy_holding (not presented in the paper)
kBerth = 3
kRoute = 3 # should be a multiple of kBerth for convoying strategy
serial_group_size = kRoute # the number of line groups
cp_ratio_group = 1 # common-line patron ratio, [0,1]

args = [kBerth, serial_group_size, kRoute, tt_mean, kBerth/3*bus_demand, kBerth/3*pax_demand,\
        bus_arrival_cv, tt_cv, mode, kStop, capacity, alight_prob,\
        cp_ratio_group, cp_ratio_all, queue_rule, test_runs]
args = ' '.join(map(str, args))
print(args)
# start = timeit.default_timer()
output.write(subp.check_output("./main " + args, shell=True))
# stop = timeit.default_timer()
# print('Time: ', stop - start)
output.close()
