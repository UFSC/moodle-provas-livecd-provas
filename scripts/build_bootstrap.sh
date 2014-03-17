#!/bin/bash
#set -x

# Este script gera uma versão atualizada do pacote .tar.gz que contém a base do sistema, também chamada de bootstrap.
# A base do sistema é gerada pelo comando 'debootstrap', que baixa e instala os pacotes essenciais para o sistema,
# exceto o kernel Linux que não faz parte desta base e é instalado em uma parte da geração do sistema.
# Este script deve ser inicializado somente a partir do 'main.sh'.


old_dir="$(pwd)"
output_filename="base-${livecd_hw_arch}_${build_date}.tar.gz"

# Verifica se o bootstrap já não foi atualizado no dia atual
if [ -f "$bootstrap_dir/$output_filename" ]; then
    msg_w "$prefix Já existe um bootstrap gerado hoje: '$output_filename', deseja realmente atualizá-lo? (a atualização pode demorar vários minutos)"
    msg "$prefix Digite 'sim' para continuar ou pressione <ENTER> para sair: " '-n'
    read option
    if ! [ "$option" = 'sim' -o "$option" = 'SIM' ]; then
        msg "$prefix Saindo..."
        return
    fi
else
    msg_w "$prefix A atualização do pacote .tar.gz do bootstrap pode demorar vários minutos, deseja continuar?"
    msg "$prefix Digite 'sim' para continuar ou pressione <ENTER> para sair: " '-n'
    read option
    if ! [ "$option" = 'sim' -o "$option" = 'SIM' ]; then
        msg "$prefix Saindo..."
        return
    fi
fi

msg "$prefix Iniciando a geração do bootstrap para a arquitetura '$livecd_hw_arch'"

# Devido a um bug no debootstrap, ele não completa a geração do bootstrap corretamente
# caso o caminho absoluto até o diretório de trabalho contenha algum espaço.
# Mais detalhes sobre o bug neste endereço: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=586158
if $(echo "$bootstrap_dir" | grep ' ' >/dev/null 2>&1); then
    msg
    msg_e "$sub_prefix O caminho absoluto do diretório de trabalho contêm 'espaços' no nome, não é possível continuar."
    msg
    return
fi

cd "$bootstrap_dir"
# Baixa e prepara a base do sistema, mais os pacotes definidos no parâmetro '--include'
msg_d "$sub_prefix Baixando a nova base utilizando o debootstrap (pode demorar vários minutos)..."
sudo debootstrap --arch="$livecd_hw_arch" --include=dnsutils precise base >>"$std_out" 2>>"$std_err"

msg_d "$sub_prefix Removendo o bootstrap antigo se existir..."
rm -rf "base-${livecd_hw_arch}_"*".tar.gz"
cd 'base'

msg_d "$sub_prefix Removendo arquivos desnecessários..."
sudo rm -rf var/cache/apt/archives/*
sudo rm -rf var/log/*

msg_d "$sub_prefix Comprimindo o novo bootstrap..."
sudo tar czvf "../$output_filename" * >/dev/null 2>>"$std_err"
cd ..

msg_d "$sub_prefix Removendo arquivos temporários..."
sudo rm -rf 'base'

msg_d "$sub_prefix Corrigindo o proprietário e grupo do arquivo gerado..."
group="$(id -g -n $USER)"
sudo chown "$USER:$group" "$output_filename"

if [ -f "$output_filename" ]; then
    bootstrap_size="$(du "$bootstrap_dir/$output_filename" | cut -f1)"
    bootstrap_size=$(($bootstrap_size/1024))

    msg_ok "$sub_prefix Novo bootstrap gerado em '$bootstrap_dir/$output_filename'"
    msg "$sub_prefix Tamanho do bootstrap gerado: $bootstrap_size MB"
else
    msg_e "$sub_prefix Erro ao gerar o novo bootstrap."
fi

cd "$old_dir"

