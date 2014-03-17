#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import pwd
import grp
from threading import Thread
from evdev import InputDevice, ecodes, list_devices
from select import select
import subprocess
from subprocess import Popen
from time import sleep
import pickle
import zmq
import yaml
import logging
import logging.config


def initializeUi(seat, display):
    displayStr = {"DISPLAY" : ':' + str(display)}

    uiSeatCmd = '/usr/bin/env python ui.py ' + str(seat)
    
    try:
        uiSeat = Popen(uiSeatCmd.split(), env=displayStr, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    except:
        print("Erro ao executar a ui no seat %s", seat)


def callUiMethod(seat, window, method, params=None):
    cmd = [window, method, params]
    string = pickle.dumps(cmd)
    ui[seat]['socket'].send(string)

    # Get the reply.
    reply = ui[seat]['socket'].recv()
    log.debug("%s received reply: %s", seat, reply)


if __name__ == "__main__":
    qtdParams = 3
    if len(sys.argv) != qtdParams:
        print("Error: Invalid number of parameters, takes: %d, given: %d." % (qtdParams, len(sys.argv)))
        sys.exit(1)

    seat = sys.argv[1]
    display = sys.argv[2]

    initializeUi(seat, display)

    sleep(10000)
