// -----------------------------------------------------------------------------------
var debug = false;
// -----------------------------------------------------------------------------------
var byId = function (id) { return document.getElementById(id); };
var byClass = function (id) { return document.getElementsByClassName(id); };
var getIcon = function (icon) { return '<span class="icon-' + icon + '"></span>'; };
byId('index').style.display = "inline";
// -----------------------------------------------------------------------------------
var help_wifi = '<p><span>Verbinde dein WLANThermo mit deinem Heimnetzwerk und erweitere somit die Funktionsvielfalt deines WLANThermo.</span></p><p><span>Siehe auch: </span><a href="https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/WLAN" target="_blank"><span style="color:#3366ff">Wiki - WLAN</span></a></p>';
var help_bluetooth = '<p><span>Verbinde dein WLANThermo mit einem oder mehreren Bluetooth-Geräten und erweitere somit die Funktionsvielfalt deines WLANThermo.</span></p>';
var help_settings = '<p><span>Konfiguriere dein WLANThermo Nano mit Hilfe der Systemeinstellungen und mach es zu DEINEM WLANThermo.</span></p><p><span>Siehe auch: </span><a href="https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/System-Einstellungen" target="_blank"><span style="color:#3366ff">Wiki - System</span></a></p>';
var help_pitmaster = '<p><span>Willst du deine Grilltemperaturen nicht nur beobachten, sondern auch regeln? Dann besorge dir die passende Pitmaster-Hardware und leg los. Ben&ouml;tigt Pitmaster-Hardware.</span></p><p><span>Siehe auch: </span><a href="https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/Pitmaster" target="_blank"><span style="color:#3366ff">Wiki - Pitmaster</span></a></p>';
var help_notification = '<p><span>Lass dich bei einem Kanalalarm mit einer Push-Nachricht über den Nachrichtendienst Telegram oder Pushover benachrichtigen. Ben&ouml;tigt Internetzugang.</span></p><p><span>Siehe auch: </span><a href="https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/Push-Notification" target="_blank"><span style="color:#3366ff">Wiki - Notification</span></a></p>';
var help_mqtt = '<p><span>Kopple dein WLANThermo Nano mit deiner Homeautomatic und nutze dafür die MQTT-Schnittstelle deines WLANThermo. Ben&ouml;tigt einen MQTT-Broker.</span></p><p><span>Siehe auch: </span><a href="https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/Private-MQTT" target="_blank"><span style="color:#3366ff">Wiki - Private-MQTT</span></a></p>';
var help_cloud = '<p><span>Der Nano Cloud Service erm&ouml;glicht dir den gesch&uuml;tzten Zugriff auf deine Temperaturdaten auch au&szlig;erhalb deines Heim-Netzwerks. Ben&ouml;tigt Internetzugang. </span></p><p><span>Siehe auch: </span><a href="https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/Nano-Cloud-Service" target="_blank"><span style="color:#3366ff">Wiki - Nano Cloud</span></a></p>';
var help_channel = '<p><span>Mit Hilfe der Kanal-Einstellungen passt du den Messkanal an den angeschlossenen F&uuml;hler an. Zus&auml;tzlich kannst du Grenzwerte für die Alarmfunktion vorgeben.</span></p><p><span>Siehe auch: </span><a href="https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/Kanal-Einstellungen" target="_blank"><span style="color:#3366ff">Wiki - Kanal</span></a></p>';
// -----------------------------------------------------------------------------------
(function (window, document) {
    var layout = byId('layout'), menu = byId('menu'), menuLink = byId('menuLink'), menuHome = byId('menuHome'), menuPitmaster = byId('menuPitmaster'), menuNotification = byId('menuNotification'), menuWlan = byId('menuWlan'), menuBluetooth = byId('menuBluetooth'), menuSystem = byId('menuSystem'), menuIOT = byId('menuIOT'), menuHistory = byId('menuHistory'), menuAbout = byId('menuAbout'), content = byId('main');
    function toggleClass(element, className) {
        var classes = element.className.split('/\s+/'), length = classes.length;
        for (var i = 0; i < length; i++) {
            if (classes[i] === className) {
                classes.splice(i, 1);
                break;
            }
        }
        // The className is not found
        if (length === classes.length) {
            classes.push(className);
        }
        element.className = classes.join(' ');
    }

    function toggleAll(e) {
        var active = 'active';
        e.preventDefault();
        toggleClass(layout, active);
        toggleClass(menu, active);
        toggleClass(menuLink, active);
    }

    menuLink.onclick = function (e) { toggleAll(e); };
    menuHome.onclick = function (e) { if (menu.className.indexOf('active') !== -1) { toggleAll(e); showIndex(); } };
    menuWlan.onclick = function (e) { if (menu.className.indexOf('active') !== -1) { toggleAll(e); showWLAN(); } };
    menuBluetooth.onclick = function (e) { if (menu.className.indexOf('active') !== -1) { toggleAll(e); showBluetooth(); } };
    menuIOT.onclick = function (e) { if (menu.className.indexOf('active') !== -1) { toggleAll(e); showIOT(); } };
    menuPitmaster.onclick = function (e) { if (menu.className.indexOf('active') !== -1) { toggleAll(e); showPitmaster(0); } };
    menuNotification.onclick = function (e) { if (menu.className.indexOf('active') !== -1) { toggleAll(e); showPushNotification(); } };
    menuSystem.onclick = function (e) { if (menu.className.indexOf('active') !== -1) { toggleAll(e); showSystem(); } };
    menuHistory.onclick = function (e) { if (menu.className.indexOf('active') !== -1) { toggleAll(e); showHistory(); } };
    menuAbout.onclick = function (e) { if (menu.className.indexOf('active') !== -1) { toggleAll(e); showAbout(); } };
    content.onclick = function (e) { if (menu.className.indexOf('active') !== -1) { toggleAll(e); } };
}(this, this.document));

document.addEventListener("DOMContentLoaded", function (event) {
    setThermoName();
    setInterval("readTemp();", 2000);
});

function showWLAN() {
    hideAll();
    byId('wlan_config').style.display = "inline";
    networklist();
    networkscan();
}

function wlan_active() {
    if (byId('wlan_active').checked) {

    } else {
        txt = 'Soll die Wifi-Verbindung bis zum nächsten Restart des Systems wirklich abgeschaltet werden?';
        if (confirm(txt) == true) {
            loadJSON('stopwifi', '', '60000', function (response) {
                if (response == 'true') {
                    self.location.href = 'about:blank'
                }
            });
        } else {
            byId('wlan_active').checked = true;
        }
    }
}

function clearWifi() {
    txt = 'Sollen wirklich alle Netzwerkverbindungsdaten gelöscht werden?\nNano startet danach im AP-Modus neu.';
    if (confirm(txt) == true) {
        loadJSON('clearwifi', '', '60000', function (response) {
            if (response == 'true') {
                self.location.href = 'about:blank'
            }
        });
    }
}

function showBluetooth() {
    hideAll();
    byId('bluetooth_config').style.display = "inline";
    loadBluetoothList();
}

function bt_active() {
    if (byId('bt_active').checked) {
        loadJSON('btdevices', '', '60000', function (response) {
            if (response == 'true') {
                self.location.href = 'about:blank'
            }
        });
    } else {

    }
}

