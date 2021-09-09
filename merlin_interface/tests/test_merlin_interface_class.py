import unittest
import time
from merlin_interface.merlin_interface import MerlinInterface
import merlin_interface.testing_tools as tt


class test_merlin_interface_class_structure(unittest.TestCase):

    def test_init_simple(self):
        MerlinInterface()

    def test_init_set(self):
        MerlinInterface(
                tcp_ip='10.10.9.9', tcp_port=513, test_mode=True)

    def test_repr(self):
        merlin = MerlinInterface(
                tcp_ip='10.10.9.9', tcp_port=513)
        self.assertEqual(
                merlin.__repr__(),
                '<MerlinInterface, 10.10.9.9:513>')
        merlin.test_mode = True
        self.assertEqual(
                merlin.__repr__(),
                '<MerlinInterface, 10.10.9.9:513 (test mode)>')

    def test_make_message_string(self):
        merlin = MerlinInterface()
        message_string0 = merlin._make_message_string(b'test', b'stuff')
        self.assertEqual(message_string0, b'MPX,0000000011,stuff,test')
        message_string1 = merlin._make_message_string(
                b'test', b'stuff', [20, 30])
        self.assertEqual(
                message_string1, b'MPX,0000000017,stuff,test,20,30')
        with self.assertRaises(TypeError):
            merlin._make_message_string('test', 'stuff')


class test_tcp_ip_interface_socket(unittest.TestCase):

    def setUp(self):
        self.merlin_conn = tt.TestMerlinConn()
        self.merlin_conn.start_command_listening()
        time.sleep(0.01)
        self.merlin = MerlinInterface(
                tcp_ip=self.merlin_conn.ip.value,
                tcp_port=self.merlin_conn.port.value)

    def tearDown(self):
        self.merlin_conn.command_listen_process.terminate()

    def test_acquisitionperiod(self):
        self.merlin.acquisitionperiod
        self.merlin.acquisitionperiod = 40

    def test_acquisitiontime(self):
        self.merlin.acquisitiontime
        self.merlin.acquisitiontime = 50

    def test_chargesumming(self):
        self.merlin.chargesumming
        self.merlin.chargesumming = 1

    def test_colourmode(self):
        self.merlin.colourmode
        self.merlin.colourmode = 1

    def test_continuousrw(self):
        self.merlin.continuousrw
        self.merlin.continuousrw = 1

    def test_counterdepth(self):
        self.merlin.counterdepth
        self.merlin.counterdepth = 1

    def test_detectorstatus(self):
        self.merlin.detectorstatus

    def test_enablecounter1(self):
        self.merlin.enablecounter1
        self.merlin.enablecounter1 = 1

    def test_filedirectory(self):
        self.merlin.filedirectory
        self.merlin.filedirectory = "test"

    def test_fileenable(self):
        self.merlin.fileenable
        self.merlin.fileenable = 1

    def test_filename(self):
        self.merlin.filename
        self.merlin.filename = "afilename.mib"

    def test_gain(self):
        self.merlin.gain
        self.merlin.gain = 20

    def test_hvbias(self):
        self.merlin.hvbias
        self.merlin.hvbias = 60

    def test_numframespertrigger(self):
        self.merlin.numframespertrigger
        self.merlin.numframespertrigger = 2

    def test_numframestoacquire(self):
        self.merlin.numframestoacquire
        self.merlin.numframestoacquire = 2

    def test_reset(self):
        self.merlin.reset()

    def test_softtrigger(self):
        self.merlin.softtrigger()

    def test_softwareversion(self):
        self.merlin.softwareversion

    def test_startacquisition(self):
        self.merlin.startacquisition()

    def test_stopacquisition(self):
        self.merlin.stopacquisition()

    def test_temperature(self):
        self.merlin.temperature

    def test_thnumsteps(self):
        self.merlin.thnumsteps
        self.merlin.thnumsteps = 2

    def test_threshold0(self):
        self.merlin.threshold0
        self.merlin.threshold0 = 5

    def test_threshold1(self):
        self.merlin.threshold1
        self.merlin.threshold1 = 5

    def test_threshold2(self):
        self.merlin.threshold2
        self.merlin.threshold2 = 5

    def test_threshold3(self):
        self.merlin.threshold3
        self.merlin.threshold3 = 5

    def test_threshold4(self):
        self.merlin.threshold4
        self.merlin.threshold4 = 5

    def test_threshold5(self):
        self.merlin.threshold5
        self.merlin.threshold5 = 5

    def test_threshold6(self):
        self.merlin.threshold6
        self.merlin.threshold6 = 5

    def test_threshold7(self):
        self.merlin.threshold7
        self.merlin.threshold7 = 5

    def test_thscan(self):
        self.merlin.thscan
        self.merlin.thscan = 5

    def test_thstart(self):
        self.merlin.thstart
        self.merlin.thstart = 5

    def test_thstep(self):
        self.merlin.thstep
        self.merlin.thstep = 5

    def test_thstop(self):
        self.merlin.thstop
        self.merlin.thstop = 5

    def test_triggeroutlvds(self):
        self.merlin.triggeroutlvds
        self.merlin.triggeroutlvds = 5

    def test_triggeroutlvdsdelay(self):
        self.merlin.triggeroutlvdsdelay
        self.merlin.triggeroutlvdsdelay = 5

    def test_triggeroutlvdsy(self):
        self.merlin.triggeroutlvdsinvert
        self.merlin.triggeroutlvdsinvert = 5

    def test_triggeroutttl(self):
        self.merlin.triggeroutttl
        self.merlin.triggeroutttl = 5

    def test_triggeroutttldelay(self):
        self.merlin.triggeroutttldelay
        self.merlin.triggeroutttldelay = 5

    def test_triggeroutttlvinert(self):
        self.merlin.triggeroutttlinvert
        self.merlin.triggeroutttlinvert = 5

    def test_triggerstart(self):
        self.merlin.triggerstart
        self.merlin.triggerstart = 5

    def test_triggerstop(self):
        self.merlin.triggerstop
        self.merlin.triggerstop = 5

    def test_triggerusedelay(self):
        self.merlin.triggerusedelay
        self.merlin.triggerusedelay = 5
