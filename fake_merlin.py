import socket
import numpy as np

type_dict = {'U08': np.uint8, 'U16': np.uint16,
             'U32': np.uint32, 'F': np.float32}


HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 6342        # The port used by the server
im_type = 'U08'


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen(1)
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)
        acq_str = ',HDR,\t\r\nTime and Date Stamp (day, mnth, yr, hr, min, s):\t13/08/2021 14:10:17\r\nChip ID:\tW509_L5, - , - , - \r\nChip Type (Medipix 3.0, Medipix 3.1, Medipix 3RX):\tMedipix 3RX\r\nAssembly Size (NX1, 2X2):\t   1x1\r\nChip Mode  (SPM, CSM, CM, CSCM):\tSPM\r\nCounter Depth (number):\t1\r\nGain:\tSLGM\r\nActive Counters:\tAlternating\r\nThresholds (keV):\t0.000000E+0,5.110000E+2,0.000000E+0,0.000000E+0,0.000000E+0,0.000000E+0,0.000000E+0,0.000000E+0\r\nDACs:\t000,511,000,000,000,000,000,000,100,255,125,125,100,100,082,100,086,030,128,004,255,074,128,123,109,511,511; ; ; \r\nbpc File:\tc:\\Merlin_Quad_Config\\W509_L5\\W509_L5_SPM.bpc,,,\r\nDAC File:\tc:\\Merlin_Quad_Config\\W509_L5\\W509_L5_SPM.dacs,,,\r\nGap Fill Mode:\tNone\r\nFlat Field File:\tNone\r\nDead Time File:\tDummy (C:\\<NUL>\\)\r\nAcquisition Type (Normal, Th_scan, Config):\tNormal\r\nFrames in Acquisition (Number):\t1\r\nFrames per Trigger (Number):\t1\r\nTrigger Start (Positive, Negative, Internal):\tInternal\r\nTrigger Stop (Positive, Negative, Internal):\tInternal\r\nSensor Bias (V):\t120 V\r\nSensor Polarity (Positive, Negative):\tPositive\r\nTemperature (C):\tBoard Temp 0.000000 Deg C\r\nHumidity (%):\tBoard Humidity 0.000000 \r\nMedipix Clock (MHz):\t120MHz\r\nReadout System:\tMerlin Quad\r\nSoftware Version:\t0.77.0.16\r\nEnd\t'
        tcp_str = 'MPX,'+str(len(acq_str)).zfill(10)+','
        acq = bytes(tcp_str+acq_str, 'utf-8')
        conn.send(acq)
        while True:
            im = np.random.randint(0, 256, size=(256, 256),
                                   dtype=type_dict[im_type])
            ny, nx = im.shape
            tcp_str = 'MPX,0000001024,'
            head_str = 'MQ1,000001,00384,01,' + \
                str(ny).zfill(4) + ',' + str(nx).zfill(4) + \
                ',' + im_type + ','
            head_str = tcp_str + head_str.ljust(384, '0')
            head = bytes(head_str, 'utf-8')
            payload = im.tobytes(order='C')
            try:
                conn.send(head + payload)
            except (ConnectionResetError, BrokenPipeError):
                print('Connection closed')
                break
   
