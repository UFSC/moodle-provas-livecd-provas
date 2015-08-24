#!/bin/bash


# Registra no log a informação passada como parâmetro
# Data no formato: 2013-05-29 11:52:43,815 , onde 815 são os milisegundos.
log() {
    # O parâmetro '${0##*/}' insere o nome do arquivo que fez a chamada para a função
    log_msg="${0##*/} $1"
    date="$(date +"%F %T,%N")"

    echo "${date:0:23} $log_msg" >> "$log_file_provas"
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
    msg='A conexão com a Internet não está funcionando. Verifique se há algum problema temporário na \nrede local ou na conexão com a Internet. Outra possibilidade é que seja necessário configurar \nmanualmente a conexão de rede cabeada ou rede sem fio do computador. \n\nSe o problema persistir, sugerimos consultar o administrador da rede local.'

    #zenity --info --title 'Problema na conexão com a Internet' --no-wrap --text "$msg"
    zenity --error --title 'Problema na conexão com a Internet' --no-wrap --text "$msg"
}

# Pergunta ao usuário se ele quer enviar os arquivos de log e oferece a opção de digitar um e-mail e uma descrição do problema.
should_send_logs() {
    msg="As teclas <b>Ctrl</b>+<b>PrintScreen</b> foram pressionadas, resultando na coleta de uma série de <b>dados para diagnóstico</b>. \n\nVocê deseja enviar estes dados para a equipe de suporte do Moodle?"

    zenity --forms --title='Teclas Ctrl+PrintScreen pressionadas' --text="$msg" --add-entry="Seu e-mail" --add-entry="Descrição do problema"
}

# Pergunta ao usuário se ele quer enviar uma foto da tela e oferece a opção de digitar um e-mail e uma descrição do problema.
should_send_screenshot() {
    msg="A tecla <b>PrintScreen</b> foi pressionada e uma <b>imagem da tela atual</b> foi salva na área de trabalho. \n\nVocê deseja enviar esta imagem para a equipe de Suporte do Moodle?"

    zenity --forms --title='Tecla PrintScreen pressionada' --text="$msg" --add-entry="Seu e-mail" --add-entry="Descrição do problema"
}

# Mostra uma mensagem no monitor do usuário dizendo que houve sucesso no envio da screenshot
show_send_file_success() {
    # Devido a algum bug no zenity 3.4.0 (Ubuntu 12.04), ele deixa a janela com a altura bem maior do que o necessário quando
    # o texto não tem quebras de linha e a opção --no-wrap não é utilizada, na versão 3.16 do zenity isso não ocorre.
    msg="Os dados foram enviados para a equipe de suporte do Moodle. \n\nSe necessário, entre em contato através do e-mail $institution_moodle_support_email ."

    zenity --info --title "Dados enviados" --no-wrap --text "$msg"
 }

# Mostra uma mensagem no monitor do usuário dizendo que ocorreu algum erro no envio da screenshot
show_send_file_error() {
    msg='Erro ao enviar os dados para o servidor da equipe de suporte do Moodle.'

    # A opção --no-markup deve ser usada, pois podem ocorrer erros no zenity se a mensagem a ser impressa contiver tags HTML
    # (uma página de erro, por exemplo), assim as quebras de linhas devem ser feitas aqui, não dá pra usar o \n com --no-markup.
    zenity --error --no-markup --title 'Erro no envio dos dados' --no-wrap --text "$msg

Mensagem:
$1"
}

# Configura a página inicial do navegador Mozilla Firefox.
set_browser_homepage() {
    log 'Atualizando as configuração da página inicial do Firefox'
    sed -i "s|about:blank|$moodle_provas_url|g" "$firefox_syspref"
}

# Configura o arquivo de autoconfig do navegador Mozilla Firefox.
configure_browser() {
    log "Matando os processos do Firefox caso o usuário já o tenha iniciado"
    pkill firefox

    log "Copiando o arquivo de template do autoconfig do firefox"
    cp "$firefox_bin/$firefox_autoconfig.tpl" "$firefox_bin/$firefox_autoconfig"

    log "Atualizando as configurações do autoconfig do Firefox"
    sed -i "s|%livecd_version%|$livecd_version|g" "$firefox_bin/$firefox_autoconfig"
    sed -i "s|%livecd_local_ip%|$livecd_local_ip|g" "$firefox_bin/$firefox_autoconfig"
    sed -i "s|%livecd_local_network%|$livecd_local_network|g" "$firefox_bin/$firefox_autoconfig"
}

