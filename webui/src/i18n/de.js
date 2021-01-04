export default {
	// validation
	v_must_between: 'Muss zwischen {min} und [max} sein',
	// help text
	help_channel: 'Mit Hilfe der Kanal-Einstellungen passt du den Messkanal an den angeschlossenen Fühler an. Zusätzlich kannst du Grenzwerte für die Alarmfunktion vorgeben.',
	help_channel_link: 'Kanal',
	help_channel_title: "Kanal Einstellungen",
	help_system_title: "System Einstellungen",
	help_system_link: 'System',
	help_system: "Konfiguriere dein WLANThermo mit Hilfe der Systemeinstellungen und mach es zu DEINEM WLANThermo.",
	help_notification_title: "Push Einstellungen",
	help_notification_link: 'Notification',
	help_notification: "Lass dich bei einem Kanalalarm mit einer Push-Nachricht über den Nachrichtendienst Telegram oder Pushover benachrichtigen. Benötigt Internetzugang.",
	help_wlan_title: "WLAN Einstellungen",
	help_wlan_link: 'WLAN',
	help_wlan: "Verbinde dein WLANThermo mit deinem Heimnetzwerk und erweitere somit die Funktionsvielfalt deines WLANThermo.",
	help_bluetooth_title: "Bluetooth-Einstellungen",
	help_bluetooth: "Verbinde dein WLANThermo mit einem oder mehreren Bluetooth-Geräten und erweitere somit die Funktionsvielfalt deines WLANThermo.",
	help_pitmaster_title: "Pitmaster settings",
	help_pitmaster_link: 'Pitmaster',
	help_pitmaster: "Willst du deine Grilltemperaturen nicht nur beobachten, sondern auch regeln? Dann besorge dir die passende Pitmaster-Hardware und leg los. Benötigt Pitmaster-Hardware.",
	help_iot_title: "WLANThermo Cloud",
	help_iot_link: 'Cloud',
	help_iot: "Der Cloud Service ermöglicht dir den geschützten Zugriff auf deine Temperaturdaten auch außerhalb deines Heim-Netzwerks. Benötigt Internetzugang.",
	help_mqtt_title: "Private MQTT Client",
	help_mqtt_link: 'Private-MQTT',
	help_mqtt: "Kopple dein WLANThermo mit deiner Homeautomatic und nutze dafür die MQTT-Schnittstelle deines WLANThermo. Benötigt einen MQTT-Broker.",
	// update
	update_prompt: "Wollen Sie das aktuelle Update installieren?",
	current_verision: "Aktuelle Version",
	new_version: "Neue Version",
	// shared
	see_also: "Siehe auch",
	// Reiter System-Einstellungen
	system_settings: "System-Einstellungen",
	unit: "Einheit",
	german: "Deutsch",
	english: "Englisch",
	fahrenheit: "Fahrenheit",
	celsius: "Celsius",
	language: "Sprache",
	hostname: "Hostname",
	ap_name: "APN",
	hw_version: "Hardwareversion",
	update_search: "Automatisch nach Updates suchen",
	prerelease: "Beta-Versionen in der Suche mit einschießen",
	// Reiter Display-Einstellungen
	display_settings: "Display-Einstellungen",
	rotate_display: "Display drehen",
	calibrate_touch: "Touch kalibrieren",
	rotate_restart_prompt: "Neustarten, um das Display zu drehen?",
	// Buttons
	back: "Zurück",
	save: "Speichern",
	cancel: "Abbrechen",
	//Kanaleinstellungen
	channel_name: "Kanalname",
	temp_max: "Temperatur-Obergrenze",
	temp_min: "Temperatur-Untergrenze",
	temp_sensor: "Fühlertyp",
	color: "Farbe",
	push_alarm: "Push-Alarm",
	buzzer_alarm: "Piepser-Alarm",
	//Menü
	menuHome: "Home",
	menuWlan: "WLAN",
	menuBluetooth: "Bluetooth",
	menuSystem: "System",
	menuPitmaster: "Pitmaster",
	menuIOT: "IoT",
	menuHistory: "Historie",
	menuNotification: "Benachrichtigung",
	menuAbout: "Über",
	//History
	btnSaveChart: "Chart speichern",
	historyTitle: "Historie",
	date: "Datum",
	historyTableName: "Name",
	//Menü About
	aboutTitle: "WLANThermo Team",
	forumTitle: "WLANThermo Forum",
	forumUrl: "Wir freuen uns über deinen Besuch im Forum!",
	//Menü Notification
	notificationTitle: "Benachrichtungsdienst",
	notificationActivate: "Push-Benachrichtung aktivieren",
	notificationToken: "Token",
	notificationChatId: "Chat ID",
    notificationUserKey: "User Key",
	notificationService: "Dienst",
	repeadOnce: "1x",
	repeadThreeTimes: "3x",
	repeadFiveTimes: "5x",
	repeadTenTimes: "10x",
	notificationRepead: "Wiederholungen",
	notificationSendMessage: "Testnachricht senden",
    notificationPriorityNormal: "Normal",
    notificationPriorityHigh: "Hoch",
    notificationPriority: "Priorität",
	//Cloud
	cloudTitle: "Cloud Service",
	cloudActivate: "Cloud Service aktivieren",
	cloudUrl: "Cloud Link",
	tenSeconds: "10 Sekunden",
	fifteenSeconds: "15 Sekunden",
	thirtySeconds: "30 Sekunden",
	oneMinutes: "1 Minute",
	twoMinutes: "2 Minuten",
	fiveMinutes: "5 Minuten",
	sendInterval: "Sendeintervall",
	cloudBtnToken: "Token generieren",
	mqttTitle: "Privater MQTT Client",
	mqttActivate: "MQTT Client aktivieren",
	mqttHost: "MQTT Host",
	mqttPort: "MQTT Port",
	mqttUser: "MQTT Benutzer",
	mqttPwd: "MQTT Passwort",
	mqttQos: "QoS",
	// Menü WLAN
	wlanTitle: "WLAN-Einstellungen",
	availableNetworks: "Verfügbare Netzwerke",
	wlanActivate: "WLAN aktivieren",
	wlanNetwork: "Netzwerk wählen...",
	wlanClear: "Netzwerkverbindungsdaten löschen",
	wlanConnect: "Netzwerk verbinden",
	wlanSSID: "SSID",
	wlanPwd: "Passwort",
	clearWifiPrompt: "Sollen wirklich alle Netzwerkverbindungsdaten gelöscht werden? WLANThermo startet danach im AP-Modus neu.",
	stopWifiPrompt: "Soll die Wifi-Verbindung bis zum nächsten Restart des Systems wirklich abgeschaltet werden?",
	// Menu Bluetooth
	bluetoothActivate: "Bluetooth aktivieren",
	bluetoothChannels: "Kanäle auswählen...",
	bluetoothTitle: "Bluetooth - Einstellungen",
	channel: "Kanal",
	// Menü Pitmaster
	pitTitle: "Pitmaster-Einstellungen",
	pitPitmaster: "Pitmaster",
	pitProfile: "Profil",
	pitChannel: "Kanal",
	pitTemp: "Solltemperatur",
	pitValue: "Pitmaster-Wert (%)",
	pitProfileName: "Profil-Name",
	pitAktorTitle: "Aktorik",
	pitAktor: "Aktor",
	pitDCmin: "DCmin [0-100%]",
	pitDCmax: "DCmax [0-100%]",
	pitSPmin: "SPmin [% / ms]",
	pitSPmax: "SPmax [% / ms]",
	pidTitle: "PID",
	pidKp: "Kp",
	pidKi: "Ki",
	pidKd: "Kd",
	pidJump: "Jump Power [10-100%]",
	pidAutotune: "Autotune",
	pitAdvancesTitle: "Spezialfunktionen",
	pitNameTitle: "Allgemein",
	pitLid: "Deckelüberwachung",
	pitlink: "Aktorverknüfung",
	pitlinkdegressiv: "degressiv",
	pitlinklinear: "linear",
	pitlinklowpass: "lowpass"
};