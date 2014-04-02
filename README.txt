***** Sistema de geração do LiveCD do Moodle Provas *****

Este diretório contêm os arquivos e scripts do sistema de geração do LiveCD do Moodle Provas, o objetivo principal deste sistema é gerar um arquivo .ISO do LiveCD, que poderá ser utilizado para gravar um CD ou um pendrive e ser utilizado para realizar as provas online do Moodle Provas.


*** Pré-requisitos ***

Para utilizar este sistema você precisa atender aos seguintes requisitos:

 - Utilizar o sistema operacional Ubuntu em uma versão homologada (veja a lista de versões homologadas no arquivo config/livecd_config.conf , variável 'distro_ubuntu_versions'.
 - Possuir o utilitário 'sudo' configurado para que seu usuário possa executar comandos como root.
 - Possuir o utilitário 'make' instalado (as demais dependências serão instaladas na primeira vez em que você executar o make).


*** Utilização ***

Após atender aos pré-requisitos e estando dentro do diretório raiz do sistema de geração do LiveCD, basta executar o comando 'make', será solicitada sua senha (através do comando sudo) e será exibido um menu no terminal com as opções disponíveis, as opções são auto-descritivas.


*** Descrição dos diretórios e arquivos ***

\
 |_ bootloader - Contêm os arquivos do bootloader do LiveCD, ele é responsável por exibir o menu de inicialização.
 |_ bootstrap - Base do sistema gerada com o comando debootstrap e comprimida com tar.gz.
 |_ build.log - Arquivo de log criado durante a geração do LiveCD.
 |_ config - Diretório que contêm todos arquivos de configuração do LiveCD.
 |_ extra - Contêm arquivos extras que podem ser úteis para ativar algum recurso e documentação extra.
 |_ iso - Diretório onde os arquivos .ISO gerados são gravados.
 |_ Makefile - Arquivo Makefile que oferece as opções de gerar o LiveCD e remover arquivos temporários.
 |_ packages/built - Contêm os pacotes Debian gerados, prontos para instalação.
 |_ packages/src - Contêm os arquivos fonte usados para gerar os pacotes Debian do diretório anterior.
 |_ packages/src_3rd_party - Contêm arquivos fonte e/ou patches para pacotes mantidos por terceiros.
 |_ README.txt - Este arquivo.
 |_ scripts - Shell scripts de geração do LiveCD.
 |_ scripts/chroot - Shell scripts que são executados dentro do ambiente CHROOT.
 |_ tmp - Diretório de trabalho, onde os arquivos temporários são gravados.
