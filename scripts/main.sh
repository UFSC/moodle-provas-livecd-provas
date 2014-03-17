#!/bin/bash
#set -x

# Script principal de geração do LiveCD, requer os parâmetros descritos abaixo.
# Deve ser inicializado preferencialmente através do 'Makefile'

# Algumas opções requerem poder de root, 'sudo -v' obtem previamente esse poder e mantém em cache.
sudo -v

# Os comandos abaixo verificam se os parâmetros requeridos foram passados e se são válidos
# O script deve receber como primeiro parâmetro o caminho absoluto para o diretório que contém o
# sistema de geração de LiveCD, ou seja, o diretório que contém o 'Makefile' e o diretório 'scripts', entre outros.
if [ ! "$#" -eq 1 ]; then
    echo "==> ERRO: Você deve informar o diretório raiz do sistema de geração do LiveCD!"
    echo "      Uso: $0 <root_dir>"
    exit 1
elif [ ! -d "$1" ]; then
    echo "==> ERRO: O diretório $1 não existe!"
    exit 1
fi


root_dir="$1"
includes_dir="$root_dir/scripts/includes"

#####  Includes  #####
livecd_config="$root_dir/config/livecd_provas.conf"
functions_file="$includes_dir/functions.sh"
build_stages_file="$includes_dir/build_stages.sh"

[ -r "$livecd_config" ] && source "$livecd_config" || exit 1
[ -r "$functions_file" ] && source "$functions_file" || exit 1
[ -r "$build_stages_file" ] && source "$build_stages_file" || exit 1

# Gatilho para acionar a limpeza caso o script seja interrompido de alguma forma
trap "cleanup $?" HUP INT TERM KILL QUIT EXIT

# Verifica se o sistema onde o script foi executado é compatível
check_distro_compatibility


# Alguns utilitários do sistema podem não conseguir tratar adequadamente caminhos que possuam espaço no nome,
# podendo gerar resultados inesperados e falha na geração do LiveCD, você deve corrigir isto antes de continuar.
# Em geral os utilitários funcionam corretamente, porém alguma versão mais recente pode conter este problema,
# o utilitário 'debootstrap' possui este problema em várias versões.
if $(echo "$root_dir" | grep ' ' >/dev/null 2>&1); then
    msg_e "$prefix ERRO: O caminho absoluto do diretório raiz contêm 'espaços' no nome, isto pode causar problemas na geração do LiveCD, você deve corrigir este problema antes de continuar!"
    msg "$prefix Caminho problemático: '$root_dir'"
    exit 1
fi


# Verifica o script tem poder de root, pois não é necessário executá-lo diretamente com este poder,
# os comandos que requerem poder de root são executar utilizando o comando 'sudo'.
if is_root; then
    msg
    msg_w "AVISO: Não é recomendável executar este script com poder de root diretamente, as opções que requerem poder de root são executadas através do comando 'sudo'."
    msg
fi

# Inicializa os arquivos de log se estiver ativado no arquivo de configuração
start_log

# Oferece a opção de instalar todas dependências do sistema caso elas não estejam instaladas.
install_dependencies


show_main_menu() {
    msg 
    msg "$prefix Qual ação deseja executar? (Arquitetura: $livecd_hw_arch)"
    msg
    msg "   [1] Gerar um LiveCD com os pacotes do moodle-provas"
    msg "   [2] Atualizar os pacotes de um LiveCD já extraído"
    msg "   [3] Abrir um shell no chroot do LiveCD já extraído"
    msg "   [4] Gerar apenas o novo SquashFS e a nova ISO do LiveCD"
    msg "   [5] Gerar apenas a nova ISO do LiveCD"
    msg "   ---------------------------------------------"
    msg "   [6] Gerar novamente os pacote debian do Moodle Provas"
    msg "   [7] Gerar novamente os pacotes debian do lxpanel e lxsession (com os patches)"
    msg "   [8] Gerar novamente o pacote debian do JRE da Oracle (Java)"
    msg "   [9] Gerar novamente o pacote .tar.gz do bootstrap (base do sistema)"
    msg "   ---------------------------------------------"
    msg "   [10] Testar o bootloader (ISOLINUX) em uma máquina virtual (Qemu)"
    msg "   [11] Testar a última ISO gerada em uma máquina virtual (VirtualBox)"
    msg "   ============================================="
    msg "   [0] Sair"
    msg
}

# Exibe o menu principal com as opções disponíveis para geração do LiveCD
while true; do
    show_main_menu

    msg "Digite o número da opção desejada: " "-n"
    read option
    msg "" # Linha em branco

    case $option in
        # [1] Gerar um LiveCD com os pacotes do moodle-provas
        1 )
            sudo -v
            start_msg
            prepare_files
            chroot_install
            make_squashfs
            make_iso
            end_msg
            ;;

        # [2] Atualizar os pacotes de um LiveCD já extraído
        2 )
            sudo -v
            start_msg
            chroot_upgrade
            make_squashfs
            make_iso
            end_msg
            ;;

        # [3] Abrir um shell no chroot do LiveCD já extraído
        3 )
            sudo -v
            start_msg
            chroot_shell
            end_msg
            ;;

        # [4] Gerar apenas o novo SquashFS e a nova ISO do LiveCD
        4 )
            sudo -v
            start_msg
            make_squashfs
            make_iso
            end_msg
            ;;

        # [5] Gerar apenas a nova ISO do LiveCD"
        5 )
            sudo -v
            start_msg
            make_iso
            end_msg
            ;;

        # [6] Atualizar os pacote debian do Moodle Provas"
        6 )
            start_msg
            source "$scripts_dir/build_pkgs.sh"
            end_msg
            ;;

        # [7] Gerar novamente os pacotes debian do lxpanel e lxsession (com os patches)
        7 )
            sudo -v
            start_msg
            source "$scripts_dir/build_lxpanel_pkg.sh"
            source "$scripts_dir/build_lxsession_pkg.sh"
            end_msg
            ;;

        # [8] Atualizar o pacote debian do JRE da Oracle (Java)"
        8 ) 
            start_msg    
            source "$scripts_dir/build_jre_pkg.sh"
            end_msg
            ;; 

        # [9] Atualizar o pacote .tar.gz do bootstrap (base do sistema)"
        9 )
            sudo -v
            start_msg
            source "$scripts_dir/build_bootstrap.sh"
            end_msg
            ;;

        # [10] Testar o bootloader (ISOLINUX) em uma máquina virtual (Qemu)"
        10 )
            start_msg
            source "$scripts_dir/test_bootloader.sh"
            end_msg
            ;;

        # [11] Testar a última ISO gerada em uma máquina virtual (VirtualBox)"
        11 )
            start_msg
            source "$scripts_dir/test_livecd_iso.sh"
            end_msg
            ;;

        # [0] Sair
        0 )
            exit 0
            ;;
        * )
            msg
            msg_w " ** AVISO: A opção digitada é inválida: '$option', tente novamente ** "
            msg
            ;;
    esac
done
