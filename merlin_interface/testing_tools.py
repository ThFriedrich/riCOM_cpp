import socket
import multiprocessing as mp
from ctypes import c_char
import time


class TestMerlinConn(object):

    def __init__(self):
        self.ip = mp.Array(c_char, 20)
        self.port = mp.Value('i', 0)
        self.looper = mp.Value('b', True)

    def _start_socket(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        ip = b'127.0.0.1'
        s.bind((ip, self.port.value))
        self.ip.value = ip
        self.port.value = s.getsockname()[1]
        s.listen(1)
        return(s)

    def start_command_listening(self):
        self.command_listen_process = mp.Process(
                target=self._socket_send_process)
        self.command_listen_process.start()
        time.sleep(0.05)

    def _socket_send_process(self):
        BUFFER_SIZE = 4096
        s = self._start_socket()
        conn, addr = s.accept()
        while self.looper.value:
            command = conn.recv(BUFFER_SIZE)
            command += b',0'
            conn.sendall(command)

    def stop_socket(self):
        self.command_listen_process.terminate()
        time.sleep(0.05)