function showIOT() {
    hideAll();
    showLoader('true');
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        byId('PMQon').checked = jr.iot.PMQon;
        byId('PMQhost').value = jr.iot.PMQhost;
        byId('PMQport').value = jr.iot.PMQport;
        byId('PMQuser').value = jr.iot.PMQuser;
        byId('PMQpass').value = jr.iot.PMQpass;
        byId('PMQqos').value = jr.iot.PMQqos;
        byId('PMQinterval').value = jr.iot.PMQint;
        byId('CLon').checked = jr.iot.CLon;
        byId('CLinterval').value = jr.iot.CLint;
        byId('CLtoken').innerHTML = "https://" + jr.iot.CLurl + "?api_token=" + jr.iot.CLtoken;
        byId("CLtoken").setAttribute("href", "https://" + jr.iot.CLurl + "?api_token=" + jr.iot.CLtoken);
        byId('thingspeak').style.display = "inline";
        byId('mqttheader').style.display = "inline";
        showLoader('false');
    })
}

function newToken() {
    showLoader('true');
    loadJSON('newtoken', '', '3000', function (response) {
        var str = byId('CLtoken').innerHTML;
        var pos = str.lastIndexOf('=');
        str = str.substr(0, pos + 1);
        byId('CLtoken').innerHTML = str + response;
        byId("CLtoken").setAttribute("href", str + response);
        showLoader('false');
    })
}

function setThingspeak() {
    showLoader('true');
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        jr.iot.PMQon = byId('PMQon').checked;
        jr.iot.PMQhost = byId('PMQhost').value;
        jr.iot.PMQport = byId('PMQport').value;
        jr.iot.PMQuser = byId('PMQuser').value;
        jr.iot.PMQpass = byId('PMQpass').value;
        jr.iot.PMQqos = byId('PMQqos').value;
        jr.iot.PMQint = byId('PMQinterval').value;
        jr.iot.CLon = byId('CLon').checked;
        jr.iot.CLtoken = byId('CLtoken').value;
        jr.iot.CLint = byId('CLinterval').value;
        var data = JSON.stringify(jr.iot);
        showLoader('true');
        loadJSON('setIoT', data, '60000', function (response) {
            showIndex();
            showLoader('false');
        })
    })
}
// -----------------------------------------------------------------------------------
function showPushNotification() {
    hideAll();
    showLoader('true');
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        clearOption('push_service');
        for (var i = 0; i < jr.notes.ext.services.length; i++) {
            byId('push_service').options[byId('push_service').options.length] = new Option(jr.notes.ext.services[i], i);
        }
        byId('push_service').value = jr.notes.ext.service;
        byId('TGon').checked = jr.notes.ext.on;
        byId('TGtoken').value = jr.notes.ext.token;
        byId('TGid').value = jr.notes.ext.id;
        byId('push_repeat').value = jr.notes.ext.repeat;
        byId('push_notification').style.display = "inline";
        showLoader('false');
    })
}

function setPushNotification() {
    showLoader('true');
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        jr.notes.ext.on = byId('TGon').checked;
        jr.notes.ext.token = byId('TGtoken').value;
        jr.notes.ext.id = byId('TGid').value;
        jr.notes.ext.service = byId('push_service').value;
        jr.notes.ext.repeat = byId('push_repeat').value;
        var data = JSON.stringify(jr.notes.ext);
        showLoader('true');
        loadJSON('setPush', data, '60000', function (response) {
            showIndex();
            showLoader('false');
        })
    })
}

function sendPushNotification() {
    //eventuell zusammenfassen mit setPushNotification()
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        jr.notes.ext.on = 2;		// sende Testnachricht
        jr.notes.ext.token = byId('TGtoken').value;
        jr.notes.ext.id = byId('TGid').value;
        jr.notes.ext.service = byId('push_service').value;
        jr.notes.ext.repeat = byId('push_repeat').value;
        var data = JSON.stringify(jr.notes.ext);
        showLoader('true');
        loadJSON('setPush', data, '60000', function (response) {
            showLoader('false');
        })
    })

}
// -----------------------------------------------------------------------------------

function showHistory() {
    hideAll();
    byId('history').style.display = "inline";
    loadHistoryList();
}

function loadHistoryList() {
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        var json = new Object();
        json.device = jr.device;
        var history = { "task": "read" };
        json.history = history;
        console.log('HistoryRequest:' + JSON.stringify(json));
        loadJSON('https://api.wlanthermo.com/index1.php', JSON.stringify(json), '60000', function (response) {
            jr = JSON.parse(response);
            byId('historylist').innerHTML = '';
            var table_historylist = byId('historylist');
            for (var i = 0; i < jr.history.list.length; i++) {
                var row = table_historylist.insertRow(-1);
                addCell(row, new Date(jr.history.list[i].ts_start * 1000).format('d.m.y'), '', '0');
                addCell(row, 'ABCDEFGHIJKL', '', '1');
                addCell(row, '<span class="icon-stats-dots" onclick="goHistory(\'' + jr.history.list[i].api_token + '\')"></span>&nbsp;&nbsp;<span class="icon-download"> </span>&nbsp;&nbsp;<span class="icon-trash" onclick="deleteHistoryRow(\'' + jr.history.list[i].api_token + '\')"> </span>', '', '2');
            }
            showLoader('false');
        })
    })
}
function goHistory(history_token) {
    window.location = "https://cloud.wlanthermo.de/indexB.html?history_token=" + history_token;
}
function deleteHistoryRow(api_token) {
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        var json = new Object();
        json.device = jr.device;
        var history = { "task": "delete", "api_token": api_token };
        json.history = history;
        //alert(JSON.stringify(json));
        txt = 'Wollen Sie den aktuellen Eintrag wirklich löschen?';
        if (confirm(txt) == true) {
            loadJSON('https://api.wlanthermo.com/index1.php', JSON.stringify(json), '60000', function (response) {
                jr = JSON.parse(response);
                loadHistoryList();
                //alert(response);
                showLoader('false');
            })
        }
    })
}

function saveChartToHistory() {
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        var json = new Object();
        json.device = jr.device;
        var history = { "task": "save" };
        json.history = history;
        json.history.api_token = jr.iot.CLtoken;
        alert(JSON.stringify(json));
        loadJSON('https://api.wlanthermo.com/index1.php', JSON.stringify(json), '60000', function (response) {
            alert(response);
            loadHistoryList();
            showLoader('false');
        })
    })
}

// -----------------------------------------------------------------------------------

function showSetChannel(channel) {
    console.log(channel);
    hideAll();
    showLoader('true');
    byId('sensor').disabled = false;
    var sensors;
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        clearOption('sensor');
        sensors = jr.sensors;
        for (var i = 0; i < sensors.length; i++) {
            if (sensors[i].fixed == false) {
                byId('sensor').options[i] = new Option(sensors[i].name, sensors[i].type);
            }
        }
        loadJSON('data', '', '3000', function (response) {
            jr = JSON.parse(response);
            byId('channel_settings_save').setAttribute('onclick', 'setChannel(' + channel + ');');
            byId('channel_settings_headtitle').innerHTML = jr.channel[channel].name;
            byId('channel_name').value = jr.channel[channel].name;
            byId('temp_max').value = jr.channel[channel].max;
            byId('temp_min').value = jr.channel[channel].min;
            byId('btn-color-picker').style.color = jr.channel[channel].color;
            byId('btn-color-picker').setAttribute('data-color', jr.channel[channel].color);
            document.getElementsByClassName("color_border")[0].style.borderColor = jr.channel[channel].color;

            if (jr.channel[channel].fixed) {
                clearOption('sensor');
                for (var i = 0; i < sensors.length; i++) {
                    if (sensors[i].type == jr.channel[channel].typ) {
                        byId('sensor').options[i] = new Option(sensors[i].name, sensors[i].type);
                        break;
                    }
                }
                byId('sensor').disabled = true;
            }

            byId('sensor').value = jr.channel[channel].typ;

            switch (jr.channel[channel].alarm) {
                case 0:
                    byId('pushalarm').checked = false;
                    byId('hwalarm').checked = false;
                    break;
                case 1:
                    byId('pushalarm').checked = true;
                    byId('hwalarm').checked = false;
                    break;
                case 2:
                    byId('pushalarm').checked = false;
                    byId('hwalarm').checked = true;
                    break;
                case 3:
                    byId('pushalarm').checked = true;
                    byId('hwalarm').checked = true;
                    break;
            }
            byId('channel_settings').style.display = "inline";
            showLoader('false');
        })
    })
}

