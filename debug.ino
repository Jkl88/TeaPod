void debugWEB(String msg){
  Serial.println(msg);
  String  mode =  "{\"debug\":\"";
		  mode += msg;
		  mode += "\"}";
  webSocket.broadcastTXT(mode);
}
