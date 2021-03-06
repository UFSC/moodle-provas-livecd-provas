#!/bin/bash
#set -x

provas_config_file='/opt/provas/moodle_provas.conf'
[ -r "$provas_config_file" ] && source "$provas_config_file" || exit 1
[ -r "$provas_online_config_file" ] && source "$provas_online_config_file" || exit 1

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
    cp -R "$file" "$files_dir/"
done

log 'Gravando a saída de alguns comandos...'
ck-list-sessions >"$files_dir/cmd_ck-list-sessions.log" 2>&1
ip a >"$files_dir/cmd_ip-a.log" 2>&1
lspci >"$files_dir/cmd_lspci.log" 2>&1
lspci -vvv >"$files_dir/cmd_lspci-vvv.log" 2>&1
lspci -nn >"$files_dir/cmd_lspci-nn.log" 2>&1
lsusb -t >"$files_dir/cmd_lsusb-t.log" 2>&1
lsusb -v >"$files_dir/cmd_lsusb-v.log" 2>&1
ps aux >"$files_dir/cmd_ps-aux.log" 2>&1
uname -a  >"$files_dir/cmd_uname-a.log" 2>&1

log 'Comprimindo os arquivos...'
tar czvf "/tmp/$filename" "$files_dir" || exit 1

log "Enviando arquivo comprimido para $log_script_url ..."
curl --fail -k -F "token=$log_script_token" -F "file=@/tmp/$filename" "$log_script_url"

if [ $? -eq 0 ]; then
    echo '  - Arquivo enviado com sucesso.'
    status_code=0
else
    echo '  - Erro ao enviar o arquivo.'
    status_code=1
fi

log 'Removendo os arquivos temporários...'
rm -rf "$files_dir"
rm -rf "/tmp/$filename"

log 'Feito.'

exit $status_code

