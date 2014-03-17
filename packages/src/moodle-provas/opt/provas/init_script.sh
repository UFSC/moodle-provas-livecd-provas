#!/bin/bash

provas_config='/opt/provas/moodle_provas.conf'
[ -r "$provas_config" ] && source "$provas_config" || exit 1

functions_file="$provas_dir/includes/functions.sh"
[ -r "$functions_file" ] && source "$functions_file" || exit 1


# Verifica se o script foi executado com poder de root
is_root

# O sistema for inicializado por boot remoto
if is_pxe_booted; then
    log 'Desativando o serviço network-manager do /etc/init'
    echo 'manual' > '/etc/init/network-manager.override'

    log "Parando o serviço 'network-manager' se ele já estiver rodando"
    initctl stop network-manager

    if [ -f '/etc/init.d/casper' ] && [ -f '/etc/init.d/casper.patch.bootremoto' ]; then
        log 'Aplicando o patch no casper para desligar direto, sem mostrar a mensagem de remover o CD da bandeja'
        patch '/etc/init.d/casper' '/etc/init.d/casper.patch.bootremoto' >>"$log_file_provas" 2>&1
    fi

    if [ -f '/etc/xdg/autostart/nm-applet.desktop' ]; then
        log 'Desativando o nm-applet'
        mv '/etc/xdg/autostart/nm-applet.desktop' '/etc/xdg/autostart/nm-applet.desktop.disabled'
    fi
else
    log 'Desativando as regras de acesso livre a NFS (qualquer ip)'
    sed -i '/^# NFS/,+4 { /^# NFS/ b; s/^/#/; }' "$firewall_ipv4_rules"
    sed -i '/^# NFS/,+4 { /^# NFS/ b; s/^/#/; }' "$firewall_ipv6_rules"
    /etc/init.d/iptables-persistent flush
    /etc/init.d/iptables-persistent reload

    if [ -f '/etc/init.d/casper' ] && [ -f '/etc/init.d/casper.patch.cd' ]; then
        log 'Aplicando o patch no script do serviço 'casper' para traduzir a mensagem de remover o CD da bandeja'
        patch '/etc/init.d/casper' '/etc/init.d/casper.patch.cd' >>"$log_file_provas" 2>&1
    fi
fi

# O script abaixo deve ser executado em background para não bloquear a inicialização
# do sistema, é por isso também que ele é um arquivo separado e não parte deste script.
log 'Iniciando o script /opt/provas/start_user_apps.sh em background'
$provas_dir/start_user_apps.sh &

