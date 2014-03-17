#!/bin/bash

provas_config='/opt/provas/moodle_provas.conf'
[ -r "$provas_config" ] && source "$provas_config" || exit 1

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