function setChannel(channel) {
    console.log('Test');
    console.log(byId('btn-color-picker').getAttribute('data-color'));
    showLoader('true');
    loadJSON('data', '', '3000', function (response) {
        jr = JSON.parse(response);
        jr.channel[channel].min = byId('temp_min').value;
        jr.channel[channel].max = byId('temp_max').value;
        jr.channel[channel].name = byId('channel_name').value;
        jr.channel[channel].typ = byId('sensor').value;
        var push = 0;
        if (byId('pushalarm').checked == false && byId('hwalarm').checked == false) {
            push = 0;
        } else if (byId('pushalarm').checked == true && byId('hwalarm').checked == false) {
            push = 1;
        } else if (byId('pushalarm').checked == false && byId('hwalarm').checked == true) {
            push = 2;
        } else if (byId('pushalarm').checked == true && byId('hwalarm').checked == true) {
            push = 3;
        }
        jr.channel[channel].alarm = push;
        jr.channel[channel].color = byId('btn-color-picker').getAttribute('data-color');
        var data = JSON.stringify(jr.channel[channel]);
        showLoader('true');
        loadJSON('setchannels', data, '60000', function (response) {
            showIndex();
            showLoader('false');
        })
    })
}
// -----------------------------------------------------------------------------------
function clearOption(id) {
    var selectbox = document.getElementById(id);
    var i;
    for (i = selectbox.options.length - 1; i >= 0; i--) {
        selectbox.remove(i);
    }
}
// -----------------------------------------------------------------------------------

function setThermoName() {
    showLoader('true');
    loadJSON('settings', '', '5000', function (response) {
        jr = JSON.parse(response);
        byId('thermoname').innerHTML = jr.system.host;
        if (jr.system.language != "de") {
            loadTranslation(jr.system.language);
        }

        if (jr.system.getupdate == 'false') {
            byClass("notification")[0].style.display = "none";
        } else {
            byClass("notification")[0].style.display = "inline";
        }

        byId('menuVersion').innerHTML = jr.device.sw_version;

        if (jr.features.pitmaster) {
            byId("menuPitmaster").style.display = "block";
        }

        if (jr.features.bluetooth) {
            byId("menuBluetooth").style.display = "block";
        }

        byId('menulist').style.display = "block";

        showLoader('false');
    })
}

// -----------------------------------------------------------------------------------

function clickCloudIcon() {
    showLoader('true');
    loadJSON('settings', '', '5000', function (response) {
        jr = JSON.parse(response);
        byId('thermoname').innerHTML = jr.system.host;
        if (jr.iot.CLon === true) {
            window.location = "https://" + jr.iot.CLurl + "?api_token=" + jr.iot.CLtoken;
        } else {

        }
        showLoader('false');
    })
}

function getCloudIcon(online) {
    switch (online) {
        case 0:
            return '';
            break;

        case 1:
            return '<span class="icon-cloud icon-red"> </span>';
            break;

        case 2:
            return '<span class="icon-cloud icon-green"> </span>';
            break;
        default:
            return '';
    }
}

// -----------------------------------------------------------------------------------

function showLoader(show) {
    if (show == 'true') {
        byId('cover').classList.add("cover_active");
        byId('spinner').classList.add("spinner");
    } else {
        byId('cover').classList.remove("cover_active");
        byId('spinner').classList.remove("spinner");
    }
}

// -----------------------------------------------------------------------------------

var updateActivated = 'false';
function checkUpdateActivated() {
    loadJSON('updatestatus', '', '3000', function (response) {
        jr = JSON.parse(response);
        if (jr == true) {
            showLoader('true');
        } else if (jr == false) {
            showLoader('false')
            updateActivated = 'false';
            byClass("notification")[0].style.display = "none";
            location.reload(true);
        }
    })
}

function update() {
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        txt = 'Wollen Sie das aktuelle Update installieren?\n\nAktuelle Version:' + jr.system.version + '\nNeue Version:' + jr.system.getupdate;
        if (confirm(txt) == true) {
            //data = 'version=' + jr.system.getupdate;
            loadJSON('update', '', '60000', function (response) {
                showLoader('true');
                updateActivated = 'true';
            });
        }
    })
}

// -----------------------------------------------------------------------------------

function showSystem() {
    hideAll();
    showLoader('true');
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        byId('language').value = jr.system.language;
        byId('hostname').value = jr.system.host;
        byId('apname').value = jr.system.ap;
        byId('thermoname').innerHTML = jr.system.host;
        byId('unit').value = jr.system.unit;
        byId('searchUpdate').checked = jr.system.autoupd;
        byId('prerelease').checked = jr.system.prerelease;
        clearOption('hwversion');
        for (var i = 0; i < jr.hardware.length; i++) {
            byId('hwversion').options[byId('hwversion').options.length] = new Option(jr.hardware[i], jr.hardware[i]);
        }
        byId('hwversion').value = jr.system.hwversion;
        byId('system_config').style.display = "inline";

        if (!jr.display.updname.startsWith("NX")) {
            byId('displayheader').style.display = "none";
            byId('displaybuttons').style.display = "none";
        }

        showLoader('false');
    })
}

function setSystem() {
    showLoader('true');
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        jr.system.host = byId('hostname').value.replace(/ /g, '_');
        jr.system.ap = byId('apname').value.replace(/ /g, '_');
        jr.system.hwversion = byId('hwversion').value;
        jr.system.language = byId('language').value;
        jr.system.unit = byId('unit').value;
        jr.system.autoupd = byId('searchUpdate').checked;
        jr.system.prerelease = byId('prerelease').checked;
        loadTranslation(jr.system.language);
        var data = JSON.stringify(jr.system);
        showLoader('true');
        loadJSON('setsystem', data, '60000', function (response) {
            byId('thermoname').innerHTML = jr.system.host;
            showIndex();
            showLoader('false');
        })
    })
}
// -----------------------------------------------------------------------------------

