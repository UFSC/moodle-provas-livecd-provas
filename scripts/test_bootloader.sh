#!/bin/bash
#set -x

# Este script gera uma pequena ISO temporária contendo o ISOLINUX, apenas para fins de teste,
# dessa forma as alterações na configuração do menu de inicialização podem ser testadas rapidamente.
# Este script deve ser inicializado somente a partir do 'main.sh'.


msg "$prefix Iniciando a geração de uma pequena ISO de teste do bootloader (ISOLINUX)"

test_bootloader="$working_dir/test_bootloader"

if [ -d "$test_bootloader" ]; then
    rm -rf "$test_bootloader"
fi

mkdir -p "$test_bootloader"
make_bootmenu "$test_bootloader"
cd "$test_bootloader"

msg_d "$sub_prefix Gerando a ISO de teste do bootloader... " '-n'
mkisofs -input-charset "utf-8" -D -r -V "Teste_ISO" -cache-inodes -J -l -b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -o "test_bootloader.iso" . >>"$std_out" 2>>"$std_out" && msg_ok 'OK'

if [ -f "$test_bootloader/test_bootloader.iso" ]; then
    msg_d "$sub_prefix A ISO foi gerada em $test_bootloader/test_bootloader.iso"
    msg_d "$sub_prefix Iniciando o teste da ISO no Qemu..."
    qemu-system-i386 -cdrom test_bootloader.iso >>"$std_out" 2>>"$std_out" &
    
    msg_w "$sub_prefix Após testar o menu de inicialização, pressione <ENTER> para remover os arquivos temporários: " '-n'
    read option
    msg_d "$sub_prefix Removendo os arquivos temporários..."
    rm -rf "$test_bootloader"

    msg_d "$sub_prefix Teste concluído."
else
    msg_e "$sub_prefix Erro ao gerar a ISO de teste do bootloader"
fi

# Volta para o diretório anterior
cd - >>"$std_out" 2>>"$std_err"

