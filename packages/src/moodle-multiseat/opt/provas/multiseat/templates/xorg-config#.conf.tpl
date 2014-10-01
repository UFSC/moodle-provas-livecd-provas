Section "ServerLayout"
    Identifier     "seat-config__SEAT_ID__"
    Screen         "Screen__SEAT_ID__" 0 0
    Option "Clone" "off"
    Option "DisableModInDev" "true"
    Option "SingleCard" "on"

    # Disable screen-saver:
    Option "BlankTime"      "0"
    Option "StandbyTime"    "0"
    Option "SuspendTime"    "0"
    Option "OffTime"        "0"
EndSection

Section "ServerFlags"
    Option "DRI2"           "on"
    Option "Xinerama"       "off"
    Option "AutoAddGPU"     "false"
    Option "NoPM"           "true"
    Option "IgnoreABI"      "true"
EndSection

Section "Monitor"
    Identifier   "Monitor__SEAT_ID__"
    Option "DPMS" "false"
EndSection

Section "Device"
    Identifier  "Card__SEAT_ID__"
#    Driver      "nouveau"
    BusID       "__BUSID__"
    Screen      0
    Option "NoLogo" "true"
    Option "ProbeAllGpus" "false"
EndSection

Section "Screen"
    Identifier "Screen__SEAT_ID__"
    Device     "Card__SEAT_ID__"
    Monitor    "Monitor__SEAT_ID__"
    DefaultDepth 24
    SubSection "Display"
        Viewport   0 0
        Depth     24
    EndSubSection
EndSection
