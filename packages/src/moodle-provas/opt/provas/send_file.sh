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
email="${email:0:1000}"   # Limita a string em 1000 caracteres
description="${user_input#*|}"  # Extrai o e-mail da string
description="${description:0:8000}"  # Limita a string em 8000 caracteres (devido a limitação de 8KiB por campo do header HTTP no Apache)
http_header_email="LIVECD-USER-EMAIL:$email"
http_header_description="LIVECD-USER-DESCRIPTION:$description"

curl_output="/tmp/curl.log"
curl_err="/tmp/curl-err.log"
[ -f "$curl_output" ] && rm -f "$curl_output"
[ -f "$curl_err" ] && rm -f "$curl_err"

url="${moodle_provas_url}${diag_script_receive_file_path}"
#url="https://${livecd_online_config_host}${diag_script_receive_file_path}"

log "E-mail informado pelo usuário: '$email'"
log "Descrição informada pelo usuário: '$description'"

log "Enviando o arquivo '$filename' para o endereço '$url'"
curl -m "$diag_upload_timeout" -o "$curl_output" -k -H "$http_header1" -H "$http_header2" -H "$http_header3" \
    -H "$http_header_email" -H "$http_header_description" \
    -F "token=$diag_script_token" -F "file=@$filename" "$url" 2>"$curl_err" | \
    stdbuf -oL tr '\r' '\n' | grep -o --line-buffered '[0-9]*\.[0-9]' | \
    zenity --progress --no-cancel --title="Enviando..." --text="Aguarde, os dados estão sendo enviados" --auto-close

result="$PIPESTATUS"

if [ "$result" -eq 0 ]; then
    output="$(tail -6 "$curl_output")"

    if echo "$output" | "$provas_dir/bin/jq" '.status' >/dev/null 2>&1; then
        upload_status=$(echo "$output" | "$provas_dir/bin/jq" '.status')
        upload_msg="$(echo "$output" | "$provas_dir/bin/jq" '.msg')"
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