function showPitmaster() {
    hideAll();
    document.getElementById('pitmaster_id0').style.display = "none";
    document.getElementById('pitmaster_id1').style.display = "none";
    showLoader('true');
    loadJSON('data', '', '3000', function (response) {
        jr = JSON.parse(response);
        for (var pitID = 0; pitID < jr.pitmaster.pm.length; pitID++) {
            clearOption('pitmaster_typ' + pitID.toString());
            for (var j = 0; j < jr.pitmaster.type.length; j++) {
                byId('pitmaster_typ' + pitID.toString()).options[byId('pitmaster_typ' + pitID.toString()).options.length] = new Option(jr.pitmaster.type[j], jr.pitmaster.type[j]);
            }
            byId('pitmaster_typ' + pitID.toString()).value = jr.pitmaster.pm[pitID].typ;
            clearOption('pitmaster_channel' + pitID.toString());
            for (var i = 0; i < jr.channel.length; i++) {
                if (jr.channel[i].typ != 16) {
                    byId('pitmaster_channel' + pitID.toString()).options[byId('pitmaster_channel' + pitID.toString()).options.length] = new Option('#' + jr.channel[i].number + ' - ' + jr.channel[i].name, jr.channel[i].number);
                }
            }
            byId('pitmaster_channel' + pitID.toString()).value = jr.pitmaster.pm[pitID].channel;
            byId('pitmaster_set' + pitID.toString()).value = jr.pitmaster.pm[pitID].set;
            byId('pitmaster_value' + pitID.toString()).value = jr.pitmaster.pm[pitID].value;
            byId('pitmaster_settings').style.display = "inline";
            showPitmasterProfile(pitID, jr.pitmaster.pm[pitID].pid);
            changePitmasterTyp(pitID);
            changeActuatorTyp();
            document.getElementById('pitmaster_id' + pitID.toString()).style.display = "block";
        }
        showLoader('false');
    })
}

function changePitmasterTyp(id) {
    switch (byId('pitmaster_typ' + id.toString()).value) {
        case 'off':
            byId('pitmaster_value_show' + id.toString()).style.display = "none";
            byId('pitmaster_channel_show' + id.toString()).style.display = "none";
            byId('pitmaster_set_show' + id.toString()).style.display = "none";
            byId('pitmaster_profile_show' + id.toString()).style.display = "none";
            break;
        case 'manual':
            byId('pitmaster_value_show' + id.toString()).style.display = "inline";
            byId('pitmaster_channel_show' + id.toString()).style.display = "none";
            byId('pitmaster_set_show' + id.toString()).style.display = "none";
            byId('pitmaster_profile_show' + id.toString()).style.display = "inline";
            break;
        case 'auto':
            byId('pitmaster_profile_show' + id.toString()).style.display = "inline";
            byId('pitmaster_value_show' + id.toString()).style.display = "none";
            byId('pitmaster_channel_show' + id.toString()).style.display = "inline";
            byId('pitmaster_set_show' + id.toString()).style.display = "inline";
            break;
        default:
            byId('pitmaster_profile_show' + id.toString()).style.display = "inline";
            byId('pitmaster_value_show' + id.toString()).style.display = "inline";
            byId('pitmaster_channel_show' + id.toString()).style.display = "inline";
            byId('pitmaster_set_show' + id.toString()).style.display = "inline";
    }
}

function changeProfilShow(id) {
    switch (byId('pitmaster_typ' + id.toString()).value) {
        case 'off':
            byId('pid_settings_show').style.display = "none";
            byId('advanced_settings_show').style.display = "none";
            break;
        case 'manual':
            byId('pid_settings_show').style.display = "none";
            byId('advanced_settings_show').style.display = "none";
            break;
        case 'auto':
            byId('pid_settings_show').style.display = "inline";
            byId('advanced_settings_show').style.display = "inline";
            break;
        default:
            byId('pid_settings_show').style.display = "inline";
            byId('advanced_settings_show').style.display = "inline";
    }
}

function changeActuatorTyp() {
    switch (byId('pitmaster_profile_aktor').value) {
        case '3':
            byId('pid_dcvalue_show').style.display = "inline";
            byId('pid_spvalue_show').style.display = "inline";
            byId('pid_link_show').style.display = "inline";
            break;
        case '2':
            byId('pid_dcvalue_show').style.display = "none";
            byId('pid_spvalue_show').style.display = "inline";
            byId('pid_link_show').style.display = "none";
            break;
        default:
            byId('pid_dcvalue_show').style.display = "inline";
            byId('pid_spvalue_show').style.display = "none";
            byId('pid_link_show').style.display = "none";
    }
}

function setPitmaster() {
    showLoader('true');
    loadJSON('data', '', '3000', function (response) {
        jr = JSON.parse(response);
        for (var id = 0; id < jr.pitmaster.pm.length; id++) {
            jr.pitmaster.pm[id].id = id;
            jr.pitmaster.pm[id].typ = byId('pitmaster_typ' + id.toString()).value;
            jr.pitmaster.pm[id].channel = byId('pitmaster_channel' + id.toString()).value;
            jr.pitmaster.pm[id].set = byId('pitmaster_set' + id.toString()).value;
            jr.pitmaster.pm[id].value = byId('pitmaster_value' + id.toString()).value
            jr.pitmaster.pm[id].pid = byId('pid_profile' + id.toString()).value
        }
        var data = JSON.stringify(jr.pitmaster.pm);
        loadJSON('setpitmaster', data, '60000', function (response) {
            showIndex();
            showLoader('false');
        })
    })
    showLoader('false');
}

function showPitmasterProfileSettings(profile) {
    showLoader('true');
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        clearOption('pitmaster_profile_aktor');

        for (var i = 0; i < jr.aktor.length; i++) {
            byId('pitmaster_profile_aktor').options[byId('pitmaster_profile_aktor').options.length] = new Option(jr.aktor[i], i);
        }
        byId('pitmaster_profile_aktor').value = jr.pid[profile].aktor;
        byId('pid_name').value = jr.pid[profile].name;
        byId('pid_Kp').value = jr.pid[profile].Kp;
        byId('pid_Ki').value = jr.pid[profile].Ki;
        byId('pid_Kd').value = jr.pid[profile].Kd;
        byId('pid_DCmin').value = jr.pid[profile].DCmmin;
        byId('pid_DCmax').value = jr.pid[profile].DCmmax;
        byId('pid_opl').checked = jr.pid[profile].opl;
        byId('pid_auto').checked = jr.pid[profile].tune;
        byId('pid_jump').value = jr.pid[profile].jp;
        byId('pid_SPmin').value = jr.pid[profile].SPmin;
        byId('pid_SPmax').value = jr.pid[profile].SPmax;
        byId('pid_link').value = jr.pid[profile].link;
        showLoader('false');
        changeActuatorTyp();
    })
}

function showPitmasterProfile(id, profile) {
    showLoader('true');
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        var profileBackup = byId('pid_profile' + id.toString()).value;
        clearOption('pid_profile' + id.toString());
        for (var i = 0; i < jr.pid.length; i++) {
            byId('pid_profile' + id.toString()).options[byId('pid_profile' + id.toString()).options.length] = new Option(jr.pid[i].name, i);
        }

        if (typeof profile == 'undefined') {
            profile = profileBackup;
        }
        byId('pid_profile' + id.toString()).value = profile;
        showLoader('false');
    })
}

function togglePitmasterView(id) {
    if (typeof togglePitmasterView.id == 'undefined') {
        togglePitmasterView.id = 0;
    }

    if (typeof togglePitmasterView.profile == 'undefined') {
        togglePitmasterView.profile = 0;
    }

    if (typeof id == 'undefined') {
        id = togglePitmasterView.id;
    } else {
        togglePitmasterView.profile = byId('pid_profile' + id.toString()).value;
        togglePitmasterView.id = id;
    }

    if (byId('pitmaster_settings').style.display == "inline") {
        showPitmasterProfileSettings(byId('pid_profile' + id.toString()).value);
        showPitmasterProfile(id, byId('pid_profile' + id.toString()).value);
        changeProfilShow(id);
        byId('pitmaster_settings').style.display = "none";
        byId('pitmaster_profile').style.display = "inline";
    } else {
        showPitmasterProfile(0);
        showPitmasterProfile(1);
        byId('pitmaster_settings').style.display = "inline";
        byId('pitmaster_profile').style.display = "none";
    }
}

function backPidProfile() {
    togglePitmasterView();
}

