Script em PHP para receber os logs gerados pelo LiveCD caso a opção enable_send_logs do config/livecd_provas.conf esteja ativada.

Requisitos:
- Servidor web com suporte a PHP 5.x.


Instalação e configuração:

Copie o script para algum diretório do servidor que seja acessível via internet, crie um diretório 'upload' em algum local, dê permissão de escrita para o servidor web e atualize a variável $upload_dir do script 'provas_receive_logs.php'.

O nome do script e o nome do diretório de upload podem ser alterados, porém lembre-se de alterar também a URL do arquivo de configuração do LiveCD, conforme descrito abaixo. 

No arquivo de configuração do LiveCD (config/moodle_provas.conf), as seguintes variáveis devem ser atualizadas de acordo com o servidor Web onde o script 'provas_receive_logs.php' foi instalado, abaixo a descrição das variáveis.


# Endereço IP do servidor Web onde está o script provas_receive_logs.php
log_server_ip='150.162.60.25'

# URL completa do script que deve receber os logs via upload
log_server_script='https://wwwexe.inf.ufsc.br/~juliao/provas_receive_logs.php'

# Chave de autenticação (é a senha configurada no arquivo 'provas_receive_logs.php')
log_server_auth='f0a320fd52383c42649d48ea545915a9'



OBS: A chave de autenticação serve apenas para fazer uma autenticação simples, mas pode ser utilizada também para invalidar cópias antigas do LiveCD, caso o script 'provas_receive_logs.php' seja modificado e torne-se incompatível com as versões anteriores.