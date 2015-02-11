#!/bin/bash


# Registra no log a informação passada como parâmetro
# Data no formato: 2013-05-29 11:52:43,815 , onde 815 são os milisegundos.
log() {
    # O parâmetro '${0##*/}' insere o nome do arquivo que fez a chamada para a função
    log_msg="${0##*/} $1"
    date="$(date +"%F %T,%N")"
    echo "${date:0:23} $log_msg" >> $log_file_provas
}

# Verifica se o /proc/cmdline contém a variável passada como argumento
cmdline_contains() {
    var="$1"

    if [ -f '/proc/cmdline' ]; then
        if $(grep -q "$var" '/proc/cmdline'); then
            return 0  # Contém a variável
        else
            return 1  # Não contém a variável
        fi  
    else
        return -1  # O arquivo /proc/cmdline não existe"
    fi
}

# Verifica se o sistema foi carregado através de boot remoto PXE
is_pxe_booted() {
    return $(cmdline_contains 'netboot=nfs')
}

# Verifica se a opção de enviar logs foi ativada no boot
should_send_logs() {
    return $(cmdline_contains 'send_logs')
}

# Verifica se a opção de inverter as placas de vídeo em um multiterminal foi ativada no boot
should_switch_vgas() {
    return $(cmdline_contains 'switch_vgas')
}

# Verifica se a conexão com a internet está funcionando
is_internet_online() {
    if [ -z "$timeout_host_test" ] ||
       [ -z "$host_test_1" ] ||
       [ -z "$host_test_2" ]
    then
        log 'is_internet_online() ERRO: Algumas variáveis não estão definidas, verifique o arquivo de configuração!'
        exit 1
    fi

    if ping -w "$timeout_host_test" -c1 "$host_test_1" >/dev/null 2>&1 ||
       ping -w "$timeout_host_test" -c1 "$host_test_2" >/dev/null 2>&1
    then
        return 0
    else
        return 1
    fi
}

# Verifica se o script foi executado com poder de root
is_root() {
    if [ ! "$UID" -eq 0 ]; then
        exit 1
    fi
}

# Verifica se o suporte a multiterminal foi desativado no boot
is_multiseat_enabled() {
    return $(cmdline_contains 'multiseat')
}

# Verifica se o computador suporta multiterminal
is_multiseat_capable() {
    supported_seats="$(/usr/bin/env python /opt/provas/get_supported_seats.py)"

    if [ "$supported_seats" = 1 ]; then
        return 1
    elif [ "$supported_seats" = 2 ]; then
        return 0
    else
        return -1
    fi
}

# Se não foi selecionada a opção AUTO no boot, tentará carregar o suporte a multiterminal
# O parâmetro 'multiseat' deve estar presente na linha de parâmetros do kernel.
is_multiseat() {
    if is_multiseat_enabled; then
        if is_multiseat_capable; then
            log 'is_multiseat(): O computador suporta multiterminal.'
            multiseat=0
        else
            log 'is_multiseat(): Computador não suporta multiterminal.'
            multiseat=1
        fi
    else
        log 'is_multiseat(): O suporte a multiterminal não foi ativado no boot.'
        multiseat=1
    fi

    return $multiseat
}

# Cria um usuário no sistema com o nome passado como parâmetro e com a senha bloqueada
create_user() {
    username="$1"
    return $(useradd $username -m -c "$username" -s "/bin/bash" >/dev/null 2>&1)
}

# Obtem o endereço IP do servidor NFS (função atualmente não utilizada)
get_nfs_server_address() {
    nfs_server="$(cat /proc/cmdline | awk '/nfsroot/{print $1}' RS=" " FS=":" | cut -d "=" -f 2)"

    echo "$nfs_server"
}

# Mostra uma mensagem no monitor do primeiro usuário com a mensagem de problema na conexão de rede.
show_no_connection() {
    msg='A conexão de rede não está funcionando e talvez você precise configurá-la manualmente. Para que esta mensagem não apareça novamente, configure a conexão de rede e clique em OK.'

    gxmessage -display ":1" -bg orange -geometry 500x200 -center -font "ubuntu 13" -title "Conexão de rede" -wrap "$msg"
}

# Mostra uma mensagem no monitor do primeiro usuário dizendo para aguardar o envio dos logs de diagnóstico.
show_send_logs_wait() {
    msg='Aguarde os arquivos de log serem enviados.'

    gxmessage -display ':1' -bg 'orange' -geometry '300x150' -center -font "ubuntu 16" -title 'Aguarde...' -wrap "$msg" &
}

