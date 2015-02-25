#!/usr/bin/env python
# -*- coding: utf-8 -*-


import subprocess

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
