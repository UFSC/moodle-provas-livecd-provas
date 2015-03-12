#!/bin/bash
#set -x

provas_config_file='/opt/provas/moodle_provas.conf'
[ -r "$provas_config_file" ] && source "$provas_config_file" || exit 1

functions_file="$provas_dir/includes/functions.sh"
[ -r "$functions_file" ] && source "$functions_file" || exit 1


log 'Verificando se o script foi executado com poder de root'
is_root

log 'Verificando se o computador é um multiterminal'
if is_multiseat; then
    log 'Iniciando em modo multiterminal.'
    start_multiseat_mode
else
    log 'Iniciando em modo normal.'
    start_normal_mode
fi

