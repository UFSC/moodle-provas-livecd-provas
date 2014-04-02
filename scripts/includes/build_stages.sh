#!/bin/bash

# Este arquivo contêm funções que são utilizadas em cada um dos três estágios de geração do LiveCD.



#  -------------------------------------
#  |           Estágio 1 de 3          |
#  -------------------------------------

# Prepara e/ou limpa os diretórios que serão utilizados para gerar o sistema
init() {
    if [ -d "$working_dir" ]; then
        # Oferece ao usuário a opção de limpar o diretório de trabalho
        msg_w "$prefix ATENÇÃO: O CONTEÚDO DO DIRETÓRIO $working_dir SERÁ APAGADO!!!"
        msg "$prefix Digite 'sim' para continuar ou <ENTER> para sair: " "-n"
        read option
        if [ ! "$option" = "sim" -o "$option" = "SIM" ]; then
            exit 0
        fi
    else
        mkdir "$working_dir"
    fi

    # Oferece ao usuário a opção de remover o cache de pacotes do apt-get
    if [ -d "$apt_cache_dir" ]; then
        msg_w "$prefix Deseja remover também o cache de pacotes baixados com o apt-get?"
        msg "$prefix Digite 'sim' para remover ou <ENTER> para mantê-lo: " "-n"
        read option
        if [ "$option" = "sim" -o "$option" = "SIM" ]; then
            msg_d "$sub_prefix Removendo o diretório $apt_cache_dir..."
            sudo rm -rf "$apt_cache_dir"
            mkdir "$apt_cache_dir"
        fi
    else
        mkdir "$apt_cache_dir"
    fi

    # Faz a limpeza do diretório de trabalho, removendo alguns diretórios e recriando-os
    msg_d "$sub_prefix Preparando o diretório de trabalho: $working_dir"
    sudo rm -rf "$root_iso" "$root_fs"
    mkdir "$root_iso" "$root_fs"
    mkdir "$root_iso/$squashfs_dir"
}


# Gera o pacote debian com as configurações do moodle provas, pacote moodle-provas-config.
make_config_pkg() {
    # Remove o diretório temporário se ele existir
    if [ -d "$working_dir/moodle-provas-config" ]; then
        rm -rf "$working_dir/moodle-provas-config"
    fi
    mkdir -p "$working_dir/moodle-provas-config"

    # Remove os pacotes antigos do moodle-provas-config se algum existir
    if ls -u "$pkgs_built_dir/moodle-provas-config"* >/dev/null 2>&1; then
        rm -rf "$pkgs_built_dir/moodle-provas-config"*
    fi

    msg_d "$sub_prefix Copiando src do pacote 'moodle-provas-config' para um diretório temporário..."
    cp -Rf "$pkgs_src_dir/moodle-provas-config" "$working_dir/"

    if [ -f "$config_dir/images/logout-banner.png" ]; then
        msg_d "$sub_prefix Atualizando a imagem do banner de logout..."
        cp -f "$config_dir/images/logout-banner.png" "$working_dir/moodle-provas-config/opt/provas/images/"
    fi

    if [ -f "$config_dir/images/wallpaper.png" ]; then
        msg_d "$sub_prefix Atualizando a imagem do papel de parede..."
        cp -f "$config_dir/images/wallpaper.png" "$working_dir/moodle-provas-config/opt/provas/images/"
    fi

    msg_d "$sub_prefix Copiando o arquivo de configuração interno do LiveCD..."
    cp -f "$provas_config_build" "$working_dir/moodle-provas-config/opt/provas/moodle_provas.conf"

    msg_d "$sub_prefix Gerando o novo pacote do 'moodle-provas-config'..."
    build_debian_pkg "true" "$working_dir/moodle-provas-config" "$pkgs_built_dir"
    
    msg_d "$sub_prefix Removendo os arquivos temporários do 'moodle-provas-config'..."
    rm -rf "$working_dir/moodle-provas-config"
}

