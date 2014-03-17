#!/bin/bash

cleanup_chroot() {
    msg_d "Removendo os arquivos temporários..."
    rm -rf /etc/resolv.conf >>"$std_out" 2>>"$std_err"
    rm -rf /tmp/* >>"$std_out" 2>>"$std_err"
    rm -rf ~/.bash_history >>"$std_out" 2>>"$std_err"

    msg_d "Reativando o '/sbin/initctl'..."
    rm /sbin/initctl >>"$std_out" 2>>"$std_err"
    dpkg-divert --rename --remove /sbin/initctl >>"$std_out" 2>>"$std_err"

    msg_d "Reativando o '/usr/sbin/update-grub'..."
    rm /usr/sbin/update-grub >>"$std_out" 2>>"$std_err"
    dpkg-divert --rename --remove /usr/sbin/update-grub >>"$std_out" 2>>"$std_err"

    msg_d "Desmontando o '/sys'..."
    umount /sys >>"$std_out" 2>>"$std_err"

    msg_d "Desmontando o '/dev/pts'..."
    umount /dev/pts >>"$std_out" 2>>"$std_err"

    msg_d "Desmontando o '/proc'..."
    umount /proc || umount -lf /proc

    msg_d "Removendo o arquivo '/etc/mtab'..."
    rm -rf /etc/mtab

    msg_a "${0##*/}: Saindo do ambiente CHROOT."
    exit
}


init_chroot() {
    chroot='true'
    msg_a "${0##*/}: Entrando em ambiente CHROOT"

    start_log

    # trap para acionar a limpeza caso o script seja interrompido
    trap "cleanup_chroot $?" HUP INT TERM KILL QUIT EXIT

    export HOME=/root
    export LC_ALL=C

    # OBS: O redirecionamento para /dev/stdout e /dev/stderr só funciona após o /proc estar montado.
    msg_d "Montando o '/proc'..."
    mount -t proc none /proc

    msg_d "Montando o '/sys'..."
    mount -t sysfs none /sys >>"$std_out" 2>>"$std_err"

    msg_d "Montando o '/dev/pts'..."
    mount -t devpts none /dev/pts >>"$std_out" 2>>"$std_err"

    msg_d "Gerando o arquivo '/etc/mtab'..."
    grep -v rootfs /proc/mounts > /etc/mtab
    
    msg_d "Desativando o '/sbin/initctl'..."
    dpkg-divert --local --rename --add /sbin/initctl >>"$std_out" 2>>"$std_err"
    ln -s /bin/true /sbin/initctl >>"$std_out" 2>>"$std_err"
    
    msg_d "Desativando o '/usr/sbin/update-grub'..."
    dpkg-divert --local --rename --add /usr/sbin/update-grub >>"$std_out" 2>>"$std_err"
    ln -s /bin/true /usr/sbin/update-grub >>"$std_out" 2>>"$std_err"

    msg_d "Desativando a instalação automática de pacotes recomendados..."
    echo -e 'APT::Install-Recommends "0";\nAPT::Install-Suggests "0";' > /etc/apt/apt.conf.d/00disable-recommends

    msg_d "Desativando o modo interativo do apt-get..."
    export DEBIAN_FRONTEND="noninteractive"

    cd $base_dir
}

remove_old_kernels() {
    current_kernel="linux-image-$(readlink /vmlinuz | cut --complement -d "-" -f 1)"
    dpkg --get-selections 'linux-image*' | grep -v "$current_kernel" | cut -f 1 | xargs apt-get -y purge >>"$std_out" 2>>"$std_err"
}

