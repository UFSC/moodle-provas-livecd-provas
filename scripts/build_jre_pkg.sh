#!/bin/bash
#set -x

# Este script gera um pacote debian com o Java JRE da Oracle, pois os repositórios oficiais
# do ubuntu não disponibilizam mais este pacote devido a restrições de distribuição da Oracle.
# Também não existe um repositório de terceiros que tenha o JRE, apenas o JDK que é bem maior.
# Este script deve ser inicializado somente a partir do 'main.sh'.

# Mais informações sobre o download do JRE usando o wget no endereço abaixo:
# https://ivan-site.com/2012/05/download-oracle-java-jre-jdk-using-a-script/

# Header que deve ser enviado para o site de download da oracle, para que o download seja autorizado.
oracle_header='Cookie: gpw_e24=http%3A%2F%2Fwww.oracle.com%2F; oraclelicense=accept-securebackup-cookie'

# Se a arquitetura de hardware do LiveCD estiver configurada para 'amd64', define a URL correta.
if [ "$livecd_hw_arch" = 'amd64' ]; then
   jre_url="$jre64_url"
fi

jre_file="${jre_url##*/}"
pkg_name='oracle-java8-jre'
date="$(date +%Y%m%d)01"

old_dir="$(pwd)"

# Se o diretório de trabalho ainda não foi criado...
if [ ! -d "$working_dir" ]; then
    mkdir "$working_dir"
fi

# Remove arquivos temporários antigos se existirem
if ls -u "$working_dir/$pkg_name"* >/dev/null 2>&1; then
    rm -rf "$working_dir/$pkg_name"* >>"$std_out" 2>>"$std_err"
fi

# Copia a estrutura padrão do pacote para um diretório temporário
cp -R "$pkgs_src_3rd_dir/$pkg_name" "$working_dir/"

# Entra na raíz do diretório temporário
cd "$working_dir"

msg "$prefix Iniciando a geração do pacote do JRE da Oracle"
msg_d "$sub_prefix Baixando o arquivo do jre da oracle (Versão: $jre_version)..."
if [ ! -f "$jre_file" ]; then
    wget -nv --no-cookies --no-check-certificate --header "$oracle_header" "$jre_url" >>"$std_out" 2>>"$std_err"
fi

# Cria o diretório onde o link simbólico para o plugin do navegador deve estar para o navegador reconhecer.
mkdir -p "$pkg_name/usr/lib/mozilla/plugins"

msg_d "$sub_prefix Descomprimindo o arquivo..."
tar xzvf "$jre_file" -C "$pkg_name/usr/lib/" >/dev/null 2>>"$std_err"

msg_d "$sub_prefix Ajustando a estrutura do pacote..."
cd "$pkg_name/usr/lib/"
mv "jre$jre_version" "$pkg_name"
ln -s "../../$pkg_name/lib/$livecd_hw_arch/libnpjp2.so" 'mozilla/plugins/pluginjava.so'

# Volta ao diretório anterior
cd - >>"$std_out" 2>>"$std_err"

msg_d "$sub_prefix Atualizando a versão do pacote..."
sed -i "s/Version: .*/Version: ${jre_version/_/u}-$date/g" "$pkg_name/DEBIAN/control"

msg_d "$sub_prefix Atualizando a arquitetura do pacote..."
sed -i "s/Architecture: .*/Architecture: $livecd_hw_arch/g" "$pkg_name/DEBIAN/control"

# Gerando o pacote...
build_debian_pkg "false" "$pkg_name" "$working_dir"

msg_d "$sub_prefix Removendo arquivos temporários..."
rm -rf "$working_dir/$pkg_name"
rm -rf "$working_dir/$jre_file"

# Volta ao diretório original
cd "$old_dir"

if ls -u "$working_dir/$pkg_name"*"$livecd_hw_arch.deb" >/dev/null 2>&1; then
    msg_d "$sub_prefix Removendo os pacotes antigos..."
    rm -rf "$pkgs_built_dir/$pkg_name"*"$livecd_hw_arch.deb"

    msg_d "$sub_prefix Movendo o pacote gerado para o diretório final..."
    mv "$working_dir/$pkg_name"*"$livecd_hw_arch.deb" "$pkgs_built_dir"

    msg_ok "$sub_prefix Pacote gerado com sucesso no diretório $pkgs_built_dir"
else
    msg_e "$sub_prefix Erro ao gerar o pacote do JRE da Oracle"
fi

