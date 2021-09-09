import time
import pytest
import merlin_interface.merlin_interface as mi
import merlin_interface.testing_tools as tt


@pytest.fixture(autouse=True)
def add_mi(doctest_namespace):
    merlin_conn = tt.TestMerlinConn()
    merlin_conn.start_command_listening()
    time.sleep(0.01)
    merlin = mi.MerlinInterface(
            tcp_ip=merlin_conn.ip.value, tcp_port=merlin_conn.port.value)
    doctest_namespace['merlin'] = merlin
    doctest_namespace['merlin_conn'] = merlin_conn
    yield
    merlin_conn.command_listen_process.terminate()
