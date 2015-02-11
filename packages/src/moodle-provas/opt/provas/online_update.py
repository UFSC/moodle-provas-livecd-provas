#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import traceback
from gi.repository import Gtk
import subprocess
from urllib.request import urlopen
import json
from includes.provas_config import ProvasConfig

# OnlineUpdate Core Class
class OnlineUpdate():
    def __init__(self, provas_config):
        self.config_cd = ProvasConfig(provas_config)

    def run_command(self, cmd):
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        output, error = process.communicate()
        status = process.wait()

        if error:
            return status, error
        else:
            return status, output



    def get_online_config(self):
        try:
            json_data = urlopen(self.config_cd['online_config_file_url']).readall().decode('utf-8')

            # json_data = urlopen("http://150.162.9.125/~juliao/config.json").readall().decode('utf-8')
            #json_data = urlopen("http://localhost/~juliao/config.json").readall().decode('utf-8')
        except:
            pass
            #raise

        self.config = json.loads(json_data)
        self.moodle_provas_minimum_version = self.config["moodle_provas_minimum_version"]
        self.general_settings = self.config["general_settings"]
        self.mainstream_log_server_settings = self.config["mainstream_log_server_settings"]
        self.institutions = self.config["institutions"]

        self.require_load_confirmation = True if self.general_settings["require_load_confirmation"] == "yes" else False
        self.load_default_institution = True if self.general_settings["load_default_institution"] == "yes" else False
        self.default_institution_id = str(self.general_settings["default_institution_id"])


    def save_provas_config(self, institution_id):
        """

        :param institution_id: str
        :raise KeyError: Exception

        """
        try:
            institution = self.institutions[institution_id]
        except:
            raise KeyError("Não existe uma instituição com o ID = " + institution_id)


        use_custom_log_server_settings = True if institution["use_custom_log_server_settings"] == "yes" else False

        if use_custom_log_server_settings:
            log_server_settings = institution["custom_log_server_settings"]
        else:
            log_server_settings = self.mainstream_log_server_settings

        provas_config = open(self.config_cd['provas_online_config'], 'w')
        provas_config.write('#!/bin/bash\n\n')

        # Write the general settings
        for key in sorted(institution.keys()):
            if key != "custom_log_server_settings":
                print(str(key) + "=\"" + str(institution[key]) + "\"")
                provas_config.write(str(key) + "=\"" + str(institution[key]) + "\"\n")

        # Write the log server settings
        for key in sorted(log_server_settings.keys()):
            print(str(key) + "=\"" + str(log_server_settings[key]) + "\"")
            provas_config.write(str(key) + "=\"" + str(log_server_settings[key]) + "\"\n")

        provas_config.close()

