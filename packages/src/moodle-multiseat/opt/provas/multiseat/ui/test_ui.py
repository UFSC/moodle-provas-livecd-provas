#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import pickle
import zmq


def callUiMethod(seat, window, method, params=None):
    cmd = [window, method, params]
    string = pickle.dumps(cmd)
    socket.send(string)

    # Get the reply.
    reply = socket.recv()
    print("%s received reply: %s", seat, reply)


if __name__ == "__main__":
    qtdParams = 4
    if len(sys.argv) != qtdParams:
        print("Error: Invalid number of parameters, takes: %d, given: %d." % (qtdParams, len(sys.argv)))
        sys.exit(1)

    seat = sys.argv[1]
    window = sys.argv[2]
    method = sys.argv[3]
    #params = sys.argv[4]

    print("Connecting to ui of seat{0} at /tmp/multiseat-pipe.{0}".format(seat))
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    socket.connect("ipc:///tmp/multiseat-pipe." + str(seat))

    callUiMethod(seat, window, method)
