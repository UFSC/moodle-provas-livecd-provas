#!/bin/bash
#set -x

# OBS: Este script só pode ser carregado pelo comando 'source' em algum arquivo que tenha carregado
# o arquivo de configurações e o arquivo de funções do moodle provas.
if [ -z "$provas_dir" ]; then
    echo "ERRO: Este arquivo não pode ser executado diretamente."
    exit 1
fi

filename="$1"
user_input="$2"

email="${user_input%|*}"  # Extrai o e-mail da string
email="${email:-NO_EMAIL}"   # Define um valor padrão se nenhum email foi informado.
description="${user_input#*|}"  # Extrai o e-mail da string
description="${description:-NO_DESCRIPTION}"  # Define um valor padrão se nenhum descrição foi informada.

curl_output="/tmp/curl.log"
curl_err="/tmp/curl-err.log"
[ -f "$curl_output" ] && rm -f "$curl_output"
[ -f "$curl_err" ] && rm -f "$curl_err"

url="${moodle_provas_url}/webservice/rest/server.php"
webservice="local_exam_authorization_receive_file"

param1="wstoken=$moodle_webservices_token"
param2="moodlewsrestformat=json"
param3="wsfunction=$webservice"

data1="exam_client_livecd_version=$livecd_version"
data2="exam_client_livecd_build=$livecd_build"
data3="exam_client_ip=$livecd_local_ip"
data4="exam_client_network=$livecd_local_network"
data5="exam_client_user_email=$email"
data6="exam_client_user_description=$description"

log "E-mail informado pelo usuário: '$email'"
log "Descrição informada pelo usuário: '$description'"

log "Enviando o arquivo '$filename' para o webservice '$webservice' no endereço '$url'"
curl -m "$diag_upload_timeout" -o "$curl_output" -k \
    -F "$param1" -F "$param2" -F "$param3" \
    -F "$data1" -F "$data2" -F "$data3" -F "$data4" -F "$data5" -F "$data6" \
    -F "file=@$filename" "$url" 2>"$curl_err" | \
    stdbuf -oL tr '\r' '\n' | grep -o --line-buffered '[0-9]*\.[0-9]' | \
    zenity --progress --no-cancel --title="Enviando..." --text="Aguarde, os dados estão sendo enviados" --auto-close

result="$PIPESTATUS"

if [ "$result" -eq 0 ]; then
    output="$(tail -6 "$curl_output")"

    # Se for uma exceção do Moodle:
    if echo "$output" | "$provas_dir/bin/jq" '.exception' >/dev/null 2>&1; then
        upload_status=1
        upload_msg="Exceção no Moodle: $(echo "$output" | "$provas_dir/bin/jq" '.message')"
    else if echo "$output" | "$provas_dir/bin/jq" '.status' >/dev/null 2>&1; then
        upload_status=$(echo "$output" | "$provas_dir/bin/jq" '.status')
        upload_msg="$(echo "$output" | "$provas_dir/bin/jq" '.message')"
    else
        upload_status=$?
        upload_msg="$output"
    fi
else
    upload_status="$result"
    upload_msg="$(tail -6 "$curl_err")"
fi

if [ $upload_status -eq 0 ]; then
    log 'Arquivo enviado com sucesso.'
    show_send_file_success
else
    log 'Erro ao enviar o arquivo.'
    show_send_file_error "$upload_msg"
fi
log "Resposta do servidor: $upload_msg"

log 'Removendo os arquivos temporários'
rm -rf "$work_dir"
rm -rf "$filename"

log 'Feito.'
log '-------------------------------------------------------------'

exit $upload_status

