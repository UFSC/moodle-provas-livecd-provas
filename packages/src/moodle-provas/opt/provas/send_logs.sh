#!/bin/bash
#set -x

provas_config_file='/opt/provas/moodle_provas.conf'
[ -r "$provas_config_file" ] && source "$provas_config_file" || exit 1
[ -r "$provas_online_config_file" ] && source "$provas_online_config_file" || exit 1

functions_file="$provas_dir/includes/functions.sh"
[ -r "$functions_file" ] && source "$functions_file" || exit 1


# Redefine o arquivo de log para este script, pois ele não pode gravar no /var/log.
log_file_provas="$HOME/livecd_diagnostic.log"

# Só prossegue se os parâmetros do diagnostic_server estão definidos.
if [ "$diagnostic_server_enabled" = "no" ]; then
    log "Os parâmetros do diagnostic_server não foram definidos, esta opção está desativada."
    exit 0
fi

# Só prossegue se o envio de logs estiver ativado.
if [ "$diag_allow_send_logs" = "no" ]; then
    log "O envio de logs está desativado."
    exit 0
fi

# Diretório temporário onde os logs serão salvos
base_dir="/tmp"
tmp_dir="$(date +%Y%m%d-%Hh%M)"
work_dir="$base_dir/$tmp_dir"

logs_dir="$work_dir/logs"
cmds_dir="$work_dir/cmds"
filename="logs.tar.gz"

if [ -d "$work_dir" ]; then
    rm -rf "$work_dir"
fi
mkdir -p "$logs_dir" "$cmds_dir"

log "Copiando alguns arquivos do sistema para $work_dir..."
for file in $diag_system_files_to_copy; do
    cp -R "$file" "$logs_dir/"
done

log 'Gravando a saída de alguns comandos...'
ck-list-sessions >"$cmds_dir/ck-list-sessions.log" 2>&1
date >"$cmds_dir/date.log" 2>&1
ip a >"$cmds_dir/ip-a.log" 2>&1
lspci >"$cmds_dir/lspci.log" 2>&1
lspci -vvv >"$cmds_dir/lspci-vvv.log" 2>&1
lspci -nn >"$cmds_dir/lspci-nn.log" 2>&1
lsusb -t >"$cmds_dir/lsusb-t.log" 2>&1
lsusb -v >"$cmds_dir/lsusb-v.log" 2>&1
ps aux >"$cmds_dir/ps-aux.log" 2>&1
uname -a  >"$cmds_dir/uname-a.log" 2>&1

log 'Comprimindo os arquivos...'
tar czvf "$base_dir/$filename" -C "$base_dir" "$tmp_dir" || exit 1

# Se o usuário NÃO autorizar o envio dos logs de diagnóstico, aborta o script...
if ! user_input=$(should_send_logs); then
    log "O envio dos logs de diagnóstico foi cancelado pelo usuário."
    log '-------------------------------------------------------------'
    exit 0
fi

source $provas_dir/send_file.sh "/tmp/$filename" "$user_input"

