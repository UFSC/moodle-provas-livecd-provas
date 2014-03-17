Descrição dos arquivos:

Dependência principal: make

apt-get install make

As demais dependências serão instaladas a primeira vez que você executar o make.



bootloader - Contêm os arquivos do bootloader do LiveCD, o processo responsável por exibir o menu de inicialização.

bootstrap - Base do sistema gerada com o comando debootstrap e comprimida.

dependencies.txt - Contém uma lista de pacotes necessários para gerar o LiveCD.

docs - Contêm informações sobre o funcionamento do LiveCD.

includes - Arquivos utilizados por alguns shell scripts.

Makefile

pkgs_built - Contêm os pacotes Debian gerados, prontos para instalação.

pkgs_src - Contêm os arquivos fonte usados para gerar os pacotes Debian do diretório anterior.

pkgs_src_3rd - Contêm arquivos fonte e patches para pacotes mantidos por terceiros ou que não estão disponíveis nos repositórios oficiais.

scripts - Shell scripts

bootstrap - Base do sistema gerada com o comando debootstrap e comprimida.

livecd_provas.conf - Arquivo de configuração global para geração do LiveCD.

make_livecd.sh

root_fs
