#!/bin/bash

# Este script imprime no console a lista de sinks de áudio detectados,
# separados por vírgula simples, cada elemento da list contém o número
# do sink e o código do barrramento onde ele está conectado, separados
# pelo símbolo @ (arroba).

# Dependências: pactl, sed, cut, grep

# O idioma do terminal deve ser inglês, para a palavra "Sink" não ser
# traduzida na saída do comando 'pactl'.
export LANG="en_US.UTF-8"

# Gera uma string com o número de cada sink separados por 1 espaço
sinks=$(pactl list sinks | sed -n '/Sink #/p' | cut -d "#" -f2)

for sink in $sinks; do
    hub="${sink}@$(pactl list sinks 2>/dev/null | sed -n "/Sink #${sink}/,/Sink #/p" | grep "device.bus_path" | sed 's/.*pci-//' | sed 's/"//' | sed 's/usb-[0-9]://' | cut -c1-14),"
    hubs="${hubs}${hub}"
done

hubs=$(echo $hubs | sed 's/,$//')
echo -n "$hubs"



# aplay -l | grep "Generic USB Audio Device" | awk '{ print $6 }'
# aplay -l | grep "Generic USB Audio Device" | awk '{ print $9 }'

#  dbus-send --session --print-reply --reply-timeout=2000 --type=method_call --dest=org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus.ListNames

#  pactl list sinks 2>/dev/null | grep -A50 "Sink #0" | grep "device.bus_path"


# /dev/input/by-path/ "porta do meio"
#  HUB1 = pci-0000:00:04.1-usb-0:3.1:1.0-event-mouse
#  HUB2 = pci-0000:00:04.1-usb-0:4.1:1.0-event-mouse

# /dev/input/by-path/ "porta da ponta"
#  HUB1 = pci-0000:00:04.1-usb-0:3.2:1.0-event-mouse
#  HUB2 = pci-0000:00:04.1-usb-0:4.2:1.0-event-mouse


# #Bloco entre as strings informadas:
# pactl list sinks 2>/dev/null | sed -n '/Sink #0/,/Sink #1/p' | grep "device.bus_path" | sed 's/.*pci-//' | sed 's/"//'

# python:
# usb-0000:00:04.1-4.2/input0

# shell:
# 0000:00:04.1-usb-0:3.3:1.0
# 0000:00:04.1-usb-0:4.3:1.0

# 0000:00:04.1-3.3:1.0
# 0000:00:04.1-4.3:1.0

# 0000:00:04.1-4


# HUB:
# pactl list sinks 2>/dev/null | sed -n '/Sink #1/,/Sink #2/p' | grep "device.bus_path" | sed 's/.*pci-//' | sed 's/"//' | sed 's/-usb-[0-9]:/-/' | cut -c1-14
