// NÃO REMOVA ESTA LINHA, a configuração é lida somente a partir da segunda linha.

Components.classes["@mozilla.org/observer-service;1"].getService(Components.interfaces.nsIObserverService ).addObserver({
    observe : function(subject, topic, data) {
        var channel = subject.QueryInterface(Components.interfaces.nsIHttpChannel);

        channel.setRequestHeader("MOODLE-PROVAS-VERSION", "%livecd_version%", false);
        channel.setRequestHeader("MOODLE-PROVAS-IP", "%livecd_local_ip%", false);
        channel.setRequestHeader("MOODLE-PROVAS-NETWORK", "%livecd_local_network%", false);
    }
},"http-on-modify-request",false);
