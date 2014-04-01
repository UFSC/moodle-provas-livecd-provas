#!/bin/bash

provas_config='/opt/provas/moodle_provas.conf'
[ -r "$provas_config" ] && source "$provas_config" || exit 1

functions_file="$provas_dir/includes/functions.sh"
[ -r "$functions_file" ] && source "$functions_file" || exit 1


# Diretório temporário onde os arquivos serão copiados
files_dir='/tmp/logs'
filename="logs-$(date +%Y%m%d-%Hh%M).tar.gz"


if [ -d "$files_dir" ]; then
    rm -rf "$files_dir"
fi
mkdir "$files_dir"

log "Copiando alguns arquivos do sistema para $files_dir..."
for file in $log_system_files; do
    cp "$file" "$files_dir/" >>"$log_file_provas" 2>&1
done

log 'Gravando a saída de alguns comandos...'
ck-list-sessions >"$files_dir/cmd_ck-list-sessions.log" 2>&1
ip a >"$files_dir/cmd_ip-a.log" 2>&1
lspci >"$files_dir/cmd_lspci.log" 2>&1
lspci -v >"$files_dir/cmd_lspci-v.log" 2>&1
lsusb -t >"$files_dir/cmd_lsusb-t.log" 2>&1
lsusb -v >"$files_dir/cmd_lsusb-v.log" 2>&1
ps aux >"$files_dir/cmd_ps-aux.log" 2>&1
uname -a  >"$files_dir/cmd_uname-a.log" 2>&1

log 'Comprimindo os arquivos...'
tar czvf "/tmp/$filename" "$files_dir" >>"$log_file_provas" 2>&1 || exit 1

log 'Enviando arquivo comprimido...'
curl -k -F "auth=$log_server_auth" -F "file=@/tmp/$filename" "$log_server_script" >>"$log_file_provas" 2>&1 || exit 1

log 'Removendo os arquivos temporários...'
rm -rf "$files_dir"
rm -rf "/tmp/$filename"

log 'Feito.'

