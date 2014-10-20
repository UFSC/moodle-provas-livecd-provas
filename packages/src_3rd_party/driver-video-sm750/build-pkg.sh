#!/bin/bash

pkg_src_dir='/tmp/driver-video-sm750'
pkg_output_dir='/tmp'

apt-get update
apt-get -y install xserver-xorg-dev-lts-saucy xutils-dev libpci-dev gawk autoconf build-essential libtool automake pkg-config fakeroot


cd /tmp/siliconmotion_4.0.11
make distclean
sh ./autogen.sh
./configure --prefix="$pkg_src_dir/usr" --disable-smirandr
make && make install prefix="$pkg_src_dir/usr"

if [ -e "$pkg_src_dir/DEBIAN/control" ]; then
    build_date=$(date +%Y%m%d)
    version="4.0.11-${build_date}01"

    # Atualiza a versão do pacote
    sed -i "s/^Version:.*\$/Version: $version/g" "$pkg_src_dir/DEBIAN/control"
else
    echo "ERRO: Arquivo '$pkg_src_dir/DEBIAN/control' não encontrado!"
    exit 1
fi

cd "$pkg_src_dir/usr/lib/xorg/modules/drivers"
rm -rf *.la
chmod -x *.so
cd -

echo "- Gerando o pacote em $pkg_output_dir... "
fakeroot dpkg-deb -b "$pkg_src_dir" "$pkg_output_dir"


#apt-get -y autoremove xserver-xorg-dev-lts-saucy xutils-dev libpci-dev gawk autoconf build-essential libtool automake pkg-config fakeroot
