#!/bin/bash

# Este arquivo contêm funções de uso geral


# Função interna para exibir mensagens coloridas, é utilizada apenas pelas outras funções que iniciam por 'msg'.
# O primeiro parâmetro é a mensagem
# O segundo parâmetro são flags que podem ser passadas para o comando 'echo'
# O terceiro parâmetro é a cor da mensagem, que pode ser uma das variáveis definidas no começo deste arquivo.
__msg() {
    msg="$1"
    params="$2"
    color="$3"
    
    if [ "$chroot" = 'true' -o "$chroot" = 'TRUE' ]; then
        echo -ne "$prefix_chroot"
    fi
    
    if [ ! "$params" = "" ]; then
        echo -e "$params" "${color}${msg}${c_default}"
    else
        echo -e "${color}${msg}${c_default}"
    fi
}

# Exibe a mensagem com um leve destaque. Utiliza os mesmos parâmetros da função __msg().
msg() {
    __msg "$1" "$2" "$c_msg"
}

# Exibe a mensagem com a cor padrão (d: default). Utiliza os mesmos parâmetros da função __msg().
msg_d() {
    __msg "$1" "$2" "$c_default"
}

# Exibe a mensagem com um pouco mais de destaque (a: attention). Utiliza os mesmos parâmetros da função __msg().
msg_a() {
    __msg "$1" "$2" "$c_attention"
}

# Exibe a mensagem como aviso (w: warning). Utiliza os mesmos parâmetros da função __msg().
msg_w() {
    __msg "$1" "$2" "$c_warning"
}

# Exibe a mensagem como erro (e: error). Utiliza os mesmos parâmetros da função __msg().
msg_e() {
    __msg "$1" "$2" "$c_error"
}

# Exibe a mensagem como sucesso. Utiliza os mesmos parâmetros da função __msg().
msg_ok() {
    __msg "$1" "$2" "$c_ok"
}

msg_ch() {
    __msg "  * CHROOT * : " "-n" "$c_chroot"
    __msg "$1" "$2" "$c_msg"
}

# Verifica se o script foi executado com poder de root.
# Retorna '0' se sim e um valor diferente de zero se não.
is_root() {
    [ "$UID" -eq 0 ]
}

# Faz a limpeza quando o script é finalizado
cleanup() {
    status="$1"
    msg "$sub_prefix Saindo..."
#    umount "$root_fs/dev/pts"

    if $(mount | grep "$root_fs/dev" >/dev/null 2>&1); then
        msg_d "$sub_prefix Desmontando $root_fs/dev..."
        sudo umount -l "$root_fs/dev" >>"$std_out" 2>>"$std_err"
    fi

    if $(mount | grep "$root_fs/var/cache/apt" >/dev/null 2>&1); then
        msg_d "$sub_prefix Desmontando $root_fs/var/cache/apt..."
        sudo umount "$root_fs/var/cache/apt" >>"$std_out" 2>>"$std_err"
    fi

    msg_d "$sub_prefix Feito."
    msg
    exit $status
}

start_log() {
    if [ "$log_type" = 'NONE' ]; then
        std_out="/dev/null"
        std_err="/dev/null"
    elif [ "$log_type" = 'STDOUT_ALL' ]; then
        std_out="/dev/stdout"
        std_err="/dev/stderr"
    elif [ "$log_type" = 'ERRORS_ONLY' ]; then
        std_out="/dev/null"
        std_err="/dev/stderr"
    elif [ "$log_type" = 'FILE' ]; then
        if [ "$chroot" = "true" ]; then
            log_file="/tmp/${build_date}_$build_time.log"
        else
            if [ ! -d "$logs_dir" ]; then
                mkdir "$logs_dir"
            fi
        
            log_file="$logs_dir/${build_date}_$build_time.log"
        fi

        std_out="$log_file"
        std_err="$log_file"
    fi 
}

random_16() {
    cat /dev/urandom | tr -cd 'a-f0-9' | head -c 16
}

# Gera o arquivo sources.list que deve ser saldo em /etc/apt/
make_sources_list() {
    output_dir="$1"
    filename="sources.list"

    if [ ! -d "$output_dir" ]; then
        msg_e "$sub_prefix O diretório '$output_dir' não existe!"
        return
    fi

    msg_d "$sub_prefix Gerando o arquivo '$filename' no '/tmp'..."

    cat > "/tmp/$filename" <<-EOF
# main
deb http://br.archive.ubuntu.com/ubuntu/ $distro_codename main restricted multiverse universe
deb-src http://br.archive.ubuntu.com/ubuntu/ $distro_codename main restricted multiverse universe

# updates
deb http://br.archive.ubuntu.com/ubuntu/ $distro_codename-updates main restricted multiverse universe
deb-src http://br.archive.ubuntu.com/ubuntu/ $distro_codename-updates main restricted multiverse universe

# backports
deb http://br.archive.ubuntu.com/ubuntu/ $distro_codename-backports main restricted universe multiverse
deb-src http://br.archive.ubuntu.com/ubuntu/ $distro_codename-backports main restricted universe multiverse

# security
deb http://security.ubuntu.com/ubuntu $distro_codename-security main restricted universe multiverse
deb-src http://security.ubuntu.com/ubuntu $distro_codename-security restricted main multiverse universe

# partner
deb http://archive.canonical.com/ubuntu $distro_codename partner
deb-src http://archive.canonical.com/ubuntu $distro_codename partner
EOF

    msg_d "$sub_prefix Movendo o arquivo '$filename' para o '$output_dir'..."
    sudo mv "/tmp/$filename" "$output_dir/$filename"

    if [ -f "$output_dir/$filename" ]; then
        msg_ok "$sub_prefix Arquivo $output_dir/$filename gerado com sucesso."
    else
        msg_e "$sub_prefix Falha ao gerar o arquivo '$output_dir/$filename'."
    fi
}

