Descrição:

Este script em PHP serve para receber as screenshots e os logs gerados pelo LiveCD, caso o usuário pressione as teclas PrintScreen e Ctrl+PrintScreen, respectivamente, e essas funções estejam autorizadas no arquivo de configuração online.

Requisitos:

- Servidor web com suporte a PHP 5.x (deve ser o mesmo do Moodle Provas).
- Diretório com permissão de gravação pelo webserver.


Configuração:

É necessário configurar três parâmetros dentro do script, que são os seguintes:

- $upload_dir: Diretório onde os arquivos recebidos serão salvos, deve ter permissão de gravação pelo webserver.
- $max_size_upload_dir: Tamanho máximo do diretório onde esses arquivos são salvos (é a soma do tamanho de todos arquivos).
- $token: Token de autorização para o envio dos arquivos.

Instalação:

Copie o script para algum diretório do servidor que seja acessível via internet, configure os parâmetros descritos acima e certifique-se de que o script consiga gravar no diretório de upload.

O nome do script e o nome do diretório de upload podem ser alterados, porém lembre-se de alterar também o caminho do script no arquivo de configuração online, parâmetro "diag_script_receive_file_path".


OBS1: O servidor onde este script deve ser instalado, precisa ser o mesmo onde o Moodle Provas está instalado, pois os scripts utilizam o Host do Moodle Provas para enviar os arquivos.

OBS2: O token serve apenas para autorizar o envio de arquivos pelos usuários, lembre-se que ao alterar esse token, a função de envio de arquivos dos CDs antigos que o utilizam, deixaram de funcionar.