# Libera o acesso de saída aos IPs e portas definidos na variável $allowed_tcp_out_ipv4
configure_firewall_ipv4() {
    for entry in $allowed_tcp_out_ipv4; do
        ip="${entry%#*}"
        port="${entry#*#}"
        protocol='tcp'

        log "configure_firewall_ipv4(): Liberando o acesso ao IP $ip na PORTA $port com o protocolo $protocol"
        $IPTABLES_IPV4 -A OUTPUT -d "$ip" -p "$protocol" --dport "$port" -j ACCEPT >>"$log_file_provas" 2>&1
    done
}

# Libera o acesso de saída aos IPs e portas definidos na variável $allowed_tcp_out_ipv6
configure_firewall_ipv6() {
    for entry in $allowed_tcp_out_ipv6; do
        ip="${entry%#*}"
        port="${entry#*#}"
        protocol='tcp'

        log "configure_firewall_ipv6(): Liberando o acesso ao IP $ip na PORTA $port com o protocolo $protocol"
        $IPTABLES_IPV6 -A OUTPUT -d "$ip" -p "$protocol" --dport "$port" -j ACCEPT >>"$log_file_provas" 2>&1
    done
}

# Libera o acesso de saída de acordo com as listas de IPs e portas liberadas, em seguida salva as regras atualizadas.
configure_firewall() {
    configure_firewall_ipv4
    configure_firewall_ipv6

    log "configure_firewall() Salvando as regras atualizadas do firewall"
    /etc/init.d/iptables-persistent save
}

# Libera o acesso aos ips do servidor de configuração online, que possue o arquivo JSON.
allow_access_to_online_config_server() {
    log "allow_access_to_online_config_server() Liberando o acesso HTTPS ao servidor de configuração no IPv4: $livecd_online_config_host_ip"
    $IPTABLES_IPV4 -A OUTPUT -d "$livecd_online_config_host_ip" -p tcp --dport 443 -j ACCEPT

#    Código que adiciona suporte a IPv6, não está ativado porque existem algumas dependências
#    em outras partes, por exemplo a variável $livecd_local_ip depende da variável $livecd_online_config_host_ip ,
#    os headers HTTP enviados pelo firefox contém apenas o IPv4, deveria conter também o IPv6, mas
#    isso requer alterações no módulo de provas do servidor do Moodle.
#    Código adicionado em 11/03/2015.
#    livecd_online_config_host_ipv4=$(dig $livecd_online_config_host a +short)
#    livecd_online_config_host_ipv6=$(dig $livecd_online_config_host aaaa +short)
#
#    log "allow_access_to_online_config_server() O host $livecd_online_config_host tem os endereços IPv4: '$livecd_online_config_host_ipv4'"
#    log "allow_access_to_online_config_server() O host $livecd_online_config_host tem os endereços IPv6: '$livecd_online_config_host_ipv6'"
#
#    if [ -n "$livecd_online_config_host_ipv4" ]; then
#        for ipv4 in $livecd_online_config_host_ipv4; do
#            log "allow_access_to_online_config_server() Liberando o acesso HTTPS ao servidor de configuração no IPv4: $ipv4"
#            $IPTABLES_IPV4 -A OUTPUT -d "$ipv4" -p tcp --dport 443 -j ACCEPT
#        done
#    fi
#
#    if [ -n "$livecd_online_config_host_ipv6" ]; then
#        for ipv6 in $livecd_online_config_host_ipv6; do
#            log "allow_access_to_online_config_server() Liberando o acesso HTTPS ao servidor de configuração no IPv6: $ipv6"
#            $IPTABLES_IPV6 -A OUTPUT -d "$ipv6" -p tcp --dport 443 -j ACCEPT
#        done
#    fi
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

# Atualiza a lista de servidores NTP e força a atualização do horário
set_ntp_servers() {
    if [ -e '/etc/default/ntpdate' ]; then
        log "Definindo os novos servidores NTP no arquivo /etc/default/ntpdate: $ntp_servers"
        sed -i "s|NTPSERVERS=.*|NTPSERVERS=\"$ntp_servers\"|g" /etc/default/ntpdate

        ntpdate-debian &
    else
        log 'ERRO: O arquivo /etc/default/ntpdate não pode ser lido.'
    fi
}

