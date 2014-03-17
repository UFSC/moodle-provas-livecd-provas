Section "ServerLayout"
    Identifier     "seat__SEAT_ID__"
    Screen         "Screen__SEAT_ID__" 0 0
    InputDevice    "Mouse__SEAT_ID__" "CorePointer"
    InputDevice    "Keyboard__SEAT_ID__" "CoreKeyboard"
    Option "Clone" "off"
    Option "AutoAddDevices"     "off"
    Option "DisableModInDev"    "true"
    Option "SingleCard"         "on"

    # Disable screen-saver:
    Option "BlankTime"      "0" 
    Option "StandbyTime"    "0" 
    Option "SuspendTime"    "0" 
    Option "OffTime"        "0"
EndSection

Section "ServerFlags"
    Option "AllowMouseOpenFail" "true"
    Option "AutoAddDevices"     "false"
    Option "AutoEnableDevices"  "false"
    Option "DRI2"               "on"
    Option "Xinerama"           "off"
    Option "AutoAddGPU"         "false"
    Option "DontVTSwitch"       "true"
    Option "DontZap"            "true"
    Option "VTSysReq"           "true"
    Option "NoPM"               "true"
    Option "IgnoreABI"          "true"
EndSection

Section "InputDevice"
    Identifier  "Keyboard__SEAT_ID__"
    Driver      "evdev"
    Option      "Device" "/dev/multiseat/seat__SEAT_ID__/keyboard1"
    Option      "XkbRules" "xorg"
    Option      "XkbModel" "evdev"
    Option      "XkbLayout" "br"
    Option      "GrabDevice" "yes"
EndSection

Section "InputDevice"
    Identifier  "Mouse__SEAT_ID__"
    Driver      "evdev"
    Option      "Protocol" "auto"
    Option      "Device" "/dev/multiseat/seat__SEAT_ID__/mouse1"
    Option      "ZAxisMapping" "4 5 6 7"
    Option      "GrabDevice" "yes"
EndSection

Section "Monitor"
    Identifier   "Monitor__SEAT_ID__"
    Option "DPMS" "off"
EndSection

Section "Device"
    Identifier  "Card__SEAT_ID__"
    BusID       "__BUSID__"
    Screen      0
    Option "NoLogo" "true"
    Option "ProbeAllGpus" "false"
EndSection

Section "Screen"
    Identifier "Screen__SEAT_ID__"
    Device     "Card__SEAT_ID__"
    Monitor    "Monitor__SEAT_ID__"
    SubSection "Display"
        Viewport   0 0
        Depth     24
    EndSubSection
EndSection
