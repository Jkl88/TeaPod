void MQTT_init () {
  clientMQTT.setServer(MQTT.c_str(), MQTT_PORT);
  clientMQTT.setCallback(callback);
}
void callback(char* topic, byte* payload, unsigned int length) {
  /*Serial.print*/debugWEB("Получено сообщение:");
  /* Serial.print*/debugWEB(topic);
  for (int i = 0; i < length; i++) {
    Serial.print(payload[i]);
  }
  Serial.println("");
  String mqtt_name = NAME_ESP;
  if (!mqtt_name.startsWith("/")) {
    mqtt_name = "/" + mqtt_name;
  }
  /*Serial.println*/debugWEB("JSONPars send....");
  JSONPars(0, payload);
}
void reconnect() {
  // Loop until we're reconnected
  int _heating, _start;
  while (!clientMQTT.connected() && connectMQTT) {
    Serial.printf("Attempting MQTT connection to %s:%d", MQTT.c_str(), MQTT_PORT);
    connectMQTT--;
    String  mode =  "{\"onlineMQTT\":\"";
    mode += clientMQTT.connected();
    mode += "\"}";
    webSocket.broadcastTXT(mode);
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (clientMQTT.connect(clientId.c_str())) {
      /*Serial.println*/debugWEB("MQTT connected");
      connectMQTT = 5;
      mode =  "{\"onlineMQTT\":\"";
      mode += clientMQTT.connected();
      mode += "\"}";
      webSocket.broadcastTXT(mode);
      // Once connected, publish an announcement...
      mode =  "{\",\"IP\":\"";
      mode += IP();
      mode += "\",\"nameESP\":\"";
      mode += NAME_ESP;
      mode += "\",\"currentTemp\":\"";
      mode += temp_;
      mode += "\",\"setTemp\":\"";
      mode += sellect_temp;
      mode += "\",\"water\":\"";
      mode += water_level;
      mode += "\",\"heating\":\"";
      if (heating_rel_flag) {
        _heating = 1;
      }
      else {
        _heating = 0;
      }
      mode += _heating;
      mode += "\",\"start\":\"";
      if (prog) {
        int _start = 1;
      }
      else {
        int _start = 0;
      }
      mode += _start;
      mode += "\",\"checkMQTT\":\"";
      mode += connectMQTT;
      mode += "\",\"mqtt\":\"";
      mode += MQTT;
      mode += "\",\"onlineMQTT\":\"";
      mode += clientMQTT.connected();
      mode += "\"}";
      String mqtt_name = NAME_ESP;
      if (!mqtt_name.startsWith("/")) {
        mqtt_name = "/" + mqtt_name;
      }
      clientMQTT.publish(mqtt_name.c_str(), mode.c_str());
      // ... and resubscribe
      clientMQTT.subscribe("/ESP");
      clientMQTT.subscribe(mqtt_name.c_str());
    } else {
      /*Serial.print*/debugWEB("failed, rc=");
      /*Serial.print*/debugWEB(String(clientMQTT.state()));
      /*Serial.println*/debugWEB(" try again in 1 second");
      // Wait 1 second before retrying
      delay(1000);
    }
  }
}
void mqttSend(int _val) {
  String mqtt_name = NAME_ESP;
  if (!mqtt_name.startsWith("/")) {
    mqtt_name = "/out" + mqtt_name;
  }
  String _mode;
  switch (_val) {
    case 0:
      _mode =  "{\"start\":\"";
      _mode += String(prog);
      _mode += "\"}";
      webSocket.broadcastTXT(_mode);
      clientMQTT.publish(mqtt_name.c_str(), _mode.c_str());                                                    //программа
      break;
    case 1:
      _mode =  "{\"water\":\"";
      _mode += String(water_level);
      _mode += "\"}";
      webSocket.broadcastTXT(_mode);
      clientMQTT.publish(mqtt_name.c_str(), _mode.c_str());                                                  //уровень воды
      break;
    case 2:
      _mode =  "{\"heating\":\"";
      _mode += String(heating_rel_flag);
      _mode += "\"}";
      webSocket.broadcastTXT(_mode);
      clientMQTT.publish(mqtt_name.c_str(), _mode.c_str());                                                  //Поддержание
      break;
    case 3:
      _mode =  "{\"currentTemp\":\"";
      _mode += String(temp_);
      _mode += "\"}";
      webSocket.broadcastTXT(_mode);
      clientMQTT.publish(mqtt_name.c_str(), _mode.c_str());                                                  //текущая температура
      break;
    case 4:
      _mode =  "{\"setTemp\":\"";
      _mode += String(sellect_temp);
      _mode += "\"}";
      webSocket.broadcastTXT(_mode);
      clientMQTT.publish(mqtt_name.c_str(), _mode.c_str());                                                  //выбранная температура
      break;
  }
}
