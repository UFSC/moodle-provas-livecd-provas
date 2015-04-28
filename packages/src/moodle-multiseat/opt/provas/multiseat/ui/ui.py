#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
from sys import argv
from gi.repository import Gtk, GObject
import pickle
import zmq
import yaml
import logging
import logging.config

class Window():
    def __init__(self, seat, name):
        self.gladeFile = "ui.glade"
        self.gladeFilePath = os.path.join(os.path.dirname(__file__), self.gladeFile)
        self.name = name
        self.seat = 'Seat' + str(seat)
        self.windowTree = Gtk.Builder()
        self.windowTree.add_from_file(self.gladeFilePath)
        self.window = self.windowTree.get_object(name)
        self.screen = self.window.get_screen()
        self.width = self.screen.get_width()
        self.height = self.screen.get_height()
        self.window.set_size_request(self.width, self.height)
        self.window.fullscreen()
        self.windowTree.connect_signals(Handler())

    def show(self):
        log.debug("%s - Showing %s", self.seat, self.name)
        self.window.show_all()

    def hide(self):
        log.debug("%s - Hiding %s", self.seat, self.name)
        self.window.hide()

    def updateMessage(self, message):
        log.debug("%s - Updating message %s", self.seat, self.name)
        self.message.set_text(message)

    def updateInfoLabels(self, seatId, vgaBusId, assignedKeyboards, assignedMouses):
        self.seatId.set_text(seatId)
        self.vgaBusId.set_text(vgaBusId)
        self.assignedKeyboards.set_text(assignedKeyboards)
        self.assignedMouses.set_text(assignedMouses)

    def updateInfoGeneral(self, seatId, qtdSeats, vgaBusId):
        log.debug("%d - Updating info general at %s", self.seat, self.name)
        strSeats = str(seatId) + " de " + str(qtdSeats)
        self.seatIdValue.set_text(strSeats)
        self.vgaBusIdValue.set_text(vgaBusId)

    def updateInfoInput(self, qtdAssignedKeyboards, qtdAvailableKeyboards, qtdAssignedMouses, qtdAvailableMouses):
        log.debug("%d - Updating info input at %s", self.seat, self.name)
        strKeyboards = str(qtdAssignedKeyboards) + " de " + str(qtdAvailableKeyboards)
        strMouses = str(qtdAssignedMouses) + " de " + str(qtdAvailableMouses)
        self.assignedKeyboardsValue.set_text(strKeyboards)
        self.assignedMousesValue.set_text(strMouses)
    
    def updateImage(self, imagePath):
        log.debug("%s - Updating image at %s", self.seat, self.name)
        self.image.set_from_file(imagePath)
    

class WindowPressKey(Window):
    def __init__(self, seat):
        Window.__init__(self, seat, "WindowPressKey")
        self.message = self.windowTree.get_object("message_WPressKey")
        self.image = self.windowTree.get_object("image_WPressKey")
        self.seatId = self.windowTree.get_object("seatId_WPressKey")
        self.seatIdValue = self.windowTree.get_object("seatIdValue_WPressKey")
        self.vgaBusId = self.windowTree.get_object("vgaBusId_WPressKey")
        self.vgaBusIdValue = self.windowTree.get_object("vgaBusIdValue_WPressKey")
        self.assignedKeyboards = self.windowTree.get_object("assignedKbds_WPressKey")
        self.assignedKeyboardsValue = self.windowTree.get_object("assignedKbdsValue_WPressKey")
        self.assignedMouses = self.windowTree.get_object("assignedMouses_WPressKey")
        self.assignedMousesValue = self.windowTree.get_object("assignedMousesValue_WPressKey")


class WindowMouse(Window):
    def __init__(self, seat):
        Window.__init__(self, seat, "WindowMouse")
        self.message = self.windowTree.get_object("message_WMouse")
        self.image = self.windowTree.get_object("image_WMouse")
        self.seatId = self.windowTree.get_object("seatId_WMouse")
        self.seatIdValue = self.windowTree.get_object("seatIdValue_WMouse")
        self.vgaBusId = self.windowTree.get_object("vgaBusId_WMouse")
        self.vgaBusIdValue = self.windowTree.get_object("vgaBusIdValue_WMouse")
        self.assignedKeyboards = self.windowTree.get_object("assignedKbds_WMouse")
        self.assignedKeyboardsValue = self.windowTree.get_object("assignedKbdsValue_WMouse")
        self.assignedMouses = self.windowTree.get_object("assignedMouses_WMouse")
        self.assignedMousesValue = self.windowTree.get_object("assignedMousesValue_WMouse")