# Prepara a base do sistema e adiciona algum arquivo que seja necessário antes de entrar no chroot,
# como por exemplo o arquivo /etc/apt/sources.list, essencial para o apt-get funcionar.
prepare_files() {
    init

    # Se o arquivo de bootstrap não existir, chama o script que gerará.
    if ! ls -u "$bootstrap_dir/base-${distro_codename}-$livecd_hw_arch"*".tar.gz" >/dev/null 2>&1; then
        msg_w "$prefix AVISO: Nenhum bootstrap para a distro '$distro_codename ($livecd_hw_arch)' foi encontrado, um novo bootstrap será gerado."
        source "$scripts_dir/build_bootstrap.sh"

        if [ "$?" -eq 1 ]; then
            return
        fi
    fi

    msg_d "$sub_prefix Extraindo o bootstrap para o root_fs em $root_fs"
    sudo tar xzvf "$bootstrap_dir/base-${distro_codename}-$livecd_hw_arch"*".tar.gz" -C "$root_fs/" >/dev/null 2>>"$std_err"

    make_sources_list "$root_fs/etc/apt"
    make_resolv_conf "$root_fs/etc"
    make_config_pkg
    sudo cp "$pkgs_built_dir/"*"all.deb" "$root_fs/tmp/"
    sudo cp "$pkgs_built_dir/"*"$livecd_hw_arch.deb" "$root_fs/tmp/"
}




#  -------------------------------------
#  |           Estágio 2 de 3          |
#  -------------------------------------

# Executa o script passado como primeiro parâmetro no ambiente chroot definido no arquivo de configuração.
# Nesse processo alguns sistemas de arquivos devem ser montados e o script deve ser copiado para dentro
# do chroot, no final os sistemas de arquivos são desmontados e os arquivos desnecessários são removidos.
chroot_run_script() {
    script_name="$1"

    # Copia os scripts de arquivos de configuração para dentro do CHROOT
    if [ -d "$root_fs/tmp/chroot" ]; then
        sudo rm -rf "$root_fs/tmp/chroot"
    fi
    msg_d "$sub_prefix Copiando os scripts de chroot e os includes para o diretório /tmp/chroot dentro do CHROOT..."
    sudo cp -R "$scripts_dir/chroot" "$root_fs/tmp/"
    sudo cp "$scripts_dir/includes/functions.sh" "$root_fs/tmp/chroot/includes/"
    sudo cp "$livecd_config" "$root_fs/tmp/chroot/"
    sudo cp "$provas_config_build" "$root_fs/tmp/chroot/"

    if ! $(mount | grep "$root_fs/var/cache/apt" >/dev/null 2>&1); then
        msg_d "$sub_prefix Montando o /var/cache/apt do root_fs..."
        sudo mount --bind "$apt_cache_dir" "$root_fs/var/cache/apt"
    fi

    if ! $(mount | grep "$root_fs/dev" >/dev/null 2>&1); then
        msg_d "$sub_prefix Montando o /dev em $root_fs/dev..."
        sudo mount --bind /dev/ "$root_fs/dev"
    fi

    # Passa o controle ao script especificado, que será executado no ambiente CHROOT
    msg_d "$sub_prefix Executando o script '$script_name' dentro do CHROOT"
    sudo chroot "$root_fs" /bin/bash -c "/tmp/chroot/$script_name"

    # Espera alguns segundos antes de tentar desmontar os dispositivos abaixo
    sleep 5

    if $(mount | grep "$root_fs/dev" >/dev/null 2>&1); then
        msg_d "$sub_prefix Desmontando o $root_fs/dev..."
        sudo umount "$root_fs/dev"
    fi 

    if $(mount | grep "$root_fs/var/cache/apt" >/dev/null 2>&1); then
        msg_d "$sub_prefix Desmontando $root_fs/var/cache/apt..."
        sudo umount "$root_fs/var/cache/apt"
    fi

    msg_d "$sub_prefix Removendo arquivos desnecessários do /var..."
    sudo rm -rf "$root_fs/var/cache/apt/"*
    sudo rm -rf "$root_fs/var/lib/apt/lists/"*
    sudo rm -rf "$root_fs/var/log/"*
}