function setPitmasterProfile() {
    loadJSON('settings', '', '3000', function (response) {
        jr = JSON.parse(response);
        profile = togglePitmasterView.profile;
        jr.pid[profile].aktor = byId('pitmaster_profile_aktor').value;
        jr.pid[profile].name = byId('pid_name').value;
        jr.pid[profile].Kp = byId('pid_Kp').value;
        jr.pid[profile].Ki = byId('pid_Ki').value;
        jr.pid[profile].Kd = byId('pid_Kd').value;
        jr.pid[profile].DCmmin = byId('pid_DCmin').value;
        jr.pid[profile].DCmmax = byId('pid_DCmax').value;
        jr.pid[profile].opl = byId('pid_opl').checked;
        jr.pid[profile].tune = byId('pid_auto').checked;
        jr.pid[profile].jp = byId('pid_jump').value;
        jr.pid[profile].SPmin = byId('pid_SPmin').value;
        jr.pid[profile].SPmax = byId('pid_SPmax').value;
        jr.pid[profile].link = byId('pid_link').value;
        var data = JSON.stringify(jr.pid);
        loadJSON('setpid', data, '60000', function (response) {
            togglePitmasterView();
        })
    })
    showLoader('false');
}

function setDC(dc) {
    if (dc == '0') {
        var_val = byId('pid_DCmin').value;
    } else {
        var_val = byId('pid_DCmax').value;
    }
    txt = 'Soll der Pitmasterausgang für 10 Sekunden mit dem angegebenen Wert angesteuert werden?';
    if (confirm(txt) == true) {
        data = '{"aktor":' + byId('pitmaster_profile_aktor').value + ',"dc":' + dc + ',"val":' + var_val * 10 + '}';
        loadJSON('setDC', data, '60000', function (response) {
            showLoader('true');
            dcActivated = 'true';
        })
    }
}

// -----------------------------------------------------------------------------------

function showIndex() {
    hideAll();
    byId('index').style.display = "inline";
}

function showAbout() {
    hideAll();
    byId('about').style.display = "inline";
}

function hideAll() {
    hideIDs = ['index', 'wlan_config', 'bluetooth_config', 'system_config', 'system_config', 'history', 'about', 'wlan_set', 'thingspeak', 'push_notification', 'channel_settings', 'pitmaster_settings', 'pitmaster_profile', 'extended_settings'];
    for (var i = 0; i < hideIDs.length; i++) {
        byId(hideIDs[i]).style.display = "none";
    }
    pk.close();
}

// -----------------------------------------------------------------------------------

function setgodmode(gnum) {
    txt = 'Änderungen durchführen?';
    if (confirm(txt) == true) {
        data = '{"god":' + gnum + '}';
        loadJSON('god', data, '60000', function (response) { })
    }
}

// -----------------------------------------------------------------------------------
function loadJSON(url, data, timeout, callback) {
    var xobj = new XMLHttpRequest();
    xobj.overrideMimeType("application/json");
    xobj.open('POST', url, true);
    if (url.indexOf("api.wlanthermo.com") != -1) {
        xobj.setRequestHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
    } else {
        xobj.setRequestHeader("Content-Type", "application/json");
    }
    xobj.timeout = timeout;
    xobj.onreadystatechange = function () {
        if (xobj.readyState == 4 && xobj.status == "200") {
            callback(xobj.responseText);
        }
    }
    xobj.ontimeout = function (e) {
        if (updateActivated != 'true') {
            showLoader('false');
        }
    };
    xobj.send(data);
}

// -----------------------------------------------------------------------------------

var showBatteryWlanIcon = true;
function setshowBatteryWlanIcon() {
    showBatteryWlanIcon = false;
    readTemp();
    setTimeout(function () {
        showBatteryWlanIcon = true;
    }, 10000);
}

function getBatteryIcon(percent, charge) {
    data = '';
    if (showBatteryWlanIcon == true) {
        if (charge == true) {
            data = '<span class="icon-power-cord"> </span>';
        }

        if (percent >= '90') {
            data = data + getIcon('battery-100');
        } else if (percent >= '75') {
            data = data + getIcon('battery-75');
        } else if (percent >= '50') {
            data = data + getIcon('battery-50');
        } else if (percent >= '15') {
            data = data + getIcon('battery-25');
        } else if (percent >= '10') {
            data = data + getIcon('battery-0 icon-red');
        } else {
            data = data + getIcon('battery-0 icon-red icon-blinker');
        }
        return data + '&nbsp;';
    } else {
        if (charge == true) {
            data = getIcon('power-cord');
        } else {
            data = '';
        }
        data = data + percent + '%';
        return data + '&nbsp;';
    }
}

// -----------------------------------------------------------------------------------
// Initialized Channels
// -----------------------------------------------------------------------------------

var channel_init = false;
var channel_count = false;

function addChannel(number) {
    if (debug === true) { console.log('function addChannel number:' + number); }
    for (i = 0; i < number; i++) {
        var parent = document.getElementById("channel_template");
        NodeList.prototype.forEach = Array.prototype.forEach;
        var children = parent.childNodes;
        children.forEach(function (item) {
            var cln = item.cloneNode(true);
            document.getElementById("index_content").appendChild(cln);
        });
        document.getElementById("index_content").lastElementChild.classList.add("temp_index");
        document.getElementById("index_content").lastElementChild.setAttribute('data-channel', i);
    }
    channel_init = true;
    if (debug === true) { console.log('channel_init:' + channel_init); }
    channel_count = number;
    if (debug === true) { console.log('channel_count:' + number); }
}

// read temp values
// -----------------------------------------------------------------------------------

