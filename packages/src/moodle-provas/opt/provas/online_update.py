#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import traceback
from urllib.request import urlopen
import json

from gi.repository import Gtk

from includes.provas_config import ProvasConfig


# OnlineUpdate Core Class
class OnlineUpdate():
    def __init__(self, provas_config_file):
        self.config_cd = ProvasConfig(provas_config_file)

    def get_online_config(self):
        try:
            json_data = urlopen(self.config_cd['livecd_online_config_url']).readall().decode('utf-8')
        except:
            raise

        self.config = json.loads(json_data)
        self.livecd_minimum_version = self.config["livecd_minimum_version"]
        self.require_load_confirmation = True if self.config["require_load_confirmation"] == "yes" else False
        self.mainstream_log_server_settings = self.config["mainstream_log_server_settings"]
        self.institutions = self.config["institutions"]


    def save_provas_config_file(self, institution_id):
        """

        :param institution_id: str
        :raise KeyError: Exception

        """
        try:
            institution = self.institutions[institution_id]
        except:
            raise KeyError("Não existe uma instituição com o ID = " + institution_id)

        # Check if the institution will use custom log_server_settings
        if institution["custom_log_server_settings"]:
            log_server_settings = institution["custom_log_server_settings"]
        else:
            log_server_settings = self.mainstream_log_server_settings

        provas_config_file = open(self.config_cd['provas_online_config_file'], 'w')
        provas_config_file.write('#!/bin/bash\n\n')

        # Write the general settings
        for key in sorted(institution.keys()):
            if key != "custom_log_server_settings":
                print(str(key) + "=\"" + str(institution[key]) + "\"")
                provas_config_file.write(str(key) + "=\"" + str(institution[key]) + "\"\n")

        # Write the log server settings
        for key in sorted(log_server_settings.keys()):
            print(str(key) + "=\"" + str(log_server_settings[key]) + "\"")
            provas_config_file.write(str(key) + "=\"" + str(log_server_settings[key]) + "\"\n")

        provas_config_file.close()

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
        title.set_text('<span size=\"25000\">Selecione a sua instituição e clique em "Prosseguir":</span>')
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

        current_livecd_version = cd_config['livecd_version']
        if current_livecd_version < server.livecd_minimum_version:
            errorExamVersionDialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR,
                                                       Gtk.ButtonsType.OK,
                                                       "Erro: Versão do LiveCD não suportada")
            errorExamVersionDialog.format_secondary_text(
                "Esta versão do LiveCD não é mais suportada, por favor utilize uma versão mais recente.\n\n"
                " - Versão do seu CD:   " + current_livecd_version + "\n - Versão mínima requerida:   " + str(
                    server.livecd_minimum_version))
            errorExamVersionDialog.run()
            errorExamVersionDialog.destroy()
            exit(1)

        # If there is only one institution, loads by default
        if len(server.institutions) == 1:
            try:
                server.save_provas_config_file("1")
            except Exception as err:
                print(traceback.print_exc())
                errorSavingDialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR,
                                                      Gtk.ButtonsType.OK,
                                                      "Erro ao salvar o arquivo de configuração do Moodle Provas")
                errorSavingDialog.format_secondary_text("Não foi possível salvar o arquivo moodle_provas.conf\n\n"
                                                        " - Exceção: " + str(err))
                errorSavingDialog.run()
                errorSavingDialog.destroy()
                exit(1)
            exit(0)

        for institution_id in server.institutions.keys():
            institution = server.institutions[institution_id]
            self.store.append([str(institution_id), institution['institution_name'], institution['moodle_provas_url']])

        self.treeview = Gtk.TreeView(model=self.store)
        self.selection = self.treeview.get_selection()

        id = Gtk.CellRendererText()
        institution = Gtk.CellRendererText()
        url = Gtk.CellRendererText()

        # column_1 = Gtk.TreeViewColumn("ID", id, text=0)
        # column_2 = Gtk.TreeViewColumn("Instituição", institution, text=1)
        # column_2.set_min_width(400)
        # column_2.set_max_width(500)
        # column_3 = Gtk.TreeViewColumn("Página do Moodle", url, text=2)
        column_1 = Gtk.TreeViewColumn("Instituição", institution, text=1)
        column_1.set_min_width(400)
        column_1.set_max_width(500)
        column_2 = Gtk.TreeViewColumn("Página do Moodle", url, text=2)

        column_1.set_sort_column_id(0)
        column_2.set_sort_column_id(1)
        # column_3.set_sort_column_id(2)
        self.store.set_sort_column_id(0, Gtk.SortType.ASCENDING)

        self.treeview.append_column(column_1)
        self.treeview.append_column(column_2)
        # self.treeview.append_column(column_3)

        submit = Gtk.Button("Prosseguir")
        submit.connect("clicked", self.__onClickSubmit)
        submit.set_size_request(100, 50)
        submit.set_margin_left(400)
        submit.set_margin_right(400)
        submit.set_margin_top(100)

        scrolledWindow = Gtk.ScrolledWindow()
        scrolledWindow.add(self.treeview)
        scrolledWindow.set_min_content_height(250)
        scrolledWindow.set_margin_left(150)
        scrolledWindow.set_margin_right(150)

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
                    institution_id = str(model[treeiter][0])
                    confirmDialog.destroy()
                elif response == Gtk.ResponseType.NO:
                    confirmDialog.destroy()
                    return
            else:
                institution_id = str(model[treeiter][0])

            try:
                server.save_provas_config_file(institution_id)
            except Exception as err:
                print(traceback.print_exc())
                errorSavingDialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR,
                                                      Gtk.ButtonsType.OK,
                                                      "Erro ao salvar o arquivo de configuração do Moodle Provas")
                errorSavingDialog.format_secondary_text("Não foi possível salvar o arquivo " +
                                                        cd_config['provas_online_config_file'] + "\n\n"
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

    provas_config_file = sys.argv[1]

    if not os.path.exists(provas_config_file):
        print("Erro: O caminho para o arquivo '" + provas_config_file + "' não existe!")
        exit(1)

    cd_config = ProvasConfig(provas_config_file)
    server = OnlineUpdate(provas_config_file)
    window = MainWindow()
    window.show_all()
    Gtk.main()
