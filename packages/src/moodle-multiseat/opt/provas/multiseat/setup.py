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

# Códigos usados como retorno de algumas funções
MOUSE = 1
KEYBOARD = 2


# device deve estar no formato "/dev/input/event#"
def isMouseOrKeyboard(device):
    dev = InputDevice(device)

    result1 = str(dev.capabilities(verbose=True)).find('KEY_F1')
    result2 = str(dev.capabilities(verbose=True)).find('BTN_MOUSE')

    if result1 != -1:
        log.debug("isMouseOrKeyboard() - " + device + " is a keyboard")
        return KEYBOARD
    elif result2 != -1:
        log.debug("isMouseOrKeyboard() - " + device + " is a mouse")
        return MOUSE
    else:
        log.debug("isMouseOrKeyboard() - " + device + " is not a mouse neither a keyboard")
        return -1
    

# Retorna uma lista com todos mouses detectados, cada mouse é representado por um caminho "/dev/input/event#"
def getMouses():
    mouseList = []

    for device in list_devices():
        dev = InputDevice(device)
        result = str(dev.capabilities(verbose=True)).find('BTN_MOUSE')
        if result != -1:
            #print(device + " é um Mouse")
            mouseList.append(device)

    return mouseList


# Retorna uma lista com todos teclados detectados, cada teclado é representado por um caminho "/dev/input/event#"
def getKeyboards():
    keyboard_list = []

    for device in list_devices():
        dev = InputDevice(device)
        result = str(dev.capabilities(verbose=True)).find('KEY_F1')
        if result != -1:
            #print(device + " é um Teclado")
            keyboard_list.append(device)

    return keyboard_list
    

# mode pode ser "w" (write) para gravar e sobreescrever o arquivo ou "a" (append) para adicionar no fim do arquivo
def writeAndReloadUdevRules(inputSeat):
    mode = "w"
    seat = inputSeat['seat']
    mouse = inputSeat['mouse']
    keyboard = inputSeat['keyboard']
    mouseEvent = mouse[11:]
    keyboardEvent = keyboard[11:]
    
    udevMouse = "KERNEL==\"" + mouseEvent + "\", SUBSYSTEM==\"input\", SYMLINK+=\"multiseat/" + seat + "/mouse1\"\n"
    udevTeclado = "KERNEL==\"" + keyboardEvent + "\", SUBSYSTEM==\"input\", SYMLINK+=\"multiseat/" + seat + "/keyboard1\"\n"

    multiseatUdevRulesFile = "/etc/udev/rules.d/00-multiseat.rules"

    try:
        if os.path.exists(multiseatUdevRulesFile):
            mode = "a"
        file = open(multiseatUdevRulesFile, mode)
        file.write(udevMouse)
        file.write(udevTeclado)
        file.close()
    except:
        log.info("Erro ao gravar as regras do udev.")

    try:
        os.system("udevadm control --reload-rules")
        os.system("udevadm trigger")
    except:
        log.info("Erro ao recarregar as regras do udev.")
  
 
def getAudioSinkSeat(inputSeat):
    try:
        p = Popen('/opt/provas/multiseat/scripts/get_audio_devices.sh', shell=True, stdout=subprocess.PIPE)

        output = p.communicate()[0]
        sinks = output.split(',')
        hubs = {}
    
        for s in sinks:
            sink, path = s.split('@')
            hubs[sink] = path
    
        # Tenta primeiro verificando se o teclado está no mesmo hub usb...
        dev = str(InputDevice(inputSeat['keyboard']))
    
        for hub in hubs:
            if not (dev.find(hubs[hub]) == -1):
                return hub
            
        # Se o teclado não estava no hub usb, testa se o mouse está...
        dev = str(InputDevice(inputSeat['mouse']))
        
        for hub in hubs:
            if not (dev.find(hubs[hub]) == -1):
                return hub
    except:
        log.info("Erro ao executar getAudioSinkSeat()")
        
    # Se nem o teclado, nem o mouse estiverem no hub, retorna -1
    return -1

    