chroot_install() {
    # Verifica se existe um root_fs extraído
    if ! is_root_fs_valid; then
        return
    fi

    chroot_run_script "install_packages.sh"
}

chroot_upgrade() {
    # Verifica se existe um root_fs extraído
    if ! is_root_fs_valid; then
        return
    fi 

    make_resolv_conf "$root_fs/etc"
    make_config_pkg
    sudo cp "$pkgs_built_dir/"*'.deb' "$root_fs/tmp/"

    chroot_run_script "upgrade_packages.sh"
}

chroot_shell() {
    # Verifica se existe um root_fs extraído
    if ! is_root_fs_valid; then
        return
    fi

    chroot_run_script "open_shell.sh"
}




#  -------------------------------------
#  |           Estágio 3 de 3          |
#  -------------------------------------

# Atualiza a versão do kernel utilizado no boot do livecd
update_kernel() {
    kernel_version="$(get_kernel_version)"

    if [ -z "$kernel_version" ]; then
        msg_e "$sub_prefix Erro ao obter a versão do kernel."
        return
    fi

    msg_d "$sub_prefix Atualizando o kernel do Live-CD para a versão: $kernel_version"
    sudo cp -f "$root_fs/boot/initrd.img-$kernel_version" "$root_iso/$squashfs_dir/initrd.lz"
    sudo cp -f "$root_fs/boot/vmlinuz-$kernel_version" "$root_iso/$squashfs_dir/vmlinuz"

    # Corrige o proprietário e grupo dos arquivos
    group=$(id -g -n $USER)
    sudo chown "$USER:$group" "$root_iso/$squashfs_dir/initrd.lz"
    sudo chown "$USER:$group" "$root_iso/$squashfs_dir/vmlinuz"
}

