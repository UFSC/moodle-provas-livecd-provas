<?php

$upload_dir = '/home/cco/juliao/public_exe/upload/';
$token = 'f0a320fd52383c42649d48ea545915a9';

#print_r($_FILES);  # DEBUG
#print_r($_POST);  # DEBUG

# Se o token não estiver definido ou for diferente do que deve ser...
if (! isset($_POST['token']) || $_POST['token'] != $token)
    die ("*** Erro\n\tResposta do servidor: Erro de autenticação, talvez você esteja usando uma versão desatualizada.\n");


if (isset($_FILES['file'])) {
    $filename = $_FILES['file']['name'];
    $size = $_FILES['file']['size'];

    if (move_uploaded_file($_FILES['file']['tmp_name'], $upload_dir . $filename)) {
        $file = 'upload/' . $filename . '.txt';
        $remote_ip = isset($_SERVER['REMOTE_ADDR']) ? $_SERVER['REMOTE_ADDR'] : 'Variável indisponível';
        $host = isset($_SERVER['REMOTE_ADDR']) ? gethostbyaddr($_SERVER['REMOTE_ADDR']) : 'IP não disponível';
        $timestamp = date("d/m/Y - H:i:s");
        $data = "IP remoto: " . $remote_ip . "\nHost: " . $host . "\nData e hora do envio: " . $timestamp . "\n";

        file_put_contents($file, $data, 0);

        echo "OK\n\tResposta do servidor: Arquivo '$filename' ($size bytes) recebido.\n";
    }
    else
        echo "*** Erro\n\tResposta do servidor: Erro na recepção do arquivo!\n";
}
