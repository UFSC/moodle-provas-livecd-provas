#!/bin/bash

# Este script copia os loaders para boot via PXE do Syslinux.
# Configure a versão do Syslinux abaixo com a mais recente disponível.

syslinux_version='syslinux-6.03'
syslinux_file="${syslinux_version}.tar.xz"
url="https://www.kernel.org/pub/linux/utils/boot/syslinux/$syslinux_file"

bios_dir='bios'
efi32_dir='efi32'
efi64_dir='efi64'

loaders="$bios_dir $efi32_dir $efi64_dir"

if [ ! -f "$syslinux_file" ]; then
    wget "$url"
fi

rm -rf "$syslinux_version" "$bios_dir" "$efi32_dir" "$efi64_dir"
tar xJvf "$syslinux_file"

for loader in $loaders; do
    mkdir "$loader"
    cp $syslinux_version/$loader/com32/lib/libcom32.c32 "$loader"
    cp $syslinux_version/$loader/com32/libutil/libutil.c32 "$loader"
    cp $syslinux_version/$loader/com32/menu/vesamenu.c32 "$loader"
    cp $syslinux_version/$loader/com32/modules/reboot.c32 "$loader"
    ln -s '../pxelinux.cfg' "$loader/pxelinux.cfg"
    ln -s '../export' "$loader/export"

done

# BIOS
cp $syslinux_version/$bios_dir/core/pxelinux.0 "$bios_dir"
cp $syslinux_version/$bios_dir/com32/elflink/ldlinux/ldlinux.c32 "$bios_dir"

# EFI32
cp $syslinux_version/$efi32_dir/efi/syslinux.efi "$efi32_dir"
cp $syslinux_version/$efi32_dir/com32/elflink/ldlinux/ldlinux.e32 "$efi32_dir"

# EFI64
cp $syslinux_version/$efi64_dir/efi/syslinux.efi "$efi64_dir"
cp $syslinux_version/$efi64_dir/com32/elflink/ldlinux/ldlinux.e64 "$efi64_dir"

# Extra
#cp $syslinux_version/utils/pxelinux-options .

rm -rf "$syslinux_version" "$syslinux_file"
