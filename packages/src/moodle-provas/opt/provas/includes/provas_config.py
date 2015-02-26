#!/usr/bin/env python3

# Módulo para leitura dos arquivos de configuração do Moodle Provas, que são shell scripts.

import configparser

class ProvasConfig(configparser.ConfigParser):
    def __init__(self, file):
        """ Adiciona uma seção no começo do arquivo, porque o ConfigParser não suporta arquivos sem pelo menos uma seção. """
        configparser.ConfigParser.__init__(self)
        fp = open(file)
        fp_str = fp.read()
        fp_str = '[SECTION_GLOBAL]\n' + fp_str
        configparser.ConfigParser.read_string(self, fp_str, source='<string>')

    def __getitem__(self, item):
        """ Permite acessar os elementos como se fosse um dicionário, por exemplo: config['livecd_version']. """
        value = configparser.ConfigParser.get(self, 'SECTION_GLOBAL', item)

        # Substitui as variáveis do shell script, por exemplo provas_config_file="$provas_dir/moodle_provas.conf".
        if item is 'livecd_online_config_url':
            if '$livecd_online_config_host' in value:
                livecd_online_config_host = configparser.ConfigParser.get(self, 'SECTION_GLOBAL', 'livecd_online_config_host')
                livecd_online_config_host = livecd_online_config_host.lstrip('\'').rstrip('\'').lstrip('\"').rstrip('\"')
                value = value.replace('$livecd_online_config_host', livecd_online_config_host)
        elif item is 'provas_config_file' or item is 'provas_online_config_file':
            if '$provas_dir' in value:
                provas_dir = configparser.ConfigParser.get(self, 'SECTION_GLOBAL', 'provas_dir')
                provas_dir = provas_dir.lstrip('\'').rstrip('\'').lstrip('\"').rstrip('\"')
                value = value.replace('$provas_dir', provas_dir)

        return value.lstrip('\'').rstrip('\'').lstrip('\"').rstrip('\"')
