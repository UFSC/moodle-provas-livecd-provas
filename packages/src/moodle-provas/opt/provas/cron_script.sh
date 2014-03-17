#!/bin/bash

provas_config='/opt/provas/moodle_provas.conf'
[ -r "$provas_config" ] && source "$provas_config" || exit 1

functions_file="$provas_dir/includes/functions.sh"
[ -r "$functions_file" ] && source "$functions_file" || exit 1


if is_internet_online; then
    header1="MOODLE-PROVAS-VERSION:$provas_version"
    header2="MOODLE-PROVAS-IP:$local_ip"
    header3="MOODLE-PROVAS-NETWORK:$local_network"

    # Envia os dados via POST, o --insecure é necessário se o certificado for auto-assinado
    log "Enviando os dados via POST..."
    log "header1 = '$header1'"
    log "header2 = '$header2'"
    log "header3 = '$header3'"
    curl --insecure --header "$header1" --header "$header2" --header "$header3" "$provas_receive_data_url"
fi
