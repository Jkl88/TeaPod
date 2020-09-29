boolean autoConnect(){
  
  WiFi.mode(WIFI_STA);

  if (wifiManager.connectWifi("", "") == WL_CONNECTED)   {
    Serial.println("IP Address:");
    Serial.println(WiFi.localIP());
    //connected
    
    return true;
  }

  
  return wifiManager.startConfigPortal(NAME_ESP.c_str(), PASS_ESP.c_str());
  
}
