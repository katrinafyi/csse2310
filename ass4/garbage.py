#!/usr/bin/env python3

import socket
import socketserver
import threading
import random
import time
import sys

ADJECTIVES = ('smelly', 'gross', 'old', 'dusty', 'mouldy', 'clean', 'fresh',
        'biohazardous', 'nuclear', 'radioactive')
NOUNS = ('garbage', 'rubbish', 'trash', 'junk', 'dirt', 'sand', 'water',
        'waste', 'refuse', 'leftovers', 'bread', 'litter', 'scrap', 'waste')
ACTIONS = ('Deliver', 'Withdraw')

IM = 'IM:{}:garbage.py\n'


def garbage_thread(f, my_port):
    f.write(IM.format(my_port).encode('utf-8'))
    f.flush()

    base = 1.0002
    i = 0
    while True:
        n = round(base ** i)
        s = '{}:{}:{}_{}\n'.format(random.choice(ACTIONS), n, 
                random.choice(ADJECTIVES), random.choice(NOUNS))
        f.write(s.encode('utf-8'))
        f.flush()
        time.sleep(1+0.9*random.random())
        i += 1

def start_garbage_thread(host, port, my_port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, int(port)))

    print('connecting garbage to', port)
    with s.makefile('wb') as f:
        garbage_thread(f, my_port)

class DepotHandler(socketserver.StreamRequestHandler):
    def handle(self):
        print('received connection from', self.client_address[1])
        garbage_thread(self.wfile, self.server.server_address[1])

def matchmaker_thread(host, port, my_port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))

    s.sendall(IM.format(my_port).encode('utf-8'))

    with s.makefile('r') as f:
        while True:
            msg = f.readline().strip()
            print('received from matchmaker:', repr(msg))
            if msg.startswith('Connect:'):
                new_port = msg.replace('Connect:', '')
                t = threading.Thread(target=start_garbage_thread,
                        args=(host, new_port, my_port))
                t.daemon = True
                t.start()

if __name__ == '__main__':
    HOST, PORT = 'localhost', int(sys.argv[1])

    with socketserver.ThreadingTCPServer((HOST,  0), DepotHandler) as s:
        print('server', s.server_address)
        t = threading.Thread(target=s.serve_forever)
        t.daemon = True
        t.start()

        matchmaker_thread(HOST, PORT, s.server_address[1])

