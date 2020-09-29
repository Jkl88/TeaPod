//**********************************************************************  Библиотеки
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>						//https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>						//https://github.com/tzapu/WiFiManager
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <FS.h>									// файловая система
#include <WebSocketsServer.h>
#include <Hash.h>
#include <PubSubClient.h>
#include "GyverButton.h"

//#define Serial.println(x)
//#define Serial.print(x)

//**********************************************************************  Константы
#define sensor_t A0                                                   //Датчик температуры
#define heating_butt 0                                                //Кнопка Старт
#define heating_rel 16                                                //Нагрватель
#define but_maint 3                                                   //Кнопка поддержания темературы
#define but_sel 2                                                     //Кнопка выбора температуры

#define led_b 15                                                      //Светодиод внвутри Синий
#define led_g 12                                                      //Светодиод внвутри Зелёный
#define led_r 14                                                      //Светодиод внвутри Красный
#define led_t 4                            //Светодиоды температуры

#define led_p 1                           //Светодиод Поддержания температуры

#define min_w 13                                                      //Минимальный уровень воды
#define half_w 5                                                      //Средний уровень воды
//**********************************************************************  Переменные
String NAME_ESP = "TeaPod";				// имя устройства
String PASS_ESP = "05683384";			// пароль устройства
String MQTT = "192.168.0.255";				// ip MQTT брокера
int MQTT_PORT = 1883;						// порт MQTT
byte connectMQTT = 5;						// количество попыток подключения

int prog;                                                           //Переменная номера программы
int sellect_temp = 100;                                             //Выбранная температура. По умолчанию 100.
bool maintence_flag = false;                                        //Переменная пддержания. По умолчанию выкл.
bool heating_rel_flag = false;
int pwm_r;                                                          //ШИМ красного светодиода
int pwm_g;                                                          //ШИМ зелёного светодиода
int pwm_b;                                                          //ШИМ синего светодиода
int temp_;                              //Температура
int water_level;                          //Уровень воды
unsigned long time_;                                                //Переменная времени

GButton heating_but(heating_butt);                                  //Кнопка Нагрева
GButton but_maintence(but_maint);                                   //Кнопка поддержания температуры
GButton but_sel_t(but_sel);                                         //Кнопка выбора температуры


ESP8266WebServer server(80);
WiFiManager wifiManager;
WiFiClient clientESP;
WebSocketsServer webSocket = WebSocketsServer(81);
StaticJsonDocument<200> doc;
File FileFS;
PubSubClient clientMQTT(clientESP);

void setup() {
  // проверка нажаты ли кнопки поддержания температуры и выбора для сброса всех настроек
    if (digitalRead(but_sel) && digitalRead(but_maint)) {
    saveEEPROM(0); // сброс на дефолтные настройки
    }
  

  Serial.begin(115200);
  pinMode(led_b, OUTPUT);                                           //Выход на RGB светодиод внутри чайника
  pinMode(led_g, OUTPUT);                                           //Выход на RGB светодиод внутри чайника
  pinMode(led_r, OUTPUT);                                           //Выход на RGB светодиод внутри чайника
  //  pinMode(led_t, OUTPUT);                                         //Выход на светодиоды тепературы
  pinMode(led_p, FUNCTION_0);                                           //Выход на светодиод Поддержания тепературы
  pinMode(but_maint, FUNCTION_0);

  pinMode(min_w, INPUT);                                            //Вход датчиков Уровня воды
  pinMode(half_w, INPUT);                                           //Вход датчиков Уровня воды

  heating_but.setTickMode(AUTO);
  but_maintence.setTickMode(AUTO);
  but_sel_t.setTickMode(AUTO);

  EEPROM.begin(70);

  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
  loadEEPROM();
  Serial.println("Подключение - ");
  autoConnect();										// подключение к WiFi или точка доступа
  Serial.print("IP адрес "); Serial.println(IP());

  serverInit();										// инициализация серверных функций

  MQTT_init ();										// инициализация MQTT
}

void loop() {

  server.handleClient();
  MDNS.update();
  webSocket.loop();
  if (!clientMQTT.connected() && connectMQTT) {
    reconnect();
  }
  clientMQTT.loop();

  rel_mode();                            //Запуск функции реле нагревателя
  if (millis() - time_ >= 1000) {                                 //Запуск функции монитора
    temp_C();                         //Запуск функции определения температуры
    water();                          //Запуск функции определения уровня воды
    //_Serial_();                       //Вывод информации
  }

  led_RGB();                            //Функция RGB подсветки
  
  switch (prog) {
    case 0:
      expectation();                                                //Ожидание
      break;
    case 1:
      heating();                                                  //Нагрев
      break;
    case 2:
      maintence();                                                //Поддержание
      break;
    case 3:
      no_water();                                                 //Нет воды
      break;
  }


}
