from merlin_interface.merlin_interface import MerlinInterface
m = MerlinInterface(tcp_ip = "127.0.0.1" , tcp_port=6341)
m.hvbias = 120
m.threshold0 = 200
m.threshold1 = 511
m.continuousrw = 1
m.counterdepth = 1
m.acquisitiontime = 75
m.acquisitionperiod = 75
m.numframestoacquire = 65792
m.fileenable = 0
m.triggerstart = 1
m.startacquisition()