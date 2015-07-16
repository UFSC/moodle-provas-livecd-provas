#!/bin/bash
#set -x

provas_config_file='/opt/provas/moodle_provas.conf'
[ -r "$provas_config_file" ] && source "$provas_config_file" || exit 1
[ -r "$provas_online_config_file" ] && source "$provas_online_config_file" || exit 1

functions_file="$provas_dir/includes/functions.sh"
[ -r "$functions_file" ] && source "$functions_file" || exit 1


# Redefine o arquivo de log para este script, pois ele não pode gravar no /var/log.
log_file_provas="$HOME/livecd_screenshots.log"

# Se o usuário NÃO autorizar o envio da screenshot, aborta o script...
if ! user_input=$(should_send_screenshot); then
    log "O envio da screenshot foi cancelado pelo usuário."
    log '-------------------------------------------------------------'
    exit 0
fi

# Diretório temporário onde o arquivo será salvo
base_dir="/tmp"
tmp_dir="$(date +%Y%m%d-%Hh%M)"
work_dir="$base_dir/$tmp_dir"

if [ -d "$work_dir" ]; then
    rm -rf "$work_dir"
fi
mkdir "$work_dir"

log "Gerando uma foto da tela e salvando em $tmp_dir"
#import -window root $work_dir/screenshot-$(date +%Y%m%d-%Hh%Mm%Ss).png
#scrot "$work_dir/%Y%m%d-%Hh%Mm%Ss_screenshot-\$wx\$h.png" -e "cp -f \$f $HOME/Desktop/"
scrot "$work_dir/screenshot-\$wx\$h.png" -e "cp -f \$f $HOME/Desktop/"
filename="$(ls "$work_dir/"*".png")"

source $provas_dir/send_file.sh "$filename" "$user_input"