def setAudioDevice(inputSeat):
    sink = getAudioSinkSeat(inputSeat)
    seat = inputSeat['seat']
    log.debug("Current sink: %s", sink)
    username = username_base + seat[4:]
    
    if not sink == -1:
        log.info("Writing audio sink%s config to /home/%s%s/.pulse/client.conf", sink, username_base, seat[4:])
        
        pulseaudioDir = "/home/" + username + "/.pulse"
        if not os.path.exists(pulseaudioDir):
            os.makedirs(pulseaudioDir)

        file = open(pulseaudioDir + "/client.conf", 'w')
        file.write("default-sink=" + str(sink) + "\n")
        file.write("autospawn=no\n")
        file.close()

        uid = pwd.getpwnam(username).pw_uid
        gid = grp.getgrnam(username).gr_gid
        os.chown(pulseaudioDir, uid, gid)
        os.chown(pulseaudioDir + "/client.conf", uid, gid)
        os.chmod(pulseaudioDir + "/client.conf", 444)


class WaitMouseClick(Thread):
    def __init__(self, device, event, inputSeat):
        Thread.__init__(self)
        self.device = device
        self.running = True
        self.event = event
        self.inputSeat = inputSeat

    def run(self):
        if isMouseOrKeyboard(self.device) != MOUSE:
            return -1
        
        dev = InputDevice(self.device)
        log.debug("Thread running for device = %s", self.device)
        while self.running:
            r, w, x = select([dev], [], [])
            for event in dev.read():
                # Se o código da tecla desejada e o estado (event.value) for 'key_up' (key_up=0, key_down=1, key_hold=2)
                # self.key deve ser o código de uma tecla, por exemplo: ecodes.BTN_MOUSE
                if (event.code == self.event) & (event.value == 0):
                    log.info("Mouse button pressed, device = %s", self.device)
                    self.inputSeat['mouse'] = self.device
                    return
                
    def stop(self):
        self.running = False
        log.debug("%s leaving", self.name)
            

class WaitKeyboardKey(Thread):
    def __init__(self, device, key, inputSeat):
        Thread.__init__(self)
        self.device = device
        self.running = True
        self.key = key
        self.inputSeat = inputSeat
        
    def run(self):
        if isMouseOrKeyboard(self.device) != KEYBOARD:
            return -1
        
        dev = InputDevice(self.device)
        log.debug("Thread running for device = %s", self.device)
        while self.running:
            r, w, x = select([dev], [], [])
            for event in dev.read():
                # Se o código da tecla desejada E o estado (event.value) for 'key_up'
                # (key_up=0, key_down=1, key_hold=2)
                # self.key deve ser o código de uma tecla, por exemplo: ecodes.KEY_F1
                if (event.code == self.key) & (event.value == 0):
                    log.info("Keyboard key pressed, device = %s", self.device)
                    self.inputSeat['keyboard'] = self.device
                    return
    
    def stop(self):
        self.running = False
        log.debug("%s leaving", self.name)