# Gera o menu de boot do livecd
# O primeiro parâmetro da função é o local onde o diretório 'isolinux' será colocado,
# geralmente é a raiz do LiveCD, mas pode ser um diretório utilizado apenas para testes.
make_bootmenu() {
    dest_dir="$1"

    # Se o tamanho do primeiro parâmetro da função estiver vazio, então o destino será o
    # diretório raiz do LiveCD, definido no arquivo de configuração.
    if [ -z "$1" ]; then
        dest_dir="$root_iso"
    fi

    # Copia o diretório do ISOLINUX para a raiz do LiveCD
    msg_d "$sub_prefix Copiando o bootloader..."
    if [ -d "$dest_dir/isolinux" ]; then
        rm -rf "$dest_dir/isolinux"
    fi
    cp -r "$bootloader_dir/isolinux" "$dest_dir/"

    # Habilita as opções de multiterminal no menu de boot
    if [ "$enable_multiseat" = "yes" ]; then
        sed -i '/^## MULTITERMINAL/,+8 { /^## MULTITERMINAL/ b; s/^#//; }' "$dest_dir/isolinux/menu.cfg.utf-8"
    fi

    # Habilita as opções de envio de logs no menu de boot.
    if [ "$enable_send_logs" = "yes" ]; then
        sed -i '/^## SEND_LOGS/,+12 { /^## SEND_LOGS/ b; s/^#//; }' "$dest_dir/isolinux/menu.cfg.utf-8"
    fi

    kernel_version="$(get_kernel_version)"
    if [ -z "$kernel_version" ]; then
        msg_w "$sub_prefix AVISO: Não foi possível obter a versão do kernel, talvez você não possua um root_fs válido."
    fi
    
    # Substitui as variáveis presentes nos arquivos de configuração
    msg_d "$sub_prefix Configurando o bootloader..."
    sed -i "s/%TIMEOUT%/$boot_timeout/g" "$dest_dir/isolinux/isolinux.cfg"

    sed -i "s/%MSG_TIMEOUT%/$msg_timeout/g" "$dest_dir/isolinux/menu.cfg.utf-8"
    sed -i "s/%MSG_BOTTOM%/$msg_bottom/g" "$dest_dir/isolinux/menu.cfg.utf-8"
    sed -i "s/%PROVAS_VERSION%/$provas_version/g" "$dest_dir/isolinux/menu.cfg.utf-8"

    sed -i "s/%PROVAS_VERSION%/$provas_version/g" "$dest_dir/isolinux/pt_BR.hlp.utf-8"
    sed -i "s/%BUILD_DATE%/$build_date/g" "$dest_dir/isolinux/pt_BR.hlp.utf-8"
    sed -i "s/%INSTITUTION_NAME%/$institution_name/g" "$dest_dir/isolinux/pt_BR.hlp.utf-8"
    sed -i "s/%HARDWARE_ARCH%/$livecd_hw_arch/g" "$dest_dir/isolinux/pt_BR.hlp.utf-8"

    if [ ! -z "$kernel_version" ]; then
        sed -i "s/%KERNEL_VERSION%/$kernel_version/g" "$dest_dir/isolinux/pt_BR.hlp.utf-8"
    fi
    
    # Copia os arquivos binários do ISOLINUX para o diretório 'isolinux' do LiveCD.
    cp /usr/lib/syslinux/isolinux.bin "$dest_dir/isolinux/" ||
        msg_e "$sub_prefix ERRO: Não foi possível copiar o arquivo /usr/lib/syslinux/isolinux.bin"
    cp /usr/lib/syslinux/vesamenu.c32 "$dest_dir/isolinux/" ||
        msg_e "$sub_prefix ERRO: Não foi possível copiar o arquivo /usr/lib/syslinux/vesamenu.c32"
    
    # Converte os arquivos de utf-8 para latin-1, para que sejam exibidos corretamente no menu de boot
    iconv -f "utf-8" -t "iso8859-1" "$dest_dir/isolinux/menu.cfg.utf-8" > "$dest_dir/isolinux/menu.cfg"
    iconv -f "utf-8" -t "iso8859-1" "$dest_dir/isolinux/pt_BR.hlp.utf-8" > "$dest_dir/isolinux/pt_BR.hlp"
    
    # Descomprime a fonte de console e salva no diretório 'isolinux' do LiveCD
    zcat "$console_font" > "$dest_dir/isolinux/font16.psf" || msg_e "$sub_prefix ERRO: Não foi possível extrair a fonte '$console_font'"

    # Copia uma nova imagem para o fundo do menu de inicialização, caso ela exista no diretório $config_dir
    if [ -f "$config_dir/images/bootloader_splash.png" ]; then
        cp -f "$config_dir/images/bootloader_splash.png" "$dest_dir/isolinux/splash.png"
    fi

    # Remove os arquivos desnecessários
    rm "$dest_dir/isolinux/menu.cfg.utf-8"
    rm "$dest_dir/isolinux/pt_BR.hlp.utf-8"
}

# Gera o novo SquashFS, juntamento com outros arquivos necessários para o seu funcionamento.
make_squashfs() {
    if ! is_root_fs_valid; then
        return
    fi

    msg "$sub_prefix Iniciando a geração do novo SquashFS..."

    msg_d "$sub_prefix Gerando o arquivo '$root_iso/$squashfs_dir/filesystem.manifest'..."
    sudo chroot "$root_fs" dpkg-query -W --showformat='${Package}\t${Version}\n' > "$root_iso/$squashfs_dir/filesystem.manifest"
    msg_d "$sub_prefix Gerando o novo SquashFS em: '$root_iso/$squashfs_dir/$squashfs_file'... " '-n'
    sudo mksquashfs "$root_fs" "$root_iso/$squashfs_dir/$squashfs_file" -noappend >>"$std_out" 2>>"$std_err" && msg_ok 'OK'

    msg_d "$sub_prefix Gerando o arquivo 'filesystem.size'..."
    du -sc --block-size=1 "$root_iso/casper/filesystem.squashfs" | cut -f1 | head -1 > "$root_iso/casper/filesystem.size"
}