check_dependencies() {
    must_install=""

    for dependency in $build_dependencies; do
        if ! $(dpkg -s "$dependency" >/dev/null 2>&1); then
            must_install="$must_install $dependency"
        fi
    done

    echo "$must_install"
}

install_dependencies() {
    # Se este arquivo existir é porque as dependências já estão instaladas
    if [ -f "$working_dir/OK_DEPENDENCIES" ]; then
        return
    elif [ ! -d "$working_dir" ]; then
        mkdir "$working_dir"
    fi

    must_install=$(check_dependencies)

    if [ ! -z "$must_install" ]; then
        msg_w "$sub_prefix AVISO: Os seguintes pacotes são necessários para que este sistema funcione corretamente, deseja instalá-los?"
        msg "$sub_prefix Pacotes faltantes: $must_install"
        msg "$sub_prefix Digite 'sim' para instalá-los ou pressione <ENTER> para sair: " "-n"

        read option
        if [ "$option" = "sim" -o "$option" = "SIM" ]; then
            msg_d "$sub_prefix Executando o apt-get update..."
            sudo apt-get update >>"$std_out" 2>>"$std_err"
            msg_d "$sub_prefix Executando o apt-get install $must_install... "
            sudo apt-get install -y $must_install

            must_install=$(check_dependencies)
            if [ -z "$must_install" ]; then
                msg_ok "$sub_prefix Pacotes instalados com sucesso."
                touch "$working_dir/OK_DEPENDENCIES"
            else
                msg_e "$sub_prefix Erro ao instalar os pacotes"
                exit 1
            fi
        else
            msg_w "$sub_prefix AVISO: Você escolheu não instalar os pacotes, não é possível continuar."
            exit 1
        fi
    else
        touch "$working_dir/OK_DEPENDENCIES"
    fi
}

get_dns_list() {
    dns_list_tmp=$(nm-tool | grep DNS | mawk '{ print $2 }')
    dns_list=""

    for dns in $dns_list_tmp; do
        if [ -z "$dns_list" ]; then
            dns_list="$(echo $dns | tr -d '\n')"
        else
            dns_list="$dns_list $dns"
        fi
    done

    echo "$dns_list"
}

make_resolv_conf() {
    output_dir="$1"
    filename="resolv.conf"

    if [ ! -d "$output_dir" ]; then
        msg_e "$sub_prefix O diretório '$output_dir' não existe!"
        return
    fi  

    msg_d "$sub_prefix Gerando o arquivo '$filename' no /tmp"
    for name_server in $(get_dns_list); do
        rm -rf "/tmp/$filename" >>"$std_out" 2>>"$std_err"
        echo "nameserver $name_server" >> "/tmp/$filename"
    done

    msg_d "$sub_prefix Movendo o arquivo '$filename' para o $output_dir"
    sudo mv "/tmp/$filename" "$output_dir/$filename"

    if [ -f "$output_dir/$filename" ]; then
        msg_ok "$sub_prefix Arquivo $output_dir/$filename gerado com sucesso."
    else
        msg_e "$sub_prefix Falha ao gerar o arquivo '$output_dir/$filename'."
    fi
}

build_debian_pkg() {
    update_version="$1"
    package_src="$2"
    package_dst="$3"
    package_name="${package_src##*/}"

    if [ -e "$package_src/DEBIAN/control" ]; then
        # Se a opção de atualizar a versão do pacote estiver ativada...
        if [ "$update_version" = "true" -o "$update_version" = "TRUE" ]; then
            new_build="${build_date}01"
            #version=$(grep "Version:" "$package_src/DEBIAN/control" | awk '/Version:/{print $2}' | cut -d "-" -f 1)
            version="$provas_version"
            old_build=$(grep "Version:" "$package_src/DEBIAN/control" | awk '/Version:/{print $2}' | cut -d "-" -f 2)

            # Se estiver gerando uma nova versão no mesmo dia, não precisará incrementar até chegar a versão atual.
            if [ "$new_build" -le "$old_build" ]; then
                new_build="$old_build"
            fi

            # Incrementa a versão do novo pacote até ela ser maior que a versão atual
            while [ "$new_build" -le "$old_build" ]; do
                ((new_build++))
            done

            # Atualiza a versão do pacote
            sed -i "s/^Version:.*\$/Version: $version-$new_build/g" "$package_src/DEBIAN/control"
        fi

        msg_d "$sub_prefix Gerando o pacote '$package_name'... " -n
        fakeroot dpkg-deb -b "$package_src" "$package_dst" >>"$std_out" 2>>"$std_err"

        if ls -u "$package_dst/$package_name"*"$livecd_hw_arch.deb" >/dev/null 2>&1 ||
           ls -u "$package_dst/$package_name"*"all.deb" >/dev/null 2>&1; then
            msg_ok "OK"
        else
            msg_e "*** Erro ao gerar o pacote '$package_name'"
            return 1
        fi
    fi  
}