# Mostra uma mensagem no monitor do primeiro usuário dizendo que houve sucesso no envio dos logs de diagnóstico.
show_send_logs_ok() {
    pkill gxmessage
    msg="Os arquivos de log foram enviados com sucesso, agora você pode desligar este computador e enviar um e-mail para $log_email informando que o procedimento foi realizado."

    gxmessage -display ':1' -bg 'green' -geometry '450x200' -center -font 'ubuntu 13' -title 'Envio dos arquivos de log' -wrap "$msg"
}

# Mostra uma mensagem no monitor do primeiro usuário dizendo que ocorreu algum erro no envio dos logs de diagnóstico.
show_send_logs_bad() {
    pkill gxmessage
    msg='Erro no envio dos arquivos de log, informe o problema ao suporte técnico.'

    gxmessage -display ':1' -bg 'red' -fg 'white' -geometry '700x500' -center -font 'ubuntu 13' -title "$msg" -file '/tmp/send_logs.log'
}

# Prepara e envia os logs para o servidor remoto via POST, utilizando o script /opt/provas/send_logs.sh
send_logs() {
    log 'Preparando para enviar os logs'
    log 'Liberando o acesso ao servidor de logs'
    $IPTABLES -A OUTPUT -d "$log_server_ip" -p tcp --dport 443 -j ACCEPT >>"$log_file_provas" 2>&1

    export LANG="pt_BR.UTF-8"
    export XAUTHORITY="/home/${username_base}1/.Xauthority"
    export DISPLAY=':1'
    show_send_logs_wait

    log "Executando o script que envia os logs, /opt/provas/send_logs.sh"
    if /opt/provas/send_logs.sh >/tmp/send_logs.log 2>&1; then
        show_send_logs_ok
    else
        log "Erro ao enviar os arquivos de log."
        show_send_logs_bad
    fi

    exit
}

# Configura a página inicial do navegador Mozilla Firefox.
set_browser_homepage() {
    log 'Atualizando as configuração da página inicial do Firefox'
    $SED -i "s|about:blank|$provas_homepage|g" "$firefox_syspref"
}

# Configura o arquivo de autoconfig do navegador Mozilla Firefox.
configure_browser() {
    log "Matando os processos do Firefox caso o usuário já o tenha iniciado"
    pkill firefox

    log "Copiando o arquivo de template do autoconfig do firefox"
    cp "$firefox_bin/$firefox_autoconfig.tpl" "$firefox_bin/$firefox_autoconfig"

    log "Atualizando as configurações do autoconfig do Firefox"
    $SED -i "s|%provas_version%|$provas_version|g" "$firefox_bin/$firefox_autoconfig"
    $SED -i "s|%local_ip%|$local_ip|g" "$firefox_bin/$firefox_autoconfig"
    $SED -i "s|%local_network%|$local_network|g" "$firefox_bin/$firefox_autoconfig"
}

configure_firewall_ipv4() {
    for entry in $allowed_out_ipv4; do
        protocol="${entry/*:/}"
        ip_port="${entry%:*}"
        ip="${ip_port%:*}"
        port="${ip_port#*:}"

        log "Liberando o acesso ao IP $ip na PORTA $port com o protocolo $protocol"
        $IPTABLES -A OUTPUT -d "$ip" -p "$protocol" --dport "$port" -j ACCEPT >>"$log_file_provas" 2>&1
    done
}

#configure_firewall_ipv4() {
#    # Libera o acesso aos IPs via HTTP (porta 80)
#    for ip in $allowed_ipv4_http; do
#        log "Liberando o acesso via HTTP para o IP $ip"
#        $IPTABLES -A OUTPUT -d "$ip" -p tcp --dport 80 -j ACCEPT >>"$log_file_provas" 2>&1
#    done
#
#    # Libera o acesso aos IPs via HTTPS (porta 443)
#    for ip in $allowed_ipv4_https; do
#        log "Liberando o acesso via HTTPS para o IP $ip"
#        $IPTABLES -A OUTPUT -d "$ip" -p tcp --dport 443 -j ACCEPT >>"$log_file_provas" 2>&1
#    done
#}

#configure_firewall_ipv6() {
#    # Libera o acesso aos IPs via HTTP (porta 80)
#    for ip in $allowed_ipv6_http; do
#        log "Liberando o acesso via HTTP para o IP $ip"
#        $IPTABLES -A OUTPUT -d "$ip" -p tcp --dport 80 -j ACCEPT >>"$log_file_provas" 2>&1
#    done
#
#    # Libera o acesso aos IPs via HTTPS (porta 443)
#    for ip in $allowed_ipv6_https; do
#        log "Liberando o acesso via HTTPS para o IP $ip"
#        $IPTABLES -A OUTPUT -d "$ip" -p tcp --dport 443 -j ACCEPT >>"$log_file_provas" 2>&1
#    done
#}

