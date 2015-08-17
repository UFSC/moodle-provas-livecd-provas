#!/bin/bash
#set -x

provas_config_file='/opt/provas/moodle_provas.conf'
[ -r "$provas_config_file" ] && source "$provas_config_file" || exit 1

functions_file="$provas_dir/includes/functions.sh"
[ -r "$functions_file" ] && source "$functions_file" || exit 1


IPTABLES_IPV4='/sbin/iptables'
IPTABLES_IPV6='/sbin/ip6tables'

is_root

first_user_id='1'
second_user_id='2'

first_user="${username_base}${first_user_id}"
second_user="${username_base}${second_user_id}"

# Aguarda a sessão do usuário carregar (Xorg + Desktop)
wait_session_load_for_user "$first_user_id"

# Se não tiver iniciado via PXE e não tiver internet, exibe uma mensagem sobre a conexão de rede.
if ! is_pxe_booted; then
    log "O computador não iniciou via PXE, a conexão com a internet será verificada..."
    while ! is_internet_online; do
        log "A internet não está funcionando"
        export LANG="pt_BR.UTF-8"
        export XAUTHORITY="/home/$first_user/.Xauthority"
        export DISPLAY=":$first_user_id"
        show_no_connection
        sleep 1
    done

    log "A internet está funcionando, recarregando as variáveis 'livecd_local_ip' e 'livecd_local_network'."
    source "$provas_config_file"
fi

# Configura os servidores NTP do computador, para sincronizar o horário do computador.
# (Primeira configuração, para agilizar a atualização do horário do computador).
set_ntp_servers

if [ -f "$provas_online_config_file" ]; then
    log "Carregando arquivo de configuração online padrão: $provas_online_config_file"
    source "$provas_online_config_file"
else
    log "O arquivo de configuração online padrão não existe ($provas_online_config_file)."
    allow_access_to_online_config_server

    log "Iniciando o programa de configuração online"
    export LANG="pt_BR.UTF-8"
    export XAUTHORITY="/home/$first_user/.Xauthority"
    export DISPLAY=":$first_user_id"
    "$provas_dir/online_update.py" "$provas_config_file" >$provas_log_dir/online_update.log 2>&1
    if [ $? -eq 0 ]; then
        source "$provas_online_config_file"
    else
        exit 1
    fi
fi

if [ "$show_institution_name_in_desktop" = "yes" ]; then
    log "Atualizando o papel de parede com o nome da instituição"
    "$provas_dir/update_desktop_wallpaper.sh" 2 'yes' >$provas_log_dir/update_desktop_wallpaper.log 2>&1 &
fi

# Habilita o envio de screenshots para um servidor remoto, para o primeiro usuário.
if [ "$allow_screenshots" = "yes" ]; then
    enable_printscreen_key "$first_user_id"
fi
# Reconfigura os servidores NTP do computador com os definidos na configuração online.
set_ntp_servers

# Configura o navegador de internet com a página inicial definida no arquivo de configuração.
set_browser_homepage

# A configuração do autoconfig do Firefox só pode ser feita após a conexão com a internet estar funcionando.
log 'Configurando o navegador Mozilla Firefox...'
configure_browser

log 'Configurando o firewall...'
configure_firewall

log "Executando o script '/opt/provas/cron_script.sh' pela primeira vez..."
$provas_dir/cron_script.sh

log 'Ajustando o volume dos canais de som para 30%...'
amixer -D pulse set Master 30% unmute >>"$log_file_provas" 2>&1

log "Iniciando o navegador na sessão do primeiro usuário"
start_browser_for_user "$first_user_id"

log 'Verificando se o computador é um multiterminal...'
if is_multiseat; then
    log 'O computador é um multiterminal, iniciando a configuração para o segundo usuário...'

    # Habilita o envio de screenshots para um servidor remoto, para o segundo usuário.
    if [ "$allow_screenshots" = "yes" ]; then
        enable_printscreen_key "$second_user_id"
    fi

    # Aguarda a sessão do usuário carregar (Xorg + Desktop)
    wait_session_load_for_user "$second_user_id"

    log "Iniciando o navegador na sessão do segundo usuário"
    start_browser_for_user "$second_user_id"
else
    log 'O computador não é um multiterminal, nada mais a ser feito.'
fi

log 'Feito.'

