#!/bin/bash
#set -x

# OBS: Este script só pode ser executado pelo comando 'source' em algum arquivo que tenha carregado
# o arquivo de configurações e o arquivo de funções do moodle provas.

filename="$1"
user_input="$2"

email="${user_input%|*}"
description="${user_input#*|}"
http_header_email="LIVECD-USER-EMAIL:$email"
http_header_description="LIVECD-USER-DESCRIPTION:$description"

curl_output="/tmp/curl.log"
curl_err="/tmp/curl-err.log"
[ -f "$curl_output" ] && rm -f "$curl_output"
[ -f "$curl_err" ] && rm -f "$curl_err"

#url="${moodle_provas_url}${log_script_receive_file_path}"
url="https://wwwexe.inf.ufsc.br${log_script_receive_file_path}"

log "Enviando o arquivo '$filename' para o endereço '$url'"
curl -o "$curl_output" -k -H "$http_header1" -H "$http_header2" -H "$http_header3" \
    -H "$http_header_email" -H "$http_header_description" \
    -F "token=$log_script_token" -F "file=@$filename" "$url" 2>"$curl_err" | \
    stdbuf -oL tr '\r' '\n' | grep -o --line-buffered '[0-9]*\.[0-9]' | \
    zenity --progress --title="Enviando..." --text="Aguarde, o arquivo está sendo enviado" --auto-close

result="$PIPESTATUS"

if [ "$result" -eq 0 ]; then
    output=$(<"$curl_output")
    upload_status=$(echo "$output" | "$provas_dir/bin/jq" '.status')
    upload_msg=$(echo "$output" | "$provas_dir/bin/jq" '.msg')
else
    upload_status="$result"
    upload_msg="$(<"$curl_err")"
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