function readTemp() {
    if (updateActivated == 'true') {
        checkUpdateActivated();
    } else if (dcActivated == 'true') {
        checkDCActivated();
    } else {
        loadJSON('data', '', '3000', function (response) {
            jr = JSON.parse(response);

            if (channel_init === false) { addChannel(jr.channel.length); }

            if (typeof jr.system.soc != "undefined") {
                BatteryIcon = getBatteryIcon(jr.system.soc, jr.system.charge);
                byClass("battery")[0].innerHTML = BatteryIcon;
            }
            byClass("cloud")[0].innerHTML = getCloudIcon(jr.system.online);
            RSSIIcon = getRSSIIcon(jr.system.rssi);
            byClass("wifi")[0].innerHTML = RSSIIcon;
            var x = byClass("pure-u-1-1 pure-u-md-1-2 pure-u-xl-1-4 temp_index");
            var i;
            var pitID = 0;
            var ch_count = 0;

            for (i = 0; i < x.length; i++) {
                x[i].getElementsByClassName('1-box channel')[0].style.borderColor = jr.channel[i].color;
                x[i].getElementsByClassName('chtitle')[0].innerHTML = jr.channel[i].name;

                for (pitID = 0; pitID < jr.pitmaster.pm.length; pitID++) {
                    if (jr.pitmaster.pm[pitID].channel == jr.channel[i].number)
                        break;
                }

                if (pitID < jr.pitmaster.pm.length) {
                    if (jr.pitmaster.pm[pitID].typ == 'auto') {
                        var a = getIcon('uniF2C7') + jr.pitmaster.pm[pitID].set + '°' + ' / ';
                        var e = ' ' + jr.pitmaster.pm[pitID].value + '%';
                        if (jr.pitmaster.pm[pitID].value == '0') {
                            x[i].getElementsByClassName('chtitle')[0].innerHTML = a + getIcon('fan') + ' ' + jr.pitmaster.pm[pitID].value + '%';
                        } else if (jr.pitmaster.pm[pitID].value <= '25') {
                            x[i].getElementsByClassName('chtitle')[0].innerHTML = a + getIcon('fan icon-rotate-25') + e;
                        } else if (jr.pitmaster.pm[pitID].value <= '50') {
                            x[i].getElementsByClassName('chtitle')[0].innerHTML = a + getIcon('fan icon-rotate-50') + e;
                        } else if (jr.pitmaster.pm[pitID].value <= '75') {
                            x[i].getElementsByClassName('chtitle')[0].innerHTML = a + getIcon('fan icon-rotate-75') + e;
                        } else if (jr.pitmaster.pm[pitID].value <= '100') {
                            x[i].getElementsByClassName('chtitle')[0].innerHTML = a + getIcon('fan icon-rotate-100') + e;
                        }
                    } else {
                        x[i].getElementsByClassName('chtitle')[0].innerHTML = jr.channel[i].name;
                    }
                } else {
                    x[i].getElementsByClassName('chtitle')[0].innerHTML = jr.channel[i].name;
                }

                if (jr.channel[i].typ == 16) {
                    if (jr.channel[i].connected == true)
                        x[i].getElementsByClassName('chnumber')[0].innerHTML = getIcon('bluetooth_1') + '\t' + "#" + jr.channel[i].number;
                    else
                        x[i].getElementsByClassName('chnumber')[0].innerHTML = getIcon('bluetooth_1 icon-disabled') + '\t' + "#" + jr.channel[i].number;
                }
                else {
                    x[i].getElementsByClassName('chnumber')[0].innerHTML = "#" + jr.channel[i].number;
                }


                x[i].getElementsByClassName('tempmin')[0].innerHTML = getIcon('temp_down') + jr.channel[i].min + "°";
                x[i].getElementsByClassName('tempmax')[0].innerHTML = getIcon('temp_up') + jr.channel[i].max + "°";
                if (jr.channel[i].temp == '999') {
                    x[i].style.display = 'none';
                    x[i].getElementsByClassName('temp')[0].innerHTML = 'OFF';
                    x[i].getElementsByClassName('temp')[0].style.color = "#FFFFFF";
                    x[i].getElementsByClassName('temp')[0].style.fontWeight = 'normal';
                } else {
                    ch_count++;
                    x[i].style.display = 'inline';
                    x[i].getElementsByClassName('temp')[0].innerHTML = jr.channel[i].temp.toFixed(1) + "°" + jr.system.unit;
                    if (jr.channel[i].temp < jr.channel[i].min) {
                        x[i].getElementsByClassName('temp')[0].style.color = "#1874cd";
                        x[i].getElementsByClassName('temp')[0].style.fontWeight = 'bold';
                    } else if (jr.channel[i].temp > jr.channel[i].max) {
                        x[i].getElementsByClassName('temp')[0].style.color = "red";
                        x[i].getElementsByClassName('temp')[0].style.fontWeight = 'bold';
                    } else {
                        x[i].getElementsByClassName('temp')[0].style.color = "#FFFFFF";
                        x[i].getElementsByClassName('temp')[0].style.fontWeight = 'normal';
                    }
                }
            }
            if (ch_count == 0) {
                for (i = 0; i < x.length; i++) {
                    x[i].style.display = 'inline';
                }
            }
        })
    }
}

// Wifi connection
// -----------------------------------------------------------------------------------

function showSetWLAN() {
    hideAll();
    byId('wlan_set').style.display = "inline";
}

function setWLAN(ssid, passwd) {
    showLoader('true');
    var data = '{"ssid":"' + ssid + '","password":"' + passwd + '"}';
    loadJSON('setnetwork', data, '60000', function (response) {
        showWLAN();
        showLoader('false');
    })

}

function networkscan() {
    loadJSON('networkscan', '', '3000', function (response) {
        byId('networkrefresh').innerHTML = '<h3><span class="icon-refresh icon-rotate-50" onclick="networkscan();"></span></h3>'
        if (response == "OK") {
            setTimeout("networklist();", 3000);
        }
    })
}

function networklist() {
    loadJSON('networklist', '', '3000', function (response) {
        try {
            jr = JSON.parse(response);
            jr.Scan.sort(function (a, b) { return b.RSSI - a.RSSI; });
            byId('networklist').innerHTML = '';
            byId('networkconnect').innerHTML = '';
            var table_networklist = byId('networklist');
            var table_networkconnect = byId('networkconnect');
            if (jr.Connect) {
                byId('hr_wifi').style.display = "";
                var row = table_networkconnect.insertRow(0);
                addCell(row, getIcon('ok'), 'row-Check', '0');
                addCell(row, jr.SSID + " (" + jr.BSSID + ")", 'row-SSID', '1');
                addCell(row, getRSSIIcon(jr.RSSI), 'row-RSSI', '2');
                addCell(row, getWifiSecureIcon(jr.Enc), 'row-Enc', '3');
                addCell(row, '', 'row-link', '4');
            } else {
                byId('hr_wifi').style.display = "none";
            }
            NetworkconnectRowClick();
            for (var i = 0; i < jr.Scan.length; i++) {
                var row = table_networklist.insertRow(-1);
                addCell(row, '', 'row-Check', '0');
                addCell(row, jr.Scan[i].SSID, 'row-SSID', '1');
                addCell(row, getRSSIIcon(jr.Scan[i].RSSI), 'row-RSSI', '2');
                addCell(row, getWifiSecureIcon(jr.Scan[i].Enc), 'row-Enc', '3');
                addCell(row, getIcon('circle-right'), 'row-link', '4');
            }

            //console.log(jr.Scan);
            byId('networkrefresh').innerHTML = '<h3><span class="icon-refresh" onclick="networkscan();"></span></h3>'
            NetworklistRowClick();

        } catch (e) {
            // error in the above string (in this case, yes)!
            //alert(e);
        }
    })
}

function getUnique(arr, comp) {
    const unique = arr
        .map(e => e[comp])

        // store the keys of the unique objects
        .map((e, i, final) => final.indexOf(e) === i && i)

        // eliminate the dead keys & store unique objects
        .filter(e => arr[e]).map(e => arr[e]);

    return unique;
}

var ssidInput;
function NetworklistRowClick() {
    var table = document.getElementById("networklist");
    var rows = table.rows; // or table.getElementsByTagName("tr");
    for (var i = 0; i < rows.length; i++) {
        rows[i].onclick = (function () { // closure
            var cnt = i; // save the counter to use in the function
            return function () {
                document.getElementsByName('ssidInput')[0].value = this.cells[1].innerHTML;
                document.getElementsByName('ssidPasswordInput')[0].value = null;
                ssidInput = this.cells[1].innerHTML;
                showSetWLAN();
            }
        })(i);
    }
}

function setssidInput() {
    document.getElementsByName('ssidInput')[0].value = ssidInput;
}

function NetworkconnectRowClick() {
    var table = byId('networkconnect');
    var rows = table.rows; // or table.getElementsByTagName("tr");
    for (var i = 0; i < rows.length; i++) {
        rows[i].onclick = (function () { // closure
            var cnt = i; // save the counter to use in the function
            return function () {
            }
        })(i);
    }
}

function getRSSIIcon(dbm) {
    data = '';
    if (showBatteryWlanIcon == true) {
        if (dbm >= '-80') {
            data = data + getIcon('Wlan100');
        } else if (dbm >= '-95') {
            data = data + getIcon('Wlan66');
        } else if (dbm >= '-105') {
            data = data + getIcon('Wlan33');
        }
        return data;
    } else {
        data = dbm + 'dBm';
        return data;
    }
}

