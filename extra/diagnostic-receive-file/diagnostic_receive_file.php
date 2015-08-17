<?php

$upload_dir = '/home/cco/juliao/public_exe/upload/';
$max_size_upload_dir = 10000000; // Bytes
$token = '9f036ba23ec2f7009c827eaa736e96d2';

// Se o token não estiver definido ou for diferente do que deve ser...
if (! isset($_POST['token']) || $_POST['token'] != $token) {
    $reply = array('status' => 1, 'msg' => "Cliente não autorizado, talvez você esteja usando uma versão desatualizada.");
}
else {
    // Se um arquivo foi enviado...
    if (isset($_FILES['file'])) {
        $current_dir_size = get_dir_size($upload_dir);
        if ($current_dir_size >= $max_size_upload_dir) {
            $reply = array('status' => 1, 'msg' => "O diretório de upload está cheio ($current_dir_size bytes).");
        }
        else {
            $timestamp = date("Ymd-H\hi\ms\s");
            $remote_ip = isset($_SERVER['REMOTE_ADDR']) ? $_SERVER['REMOTE_ADDR'] : 'REMOTE_ADDR_UNAVAILABLE';
            $remote_lan_ip = isset($_SERVER['HTTP_MOODLE_PROVAS_IP']) ? $_SERVER['HTTP_MOODLE_PROVAS_IP'] : 'NO_MOODLE_PROVAS_IP';
            $host = isset($_SERVER['REMOTE_ADDR']) ? gethostbyaddr($_SERVER['REMOTE_ADDR']) : 'NO_MOODLE_PROVAS_IP';
            $user_email = isset($_SERVER['HTTP_LIVECD_USER_EMAIL']) ? $_SERVER['HTTP_LIVECD_USER_EMAIL'] : 'NO_EMAIL';
            $user_description = isset($_SERVER['HTTP_LIVECD_USER_DESCRIPTION']) ? $_SERVER['HTTP_LIVECD_USER_DESCRIPTION'] : 'NO_DESCRIPTION';
            $filename = $timestamp . '_' . $remote_ip . '_' . $remote_lan_ip . '_' . $_FILES['file']['name'];
            $size = $_FILES['file']['size'];

            if (move_uploaded_file($_FILES['file']['tmp_name'], $upload_dir . $filename)) {
                $log_file = $upload_dir . '/' . $filename . '.txt';
                $timestamp = date("d/m/Y - H:i:s");
                $data = "Data e hora do recebimento: " . $timestamp . "\n" .
                        "IP remoto (válido): " . $remote_ip . "\n" . 
                        "IP remoto (interno): " . $remote_lan_ip . "\n" . 
                        "Host do IP válido: " . $host . "\n" . 
                        "E-mail do usuário: " . $user_email . "\n" .
                        "Descrição do problema: " . $user_description . "\n";
                file_put_contents($log_file, $data, 0);

                $reply = array('status' => 0, 'msg' => "Arquivo '$filename' ($size bytes) recebido.");
            }
            else
                $reply = array('status' => 1, 'msg' => "Erro na recepção do arquivo.");
        }
    }
}

header('Content-Type: application/json');
echo json_encode($reply);


// Retorna o tamanho do diretório informado em bytes
function get_dir_size($dir) {
    $total_size = 0;
    $iterator = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($dir));
    foreach ($iterator as $file)
        $total_size += $file->getSize();

    return $total_size;
}

