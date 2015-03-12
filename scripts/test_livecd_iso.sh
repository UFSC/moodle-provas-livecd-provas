#!/bin/bash
#set -x

# Este script cria uma máquina virtual temporária no VirtualBox e inicializa ela com a última
# ISO do LiveCD que foi gerada. No final a máquina virtual temporária poderá ser removida.
# Este script deve ser inicializado somente a partir do 'main.sh'.

# Nome que a máquina virtual terá no VirtualBox.
vm_name="$vm_name_base $(random_16)"

# Caminho completo para o diretório da máquina virtual.
vm_path="$vm_path_base/$vm_name"

# Remove a máquina virtual do VirtualBox e apaga os arquivos restantes.
remove_vm() {
    vm_name="$1"
    msg_d "$sub_prefix Removendo a VM: '$vm_name'..."
    vm_path=$(VBoxManage showvminfo "$vm_name" | grep "Config file" | cut -d: -f2 | sed 's/^[ \t]*//')
    vm_dir=$(dirname "$vm_path")

    msg_d "$sub_prefix Removendo a VM do VirtualBox..."
    VBoxManage unregistervm "$vm_name" --delete >>"$std_out" 2>>"$std_err"

    msg_d "$sub_prefix Removendo o diretório: '$vm_dir'..."
    rm -rf "$vm_dir"
}


######################### main ############################

msg "$prefix Iniciando a criação de uma VM temporária para testar a ISO do LiveCD com o VirtualBox"

# Obtem o caminho da última ISO gerada para a arquitetura de hardware definida no arquivo de configuração.
last_iso=$(ls -t "$iso_dir/"*"$livecd_hw_arch"*".iso" 2>/dev/null | head -n1)

if [ ! -f "$last_iso" ]; then
    msg_e "$prefix ERRO: Nenhuma ISO gerada foi encontrada no diretório $iso_dir"
    return
else
    msg_d "$sub_prefix ISO que será testada: " '-n'
    msg_w "${last_iso##*/}"
fi

if [ -d "$vm_path" ]; then
    msg_w "$prefix AVISO: Uma VM com o nome '$vm_name' já existe!"
    msg_w "$prefix Digite 'sim' para removê-la e recriá-la, ou ENTER para sair: " "-n"
    read option
    if [ "$option" = 'sim' -o "$option" = 'SIM' ]; then
    	remove_vm "$vm_name"
    else
	    msg_d "$prefix Saindo, nada alterado."
        return
    fi
else
    mkdir -p "$vm_path"
fi


msg_d "$sub_prefix Criando e registrando a VM '$vm_name' no VirtualBox..."
VBoxManage createvm --name "$vm_name" --ostype "Ubuntu" --register >>"$std_out" 2>>"$std_err"

msg_d "$sub_prefix Configurando a VM '$vm_name'..."
VBoxManage modifyvm "$vm_name" --memory "$vm_ram" --vram "$vm_vram" --nic1 bridged --bridgeadapter1 "$vm_bridge_if" --audio pulse --audiocontroller ac97 --usb on >>"$std_out" 2>>"$std_err"
VBoxManage storagectl "$vm_name" --name "IDE Controller" --add ide >>"$std_out" 2>>"$std_err"
VBoxManage storagectl "$vm_name" --name "SATA Controller" --add sata --sataportcount 1 >>"$std_out" 2>>"$std_err"

# O comando '${last_iso##*/}' mostra apenas o nome do arquivo e não o caminho completo, aqui foi
# utilizado o recurso chamado 'parameter substitution'.
msg_d "$sub_prefix Conectando a ISO '${last_iso##*/}' do LiveCD à VM..."
VBoxManage storageattach "$vm_name" --storagectl "IDE Controller" --port 1 --device 0 --type dvddrive --medium "$last_iso" >>"$std_out" 2>>"$std_err"

msg_d "$sub_prefix Iniciando a VM '$vm_name'..."
VBoxManage startvm "$vm_name" >>"$std_out" 2>>"$std_err"

msg_w "$sub_prefix Após testar o sistema, pressione <ENTER> para remover a VM temporária criada: " "-n"
read option
remove_vm "$vm_name"

msg_d "$sub_prefix Teste concluído."
