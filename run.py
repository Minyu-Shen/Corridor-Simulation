import subprocess as subp
# output = open("/Users/samuel/Desktop/Cases/temp.csv", 'w')
output = open("../results/temp.csv", 'w')

# single case

# kStop = 18
# kBerth = 3
# tt_mean = 20.0
# for bus_demand in [100]:
#     for pax_demand in [600]: # pax/hr
#         for bus_arrival_cv in [0.2]: # buses/hr
#             for tt_cv in [0]:
#                 for mode in [0,1,2,3,4]:
#                     for kRoute in [3]:
#                         args = [kBerth, kBerth, kRoute, tt_mean, bus_demand, pax_demand, bus_arrival_cv, tt_cv, mode, kStop]
#                         args = ' '.join(map(str, args))
#                         print args
#                         output.write(subp.check_output("./main " + args, shell=True))
# output.close()

# tt_cv = 0 inf capacity case
kStop = 18
kBerth = 3
tt_mean = 20.0
for bus_demand in [100,200,300]:
    for pax_demand in [400,800,1200,1600]: # pax/hr
        for bus_arrival_cv in [0.2,0.6]: # buses/hr
            for tt_cv in [0,0.4]:
                for mode in [0,1,2,3,4]:
                    for kRoute in [3]:
                        args = [kBerth, kBerth, kRoute, tt_mean, bus_demand, pax_demand, bus_arrival_cv, tt_cv, mode, kStop]
                        args = ' '.join(map(str, args))
                        print args
                        output.write(subp.check_output("./main " + args, shell=True))

# import multiprocessing
# kStop = 18
# kBerth = 3
# tt_mean = 20.0
# def main_body(arr_cv):
#     bus_arrival_cv = arr_cv
#     for bus_demand in [100]:
#         for pax_demand in [400]: # pax/hr
#             for tt_cv in [0]:
#                 for mode in [0,1,2,3,4]:
#                     for kRoute in [3]:
#                         args = [kBerth, kBerth, kRoute, tt_mean, bus_demand, pax_demand, bus_arrival_cv, tt_cv, mode, kStop]
#                         args = ' '.join(map(str, args))
#                         print args
#                         output.write(subp.check_output("./main " + args, shell=True))
#
# def sim_0_cv():
#     main_body(0)
#
# def sim_20_cv():
#     main_body(0.2)
#
# def sim_20_cv():
#     main_body(0.4)
#
# p1 = multiprocessing.Process(target=sim_0_cv, args())

output.close()




# draw the graph
