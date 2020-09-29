void JSONPars(uint8_t num, uint8_t * json){
	String mqtt_name = NAME_ESP;
	if (!mqtt_name.startsWith("/")) {
		mqtt_name = "/out" + mqtt_name;
	}
	if ((char*)json=="CONNECTED")
	{
		int _start;
    int _heating;
		String  mode =  "{\",\"IP\":\"";
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
        if (heating_rel_flag){_heating = 1;}
        else {_heating = 0;}
        mode += _heating;
        mode += "\",\"start\":\"";
        if (prog){int _start = 1;}
        else {int _start = 0;}
        mode += _start;
				mode += "\",\"checkMQTT\":\"";
				mode += connectMQTT;
				mode += "\",\"mqtt\":\"";
				mode += MQTT;
				mode += "\",\"onlineMQTT\":\"";
				mode += clientMQTT.connected();
				mode += "\"}";
		webSocket.sendTXT(num, mode);
		return;
	}
	
	
	DeserializationError error = deserializeJson(doc, json);
	
	// Test if parsing succeeds.
	if (error) {
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		return;
	}
	JsonVariant JSON = doc.getMember("setTemp");
	if(!JSON.isNull()){
		sellect_temp = doc["setTemp"];
		Serial.printf("setTemp - %d", sellect_temp);
		Serial.println("");
		String  mode =  "{\"setTemp\":\"";
				mode += sellect_temp;
				mode += "\"}";
		webSocket.broadcastTXT(mode);
		clientMQTT.publish(mqtt_name.c_str(), mode.c_str());
	}

  JSON = doc.getMember("heating");
  if(!JSON.isNull()){
    heating_rel_flag = doc["heating"];
    Serial.printf("heating - %d", heating_rel_flag);
    Serial.println("");
    String  mode =  "{\"heating\":\"";
        mode += heating_rel_flag;
        mode += "\"}";
    webSocket.broadcastTXT(mode);
    clientMQTT.publish(mqtt_name.c_str(), mode.c_str());
  }
  JSON = doc.getMember("start");
  if(!JSON.isNull()){
    prog = doc["start"];
    progsMode(prog);
    Serial.printf("start - %d", prog);
    Serial.println("");
    String  mode =  "{\"start\":\"";
        mode += heating_rel_flag;
        mode += "\"}";
    webSocket.broadcastTXT(mode);
    clientMQTT.publish(mqtt_name.c_str(), mode.c_str());
  }
	
	JSON = doc.getMember("reset");
	if(!JSON.isNull() && doc.getMember("reset")=="1"){
		Serial.println("reset");
		String  mode =  "{\"reset\":\"1\"}";
		webSocket.broadcastTXT(mode);
		clientMQTT.publish(mqtt_name.c_str(), mode.c_str());
		delay(100);
		if(doc.getMember("wifiDC")=="true"){WiFi.disconnect(true);}
		ESP.reset();
	}
	JSON = doc.getMember("checkMQTT");
	if(!JSON.isNull()){
		connectMQTT = doc["checkMQTT"];
		String  mode =  "{\"checkMQTT\":\"";
				mode += connectMQTT;
				mode += "\"}";
		webSocket.broadcastTXT(mode);
		saveEEPROM(4);
		
	}
	JSON = doc.getMember("mqtt");
	if(!JSON.isNull()){
		MQTT = doc["mqtt"].as<String>();
		MQTT_PORT = doc["mqttPort"];
		saveEEPROM(2);
		ESP.reset();
	}
	JSON = doc.getMember("nameESP");
	if(!JSON.isNull()){
		NAME_ESP = doc["nameESP"].as<String>();
		PASS_ESP = doc["passESP"].as<String>();
		saveEEPROM(3);
		ESP.reset();		
	}
	
}
/*
void configJsonLoad (){
	
	if (SPIFFS.exists("/config.json"))
	{
		FileFS=SPIFFS.open("/config.json", "r");
		DeserializationError error = deserializeJson(doc, FileFS);
		ledVal.viewIP=doc["viewIP"];
		FileFS.close();
	}else {Serial.println("not file");}
	
}
void configJsonSave(){
	FileFS=SPIFFS.open("/config.json", "w");
	String  mode =  "{\"viewIP\":\"";
			mode += ledVal.viewIP;
			mode += "\"}";
	FileFS.print(mode);
}*/
