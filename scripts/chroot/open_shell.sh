#!/bin/bash

base_dir=$(dirname $0)
livecd_config="$base_dir/livecd_provas.conf"
functions_file="$base_dir/includes/functions.sh"
chroot_functions_file="$base_dir/includes/functions_chroot.sh"

# Includes
[ -r "$livecd_config" ] && source "$livecd_config" || exit 1
[ -r "$functions_file" ] && source "$functions_file" || exit 1
[ -r "$chroot_functions_file" ] && source "$chroot_functions_file" || exit 1


init_chroot


# Upgrade e instalação dos pacotes
msg_w "ATENÇÃO: ESTA OPÇÃO DEVE SER UTILIZADA SOMENTE PARA TESTES"
msg_w "Iniciando o '/bin/bash'... (Digite 'exit' para sair ou pressione Ctrl+D)"
/bin/bash


finish_chroot


# A função cleanup_chroot() é automaticamente executada quando o script finaliza,
# pois o sinal 'EXIT' foi incluído no 'trap' da função init().

