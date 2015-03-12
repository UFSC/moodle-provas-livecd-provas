#!/usr/bin/env python3
# -*- coding: utf-8 -*-


from gi.repository import GLib, Gtk, GObject, Pango
import threading
import subprocess
import traceback
import netifaces
import os
import sys
from includes.provas_config import ProvasConfig


# NetworkTest Core Class
class NetworkTest():
    def run_command(self, cmd):
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        output, error = process.communicate()
        status = process.wait()
        output = str(output, 'utf-8')

        if error:
            return status, error
        else:
            return status, output
        
        
    def get_default_iface(self):
        try:
            default_route_ipv4 = netifaces.gateways()['default'][netifaces.AF_INET]
            default_iface = default_route_ipv4[1]
            status = 0
            output = default_iface
        except:
             status = 1
             output = "Não foi possível determinar a interface padrão, provavelmente não há uma rota padrão configurada."
    
        return status, output
    
    
    def has_network_iface(self):
        cmd = "lspci | grep -e 'Ethernet' -e 'Network' | wc -l"
        status, output = self.run_command(cmd)

        if not status:
            if int(output) > 0:
                return True
        
        return False
    
    
    def get_ifaces(self):
        if self.has_network_iface():
            cmd = "lspci | grep -e 'Ethernet' -e 'Network'"
            status, output =  self.run_command(cmd)
        else:
            status = 1
            output = "Nenhuma interface de rede foi detectada!"

        return status, output
    
    
    def ping_ipv4(self, host):
        cmd = "ping -c 3 " + host
        
        return self.run_command(cmd)
    
    
    def traceroute(self, host):
        cmd = "mtr --report --report-cycles 3 " + host

        return self.run_command(cmd)



# NetworkTest UI Classes
class CellRendererClickablePixbuf(Gtk.CellRendererPixbuf):
    __gsignals__ = {'clicked': (GObject.SIGNAL_RUN_LAST, GObject.TYPE_NONE, (GObject.TYPE_STRING,)) }

    def __init__(self):
        Gtk.CellRendererPixbuf.__init__(self)
        self.set_property('mode', Gtk.CellRendererMode.ACTIVATABLE)

    def do_activate(self, event, widget, path, background_area, cell_area, flags):
        self.emit('clicked', path)


class DetailsWindow(Gtk.Window):
    def __init__(self):
        super(DetailsWindow, self).__init__(title="Detalhes dos comandos executados")
        self.connect("delete-event", self.__hide)
        self.set_default_size(750, 300)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.windowIcon = self.render_icon(Gtk.STOCK_INFO, Gtk.IconSize.MENU)
        self.set_icon(self.windowIcon)
        self.grid = Gtk.Grid()
        self.add(self.grid)

        scrolledWindow = Gtk.ScrolledWindow()
        scrolledWindow.set_hexpand(True)
        scrolledWindow.set_vexpand(True)
        self.grid.attach(scrolledWindow, 0, 1, 3, 1)

        self.textView = Gtk.TextView()
        self.font = Pango.FontDescription("monospace")
        self.textView.modify_font(self.font)
        #self.textView.set_wrap_mode(Gtk.WrapMode.WORD)

        self.textBuffer = self.textView.get_buffer()
        scrolledWindow.add(self.textView)

    def __hide(self, e1, e2):
        self.hide()
        return True

    def set_text(self, text):
        self.textBuffer.set_text(text)


