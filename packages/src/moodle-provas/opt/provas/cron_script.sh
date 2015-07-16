#!/bin/bash
#set -x

provas_config_file='/opt/provas/moodle_provas.conf'
[ -r "$provas_config_file" ] && source "$provas_config_file" || exit 1
[ -r "$provas_online_config_file" ] && source "$provas_online_config_file" || exit 1

functions_file="$provas_dir/includes/functions.sh"
[ -r "$functions_file" ] && source "$functions_file" || exit 1


if is_internet_online; then
    url="${moodle_provas_url}${moodle_provas_receive_data_path}"

    # Envia os dados via POST, o --insecure é necessário se o certificado for auto-assinado
    log "Enviando os dados via POST..."
    log "http_header1 = '$http_header1'"
    log "http_header2 = '$http_header2'"
    log "http_header3 = '$http_header3'"
    curl -k -H "$http_header1" -H "$http_header2" -H "$http_header3" "$url"
fi