function getWifiSecureIcon(data) {
    var lock = getIcon('lock');
    var unlock = getIcon('unlock');
    switch (data) {
        case 2: //ENC_TYPE_TKIP - WPA / PSK
            return lock;
            break;
        case 4: //ENC_TYPE_CCMP - WPA2 / PSK
            return lock;
            break;
        case 5: //ENC_TYPE_WEP - WEP
            return lock;
            break;
        case 7: //ENC_TYPE_NONE - open network
            return unlock;
            break;
        case 8:	//ENC_TYPE_AUTO - WPA / WPA2 / PSK
            return lock;
            break;
        default:
            return lock;
    }
}

// Bluetooth connection
// -----------------------------------------------------------------------------------
var bluetoothlist = null;
function loadBluetoothList() {
    btDevices = null;
    loadJSON('bluetooth', '', '3000', function (response) {
        try {
            bluetoothlist = JSON.parse(response);
            byId('bluetoothlist').innerHTML = '';
            var table_bluetoothlist = byId('bluetoothlist');

            for (var deviceIndex = 0; deviceIndex < bluetoothlist.devices.length; deviceIndex++) {
                var deviceRow = table_bluetoothlist.insertRow(-1);

                addCellBtDeviceCheckbox(deviceRow, bluetoothlist.devices, deviceIndex, 'row-Check', '0');
                addCell(deviceRow, "", 'row-Check', '1');
                addCell(deviceRow, bluetoothlist.devices[deviceIndex].name + " (" + bluetoothlist.devices[deviceIndex].address + ")", 'row-SSID', '2');

                for (var i = 0; i < bluetoothlist.devices[deviceIndex].count; i++) {
                    var row = table_bluetoothlist.insertRow(-1);
                    addCell(row, "", 'row-Check', '0');
                    addCellBtChannelCheckbox(row, bluetoothlist.devices, deviceIndex, i, 'row-Check', '1');
                    addCell(row, "Kanal " + (i + 1), 'row-SSID', '2');
                }
            }

            //byId('networkrefresh').innerHTML = '<h3><span class="icon-refresh" onclick="networkscan();"></span></h3>'

        } catch (e) {
            // error in the above string (in this case, yes)!
            //alert(e);
        }
    })
}
function btSave() {
    if (bluetoothlist != null) {
        showLoader('true');
        var data = JSON.stringify(bluetoothlist);
        loadJSON('setbluetooth', data, '60000', function (response) {
            location.reload(true);
            showLoader('false');
        });
        showLoader('false');
    }
}

// -----------------------------------------------------------------------------------

var dcActivated = 'false';
function checkDCActivated() {
    loadJSON('dcstatus', '', '3000', function (response) {
        jr = JSON.parse(response);
        if (jr == true) {
            showLoader('true');
        } else if (jr == false) {
            showLoader('false')
            dcActivated = 'false';
        }
    })
}
// -----------------------------------------------------------------------------------

function addCellBtDeviceCheckbox(row, devices, deviceIndex, cellClass, nr) {
    var checkbox = document.createElement("input");
    checkbox.type = "checkbox";
    checkbox.checked = (devices[deviceIndex].selected > 0);
    checkbox.onclick = function (evt) {
        checkbox.checked = !checkbox.checked;
    };
    var cell = row.insertCell(nr);
    cell.appendChild(checkbox);
    if (cellClass != '') {
        cell.className = cellClass;
    }

    row.onclick = function () {
        return function () {
            checkbox.checked = !checkbox.checked;
            if (checkbox.checked) {
                devices[deviceIndex].selected = (1 << devices[deviceIndex].count) - 1;
                for (var i = 1; i <= devices[deviceIndex].count; i++) {
                    var table = byId('bluetoothlist');
                    table.rows[row.rowIndex + i].cells[1].childNodes[0].checked = true;
                }
            }
            else {
                devices[deviceIndex].selected = 0;
                for (var i = 1; i <= devices[deviceIndex].count; i++) {
                    var table = byId('bluetoothlist');
                    table.rows[row.rowIndex + i].cells[1].childNodes[0].checked = false;
                }
            }

        };
    }(row);
}

function addCellBtChannelCheckbox(row, devices, deviceIndex, channelIndex, cellClass, nr) {
    var checkbox = document.createElement("input");
    checkbox.type = "checkbox";
    checkbox.checked = (devices[deviceIndex].selected & (1 << channelIndex)) > 0;
    checkbox.onclick = function (evt) {
        checkbox.checked = !checkbox.checked;
    };
    var cell = row.insertCell(nr);
    cell.appendChild(checkbox);
    if (cellClass != '') {
        cell.className = cellClass;
    }

    row.onclick = function () {
        return function () {
            checkbox.checked = !checkbox.checked;
            if (checkbox.checked) {
                devices[deviceIndex].selected |= (1 << channelIndex);
            }
            else {
                devices[deviceIndex].selected &= ~(1 << channelIndex);
            }

        };
    }(row);
}

function addCell(row, cellinnerHtml, cellClass, nr) {
    var cell = row.insertCell(nr);
    cell.innerHTML = cellinnerHtml;
    if (cellClass != '') {
        cell.className = cellClass;
    }
}

// -----------------------------------------------------------------------------------
function validateNumber(number, decimal) {
    //val = number.value.replace(/[^0-9.,]/g,'').replace(/[,]/g, '.');
    val = number.value.replace(/[^0-9\-.,]+\-/g, '').replace(/[,]/g, '.');
    switch (decimal) {
        case '1':
            if (val) { val = val.toString().match(/^-?\d+(?:\.\d{0,1})?/)[0]; }
            break;
        case '2':
            if (val) { val = val.toString().match(/^-?\d+(?:\.\d{0,2})?/)[0]; }
            break;
        case '3':
            if (val) { val = val.toString().match(/^-?\d+(?:\.\d{0,3})?/)[0]; }
            break;
        case '4':
            if (val) { val = val.toString().match(/^-?\d+(?:\.\d{0,4})?/)[0]; }
            break;
        default:
            if (val) { val = val.toString().match(/^-?\d+(?:\.\d{0,1})?/)[0]; }
    }
    if (val.split('.').length > 2) { val = val.replace(/\.+$/, ""); }
    number.value = val;
}

// reboot device
// -----------------------------------------------------------------------------------

function restart() {
    txt = 'Nano neu starten?';
    if (confirm(txt) == true) {
        loadJSON('restart', '', '60000', function (response) { });
    }
}

function rotate() {
    txt = 'Neustarten, um das Display zu drehen?';
    if (confirm(txt) == true) {
        showLoader('true');
        loadJSON('rotate', '', '60000', function (response) { });
        setTimeout(function () { updateActivated = 'true'; }, 5000);
    }
}

function calibrate() {
    loadJSON('calibrate', '', '60000', function (response) { });
}

// show hidden Menu
// -----------------------------------------------------------------------------------

function showExtendedSettings() {
    hideAll();
    byId('extended_settings').style.display = "inline";
}