class MainWindow(Gtk.Window):
    def __init__(self):
        super(MainWindow, self).__init__(title="Diagnóstico da Conexão de Rede v0.2")
        self.set_size_request(420, 260)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.set_resizable(False)
        self.connect("destroy", self.__on_destroy)
        self.windowIcon = self.render_icon(Gtk.STOCK_NETWORK, Gtk.IconSize.MENU)
        self.set_icon(self.windowIcon)

        grid = Gtk.Grid()
        self.add(grid)

        # Ícone, Descrição, "Mais detalhes", Método, Parâmetros do método, Detalhes
        self.store = Gtk.ListStore(str, str, str, str, str, str)
        self.store.append([None, "Interfaces de rede detectadas", None, "get_ifaces", None, None])
        self.store.append([None, "Interface de rede padrão", None, "get_default_iface", None, None])
        self.store.append([None, "Ping " + first_host, None, "ping_ipv4", first_host, None])
        self.store.append([None, "Ping " + second_host, None, "ping_ipv4", second_host, None])
        self.store.append([None, "traceroute " + first_host, None, "traceroute", first_host, None])

        treeview = Gtk.TreeView(model=self.store)
        treeview.set_size_request(420, 230)
        treeview.get_selection().set_mode(Gtk.SelectionMode.NONE)

        icon = Gtk.CellRendererPixbuf()
        text = Gtk.CellRendererText()
        details = CellRendererClickablePixbuf()

        column_1 = Gtk.TreeViewColumn("Status")
        column_2 = Gtk.TreeViewColumn("Verificação", text, text=1)
        column_2.set_min_width(260)
        column_2.set_max_width(260)
        column_3 = Gtk.TreeViewColumn("Detalhes", details, stock_id=2)

        treeview.append_column(column_1)
        treeview.append_column(column_2)
        treeview.append_column(column_3)

        details.connect("clicked", self.__on_click_details)

        column_1.pack_start(icon, False)
        column_1.add_attribute(icon, "stock_id", 0)

        self.spinner = Gtk.Spinner()
        self.spinner.set_margin_top(10)
        self.spinner.set_margin_bottom(10)

        self.button_execute = Gtk.Button(label="Executar testes")
        self.button_execute.connect("clicked", self.__on_click_execute)
        self.button_execute.set_size_request(80, 40)
        self.button_execute.set_margin_right(5)
        self.button_execute.set_margin_top(5)
        self.button_execute.set_margin_bottom(5)

        grid.attach(treeview, 0, 0, 2, 1)
        grid.attach(self.spinner, 0, 1, 1, 1)
        grid.attach(self.button_execute, 1, 1, 1, 1)

        self.reset_list()

    def __on_destroy(self, e):
        Gtk.main_quit()

    def __on_click_details(self, e1, e2):
        details.set_text(str(self.store[e2][5]))
        details.show_all()

    def __on_click_execute(self, e1):
        self.spinner.show()
        self.spinner.start()
        self.button_execute.set_sensitive(False)
        self.thread_list = threading.Thread(target=self.process_list)
        self.thread_list.daemon = True
        self.thread_list.start()

    def reset_list(self):
        for row in self.store:
            row[0] = Gtk.STOCK_NO
            row[2] = ''

    def process_list(self):
        self.reset_list()

        for row in self.store:
            method = row[3]
            params = row[4]

            try:
                if params:
                    status, output = getattr(network, method)(params)
                else:
                    status, output = getattr(network, method)()

                GLib.idle_add(self.update_UI, status, output, row)

            except Exception:
                print(traceback.format_exc())

        self.spinner.stop()
        self.spinner.hide()
        self.button_execute.set_sensitive(True)


    def update_UI(self, status, output, row):
        row[2] = Gtk.STOCK_ADD
        row[5] = str(output)

        # Se status não for igual a zero
        if not status:
            row[0] = Gtk.STOCK_APPLY
        else:
            row[0] = Gtk.STOCK_CANCEL


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

    try:
        # Tenta ler os dados do arquivo de configuração online, mas primeiro descobre onde ele deveria estar.
        provas_online_config_file = cd_config['provas_online_config_file']
        cd_online_config = ProvasConfig(provas_online_config_file)
        first_host = cd_online_config['moodle_provas_url'].split('/')[2]
        second_host = cd_online_config['ntp_servers'].split()[0]
    except:
        # Se não funcionar, usa os dados do arquivo de configuração do LiveCD.
        cd_config = ProvasConfig(provas_config_file)
        first_host = cd_config['host_test_1']
        second_host = cd_config['host_test_2']

    network = NetworkTest()
    window = MainWindow()
    details = DetailsWindow()
    window.show_all()

    # Calling GObject.threads_init() is not needed for PyGObject 3.10.2+
    GObject.threads_init()

    #GObject.timeout_add(100, callback)
    Gtk.main()
