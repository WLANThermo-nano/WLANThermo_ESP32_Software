export const Data = {
    settings: {
        "device": {
            "device": "mini",
            "serial": "c82b96995b70",
            "cpu": "esp32",
            "flash_size": 16777216,
            "hw_version": "v2",
            "sw_version": "v0.9.9",
            "api_version": "1",
            "language": "de"
        },
        "system": {
            "time": "1591215343",
            "unit": "C",
            "ap": "AP-Name",
            "host": "MINI-001",
            "language": "en",
            "version": "v0.9.9",
            "getupdate": "false",
            "autoupd": true,
            "prerelease": false,
            "hwversion": "V2"
        },
        "hardware": [
            "V2"
        ],
        "api": {
            "version": "1"
        },
        "sensors": [
            {
                "type": 0,
                "name": "1000K/Maverick",
                "fixed": false
            },
            {
                "type": 1,
                "name": "Fantast-Neu",
                "fixed": false
            },
            {
                "type": 2,
                "name": "Fantast",
                "fixed": false
            },
            {
                "type": 3,
                "name": "100K/iGrill2",
                "fixed": false
            },
            {
                "type": 4,
                "name": "ET-73",
                "fixed": false
            },
            {
                "type": 5,
                "name": "Perfektion",
                "fixed": false
            },
            {
                "type": 6,
                "name": "50K",
                "fixed": false
            },
            {
                "type": 7,
                "name": "Inkbird",
                "fixed": false
            },
            {
                "type": 8,
                "name": "100K6A1B",
                "fixed": false
            },
            {
                "type": 9,
                "name": "Weber_6743",
                "fixed": false
            },
            {
                "type": 10,
                "name": "Santos",
                "fixed": false
            },
            {
                "type": 11,
                "name": "5K3A1B",
                "fixed": false
            },
            {
                "type": 12,
                "name": "PT100",
                "fixed": false
            },
            {
                "type": 13,
                "name": "PT1000",
                "fixed": false
            },
            {
                "type": 14,
                "name": "ThermoWorks",
                "fixed": false
            },
            {
                "type": 15,
                "name": "Typ K",
                "fixed": true
            },
            {
                "type": 16,
                "name": "Bluetooth",
                "fixed": true
            }
        ],
        "features": {
            "bluetooth": true,
            "pitmaster": true
        },
        "pid": [
            {
                "name": "SSR SousVide",
                "id": 0,
                "aktor": 0,
                "Kp": 104.0,
                "Ki": 0.2,
                "Kd": 0.0,
                "DCmmin": 0.0,
                "DCmmax": 100.0,
                "opl": 0,
                "SPmin": 0.0,
                "SPmax": 0.0,
                "link": 0,
                "tune": 0,
                "jp": 100
            },
            {
                "name": "TITAN 50x50",
                "id": 1,
                "aktor": 1,
                "Kp": 3.8,
                "Ki": 0.01,
                "Kd": 128.0,
                "DCmmin": 25.0,
                "DCmmax": 100.0,
                "opl": 0,
                "SPmin": 0.0,
                "SPmax": 0.0,
                "link": 0,
                "tune": 0,
                "jp": 70
            },
            {
                "name": "Servo MG995",
                "id": 2,
                "aktor": 2,
                "Kp": 104.0,
                "Ki": 0.2,
                "Kd": 0.0,
                "DCmmin": 0.0,
                "DCmmax": 100.0,
                "opl": 0,
                "SPmin": 25.0,
                "SPmax": 75.0,
                "link": 0,
                "tune": 0,
                "jp": 100
            },
            {
                "name": "Damper",
                "id": 3,
                "aktor": 3,
                "Kp": 3.8,
                "Ki": 0.01,
                "Kd": 128.0,
                "DCmmin": 25.0,
                "DCmmax": 100.0,
                "opl": 0,
                "SPmin": 25.0,
                "SPmax": 75.0,
                "link": 0,
                "tune": 0,
                "jp": 70
            }
        ],
        "aktor": [
            "SSR",
            "FAN",
            "SERVO",
            "DAMPER"
        ],
        "display": {
            "updname": "",
            "orientation": 0
        },
        "iot": {
            "PMQhost": "192.168.2.1",
            "PMQport": 1883,
            "PMQuser": "",
            "PMQpass": "",
            "PMQqos": 0,
            "PMQon": false,
            "PMQint": 30,
            "CLon": false,
            "CLtoken": "c82b96995b705ec91d87bb",
            "CLint": 30,
            "CLurl": "cloud.wlanthermo.de/index.html"
        }
    },
    data: {
        "system": {
            "time": "1590008772",
            "unit": "C",
            "rssi": -32,
            "soc": 70,
            "charge": true,
            "online": 2
        },
        "channel": [
            {
                "number": 1,
                "name": "Kanal 1",
                "typ": 0,
                "temp": 999,
                "min": 10.0,
                "max": 35.0,
                "alarm": 0,
                "color": "#0C4C88",
                "fixed": true,
                "connected": true
            },
            {
                "number": 2,
                "name": "Kanal 2",
                "typ": 0,
                "temp": 999.0,
                "min": 10.0,
                "max": 35.0,
                "alarm": 0,
                "color": "#22B14C",
                "fixed": false,
                "connected": false
            },
            {
                "number": 3,
                "name": "Kanal 3",
                "typ": 0,
                "temp": 15.0,
                "min": 10.0,
                "max": 35.0,
                "alarm": 0,
                "color": "#EF562D",
                "fixed": false,
                "connected": false
            },
            {
                "number": 4,
                "name": "Kanal 4",
                "typ": 0,
                "temp": 999.0,
                "min": 10.0,
                "max": 35.0,
                "alarm": 0,
                "color": "#FFC100",
                "fixed": false,
                "connected": false
            },
            {
                "number": 5,
                "name": "Kanal 5",
                "typ": 0,
                "temp": 999.0,
                "min": 10.0,
                "max": 35.0,
                "alarm": 0,
                "color": "#A349A4",
                "fixed": false,
                "connected": false
            },
            {
                "number": 6,
                "name": "Kanal 6",
                "typ": 0,
                "temp": 999.0,
                "min": 10.0,
                "max": 35.0,
                "alarm": 0,
                "color": "#804000",
                "fixed": false,
                "connected": false
            },
            {
                "number": 7,
                "name": "Kanal 7",
                "typ": 0,
                "temp": 999.0,
                "min": 10.0,
                "max": 35.0,
                "alarm": 0,
                "color": "#5587A2",
                "fixed": false,
                "connected": false
            },
            {
                "number": 8,
                "name": "Kanal 8",
                "typ": 0,
                "temp": 999.0,
                "min": 10.0,
                "max": 35.0,
                "alarm": 0,
                "color": "#5C7148",
                "fixed": false,
                "connected": false
            },
            {
                "number": 9,
                "name": "Kanal 9",
                "typ": 16,
                "temp": 999.0,
                "min": 10.0,
                "max": 35.0,
                "alarm": 0,
                "color": "#5C7148",
                "fixed": true,
                "connected": true
            },
            {
                "number": 10,
                "name": "Kanal 10",
                "typ": 17,
                "temp": 999.0,
                "min": 10.0,
                "max": 35.0,
                "alarm": 0,
                "color": "#5C7148",
                "fixed": true,
                "connected": true
            }
        ],
        "pitmaster": {
            "type": [
                "off",
                "manual",
                "auto"
            ],
            "pm": [
                {
                    "id": 0,
                    "channel": 1,
                    "pid": 0,
                    "value": 0,
                    "set": 50.0,
                    "typ": "off",
                    "set_color": "#ff0000",
                    "value_color": "#000000"
                },
                {
                    "id": 1,
                    "channel": 2,
                    "pid": 1,
                    "value": 0,
                    "set": 50.0,
                    "typ": "off",
                    "set_color": "#FE2EF7",
                    "value_color": "#848484"
                },
                {
                    "id": 2,
                    "channel": 3,
                    "pid": 2,
                    "value": 98,
                    "set": 50.0,
                    "typ": "auto",
                    "set_color": "#FE2EF7",
                    "value_color": "#848484"
                }
            ]
        }
    },
    networklist: {
        "Connect": true,
        "SSID": "WLAN-XXXXX",
        "BSSID": "38:10:D5:D1:BC:8B",
        "IP": "192.168.178.53",
        "Mask": "255.255.255.0",
        "Gate": "192.168.178.1",
        "Scan": [
            {
                "SSID": "WLAN-XXXXX",
                "BSSID": "38:10:D5:D1:BC:8B",
                "RSSI": -42,
                "Enc": true
            },
            {
                "SSID": "EasyBox-767780",
                "BSSID": "D4:60:E3:BD:BA:9C",
                "RSSI": -104,
                "Enc": true
            },
            {
                "SSID": "FRITZ!Box 7590 LD",
                "BSSID": "44:4E:6D:3B:0E:0F",
                "RSSI": -76,
                "Enc": true
            }
        ],
        "RSSI": -46,
        "Enc": true
    },
    bluetooth: {
        "enabled": true,
        "devices": [
            {
                "name": "WT BLE",
                "address": "e4:a8:e2:bc:0d:6b",
                "count": 2,
                "selected": 3
            },
            {
                "name": "iBBQ",
                "address": "a8:e2:c1:77:7e:f3",
                "count": 8,
                "selected": 7
            }
        ]
    },
    getpush: {
        "telegram": {
            "enabled": false,
            "token": "Telegram Token",
            "chat_id": 123
        },
        "pushover": {
            "enabled": true,
            "token": "Pushover Token",
            "user_key": "User Key",
            "priority": 1
        },
        "app": {
            "enabled": true,
            "max_devices": 3,
            "devices": [
                {
                    "id": '1',
                    "name": "Dummys iPhone",
                    "id": "123",
                    "token_sha256": "Device 0 SHA256"
                },
                {
                    "id": '2',
                    "name": "Dummys iPad",
                    "id": "456",
                    "token_sha256": "Device 1 SHA256"
                }
            ]
        }
    }
}