import subprocess as subp
import timeit
# output = open("../results/test.csv", 'wb')
# output = open("../results/4_run_number_test.csv", 'wb')
output = open("../results/ttt.csv", 'wb')
# output = open("../results/5_run_number_test.csv", 'wb')


kRoute = 3
kStop = 12
bus_demand = 100
tt_mean = 60.0 * 3 # 60.0 * 3
capacity = 10000.0 # 80.0
cp_ratio_group = 1
alight_prob  = 0
cp_ratio_all = 0 # 0.2
serial_group_size = 3
bus_arrival_cv = 0.4
# queue_rule = 1 # 0-> parallel, 1->FIFO
queue_rule = 1
# run_combs = [[1, 3]]

for pax_demand in [600]: # pax/hr
    for test_runs in range(1,100,1):
    # for test_runs in [20]:
        for std_tt in [10.0]:
            tt_cv = std_tt / tt_mean
            # 0->do-nothing, 1->convoy, 2->holding, 3->convoy_holding
            for mode in [0]: #0,1,2,3
                for kBerth in [3]:
                    # serial_group_size = kBerth
                    # args = [kBerth, serial_group_size, kRoute, tt_mean, bus_demand, pax_demand,\
                    #     bus_arrival_cv, tt_cv, mode, kStop, capacity, alight_prob,\
                    #     cp_ratio_group, cp_ratio_all, queue_rule, test_runs]
                    kRoute = kBerth
                    serial_group_size = kRoute
                    args = [kBerth, serial_group_size, kRoute, tt_mean, kBerth/3*bus_demand, kBerth/3*pax_demand,\
                        bus_arrival_cv, tt_cv, mode, kStop, capacity, alight_prob,\
                        cp_ratio_group, cp_ratio_all, queue_rule, test_runs]
                    args = ' '.join(map(str, args))
                    print(args)
                    # start = timeit.default_timer()
                    output.write(subp.check_output("./main " + args, shell=True))
                    # stop = timeit.default_timer()
                    # print('Time: ', stop - start)
                    # times.append(stop - start)
output.close()
