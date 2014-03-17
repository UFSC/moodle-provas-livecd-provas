#!/bin/bash

# Funções de uso geral


# Registra no log a informação passada como parâmetro
# Data no formato: 2013-05-29 11:52:43,815 , onde 815 são os milisegundos.
log() {
    # O parâmetro '${0##*/}' insere o nome do arquivo que fez a chamada para a função
    log_msg="${0##*/} $1"
    date="$(date +"%F %T,%N")"
    echo "${date:0:23} $log_msg" >> $log_file_multiseat
}

# Retorna a quantidade de placas de vídeo detectadas, requer o comando 'lspci'.
get_qtd_vgas() {
    # Algumas placas de vídeo são identificadas como 'VGA' e outras como 'Display'.
    vga_count="$(lspci | grep 'VGA\|Display' | wc -l)"

    echo "$vga_count"
}

# Esta função obtem o bus_id da placa de video especificada no formato requerido pelo Xorg:
#
# Exemplo: "00:10.0" (lspci) ==> "PCI:0:16:0" (Xorg)
#          "02:10.0" (lspci" ==> "PCI:2:0:0" (Xorg)
#
# Observe que o '10' do primeiro exemplo se transformou em '16', isso acontece
# porque o lspci mostra o bus id em hexadecimal e o Xorg precisa dele em decimal.
get_vga_bus_id() {
    # Posição da placa de vídeo que deve fornecer o BusId
    # Valores possíveis: 1 ou 2, se o valor não for passado ele será '-1'.
    vga=${1:-'-1'}

    # Número de placas de vídeo detectadas
    vga_count="$(get_qtd_vgas)"

    if [ "$vga_count" -lt "$vga" ]; then
        log "get_vga_bus_id() ERRO: Número da vga inválido, foram encontradas somente $vga_count placa(s) de vídeo."
        echo -1
        return
    fi

    if [ "$multiseat_switch_vgas" = 'yes' ]; then
        log "get_vga_bus_id() A opção 'switch_vgas' está ativada."
        if [ "$vga" = '1' ]; then
            vga='2'
        elif [ "$vga" = '2' ]; then
            vga='1'
        fi
    fi

    # Obtem o BusId da vga especificada
    if [ "$vga" = "1" ]; then
        bus_id="$(lspci | grep 'VGA\|Display' | cut -d ' ' -f 1 | sed -n '1p' | tr '.' ':')"
    elif [ "$vga" = "2" ]; then
        bus_id="$(lspci | grep 'VGA\|Display' | cut -d ' ' -f 1 | sed -n '2p' | tr '.' ':')"
    else
        log "get_vga_bus_id() ERRO: Número da vga inválido, o número da vga só pode ser 1 ou 2."
        echo -1
        return
    fi

    # Converte os valores em Hexadecimal para Decimal e imprime na saída padrão o BusId desejado.
    # Só é compatível com o mawk, não é compatível com o gawk.
    echo "$bus_id" | mawk -F: '{ p1 = "0x" $1;
                                p2 = "0x" $2;
                                p3 = "0x" $3;
                                printf "PCI:%d:%d:%d\n", p1, p2, p3;
                              }'
}

# Esta função substitui o BusId e o ID do seat no arquivo padrão do Xorg.
make_xorg_config() {
    # ID do seat
    seat=$1

    log "make_xorg_config() Iniciando a geração dos arquivos '/etc/X11/xorg-seat${seat}.conf' e '/etc/X11/xorg-config${seat}.conf..."
    # Testa se o primeiro parâmetro é um número entre 1 e 2
    if ! [ "$seat" = '1' -o "$seat" = '2' ]; then
        log 'make_xorg_config() ERRO: Você deve especificar o número do seat como primeiro parâmetro, valores válidos: 1 e 2.'
        exit 1
    fi

    vga_count="$(get_qtd_vgas)"
    log "make_xorg_config() Número de placas de vídeo detectadas: $vga_count"

    bus_id="$(get_vga_bus_id "$seat")"
    log "make_xorg_config() BusId da placa de vídeo que será associada ao seat ${seat}: '$bus_id'"

    if [ "$bus_id" = "-1" ]; then
        log "make_xorg_config() ERRO: Não foi possível obter o BusId da placa de vídeo do seat '$seat'."
        exit 1
    fi

    # Arquivos .tpl são os modelos de configuração do Xorg
    # xorg-seat#.conf.tpl é o arquivo principal e xorg-config#.conf.tpl é o arquivo usado pelo assistente de configuração.
    # O caracter '#' é substituído pelo número do seat.
    files='xorg-seat#.conf.tpl xorg-config#.conf.tpl'
    X11_dir='/etc/X11'
    templates_dir="$provas_dir/multiseat/templates"

    # Para cada arquivo modelo, cria uma cópia em /etc/X11 e altera os parâmetros necessários.
    for file in $files; do
        new_file=$(echo "$file" | sed 's/.tpl//' | sed "s/#/$seat/")
        log "make_xorg_config() Gerando o arquivo '$X11_dir/$new_file'..."
        if [ -r "$templates_dir/$file" ]; then
            if [ -e "$X11_dir/$new_file" ]; then
                log "make_xorg_config() AVISO: O arquivo '$X11_dir/$new_file' já existe e será renomeado para '$X11_dir/$new_file.bak'."
                mv "$X11_dir/$new_file" "$X11_dir/$new_file.bak"
            fi
            cp "$templates_dir/$file" "$X11_dir/$new_file"
            sed -i "s/__BUSID__/$bus_id/g" "$X11_dir/$new_file"
            sed -i "s/__SEAT_ID__/$seat/g" "$X11_dir/$new_file"
            log "make_xorg_config() Arquivo '$X11_dir/$new_file' gerado."
        else
            log "make_xorg_config() ERRO: O arquivo '$templates_dir/$file' não existe!"
            exit 1
        fi
    done
}

# Prepara o servidor de áudio Pulseaudio para funcionar com multiseat
prepare_pulseaudio() {
    log 'prepare_pulseaudio() Iniciando a configuração do Pulseaudio...'
    pulseaudio_default="/etc/default/pulseaudio"
    pulseaudio_client="/etc/pulse/client.conf"

    if [ -w "$pulseaudio_default" ]; then
        log "prepare_pulseaudio() Configurando a opção 'PULSEAUDIO_SYSTEM_START' com o valor '1' no arquivo $pulseaudio_default"
        sed -i 's/PULSEAUDIO_SYSTEM_START=0/PULSEAUDIO_SYSTEM_START=1/g' "$pulseaudio_default"
    fi

    log "prepare_pulseaudio() Adicionando os usuários 'root', '${username_base}1' e '${username_base}2' ao grupo 'pulse-access'"
    usermod -a -G pulse-access root >/dev/null 2>&1
    usermod -a -G pulse-access ${username_base}1 >/dev/null 2>&1
    usermod -a -G pulse-access ${username_base}2 >/dev/null 2>&1

    if [ -w "$pulseaudio_client" ]; then
        log "prepare_pulseaudio() Configurando a opção 'autospawn' como 'no' no arquivo '$pulseaudio_client'"
        sed -i 's/; autospawn = yes/autospawn = no/g' "$pulseaudio_client"
    fi

    log "prepare_pulseaudio() Reiniciando o serviço 'pulseaudio'"
    service pulseaudio stop >/dev/null 2>&1
    service pulseaudio start >/dev/null 2>&1
}