enable_admin_access() {
    if id -u $admin_user >/dev/null 2>&1; then
        msg "O acesso administrativo já estava ativado."
        return
    fi

    msg "Ativando o acesso administrativo..."
    msg "Criando a conta administrativa: $admin_user:$admin_password..."
    useradd $admin_user -m -c "$admin_user" -s "/bin/bash" -G sudo >>"$std_out" 2>>"$std_err"
    echo "$admin_user:$admin_password" | chpasswd >>"$std_out" 2>>"$std_err"

    msg_d "Instalando o openssh-server para acesso remoto..."
    apt-get install openssh-server -y >>"$std_out" 2>>"$std_err"

    msg_d "Habilitando o acesso SSH no firewall..."
    if [ -r "$firewall_ipv4_rules" ]; then
        sed -i '/^# SSH/,+2 { /^# SSH/ b; s/^#//; }' "$firewall_ipv4_rules"
    else
        msg_e "ERRO: $firewall_ipv4_rules não pode ser lido."
    fi  

    if [ -r "$firewall_ipv6_rules" ]; then
        sed -i '/^# SSH/,+2 { /^# SSH/ b; s/^#//; }' "$firewall_ipv6_rules"
    else
        msg_e "ERRO: $firewall_ipv6_rules não pode ser lido."
    fi  
}

disable_admin_access() {
    if ! id -u $admin_user >/dev/null 2>&1; then
        msg "O acesso administrativo já estava desativado."
        return
    fi

    msg "Desativando o acesso administrativo..."
    msg_d "Removendo a conta administrativa: $admin_user..."
    userdel -rf $admin_user >>"$std_out" 2>>"$std_err"

    msg_d "Removendo o openssh-server..."
    apt-get purge openssh-server -y >>"$std_out" 2>>"$std_err"
    
    msg_d "Desabilitando o acesso SSH no firewall..."
    if [ -r "$firewall_ipv4_rules" ]; then
        sed -i '/^# SSH/,+2 { /^# SSH/ b; s/^/#/; }' "$firewall_ipv4_rules"
    else
        msg_e "ERRO: $firewall_ipv4_rules não pode ser lido."
    fi

    if [ -r "$firewall_ipv6_rules" ]; then
        sed -i '/^# SSH/,+2 { /^# SSH/ b; s/^/#/; }' "$firewall_ipv6_rules"
    else
        msg_e "ERRO: $firewall_ipv6_rules não pode ser lido."
    fi
}


disable_casper_scripts() {
    for script in $disabled_casper_scripts; do
        chmod -x "$script" >>"$std_out" 2>>"$std_err"
    done

    msg_d "Atualizando o initrd..."
    update-initramfs -u >>"$std_out" 2>>"$std_err"
}

# Bloqueia os comandos não permitidos removendo a permissão de execução. Mais detalhes sobre isto
# na documentação disponível no Wiki do projeto.
block_commands() {
    for cmd in $blocked_commands; do
        if [ -f "$cmd" ]; then
            chmod o-x "$cmd" >>"$std_out" 2>>"$std_err"
        fi  
    done
}

# Verifica se os pacotes do Moodle Provas definidos na variável $pkg_provas, existe em /tmp.
check_provas_packages() {
    for pkg in $pkg_provas; do
        if ! ls -u "/tmp/$pkg"* >/dev/null 2>&1; then
            msg_e "ERRO: O pacote '$pkg' não existe em /tmp"
        fi
    done
}

# Função que deve ser executada no final do CHROOT, antes do cleanup_chroot().
finish_chroot() {
    msg_d "Verificando se o acesso administrativo deve estar ativado..."
    if [ "$enable_admin_access" = 'yes' ]; then
        enable_admin_access
    else
        disable_admin_access
    fi  

    msg_d "Removendo os kernels antigos..."
    remove_old_kernels

    msg_d "Removendo os kernel-headers..."
    apt-get -y purge linux-headers* >>"$std_out" 2>>"$std_err"

    msg_d "Removendo os pacotes desnecessários..."
    apt-get -y autoremove >>"$std_out" 2>>"$std_err"

    msg_d "Desabilitando alguns scripts do casper..."
    disable_casper_scripts

    msg_d "Bloqueando alguns comandos dentro do LiveCD..."
    block_commands
}