is_root_fs_valid() {
    if [ ! -d "$root_fs/etc" ] && [ ! -d "$root_fs/bin" ] && [ ! -d "$root_fs/lib" ]; then
        msg_e "$sub_prefix ERRO: Um root_fs válido não foi encontrado em $root_fs"
        return 1
    fi

    return 0
}

check_iso_size() {
    iso_file="$1"
    iso_size=""

    if [ -f "$iso_file" ]; then
        iso_size=$(du "$iso_file" | cut -f1)  # A opção -h retorna apenas M ou K para indicar MB e KB.
        iso_size=$(($iso_size/1024))  # transforma KB em MB
    else
        msg_e "$sub_prefix ERRO: Não foi possível acessar o arquivo $iso_file"
    fi

    if [ "$iso_size" -gt "$iso_size_default" ]; then
        msg "$sub_prefix Tamanho da ISO gerada: $iso_size MB"
        return 0
    else
        msg_w "$sub_prefix AVISO: O tamanho da ISO gerada é $iso_size MB, este valor está abaixo do valor mínimo esperado que é $iso_size_default MB, verifique se não ocorreram erros na geração do LiveCD."
        return 1
    fi
}

start_msg() {
    msg
    msg
    msg_a "$prefix Processo iniciado em: $(date)"
}

end_msg() {
    msg_a "$prefix Processo finalizado em: $(date)"
    msg
    msg
}

# Verifica se a distribuição Linux (sistema operacional) é homologada para o sistema de geração do LiveCD.
check_distro_compatibility() {
    # Verifica se o comando 'lsb_release' existe
    if ! $(command -v lsb_release >/dev/null 2>&1); then
        msg_e "$prefix O utilitário 'lsb_release' não foi localizado, provelmente sua distribuição não esteja homologada."
        msg_w "$prefix A distribuição homologada é: $distro_ubuntu_name, versões: $distro_ubuntu_versions"
        exit 1
    fi

    distro_name=$(lsb_release -a 2>/dev/null | grep 'Distributor ID' | cut -f2)
    distro_version=$(lsb_release -a 2>/dev/null | grep 'Release' | cut -f2)
    system_version=""

    if [ "$distro_name" = "$distro_ubuntu_name" ]; then
        for version in $distro_ubuntu_versions; do
            if [ "$distro_version" = "$version" ]; then
                system_version="$version"
                break
            fi
        done
    else
        msg_w "$prefix A distribuição '$distro_name' não é homologada, a geração do LiveCD pode não funcionar corretamente."
        msg_w "$prefix Sistema detectado: '$distro_name' versão '$distro_version'"
        msg_w "$prefix A distribuição homologada é: $distro_ubuntu_name, versões: $distro_ubuntu_versions"
        return
    fi

    if [ -z "$system_version" ]; then
        msg_w "$prefix A versão '$distro_version' do sistema '$distro_name' não é homologada, a geração do LiveCD pode não funcionar corretamente."
        msg_w "$prefix A distribuição homologada é: $distro_ubuntu_name, versões: $distro_ubuntu_versions"
    else
        msg "$prefix Sistema homologado detectado: '$distro_name' versão '$system_version'"
    fi
}

get_hw_arch() {
    current_arch=$(uname -i)

    # Se a arquitetura for 'x86_64', então current_arch é 'amd64', para ser compatível com o arquivo de configuração.
    [ "$current_arch" = 'x86_64' ] && current_arch='amd64'

    echo "$current_arch"
}

# Retorna '0' se a arquitetura do hardware do sistema atual for a mesma do LiveCD que será gerado.
check_hw_arch() {
    current_arch=$(get_hw_arch)

    if [ "$current_arch" = "$livecd_hw_arch" ]; then
        return 0
    else
        return 1
    fi
}

get_kernel_version() {
    if ls -u "$root_fs/boot/vmlinuz"* >/dev/null 2>&1; then
        kernel_version=$(ls "$root_fs/boot/vmlinuz"* | tail -1 | xargs basename | cut -c 9-)
    else
        kernel_version=''
    fi

    echo "$kernel_version"
}

