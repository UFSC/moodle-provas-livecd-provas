#!/bin/bash
#set -x

# Description: Gera um papel de parede com o nome da instituição e/ou a versão do Moodle Provas.
# Listar as fontes disponíveis: $ convert -list font

provas_config='/opt/provas/moodle_provas.conf'
[ -r "$provas_config" ] && source "$provas_config" || exit 1
[ -r "$provas_online_config" ] && source "$provas_online_config"

functions_file="$provas_dir/includes/functions.sh"
[ -r "$functions_file" ] && source "$functions_file" || exit 1


if [ $# -lt 2 ]; then
    echo "Você deve informar pelo menos dois parâmetros"
    echo "   Uso: $0 <order> <set_wallpaper> [<wallpaper_path>]"
    echo "   order: 1=moodle_version, 2=institution_name, 3=both"
    echo "   set_wallpaper: no | yes"
    exit
fi

# 1=moodle_version, 2=institution_name, 3=both
order="$1"

# Deve configurar o papel de parede do usuário do LXDE? 'yes' ou 'no'
# TODO: O padrão poderia ser 'no'
set_wallpaper="$2"

# Caminho onde o arquivo será salvo (opcional, se não definido, usará o caminho padrão do LiveCD)
wallpaper_path="$3"

# Se o caminho não foi informado como parâmetro...
if [ -z "$wallpaper_path" ]; then
    wallpaper_orig="$provas_wallpaper_path"
else
    wallpaper_orig="$wallpaper_path"
fi

# Se já existe uma cópia de backup do arquivo, utiliza ela como base, senão gera uma cópia de backup.
if [ -f "$wallpaper_orig.bak" ]; then
    cp -f "$wallpaper_orig.bak" "$wallpaper_orig"
else
    cp -f "$wallpaper_orig" "$wallpaper_orig.bak"
fi

wallpaper_in="$wallpaper_orig"
wallpaper_out="$wallpaper_in"


# Adiciona a versão do Moodle Provas ao papel de parede
if [ "$order" = "1" ] || [ "$order" = "3" ]; then
    text="Moodle Provas LiveCD $provas_version Build $provas_build"
    font_size='18'
    font_color='white'
    #font_name='Helvetica'
    #font_name='Arial-Regular'
    #font_name='FreeSans-Medium'
    #font_name='Verdana-Regular'
    #font_name='Courier-New-Bold'
    #font_name='Courier-10-Pitch-Bold'
    #font_name='DejaVu-Sans-Bold'
    font_name='DejaVu-Sans-Mono-Bold'
    position='southeast'
    h_offset='+25'
    v_offset='+50'
    offset="${h_offset}${v_offset}"

    convert "$wallpaper_in" -font "$font_name" -pointsize "$font_size" -gravity "$position" \
        -stroke '#000C' -strokewidth 2 -annotate "$offset" "$text" \
        -stroke none -fill "$font_color" -annotate "$offset" "$text" "$wallpaper_out"
fi

# Adiciona o nome da instituição ao papel de parede
if [ "$order" = "2" ] || [ "$order" = "3" ]; then
    if [ -z "$institution_name" ]; then
        echo "Erro: A variável 'institution_name' não está definida, talvez o arquivo $provas_online_config não exista."
        exit 1
    fi

    text="$institution_name"
    font_size='28'
    font_color='white'
    #font_name='Helvetica'
    #font_name='Arial-Regular'
    #font_name='FreeSans-Medium'
    #font_name='Verdana-Regular'
    #font_name='Courier-New-Bold'
    #font_name='Courier-10-Pitch-Bold'
    font_name='DejaVu-Sans-Bold'
    #font_name='DejaVu-Sans-Mono-Bold'
    position='southeast'
    h_offset='+25'
    v_offset='+75'
    offset="${h_offset}${v_offset}"

    convert "$wallpaper_in" -font "$font_name" -pointsize "$font_size" -gravity "$position" \
        -stroke '#000C' -strokewidth 2 -annotate "$offset" "$text" \
        -stroke none -fill "$font_color" -annotate "$offset" "$text" "$wallpaper_out"
fi

# Define o papel de parede pra sessão dos usuários
if [ "$set_wallpaper" = "yes" ]; then
    export LANG="pt_BR.UTF-8"
    export XAUTHORITY="/home/${username_base}1/.Xauthority"
    export DISPLAY=":1"
    su - "${username_base}1" -c "pcmanfm --set-wallpaper=\"$wallpaper_out\" --wallpaper-mode=stretch"

    if is_multiseat; then
        export LANG="pt_BR.UTF-8"
        export XAUTHORITY="/home/${username_base}2/.Xauthority"
        export DISPLAY=":2"
        su - "${username_base}2" -c "pcmanfm --set-wallpaper=\"$wallpaper_out\" --wallpaper-mode=stretch"
    fi
fi

