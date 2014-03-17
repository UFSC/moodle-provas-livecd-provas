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


msg_d "Verificando se todos os pacotes do Moodle Provas estão presentes em /tmp/chroot..."
check_provas_packages

# Upgrade e instalação dos pacotes
msg_d "Atualizando a lista de pacotes (pode demorar alguns minutos)..."
apt-get update >>"$std_out" 2>>"$std_err"

msg_d "Atualizando o sistema com 'dist-upgrade' (pode demorar vários minutos)..."
apt-get dist-upgrade -y >>"$std_out" 2>>"$std_err"

msg_d "Instalando todos os pacotes essenciais (pode demorar vários minutos)..."
apt-get install $pkg_all -y >>"$std_out" 2>>"$std_err"

# Neste passo é normal ocorrerem erros no 'dpkg -i', devido as dependências, por isso o stderr é redirecionado pra $std_out.
msg_d "Instalando os pacotes do moodle provas..."
dpkg -i /tmp/*.deb >>"$std_out" 2>>"$std_out"
apt-get install -f -y >>"$std_out" 2>>"$std_err"

if [ "$enable_multiseat" != "yes" ]; then
    msg_d "O suporte a multiterminal está desativado, removendo o pacote moodle-multiseat..."
    apt-get purge moodle-multiseat -y >>"$std_out" 2>>"$std_err"
fi

msg_d "Definindo o 'oracle-java7-jre' como o java padrão..."
update-alternatives --set java /usr/lib/oracle-java7-jre/bin/java >>"$std_out" 2>>"$std_err"

msg_d "Configurando o locale padrão para $lang..."
locale-gen pt >>"$std_out" 2>>"$std_err"
update-locale LANG="$lang" LANGUAGE="$language" LC_ALL="$lc_all" >>"$std_out" 2>>"$std_err"

msg_d "Configurando o teclado com o layout $keyboard_layout/$keyboard_variant..."
sed -i "s/XKBLAYOUT=.*$/XKBLAYOUT=\"$keyboard_layout\"/g" /etc/default/keyboard
sed -i "s/XKBVARIANT=.*$/XKBVARIANT=\"$keyboard_variant\"/g" /etc/default/keyboard
dpkg-reconfigure console-setup >>"$std_out" 2>>"$std_out"

msg_d "Configurando o hostname do sistema como '$livecd_hostname'..."
sed -i "s/export HOST=.*/export HOST=\"$livecd_hostname\"/g" /etc/casper.conf
sed -i "s/# export FLAVOUR=.*/export FLAVOUR=\"$livecd_hostname\"/g" /etc/casper.conf

msg_d "Configurando o fuso horário padrão do sistema para a zona '$timezone'..."
echo "$timezone" > /etc/timezone
dpkg-reconfigure tzdata >>"$std_out" 2>>"$std_out"


finish_chroot


# A função cleanup_chroot() é automaticamente executada quando o script finaliza,
# pois o sinal 'EXIT' foi incluído no 'trap' da função init().