def initializeConfigSeat(seat):
    display = ':' + str(20 + seat)
    displayConfigSeat = {"DISPLAY" : display}

    log.info("Starting Xorg of seat%s with DISPLAY=%s", seat, display)

    xorgConfigCmd = 'X ' + display + ' -config xorg-config' + str(seat) + '.conf -br -audit 0 -novtswitch -noreset -dpms -tst -ignoreABI'
    if seat != 1:
        xorgConfigCmd = xorgConfigCmd + ' -sharevts'

    log.debug(xorgConfigCmd)
    try:
        xorgConfig = Popen(xorgConfigCmd.split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    except:
        log.info("Erro ao executar o xorgConfig %s", seat)

    sleep(1)

    log.info("Starting ui.py of seat%s at display %s", seat, display)
    uiSeatCmd = '/usr/bin/env python /opt/provas/multiseat/ui/ui.py ' + str(seat)
    
    try:
        uiSeat = Popen(uiSeatCmd.split(), env=displayConfigSeat, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    except:
        log.info("Erro ao executar a ui no seat %s", seat)

    sleep(1)

    log.info("Connecting to ui of seat%s at /tmp/multiseat-pipe.%s", seat, seat)
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    socket.connect("ipc:///tmp/multiseat-pipe." + str(seat))

    seat = 'seat' + str(seat)
    ui[seat] = {'xorg': xorgConfig, 'ui': uiSeat, 'socket': socket}


def callUiMethod(seat, window, method, params=None):
    cmd = [window, method, params]
    string = pickle.dumps(cmd)
    ui[seat]['socket'].send(string)

    # Get the reply.
    reply = ui[seat]['socket'].recv()
    log.debug("%s received reply: %s", seat, reply)


def initLogging():
    dataMap = yaml.load(open('/opt/provas/multiseat/logging.yaml', 'r'))
    logging.config.dictConfig(dataMap)
    return logging.getLogger('setup.py')


if __name__ == "__main__":
    log = initLogging()
    log.info("================ Multiseat wizard started ================ ")

    qtdParams = 3
    if len(sys.argv) != qtdParams:
        log.error("Error: Invalid number of parameters, takes: %d, given: %d." % (qtdParams, len(sys.argv)))
        sys.exit(1)

    args = sys.argv[1].split()
    seats = int(args[0])

    if seats != 2:
        log.error("Error: The number of seats must be equal to 2.")
        exit(1)

    vgaBusIds = [args[1], args[2]]
    username_base = sys.argv[2]

    ui = {}

    keyboards = getKeyboards()
    mouses = getMouses()

    keyboardsAvailable = len(keyboards)
    mousesAvailable = len(mouses)

    log.info("Seats : %d", seats)

    for seat in range(1, seats + 1):
        log.info('Initializing seat%s', seat)
        initializeConfigSeat(seat)

    for seat in range(1, seats + 1):
        seatStr = 'seat' + str(seat)

        inputSeat = {'seat': seatStr, 'keyboard': '-1', 'mouse': '-1'}

        threadsKeyboardsSeat = []
        threadsMouses = []

        log.info("%s - Starting threads waiters for keyboards", seatStr)
        for keyboard in keyboards:
            log.info(keyboard)
            threadsKeyboardsSeat.append(WaitKeyboardKey(keyboard, ecodes.KEY_F1, inputSeat))

        for thread in threadsKeyboardsSeat:
            thread.start()

        callUiMethod(seatStr, 'WindowWait', 'hide')
        callUiMethod(seatStr, 'WindowPressKey', 'updateInfoGeneral', [seat, seats, vgaBusIds[seat - 1]])
        callUiMethod(seatStr, 'WindowPressKey', 'updateInfoInput', [keyboardsAvailable - len(keyboards),
                                                                           keyboardsAvailable,
                                                                           mousesAvailable - len(mouses),
                                                                           mousesAvailable])
        callUiMethod(seatStr, 'WindowPressKey', 'show')

        while True:
            if inputSeat['keyboard'] != '-1':
                log.info("Keyboard assigned to %s: %s", seatStr, inputSeat['keyboard'])
                keyboards.remove(inputSeat['keyboard'])
                break

        for mouse in mouses:
            print(mouse)
            threadsMouses.append(WaitMouseClick(mouse, ecodes.BTN_MOUSE, inputSeat))

        for thread in threadsMouses:
            thread.start()

        callUiMethod(seatStr, 'WindowPressKey', 'hide')
        callUiMethod(seatStr, 'WindowMouse', 'updateInfoGeneral', [seat, seats, vgaBusIds[seat - 1]])
        callUiMethod(seatStr, 'WindowMouse', 'updateInfoInput', [keyboardsAvailable - len(keyboards),
                                                                        keyboardsAvailable,
                                                                        mousesAvailable - len(mouses),
                                                                        mousesAvailable])
        callUiMethod(seatStr, 'WindowMouse', 'show')

        while True:
            if inputSeat['mouse'] != '-1':
                log.info("Mouse assigned to %s: %s", seatStr, inputSeat['mouse'])
                mouses.remove(inputSeat['mouse'])
                break

        writeAndReloadUdevRules(inputSeat)
        setAudioDevice(inputSeat)

        for thread in threadsKeyboardsSeat:
            thread.stop()

        for thread in threadsMouses:
            thread.stop()

        ui[seatStr]['ui'].terminate()
        os.wait()
        ui[seatStr]['xorg'].terminate()
        os.wait()

        sleep(1)

        # O Xorg de cada usuário deve ser iniciado em background, o primeiro deles não deve ter a flag -sharevts pois será o principal.
        log.info("Running startx for seat %s", seat)

        seatCmd = "su - " + username_base + str(seat) + " -c 'startx -- :" + str(seat) + " -config xorg-seat" + str(seat) + ".conf -br -audit 0 -dpms -tst -novtswitch -nolisten tcp -ignoreABI' &"
        if seat != 1:
            seatCmd = seatCmd.replace('\' &', ' -sharevts\' &')

        log.debug(seatCmd)
        os.system(seatCmd)

    log.info("================ Multiseat wizard ended ================ ")
