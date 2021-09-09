import unittest
import socket
import time
import merlin_interface.testing_tools as tt


class test_testing_tools(unittest.TestCase):

    def test_socket(self):
        merlin_conn = tt.TestMerlinConn()
        merlin_conn.start_command_listening()

        s = socket.socket()
        s.connect((merlin_conn.ip.value, merlin_conn.port.value))
        test_bytes0 = b'test'
        s.send(test_bytes0)
        recv0 = s.recv(1024)
        s.close()
        self.assertEqual(recv0, test_bytes0 + b',0')
        time.sleep(0.1)
        merlin_conn.command_listen_process.terminate()

        merlin_conn.start_command_listening()
        s = socket.socket()
        s.connect((merlin_conn.ip.value, merlin_conn.port.value))
        test_bytes1 = b'test_nr_2'
        s.send(test_bytes1)
        recv1 = s.recv(1024)
        self.assertEqual(recv1, test_bytes1 + b',0')
        time.sleep(0.1)
        s.close()
        merlin_conn.command_listen_process.terminate()
        time.sleep(0.1)

        s = socket.socket()
        with self.assertRaises(ConnectionRefusedError):
            s.connect((merlin_conn.ip.value, merlin_conn.port.value))
        s.close()