# OnlineUpdate UI Class
class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Selecione a sua Instituição")
        # self.set_size_request(600, 250)
        #self.set_position(Gtk.WindowPosition.CENTER)
        #self.set_resizable(False)
        self.fullscreen()
        self.connect("destroy", self.__onDestroy)
        self.windowIcon = self.render_icon(Gtk.STOCK_INFO, Gtk.IconSize.MENU)
        self.set_icon(self.windowIcon)
        self.institutions = ''

        title = Gtk.Label()
        title.set_text('<span size=\"25000\">Selecione a sua instituição abaixo e clique em Prosseguir:</span>')
        title.set_use_markup(True)
        title.set_line_wrap(True)
        title.set_justify(Gtk.Justification.CENTER)
        title.set_size_request(800, 300)

        # ID, Instituição, URL
        self.store = Gtk.ListStore(str, str, str)

        while True:
            try:
                server.get_online_config()
                break
            except Exception as err:
                print(traceback.print_exc())
                confirmDialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR,
                                                  Gtk.ButtonsType.YES_NO, "Erro ao obter os dados do servidor")
                confirmDialog.format_secondary_markup("Exceção: " + str(err) + "\n\nDeseja tentar novamente?")
                response = confirmDialog.run()
                if response == Gtk.ResponseType.YES:
                    print("Confirmação: 'Sim' clicado")
                elif response == Gtk.ResponseType.NO:
                    print("Confirmação: 'Não' clicado")
                    exit(1)

                confirmDialog.destroy()

        livecd_version = config_cd['provas_version']
        if livecd_version < server.moodle_provas_minimum_version:
            errorExamVersionDialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR,
                                                       Gtk.ButtonsType.OK,
                                                       "Erro: Versão do LiveCD não suportada")
            errorExamVersionDialog.format_secondary_text(
                "Esta versão do LiveCD não é mais suportada, por favor utilize uma versão mais recente.\n\n"
                " - Versão do seu CD:   " + livecd_version + "\n - Versão mínima requerida:   " + str(
                    server.moodle_provas_minimum_version))
            errorExamVersionDialog.run()
            print("Alerta fechado")

            errorExamVersionDialog.destroy()

            exit(1)

        if server.load_default_institution:
            try:
                server.save_provas_config(server.default_institution_id)
            except Exception as err:
                print(traceback.print_exc())
                errorSavingDialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR,
                                                      Gtk.ButtonsType.OK,
                                                      "Erro ao salvar o arquivo de configuração do Moodle Provas")
                errorSavingDialog.format_secondary_text("Não foi possível salvar o arquivo moodle_provas.conf\n\n"
                                                        " - Exceção: " + str(err))
                errorSavingDialog.run()
                print("Alerta fechado")

                errorSavingDialog.destroy()

                exit(1)

            exit(0)

        for institution_id in server.institutions.keys():
            institution = server.institutions[institution_id]
            self.store.append([str(institution_id), institution['institution_name'], institution['provas_host']])

        self.treeview = Gtk.TreeView(model=self.store)
        self.selection = self.treeview.get_selection()

        id = Gtk.CellRendererText()
        institution = Gtk.CellRendererText()
        url = Gtk.CellRendererText()

        column_1 = Gtk.TreeViewColumn("ID", id, text=0)
        column_2 = Gtk.TreeViewColumn("Instituição", institution, text=1)
        column_2.set_min_width(400)
        column_2.set_max_width(500)
        column_3 = Gtk.TreeViewColumn("Página do Moodle", url, text=2)

        column_1.set_sort_column_id(0)
        column_2.set_sort_column_id(1)
        column_3.set_sort_column_id(2)
        self.store.set_sort_column_id(0, Gtk.SortType.ASCENDING)

        self.treeview.append_column(column_1)
        self.treeview.append_column(column_2)
        self.treeview.append_column(column_3)

        #submit = Gtk.Button("Prosseguir", Gtk.STOCK_INFO)
        submit = Gtk.Button("Prosseguir")
        submit.connect("clicked", self.__onClickSubmit)
        #submit.new_from_icon_name(Gtk.STOCK_ADD, 100)
        submit.set_size_request(100, 50)
        submit.set_margin_left(400)
        submit.set_margin_right(400)
        submit.set_margin_top(100)
        # submit.set_margin_bottom(50)

        scrolledWindow = Gtk.ScrolledWindow()
        #scrolledWindow.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        #scrolledWindow.set_hexpand(True)
        #scrolledWindow.set_vexpand(True)
        scrolledWindow.add(self.treeview)
        scrolledWindow.set_min_content_height(250)
        scrolledWindow.set_margin_left(150)
        scrolledWindow.set_margin_right(150)
        #scrolledWindow.set_size_request(100, 100)

        self.box = Gtk.VBox(spacing=10)
        self.box.pack_start(title, False, False, 0)
        self.box.pack_start(scrolledWindow, False, False, 0)
        self.box.pack_start(submit, False, False, 0)
        self.add(self.box)


    def __onDestroy(self, e):
        Gtk.main_quit()


    def __onClickSubmit(self, e1):
        model, treeiter = self.selection.get_selected()
        if treeiter != None:
            if server.require_load_confirmation:
                confirmDialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.QUESTION,
                                                  Gtk.ButtonsType.YES_NO, "Confirmação")
                confirmDialog.format_secondary_markup(
                    "Você selecionou a opção:\n\n <b>" + str(model[treeiter][1]) + " (" + str(
                        model[treeiter][2]) + ")</b>\n\nClique em Sim para confirmar.")
                response = confirmDialog.run()

                if response == Gtk.ResponseType.YES:
                    print("Confirmação: 'Sim' clicado")
                    institution_id = str(model[treeiter][0])
                    confirmDialog.destroy()
                elif response == Gtk.ResponseType.NO:
                    print("Confirmação: 'Não' clicado")
                    confirmDialog.destroy()
                    return
            else:
                institution_id = str(model[treeiter][0])

            try:
                server.save_provas_config(institution_id)
            except Exception as err:
                print(traceback.print_exc())
                errorSavingDialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR,
                                                      Gtk.ButtonsType.OK,
                                                      "Erro ao salvar o arquivo de configuração do Moodle Provas")
                errorSavingDialog.format_secondary_text("Não foi possível salvar o arquivo " +
                                                        config_cd['provas_online_config'] + "\n\n"
                                                        "Exceção: " + str(err))
                errorSavingDialog.run()
                errorSavingDialog.destroy()
                exit()
            exit(0)
        else:
            alertDialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.INFO,
                                            Gtk.ButtonsType.OK, "Nenhuma instituição selecionada")
            alertDialog.format_secondary_text("Você deve selecionar uma instituição!")
            alertDialog.run()
            alertDialog.destroy()


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Você deve informar o caminho do arquivo de configuração do moodle provas (Ex: moodle_provas.conf)")
        print("   Uso: " + sys.argv[0] + " </path/to/moodle_provas.conf>")
        exit(1)

    provas_config = sys.argv[1]

    if not os.path.exists(provas_config):
        print("Erro: O caminho para o arquivo '" + provas_config + "' não existe!")
        exit(1)

    config_cd = ProvasConfig(provas_config)
    server = OnlineUpdate(provas_config)
    window = MainWindow()
    window.show_all()
    Gtk.main()
