#!/bin/bash
#set -x

provas_config_file='/opt/provas/moodle_provas.conf'
[ -r "$provas_config_file" ] && source "$provas_config_file" || exit 1
[ -r "$provas_online_config_file" ] && source "$provas_online_config_file" || exit 1

functions_file="$provas_dir/includes/functions.sh"
[ -r "$functions_file" ] && source "$functions_file" || exit 1


if is_internet_online; then
    url="${moodle_provas_url}/webservice/rest/server.php"
    webservice="local_exam_authorization_receive_file"

    param1="wstoken=$moodle_webservices_token"
    param2="moodlewsrestformat=json"
    param3="wsfunction=$webservice"

    data1="exam_client_ip=$livecd_local_ip"
    data2="exam_client_network=$livecd_local_network"

    log "Enviando os seguintes dados para o webservice '$webservice':"
    log "data1 = '$data1'"
    log "data2 = '$data2'"

    # Envia os dados via POST, o --insecure (-k) é necessário se o certificado for auto-assinado
    reply=$(curl -F "$param1" -F "$param2" -F "$param3" -F "$data1" -F "$data2" "$url")

    log "Resposta do servidor:"
    log "$reply"
fi
