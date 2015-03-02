﻿#!/usr/bin/env python3
# -*- coding: utf-8 -*-


from gi.repository import GLib, Gtk, GObject, Pango
import threading
import subprocess
import traceback


# NetworkTest Core Class
class NetworkTest():
    def run_command(self, cmd):
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        output, error = process.communicate()
        status = process.wait()
        
        if error:
            return status, error
        else:
            return status, output
        
        
    def get_default_iface(self):
        cmd = "ip route show | grep default"
        status, output = self.run_command(cmd)
        
        if not status:
            ss = output.split()
            print(ss)
            iface = ss[ss.index("dev") + 1]

            status = 0
            output = iface
        else:
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
            return self.run_command(cmd)
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
        self.set_default_size(700, 350)
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
        super(MainWindow, self).__init__(title="Diagnóstico da Conexão de Rede")
        self.set_size_request(360, 250)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.set_resizable(False)
        self.connect("destroy", self.__on_destroy)
        self.windowIcon = self.render_icon(Gtk.STOCK_NETWORK, Gtk.IconSize.MENU)
        self.set_icon(self.windowIcon)

        self.box = Gtk.VBox(spacing=6)
        self.add(self.box)

        # Ícone, Spinner, Descrição, "Mais detalhes", Método, Parâmetros do método, Detalhes
        self.store = Gtk.ListStore(str, bool, str, str, str, str, str)
        self.store.append([None, False, "Interfaces de rede detectadas", None, "get_ifaces", None, None])
        self.store.append([None, False, "Interface de rede padrão", None, "get_default_iface", None, None])
        self.store.append([None, False, "Ping ufsc.br", None, "ping_ipv4", "ufsc.br", None])
        self.store.append([None, False, "Ping provas2.moodle.ufsc.br", None, "ping_ipv4", "provas2.moodle.ufsc.br", None])
        self.store.append([None, False, "traceroute provas2.moodle.ufsc.br", None, "traceroute", "provas2.moodle.ufsc.br", None])

        treeview = Gtk.TreeView(model=self.store)
        treeview.get_selection().set_mode(Gtk.SelectionMode.NONE)

        icon = Gtk.CellRendererPixbuf()
        spinner = Gtk.CellRendererSpinner()
        text = Gtk.CellRendererText()
        details = CellRendererClickablePixbuf()

        column_1 = Gtk.TreeViewColumn("Status")
        column_2 = Gtk.TreeViewColumn("Verificação", text, text=2)
        column_2.set_min_width(260)
        column_2.set_max_width(260)
        # column_3 = Gtk.TreeViewColumn("Progresso", progress, value=3, inverted=1)
        column_3 = Gtk.TreeViewColumn("Detalhes", details, stock_id=3)

        treeview.append_column(column_1)
        treeview.append_column(column_2)
        treeview.append_column(column_3)

        #treeview.connect("button-press-event", self.__onClick)
        details.connect("clicked", self.__on_click_details)

        column_1.pack_start(icon, False)
        column_1.add_attribute(icon, "stock_id", 0)
        column_1.pack_start(spinner, False)
        column_1.add_attribute(spinner, "active", 1)
        column_1.add_attribute(spinner, "pulse", 2)
        #column_1.add_attribute(spinner, "active", 1)
        #column_1.set_attributes(spinner, "pulse", 2)

        # column_1.set_attributes(icon, text=0)

        button_execute = Gtk.Button(label="Executar testes")
        button_execute.connect("clicked", self.__on_click_execute)
        #button_execute.set_size_request(100,20)
        button_execute.set_margin_left(50)
        button_execute.set_margin_right(50)
        button_execute.set_margin_top(10)
        button_execute.set_margin_bottom(15)

        self.box.pack_start(treeview, True, True, 0)
        self.box.pack_start(button_execute, True, True, 0)

    def __on_destroy(self, e):
        Gtk.main_quit()

    def __on_click_details(self, e1, e2):
        details.set_text(str(self.store[e2][6]))
        details.show_all()

    def __on_click_execute(self, e1):
        thread = threading.Thread(target=self.process_list)
        thread.daemon = True
        thread.start()

    def reset_list(self):
        for row in self.store:
            row[0] = ''
            row[1] = True
            row[3] = ''

    def process_list(self):
        self.reset_list()

        for row in self.store:
            method = row[4]
            params = row[5]

            try:
                if params:
                    status, output = getattr(network, method)(params)
                else:
                    status, output = getattr(network, method)()

                GLib.idle_add(self.update_UI, status, output, row)

            except Exception:
                print(traceback.format_exc())



    def update_UI(self, status, output, row):
        row[3] = Gtk.STOCK_ADD
        row[6] = str(output, 'utf-8')

        # Se status não for igual a zero
        if not status:
            row[0] = Gtk.STOCK_APPLY
        else:
            row[0] = Gtk.STOCK_CANCEL



if __name__ == "__main__":
    network = NetworkTest()
    window = MainWindow()
    details = DetailsWindow()
    window.show_all()

    # Calling GObject.threads_init() is not needed for PyGObject 3.10.2+
    #GObject.threads_init()

    #GObject.timeout_add(100, callback)
    Gtk.main()
