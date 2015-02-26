#!/bin/bash
#set -x


# Detecta vgas, criar xorgs.conf e chama o setup.py com o número de seats correto.


provas_config_file='/opt/provas/moodle_provas.conf'
[ -r "$provas_config_file" ] && source "$provas_config_file" || exit 1

functions_file="$provas_dir/multiseat/includes/functions.sh"
[ -r "$functions_file" ] && source "$functions_file" || exit 1



vgas="$(get_qtd_vgas)"

if [ "$vgas" -lt 2 ]; then
    echo '0'
else
    if [ -f '/etc/udev/rules.d/00-multiseat.rules' ]; then
        rm -f '/etc/udev/rules.d/00-multiseat.rules'
    fi

    prepare_pulseaudio
    make_xorg_config 1
    make_xorg_config 2
    vga_bus_id_1="$(get_vga_bus_id 1)"
    vga_bus_id_2="$(get_vga_bus_id 2)"

    echo "2 $vga_bus_id_1 $vga_bus_id_2"
fi

