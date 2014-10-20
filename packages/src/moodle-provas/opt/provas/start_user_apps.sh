#!/bin/bash

provas_config='/opt/provas/moodle_provas.conf'
[ -r "$provas_config" ] && source "$provas_config" || exit 1

functions_file="$provas_dir/includes/functions.sh"
[ -r "$functions_file" ] && source "$functions_file" || exit 1


IPTABLES='/sbin/iptables'
SED='/bin/sed'

is_root

first_user_id='1'
second_user_id='2'
username="${username_base}1"

# Configura os servidores NTP do computador, para sincronizar o horário do computador.
set_ntp_servers

# Configura o navegador de internet com a página inicial definida no arquivo de configuração.
set_browser_homepage

# Aguarda a sessão do usuário carregar (Xorg + Desktop)
wait_session_load_for_user "$first_user_id"

# Se não tiver iniciado via PXE e não tiver internet, exibe uma mensagem sobre a conexão de rede.
if ! is_pxe_booted; then
    log "O computador não iniciou via PXE, a conexão com a internet será verificada..."
    while ! is_internet_online; do
        log "A internet não está funcionando"
        export LANG="pt_BR.UTF-8"
        export XAUTHORITY="/home/$username/.Xauthority"
        export DISPLAY=":$first_user_id"
        show_no_connection
        sleep 1
    done

    log "A internet está funcionando, recarregando as variáveis 'local_ip' e 'local_network'."
    source "$provas_config"
fi

# A configuração do autoconfig do Firefox só pode ser feita após a conexão com a internet estar funcionando.
log 'Configurando o navegador Mozilla Firefox...'
configure_browser

log 'Configurando o firewall...'
configure_firewall

log "Executando o script '/opt/provas/cron_script.sh' pela primeira vez..."
$provas_dir/cron_script.sh

log 'Ajustando o volume dos canais de som para 30%...'
amixer -D pulse set Master 30% unmute >>"$log_file_provas" 2>&1

log 'Verificando se o envio de logs foi ativado no boot...'
if should_send_logs; then
    log 'O envio de logs foi ativado no boot'
    send_logs
else
    log 'O envio de logs não foi ativado no boot.'
fi

log "Iniciando o navegador na sessão do primeiro usuário"
start_browser_for_user "$first_user_id"

log 'Verificando se o computador é um multiterminal...'
if is_multiseat; then
    log 'O computador é um multiterminal, iniciando a configuração para o segundo usuário...'

    # Aguarda a sessão do usuário carregar (Xorg + Desktop)
    wait_session_load_for_user "$second_user_id"

    log "Iniciando o navegador na sessão do segundo usuário"
    start_browser_for_user "$second_user_id"
else
    log 'O computador não é um multiterminal, nada mais a ser feito.'
fi

log 'Feito.'

