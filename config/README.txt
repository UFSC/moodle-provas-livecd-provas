Este diretório contêm os principais arquivos de configuração do LiveCD, os arquivos '.conf' contêm praticamente todas as opções que você vai precisar alterar para que o LiveCD se adeque a sua instituição, sem que seja necessário mexer na estrutura interna de geração do LiveCD. E as imagens do diretório 'images' permitem uma leve personalização visual do sistema.

  * images/bootloader_splash.png - Imagem exibida como fundo do menu de inicialização (deve ser uma imagem com poucos efeitos e com poucos kilobytes, para que o menu não fique lento).

  * images/logout-banner.png - Imagem exibida quando o usuário clica no botão 'Desligar'.

  * images/wallpaper.png - Papel de parede padrão da área de trabalho do LiveCD.

  * livecd_provas.conf - Arquivo de configuração da geração do LiveCD. Este arquivo contêm as opções que afetam a geração do LiveCD.

  * moodle_provas.conf - Arquivo de configuração interno do LiveCD. Este arquivo faz parte do pacote debian 'moodle-provas-config', que é gerado a cada nova geração do LiveCD. Ele também é lido durante a geração do LiveCD pelo arquivo 'livecd_provas.conf', pois algumas das variáveis são necessárias durante este processo.


Você pode substituir as imagens, mas deve respeitar o formato, a resolução e nome delas.

Os arquivos '.conf' são arquivos shell script que contêm diversas variáveis, tome cuidado ao alterá-las para não inserir algum erro de sintaxe, muito cuidado com as aspas simples e aspas duplas.
