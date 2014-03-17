#!/bin/bash
#set -x

# Este script deve ser inicializado somente a partir do 'main.sh'.

pkg='lxpanel'
old_dir="$(pwd)"

if ! check_hw_arch; then
    msg_e "$prefix O pacote '$pkg' só pode ser gerado em um computador com a mesma arquitetura configurada no LiveCD: '$livecd_hw_arch', a arquitetura deste computador é '$(get_hw_arch)'."
    return
fi

msg "$prefix Iniciando a geração do pacote '$pkg'"

if [ -f "$working_dir/patch-$pkg" ]; then
    sudo rm -rf "$working_dir/patch-$pkg"
fi

cp -R "$pkgs_src_3rd_dir/patch-$pkg" "$working_dir/"
cd "$working_dir/patch-$pkg"

msg_d "$sub_prefix Obtendo source do pacote $pkg..."
apt-get source "$pkg" >>"$std_out" 2>>"$std_err"

msg_d "$sub_prefix Instalando as dependências de compilação do pacote $pkg..."
sudo apt-get build-dep -y "$pkg" >>"$std_out" 2>>"$std_err"

msg_d "$sub_prefix Extraindo o pacote $pkg... "
dpkg-source -x "$pkg"*".dsc" >>"$std_out" 2>>"$std_err"

msg_d "$sub_prefix Entrando no diretório extraido..."
cd "$pkg"*

msg_d "$sub_prefix Aplicando o patch1... "
patch src/panel.c ../panel.c.patch >>"$std_out" 2>>"$std_err"

msg_d "$sub_prefix Aplicando o patch2... "
patch src/plugin.c ../plugin.c.patch >>"$std_out" 2>>"$std_err"

msg_d "$sub_prefix Compilando e gerando o pacote, aguarde... "
sudo dpkg-buildpackage -rfakeroot -b >>"$std_out" 2>>"$std_err"

cd ..

msg_d "$sub_prefix Movendo o pacote para $pkgs_built_dir"
group=$(id -g -n $USER)
sudo chown "$USER:$group" "${pkg}_"*".deb"
mv "${pkg}_"*".deb" "$pkgs_built_dir/"

cd "$old_dir"
sudo rm -rf "$working_dir/patch-$pkg"

if ls -u "$pkgs_built_dir/$pkg"*"$livecd_hw_arch.deb" >/dev/null 2>&1; then
    msg_ok "$sub_prefix Pacote '$pkg' ($livecd_hw_arch) gerado com sucesso."
else
    msg_e "$sub_prefix Erro ao gerar o pacote '$pkg' ($livecd_hw_arch)."
fi

