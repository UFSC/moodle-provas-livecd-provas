#!/usr/bin/python
# -*- coding: utf-8 -*-

import subprocess
from evdev import InputDevice, list_devices


# Retorna a quantidade de placas de vídeos instaladas
def getVgaCount():
    vgas = subprocess.Popen('/usr/bin/lspci | grep \'VGA\|Display\' | wc -l', shell=True, stdout=subprocess.PIPE).communicate()[0]

    return vgas


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


if __name__ == "__main__":
    seats = 2
    vgas = getVgaCount()
    keyboards = getKeyboards()
    mouses = getMouses()

    keyboardsAvailable = len(keyboards)
    mousesAvailable = len(mouses)

    if keyboardsAvailable < seats or mousesAvailable < seats or vgas < seats:
        print(1)
    else:
        print(2)
