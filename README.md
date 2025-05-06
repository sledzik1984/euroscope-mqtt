# EuroScope MQTT Plugin

This plugin uses MQTT to send callsing (and few other informations) of a plane that controller has "in focus". 

Nothing fancy. 

Based on a Euroscope Plugin Template by LeoKle available at https://github.com/LeoKle/euroscope-plugin-template

Configuration file - euroscope-mqtt.txt have to be located in same directory as euroscope-mqtt.dll

Structure of configuration file is straightforward - you have to enter MQTT Server Address, login, password, Vatsim CID and location of ICAO_Airlines.txt

Plugin sends data as JSON to 

```commandline
/euroscope/cid_from_config_file/selected:
```

```commandline
{"callsign":"FIN62K", "pilotname":"Pilot Name", "telephony":"FINNAIR 62K"}
```

