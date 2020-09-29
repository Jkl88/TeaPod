void loadEEPROM (){
	//EEPROM.get(0,ledVal.viewIP);
	EEPROM.get(5,connectMQTT);
	MQTT = EEPROM_string_read(10);
	MQTT_PORT = EEPROM_int_read(1);
	NAME_ESP=EEPROM_string_read(30);
	PASS_ESP=EEPROM_string_read(50);
}
void saveEEPROM(int i){
	switch(i){
		//case 1: EEPROM.put(0, ledVal.viewIP); break;
		
		case 4: EEPROM.put(5, connectMQTT); break;
		
		case 2: EEPROM_int_write(1, MQTT_PORT);
				EEPROM_string_write(10, MQTT); break;
		
		case 3: EEPROM_string_write(30, NAME_ESP);
				EEPROM_string_write(50, PASS_ESP); break;
		
		case 0: EEPROM_int_write(1, MQTT_PORT);
				EEPROM_string_write(10, MQTT);
				EEPROM_string_write(30, NAME_ESP);
				EEPROM_string_write(50, PASS_ESP); break;
	}EEPROM.commit();
}
//������ int
int EEPROM_int_read(int addr) {
	byte raw[2];
	for(byte i = 0; i < 2; i++) raw[i] = EEPROM.read(addr+i);
	int &num = (int&)raw;
	return num;
}

//������ int
void EEPROM_int_write(int addr, int num) {
	byte raw[2];
	(int&)raw = num;
	for(byte i = 0; i < 2; i++) EEPROM.write(addr+i, raw[i]);
}
//������ String
void EEPROM_string_write (int Addr, String Str) {
	byte lng=Str.length();
	if (lng>19 )  lng=0;
	EEPROM.write(Addr , lng);
	unsigned char* buf = new unsigned char[19];
	Str.getBytes(buf, lng + 1);
	Addr++;
	for(byte i = 0; i < lng; i++) {EEPROM.write(Addr+i, buf[i]); delay(10);}
}
//������ String
char *EEPROM_string_read (int Addr) {
	byte lng = EEPROM.read(Addr);
	char* buf = new char[19];
	Addr++;
	for(byte i = 0; i < lng; i++) buf[i] = char(EEPROM.read(i+Addr));
	buf[lng] = '\x0';
	return buf;
}
