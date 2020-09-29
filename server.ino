/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////     инициализация сервера     ///////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void serverInit() {

  MDNS.begin(NAME_ESP);
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  server.on("/", []() {																	//  /
    FileFS = SPIFFS.open("/index.htm", "r");
    server.streamFile(FileFS, "text/html");
    FileFS.close();
  });
  server.on("/mqttOn", []() {																//  /mqttOn
    connectMQTT = 5;
    reconnect();
    if (clientMQTT.connected())
    {
      server.send(200, "text/plainl", "OK");
    } else
    {
      server.send(200, "text/plainl", "FALED");
    }

  });
  server.on("/start", []() {                               //  /start принеимает аргументы: t=50-100 (температура) h=0/1 (поддержка температуры)
    if (prog == 0) {
      for (uint8_t i = 0; i < server.args(); i++) {
        if (server.argName(i) == "t") {
          sellect_temp = server.arg(i).toInt();
        }
        else if (server.argName(i) == "h") {
          heating_rel_flag = server.arg(i).toInt();
        }
      }
      prog = 1;
      progsMode(prog);
      server.send(200, "text/plainl", "Нагрев включен");
    }
    else if (prog == 1) {
      if (server.args() > 0) {
        for (uint8_t i = 0; i < server.args(); i++) {
          if (server.argName(i) == "t") {
            sellect_temp = server.arg(i).toInt();
          }
          else if (server.argName(i) == "h") {
            heating_rel_flag = server.arg(i).toInt();
          }
        }
        server.send(200, "text/plainl", "Нагрев включен");
      }
      else {
        prog = 0;
        progsMode(prog);
        server.send(200, "text/plainl", "Нагрев выключен");
      }
    }
    else if (prog == 2) {
      if (server.args() > 0) {
        for (uint8_t i = 0; i < server.args(); i++) {
          if (server.argName(i) == "t") {
            sellect_temp = server.arg(i).toInt();
            server.send(200, "text/plainl", "Изменина температура");
          }
          else if (server.argName(i) == "h") {
            heating_rel_flag = server.arg(i).toInt();
            if (heating_rel_flag) {
              server.send(200, "text/plainl", "Поддержка температуры активен");
            }
            else {
              server.send(200, "text/plainl", "Поддержка температуры выключена");
            }
          }
        }
      }
      prog = 0;
      progsMode(prog);
      server.send(200, "text/plainl", "Нагрев выключен");
    }
  });

  server.on("/tempC", []() {                                //  /tempC
    String msg =  "Текущая температура: ";
           msg += temp_;
           msg += "*C";
    server.send(200, "text/plainl", msg);
  });

  server.on("/0wifi", []() {																//  /0wifi
    server.send(200, "text/plainl", IP());
  });
  server.on("/reset", []() {																//  /reset
    for (uint8_t i = 0; i < server.args(); i++) {
      if (server.argName(i) == "s" && server.arg(i) == "1") {
        server.send(200, "text/plainl", "Перезагрузка");
        delay(2000);
        WiFi.disconnect(true);
        delay(5000);
      }
      server.send(200, "text/plainl", "Неверный аргумент");
    }

  });
  server.on("/update", HTTP_POST, []() {													//  /update
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.setDebugOutput(true);
      WiFiUDP::stopAll();
      Serial.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield();
  });

  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);


  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      handleNotFound();
    }
  });																						//  /404

  server.begin();
}

void handleNotFound() {													//**********************404
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
String IP () {														//**********************IP
  if (WiFi.softAPIP().toString() == "(IP unset)") {
    return WiFi.localIP().toString();
  }
  else {
    return WiFi.softAPIP().toString();
  }
}

void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  Serial.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next()) {
    FileFS = dir.openFile("r");
    if (output != "[") {
      output += ',';
    }
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(FileFS.name()).substring(1);
    output += "\"}";
    FileFS.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}
void handleFileCreate() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (SPIFFS.exists(path)) {
    return server.send(500, "text/plain", "FILE EXISTS");
  }
  FileFS = SPIFFS.open(path, "w");
  if (FileFS) {
    FileFS.close();
  } else {
    return server.send(500, "text/plain", "CREATE FAILED");
  }
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileUpload() {
  if (server.uri() != "/edit") {
    return;
  }

  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    if (SPIFFS.exists(filename))
    {
      Serial.printf("File %s delete\n" , filename.c_str());
      SPIFFS.remove(filename);
    }
    FileFS = SPIFFS.open(filename, "w");
    if (!FileFS) {
      Serial.println("file open failed");
    }
    else             {
      Serial.println("file opened");
    }
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {

    if (FileFS) {
      FileFS.write(upload.buf, upload.currentSize);
      Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (FileFS) {
      FileFS.close();
      Serial.println("file closed");
    }
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
}

void handleFileDelete() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  Serial.println("handleFileDelete: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (!SPIFFS.exists(path)) {
    return server.send(404, "text/plain", "FileNotFound");
  }
  SPIFFS.remove(path);
  server.send(200, "text/plain", "OK");
  path = String();
}
String getContentType(String filename) {
  if (server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

boolean handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) {
    path += "index.htm";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) {
      path += ".gz";
    }
    FileFS = SPIFFS.open(path, "r");
    server.streamFile(FileFS, contentType);
    FileFS.close();
    return true;
  }
  return false;
}
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////     WEB SOCKET     //////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        JSONPars(num, (uint8_t*)"CONNECTED");
      }
      break;
    case WStype_TEXT:
      {
        Serial.printf("[%u] get Text: %s\n", num, payload);
        JSONPars(num, payload);

        // webSocket.sendTXT(num, "message here");
        // send data to all connected clients
        // webSocket.broadcastTXT("message here");
      }
      break;
    case WStype_BIN:
      {
        Serial.printf("[%u] get binary length: %u\n", num, length);
        hexdump(payload, length);

        // send message to client
        // webSocket.sendBIN(num, payload, length);
      }
      break;
  }

}