class WindowWait(Window):
    def __init__(self, seat):
        Window.__init__(self, seat, "WindowWait")
        self.message = self.windowTree.get_object("message_WWait")


class WindowError(Window):
    def __init__(self, seat):
        Window.__init__(self, seat, "WindowError")
        self.message = self.windowTree.get_object("message_WError")
        
        
class Handler():
    def onWindowPressKeyDestroy(self, *args):
        log.debug("%d - %s closed", self.seat, self.name)
        Gtk.main_quit(*args)

    def onWindowMouseDestroy(self, *args):
        log.debug("%d - %s closed", self.seat, self.name)
        Gtk.main_quit(*args)
                    
    def onWindowWaitDestroy(self, *args):
        log.debug("%d - %s closed", self.seat, self.name)
        Gtk.main_quit(*args)
    
    def onWindowErrorDestroy(self, *args):
        log.debug("%d - %s closed", self.seat, self.name)
        Gtk.main_quit(*args)


def callback():
    msg = ''
    try:
        msg = socket.recv(zmq.NOBLOCK)
    except zmq.ZMQError:
        return True

    cmd = pickle.loads(msg)

    callMethod(cmd[0], cmd[1], cmd[2])
    socket.send('OK')
    return True

def callMethod(window, method, args):
    log.debug("callMethod args: " + str(args))
    try:
        if args:
            getattr(windows[window], method)(*args)
        else:
            getattr(windows[window], method)()
    except KeyError:
        log.error("The object %s does not exist.", window)
    except Exception, e:
        log.error("Unknown exception on method callMethod()")
        log.exception(e)


def loadStrings(seat):
    file = open('/opt/provas/multiseat/ui/pt_BR.yaml')
    dataMap = yaml.load(file)
    file.close()

    seat = 'seat' + str(seat)

    windows['WindowPressKey'].updateMessage(dataMap['WindowPressKey'][seat]['message'])
    windows['WindowPressKey'].updateImage(dataMap['WindowPressKey'][seat]['image'])
    windows['WindowPressKey'].updateInfoLabels(
        dataMap['general']['seatId'],
        dataMap['general']['vgaBusId'],
        dataMap['general']['assignedKeyboards'],
        dataMap['general']['assignedMouses'])

    windows['WindowMouse'].updateMessage(dataMap['WindowMouse'][seat]['message'])
    windows['WindowMouse'].updateImage(dataMap['WindowMouse'][seat]['image'])
    windows['WindowMouse'].updateInfoLabels(
        dataMap['general']['seatId'],
        dataMap['general']['vgaBusId'],
        dataMap['general']['assignedKeyboards'],
        dataMap['general']['assignedMouses'])

    windows['WindowError'].updateMessage(dataMap['WindowError']['message'])

    windows['WindowWait'].updateMessage(dataMap['WindowWait']['message'])


def initLogging():
    logDir = "/var/log/multiseat"
    if not os.path.exists(logDir):
        os.makedirs(logDir)

    dataMap = yaml.load(open('/opt/provas/multiseat/logging.yaml', 'r'))
    logging.config.dictConfig(dataMap)
    return logging.getLogger('ui.py')


if __name__ == "__main__":
    log = initLogging()
    qtdParams = 2
    if len(argv) != qtdParams:
        log.error("Error: Invalid number of parameters, takes: %d, given: %d." % (qtdParams, len(argv)))
        sys.exit(1)

    seat = argv[1]

    log.info("Instantiating windows")
    windows = { 'WindowPressKey' : WindowPressKey(seat),
                'WindowMouse' : WindowMouse(seat),
                'WindowError' : WindowError(seat),
                'WindowWait' : WindowWait(seat)
    }

    log.info("Loading strings")
    loadStrings(seat)

    log.info("Starting pipe in /tmp/multiseat-pipe.%s", seat)
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    socket.bind("ipc:///tmp/multiseat-pipe." + str(seat))

    log.info("Starting WindowWait")
    windows['WindowWait'].show()
    GObject.timeout_add(50, callback)
    log.info("Starting Gtk.main loop")
    Gtk.main()