# Libera o acesso via HTTP e HTTPS aos IPs definidos no arquivo de configuração e salva as regras atualizadas.
configure_firewall() {
    configure_firewall_ipv4
#    configure_firewall_ipv6

    log "Salvando as regras do firewall atualizadas"
    /etc/init.d/iptables-persistent save
}

# Aguarda a sessão do usuário informado ser carregada para montar o diretório do userChrome.css.
lock_firefox_userchrome_file_for_user() {
    user_id="$1"
    username="${username_base}${user_id}"

    # Monta o diretório do firefox onde fica o arquivo userChrome.css como somente leitura,
    # para que o usuário não possa modificá-lo
    log "Montando o diretório do firefox como somente leitura para o usuário '$username'."
    firefox_chrome_dir="/home/$username/.mozilla/firefox/profile.default/chrome"
    if ! $(mount | grep "$firefox_chrome_dir" >/dev/null 2>&1); then
        mount --bind "$firefox_chrome_dir" "$firefox_chrome_dir" >>"$log_file_provas" 2>&1
        mount -o remount,ro "$firefox_chrome_dir" >>"$log_file_provas" 2>&1
    fi
}

#Bloqueia a execução até que o Xorg com o LXDE do usuário tenha carregado
wait_session_load_for_user() {
    user_id="$1"
    username="${username_base}${user_id}"
    
    log "Aguardando o processo 'lxsession' do usuário '$username' iniciar..."
    while ! $(pgrep -u "$username" lxsession &>/dev/null); do
        sleep 1
    done

    log "Aguardando alguns segundos para a sessão do usuário '$username' carregar..."
    sleep 2
}

# Inicia o navegador Mozilla Firefox na sessão do usuário informado.
start_browser_for_user() {
    user_id="$1"
    username="${username_base}${user_id}"

    log "Executando o Firefox na sessão do usuário '$username'"
    export LANG="pt_BR.UTF-8"
    export XAUTHORITY="/home/$username/.Xauthority"
    export DISPLAY=":$user_id"
    pkill -u "$username" firefox
    su - "$username" -c firefox &
}

# Inicia as sessões dos usuários em modo multiterminal
start_multiseat_mode() {
    # Desativa as configurações adicionais do Xorg, elas já estão incluídas nos arquivos
    # gerados em /etc/X11/xorg*, mas se este arquivo existir a seção ServerLayout será afetada.
    if [ -f '/usr/share/X11/xorg.conf.d/60-moodle-provas.conf' ]; then
        rm '/usr/share/X11/xorg.conf.d/60-moodle-provas.conf'
    fi  

    # Desativa a opção de desligar e muda a mensagem
    chmod u-s '/usr/bin/lxsession-logout'
    sed -i 's/--prompt ".*"/--prompt "Multiterminal: Utilize o botão liga\/desliga do computador."/' "/usr/bin/lxde-logout"

    create_user "${username_base}1" && log "Usuário ${username_base}1 criado."
    create_user "${username_base}2" && log "Usuário ${username_base}2 criado."

    usermod -a -G netdev ${username_base}1 >/dev/null 2>&1
    usermod -a -G netdev ${username_base}2 >/dev/null 2>&1

    lock_firefox_userchrome_file_for_user '1'
    lock_firefox_userchrome_file_for_user '2'

    seats=$(/opt/provas/multiseat/pre-setup.sh)
    log "start_multiseat_mode() Retorno do pre-setup.sh: $seats"
    log 'start_multiseat_mode() Iniciando o /opt/provas/multiseat/setup.py'
    /usr/bin/env python /opt/provas/multiseat/setup.py "$seats" "$username_base"
}

# Inicia a sessão do usuário em modo normal, sem multiterminal
start_normal_mode() {
    # Cria o usuário e adiciona ao grupo de audio
    create_user "${username_base}1" && log "Usuário ${username_base}1 criado."
    usermod -a -G audio ${username_base}1 >/dev/null 2>&1
    usermod -a -G netdev ${username_base}1 >/dev/null 2>&1

    lock_firefox_userchrome_file_for_user '1' 

    su - "${username_base}1" -c 'startx -- :1 -br -audit 0 -novtswitch -nolisten tcp' &
}

set_ntp_servers() {
    if [ -e '/etc/default/ntpdate' ]; then
        log "Definindo os novos servidores NTP no arquivo /etc/default/ntpdate: $ntp_servers"
        sed -i "s|NTPSERVERS=.*|NTPSERVERS=\"$ntp_servers\"|g" /etc/default/ntpdate

        ntpdate-debian &
    else
        log 'ERRO: O arquivo /etc/default/ntpdate não pode ser lido.'
    fi
}
