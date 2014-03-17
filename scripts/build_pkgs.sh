#!/bin/bash
#set -x

# Este script gera os pacotes debian do Moodle Provas utilizando os arquivos do diretório $pkgs_src_dir,
# os pacotes são gerados no diretório $pkgs_built_dir.
# Este script deve ser inicializado somente a partir do 'main.sh'.

build_error=0

msg "$prefix Iniciando a geração dos pacotes debian"

# Remove e recria o diretório temporário caso ele já exista.
if [ -d "$pkgs_tmp_dir" ]; then
    rm -rf "$pkgs_tmp_dir"
fi
mkdir -p "$pkgs_tmp_dir"

for package_src in "$pkgs_src_dir/"*; do
    build_debian_pkg "true" "$package_src" "$pkgs_tmp_dir" || build_error='1'
done

if [ "$build_error" -eq 0 ]; then
    msg_d "$sub_prefix Removendo os pacotes antigos e movendo os novos..."
    for package_src in "$pkgs_src_dir/"*; do
        # package_name deve ter um underscone no final pois 'moodle-provas*' casa com moodle-provas e moodle-provas-config.
        package_name="${package_src##*/}_"

        # Não queremos gerar o pacote 'moodle-provas-config' nem remover a versão atual,
        # pois ele é gerado durante a geração do LiveCD, gerando aqui ele estaria incompleto.
        if [ -e "$package_src/DEBIAN/control" ] && [ "$package_name" != 'moodle-provas-config_' ]; then
            rm -f "$pkgs_built_dir/$package_name"*
            mv "$pkgs_tmp_dir/$package_name"* "$pkgs_built_dir/"
        fi
    done

    msg_ok "$sub_prefix Os pacote foram gerados com sucesso no diretório $pkgs_built_dir"
else
    msg_e "$sub_prefix Erro ao gerar algum dos pacotes debian"
fi

# Remove o diretório temporário
rm -rf "$pkgs_tmp_dir"