var clickTimer = null;
function touchStart() {
    if (clickTimer == null) {
        clickTimer = setTimeout(function () {
            clickTimer = null;
        }, 500)
    } else {
        clearTimeout(clickTimer);
        clickTimer = null;
        showExtendedSettings();
    }
}
// translation function
// -----------------------------------------------------------------------------------
function loadTranslation(language) {
    var translate = new Translate();
    var currentLng = language;//'fr'
    var attributeName = 'data-tag';
    translate.init(attributeName, currentLng);
    translate.process();
}
function Translate() {
    //initialization
    this.init = function (attribute, lng) {
        console.log("Sprache:" + lng);
        this.attribute = attribute;
        this.lng = lng;
    }
    //translate
    this.process = function () {
        _self = this;
        var LngObject = window[this.lng];
        console.log(LngObject["name1"]);
        var allDom = document.getElementsByTagName("*");
        for (var i = 0; i < allDom.length; i++) {
            var elem = allDom[i];
            var key = elem.getAttribute(_self.attribute);
            if (key != null) {
                console.log(key);
                elem.innerHTML = LngObject[key];
            }
        }
    }
}
// -----------------------------------------------------------------------------------

function setHelpBox(title, v) {
    byId('helpBoxTitle').innerHTML = title;
    document.getElementById("helpBoxObject").innerHTML = v;
    byId('helpBox').style.display = 'block';
}
readTemp();
/* eslint no-extend-native: 0 */

(function () {
    // Defining locale
    Date.shortMonths = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec']
    Date.longMonths = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December']
    Date.shortDays = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat']
    Date.longDays = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday']
    // Defining patterns
    var replaceChars = {
        // Day
        d: function () { var d = this.getDate(); return (d < 10 ? '0' : '') + d },
        D: function () { return Date.shortDays[this.getDay()] },
        j: function () { return this.getDate() },
        l: function () { return Date.longDays[this.getDay()] },
        N: function () { var N = this.getDay(); return (N === 0 ? 7 : N) },
        S: function () { var S = this.getDate(); return (S % 10 === 1 && S !== 11 ? 'st' : (S % 10 === 2 && S !== 12 ? 'nd' : (S % 10 === 3 && S !== 13 ? 'rd' : 'th'))) },
        w: function () { return this.getDay() },
        z: function () { var d = new Date(this.getFullYear(), 0, 1); return Math.ceil((this - d) / 86400000) },
        // Week
        W: function () {
            var target = new Date(this.valueOf())
            var dayNr = (this.getDay() + 6) % 7
            target.setDate(target.getDate() - dayNr + 3)
            var firstThursday = target.valueOf()
            target.setMonth(0, 1)
            if (target.getDay() !== 4) {
                target.setMonth(0, 1 + ((4 - target.getDay()) + 7) % 7)
            }
            var retVal = 1 + Math.ceil((firstThursday - target) / 604800000)

            return (retVal < 10 ? '0' + retVal : retVal)
        },
        // Month
        F: function () { return Date.longMonths[this.getMonth()] },
        m: function () { var m = this.getMonth(); return (m < 9 ? '0' : '') + (m + 1) },
        M: function () { return Date.shortMonths[this.getMonth()] },
        n: function () { return this.getMonth() + 1 },
        t: function () {
            var year = this.getFullYear()
            var nextMonth = this.getMonth() + 1
            if (nextMonth === 12) {
                year = year++
                nextMonth = 0
            }
            return new Date(year, nextMonth, 0).getDate()
        },
        // Year
        L: function () { var L = this.getFullYear(); return (L % 400 === 0 || (L % 100 !== 0 && L % 4 === 0)) },
        o: function () { var d = new Date(this.valueOf()); d.setDate(d.getDate() - ((this.getDay() + 6) % 7) + 3); return d.getFullYear() },
        Y: function () { return this.getFullYear() },
        y: function () { return ('' + this.getFullYear()).substr(2) },
        // Time
        a: function () { return this.getHours() < 12 ? 'am' : 'pm' },
        A: function () { return this.getHours() < 12 ? 'AM' : 'PM' },
        B: function () { return Math.floor((((this.getUTCHours() + 1) % 24) + this.getUTCMinutes() / 60 + this.getUTCSeconds() / 3600) * 1000 / 24) },
        g: function () { return this.getHours() % 12 || 12 },
        G: function () { return this.getHours() },
        h: function () { var h = this.getHours(); return ((h % 12 || 12) < 10 ? '0' : '') + (h % 12 || 12) },
        H: function () { var H = this.getHours(); return (H < 10 ? '0' : '') + H },
        i: function () { var i = this.getMinutes(); return (i < 10 ? '0' : '') + i },
        s: function () { var s = this.getSeconds(); return (s < 10 ? '0' : '') + s },
        v: function () { var v = this.getMilliseconds(); return (v < 10 ? '00' : (v < 100 ? '0' : '')) + v },
        // Timezone
        e: function () { return Intl.DateTimeFormat().resolvedOptions().timeZone },
        I: function () {
            var DST = null
            for (var i = 0; i < 12; ++i) {
                var d = new Date(this.getFullYear(), i, 1)
                var offset = d.getTimezoneOffset()

                if (DST === null) DST = offset
                else if (offset < DST) { DST = offset; break } else if (offset > DST) break
            }
            return (this.getTimezoneOffset() === DST) | 0
        },
        O: function () { var O = this.getTimezoneOffset(); return (-O < 0 ? '-' : '+') + (Math.abs(O / 60) < 10 ? '0' : '') + Math.floor(Math.abs(O / 60)) + (Math.abs(O % 60) === 0 ? '00' : ((Math.abs(O % 60) < 10 ? '0' : '')) + (Math.abs(O % 60))) },
        P: function () { var P = this.getTimezoneOffset(); return (-P < 0 ? '-' : '+') + (Math.abs(P / 60) < 10 ? '0' : '') + Math.floor(Math.abs(P / 60)) + ':' + (Math.abs(P % 60) === 0 ? '00' : ((Math.abs(P % 60) < 10 ? '0' : '')) + (Math.abs(P % 60))) },
        T: function () { var tz = this.toLocaleTimeString(navigator.language, { timeZoneName: 'short' }).split(' '); return tz[tz.length - 1] },
        Z: function () { return -this.getTimezoneOffset() * 60 },
        // Full Date/Time
        c: function () { return this.format('Y-m-d\\TH:i:sP') },
        r: function () { return this.toString() },
        U: function () { return Math.floor(this.getTime() / 1000) }
    }

    // Simulates PHP's date function
    Date.prototype.format = function (format) {
        var date = this
        return format.replace(/(\\?)(.)/g, function (_, esc, chr) {
            return (esc === '' && replaceChars[chr]) ? replaceChars[chr].call(date) : chr
        })
    }
}).call(this)

window.addEventListener("load", function () {
    pk = new Piklor(".color-picker", [
        "#FFFF00"
        , "#FFC002"
        , "#00FF00"
        , "#FFFFFF"
        , "#FF1DC4"
        , "#E46C0A"
        , "#C3D69B"
        , "#0FE6F1"
        , "#0000FF"
        , "#03A923"
        , "#C84B32"
        , "#FF9B69"
        , "#5082BE"
        , "#FFB1D0"
        , "#A6EF03"
        , "#D42A6B"
        , "#FFDA8F"
        , "#00B0F0"
        , "#948A54"
    ], {
        open: ".picker-wrapper .btn"
    })
        //, wrapperEl = pk.getElm(".picker-wrapper")
        //, border = pk.getElm("color_border")
        //, footer = pk.getElm("footer")
        ;

    pk.colorChosen(function (col) {
        //wrapperEl.style.backgroundColor = col;
        //byId("btn-color-picker").style.backgroundColor = col;
        byId('btn-color-picker').style.color = col;
        var x = document.getElementsByClassName("color_border");
        var i;
        for (i = 0; i < x.length; i++) {
            x[i].style.borderColor = col;
        }
        byId('btn-color-picker').setAttribute('data-color', col);

    });
});