# Gera o diretório '.disk' na raiz do LiveCD, ele é necessário para o perfeito funcionamento de alguns
# programas, como por exemplo o 'Startup Disk Creator', utilizado para gravar a ISO em um pendrive.
make_disk_info() {
    msg_d "$sub_prefix Gerando o diretório '.disk'..."
    if [ -d "$root_iso/.disk" ]; then
        rm -rf "$root_iso/.disk"
    fi
    mkdir "$root_iso/.disk"
    cd "$root_iso/.disk/"
    touch "base_installable"
    echo -n "full_cd/single" > 'cd_type'
    echo -n "$provas_homepage" > 'release_notes_url'
    
    enabled_options=''

    if [ "$enable_admin_access" = 'yes' ]; then
        enabled_options='admin'
    fi

    if [ "$enable_multiseat" = 'yes' ]; then
        enabled_options="$enabled_options multiseat"
    fi

    if [ "$enable_send_logs" = 'yes' ]; then
        enabled_options="$enabled_options send_logs"
    fi
 
    if [ -z "$enabled_options" ]; then
        enabled_options='None'
    fi

    echo -n "Moodle Provas $provas_version - Release $livecd_hw_arch (${build_date}-${build_time}) - Enabled options: $enabled_options" > info

    cd - >>"$std_out" 2>>"$std_err"
}

# Gera uma lista de arquivos com seu respectivo hash MD5, para que a integridade do LiveCD gravado
# possa ser verificada utilizando a opção disponível no menu de inicialização.
make_md5sum() {
    msg_d "$sub_prefix Gerando novo md5sum.txt..."
    rm -f "$root_iso/md5sum.txt"
    cd "$root_iso"

    # Gera o md5sum apenas para arquivos, exceto para os arquivos 'md5sum.txt' e 'isolinux.bin'
    find  -type f -not -name 'md5sum.txt' -not -name 'isolinux.bin' -print0 | xargs -0 md5sum | tee 'md5sum.txt' >>"$std_out" 2>>"$std_err"
    cd - >>"$std_out" 2>>"$std_err"
}

# Gera a ISO do LiveCD, mas antes atualiza o kernel do boot, gera o menu de boot e gera outros arquivos necessários.
make_iso() {
    if ! is_root_fs_valid; then
        return
    fi

    # Se o diretório de saída das ISOs geradas não existir...
    if [ ! -d "$iso_dir" ]; then
        mkdir "$iso_dir"
    fi 

    msg "$sub_prefix Iniciando a geração da nova ISO..."

    update_kernel
    make_bootmenu
    make_disk_info
    make_md5sum

    iso_name="$iso_name_base"

    # Se o $admin_user existir no /etc/passwd, é porque ele foi ativado via arquivo de configuração em algum momento,
    # só é possível desativá-lo alterando a opção no arquivo de configuração e executando a atualização dos pacotes.
    if $(grep -q "$admin_user" "$root_fs/etc/passwd" 2>/dev/null); then
        iso_name="${iso_name}_admin"
    fi

    if [ "$enable_multiseat" = 'yes' ]; then
        iso_name="${iso_name}_multiseat"
    fi

    if [ "$enable_send_logs" = 'yes' ]; then
        iso_name="${iso_name}_sendlogs"
    fi

    iso_name="$iso_name.iso"

    # Gera a nova ISO do Live-CD
    msg_d "$sub_prefix Gerando a nova ISO em: '$iso_dir/$iso_name'... " '-n'
    cd "$root_iso"
    mkisofs -input-charset "utf-8" -D -r -V "$iso_label" -cache-inodes -J -l -b isolinux/isolinux.bin -no-emul-boot -boot-load-size 4 -boot-info-table -o "$iso_dir/$iso_name" . >>"$std_out" 2>>"$std_out" && msg_ok 'OK'

    cd "$iso_dir"
    msg_d "$sub_prefix Gerando o arquivo $iso_name.md5 no mesmo diretório da ISO... " '-n'
    # O comando "${iso_name/iso/md5}" troca a extensão 'iso' por 'md5', este recurso é chamado de 'parameter substitution'.
    md5sum "$iso_name" > "${iso_name/iso/md5}" 2>>"$std_err" && msg_ok 'OK'

    check_iso_size "$iso_name"

    cd - >>"$std_out" 2>>"$std_err"
}
