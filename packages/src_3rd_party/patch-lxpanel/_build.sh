#!/bin/bash

pkg="lxpanel"
tmp="tmp_build"
pkgs_deb="../pacotes_gerados"

apt-get -y install build-essential fakeroot dpkg-dev >/dev/null 2>&1

if [ ! -d $pkgs_deb ]; then
    mkdir $pkgs_deb
fi

if [ -d $tmp ]; then
    rm -rf $tmp
fi
mkdir $tmp
cd $tmp

function get_error() {
    status=$1
    if [ ! $status -eq 0 ]; then
        exit 1
    fi
}

echo -n "==> Obtendo source do pacote $pkg... "
apt-get source $pkg >/dev/null 2>&1
get_error $? && echo "OK"

echo -n "==> Instalando as dependências de compilação do pacote $pkg... "
apt-get build-dep $pkg >/dev/null 2>&1
get_error $? && echo "OK"

echo -n "==> Extraindo o pacote $pkg... "
dpkg-source -x $pkg*.dsc >/dev/null 2>&1
get_error $? && echo "OK"
cd $pkg*

echo -n "==> Aplicando o patch1... "
patch src/panel.c ../../panel.c.patch >/dev/null 2>&1
get_error $? && echo "OK"
echo -n "==> Aplicando o patch2... "
patch src/plugin.c ../../plugin.c.patch >/dev/null 2>&1
get_error $? && echo "OK"

echo -n "==> Gerando os pacotes, aguarde... "
dpkg-buildpackage -rfakeroot -b >/dev/null 2>&1
get_error $? && echo "OK"

cd ..
mv $pkg*.deb ../$pkgs_deb
echo "==> Pacotes movidos para ../$pkgs_deb"
rm -rf $pkg*

