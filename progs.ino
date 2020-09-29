void expectation() {                                                //Программа Ожидания (Готово)
  heating_rel_flag = false;                   //Выключаем нагреватель
  maintence_flag = false;                     //Выключаем поддержание температуры
  sellect_temp = 100;                       //Сбрасываем выбранную температуру на 100
  if (heating_but.isClick()) {prog = 1; progsMode(prog);}              //При нажатии кнопки Нагрев переключаем на программу Нагрев
  if (but_maintence.isClick()) {                  //При нажатии кнопки Поддержание переключаем на программу Нагрев
  prog = 1;
  progsMode(prog);
  maintence_flag = true;                      //Включаем поддержание температуры                            
  }

}

void heating() {                                                                //Программа Нагрева (Готово)
  heating_rel_flag = true;                                                      //Включаем нагреватель
  sellect_t();                                                                  //Функция выбора температуры
  maintence_flag_sellect();                                                     //Функция смены флага Поддержния температуры 
  if (heating_but.isClick()) {prog = 0; progsMode(prog);}                       //При нажатии кнопки Нагрев - на Ожидание
  if (temp_>=sellect_temp && !maintence_flag) {prog = 0; progsMode(prog);}      //Придостижении выбранной температуры без флага Поддержания температуры на программу Ожидания
  else if (temp_>=sellect_temp && maintence_flag) {prog = 2; progsMode(prog);}  //Придостижении выбранной температуры с флагом Поддержания температуры на программу Поддержания
  
}

void maintence() {                                                    //Программа Поддержания (Готово)
  heating_rel_flag = false;                                           //Выключаем нагреватель
  sellect_t();                                                        //Функция выбора температуры
  maintence_flag_sellect();                                           //Функция смены флага Поддержния температуры 
  if (heating_but.isClick() || but_maintence.isClick()) prog = 0;     //При нажатии кнопки Нагрев или Поддержание - на Ожидание
  if (temp_<sellect_temp - 10) {prog = 1; progsMode(prog);}           //Если температура опускается на 10С от выбранной, то на Нагрев
}

void maintence_flag_sellect() {                    //Функция смены флага Поддержния температуры (Готово)
  if (but_maintence.isClick()) {                  //При нажатии кнопки меняется флаг Поддержания температуры
    
    mqttSend(2);
  }
}

void no_water() {                         //Программа Нет воды (Готово)
  heating_rel_flag = false;                   //Выключаем нагреватель
}

void water() {                            //Функция уровня воды (Готово)

  if (digitalRead(min_w)) {
    if (digitalRead(half_w)) {
      water_level = 2;
    }
    else {
      water_level = 1;
    }
  }
  else {
    water_level = 0;
  }

  if (!water_level) {prog = 3; progsMode(prog);}                   //Если мало воды - на Нет воды 
  else if (prog == 3) {prog = 0; progsMode(prog);}                 //При наличии воды с программы 3 переводит на 0, на программах 1,2 ни чего не происходит
  mqttSend(1);
}

void rel_mode() {                         //Функция управления реле нагревателя (Готово)
  if (heating_rel_flag) digitalWrite(heating_rel, HIGH);
  else digitalWrite(heating_rel, LOW);
}

void sellect_t() {                          //Функция выбора температуры (Готово)
  if (but_sel_t.isClick()) {                    //При нажатии кнопки выбора температуры
    if (sellect_temp < 100) sellect_temp += 10;                   //Переключаем температуру 50-100С с шагом 10C
    else sellect_temp = 50;
    mqttSend(4);
  }
}

void temp_C() {                                                     //Функция перевода температуры в градусы (Готово)
  temp_ = analogRead(sensor_t);
  temp_ = map(temp_, 0, 1024, 0, 200);
  mqttSend(3);              
}

void led_RGB() {                          //Функция управления светодиодом RGB (Готово)

  if (prog && prog != 3) {                                        //Работает только на программах 1 и 2
    if (temp_ <= 50) {                      //До 50С работают синий и зелёный
      pwm_g = map(temp_, 10, 50, 0, 255);                     //Переводим температуру в число для шим зелёного
      pwm_b = map(temp_, 10, 50, 255, 0);                     //Переводим температуру в число для шим синего
      pwm_g = constrain(pwm_g, 0, 255);                       //Ограничиваем PWM
      pwm_b = constrain(pwm_b, 0, 255);                       //Ограничиваем PWM

    }
    else {                            //Свыше 50С работают зелёный и красный
      pwm_r = map(temp_, 50, 100, 0, 255);                    //Переводим температуру в число для шим зелёного
      pwm_g = map(temp_, 50, 100, 255, 0);                    //Переводим температуру в число для шим синего
      pwm_g = constrain(pwm_g, 0, 255);                       //Ограничиваем PWM
      pwm_r = constrain(pwm_r, 0, 255);                       //Ограничиваем PWM

    }
  }
  else if (prog == 3) {                                           //Работает только на программе 3. При отсутствии воды горит феолетовый.
    pwm_g = 0;                                                  //Выключаем яркость светодиоду
    pwm_b = 255;                                                //Задаём яркость светодиоду
    pwm_r = 255;                                                //Задаём яркость светодиоду
  }
  else {                              //На программе 0 RGB выключен
    pwm_g = 0;                                                  //Выключаем яркость светодиоду
    pwm_b = 0;                                                  //Выключаем яркость светодиоду
    pwm_r = 0;                                                  //Выключаем яркость светодиоду
  }
  analogWrite(led_r, pwm_r);                                      //Зажигаем красный светодиод
  analogWrite(led_g, pwm_g);                                      //Зажигаем зелёный светодиод
  analogWrite(led_b, pwm_b);                                      //Зажигаем синий светодиод
}

void _Serial_() {                          //Функция монитора порта (Готово)
  time_ = millis();
  //if (Serial.available()){                    //Включается при отправки чего-либо в порт

  Serial.print("уровень воды: ");
  Serial.println(water_level);
  Serial.print("Выбранная Программа: ");
  Serial.println(prog);
  Serial.print("Выбранная температура: ");
  Serial.println(sellect_temp);
  Serial.print("Текущаа температура: ");
  Serial.println(temp_);
  Serial.print("Флаг Поддержания: ");
  Serial.println(maintence_flag);
  Serial.print("PWM Красного светодиода: ");
  Serial.println(pwm_r);
  Serial.print("PWM Зелёного светодиода: ");
  Serial.println(pwm_g);
  Serial.print("PWM Синего светодиода: ");
  Serial.println(pwm_b);
  Serial.print("Реле нагрева/Флаг: ");
  Serial.print(digitalRead(heating_rel));
  Serial.print("/");
  Serial.println(heating_rel_flag);
  Serial.print("Кнопка Нагрева: ");
  Serial.println(heating_but.isClick());
  Serial.print("Кнопка Поддержания: ");
  Serial.println(but_maintence.isClick());
  Serial.print("Кнопка Выбора: ");
  Serial.println(but_sel_t.isClick());
  Serial.println("_______________________________________________");
  //}
}

void progsMode(int _prog){
  switch (_prog) {
    case 0:
      mqttSend(0);                                                //Ожидание
      break;
    case 1:
      mqttSend(0);                                                  //Нагрев
      break;
    case 2:
      mqttSend(0);                                                //Поддержание
      break;
    case 3:
      mqttSend(0);                                                 //Нет воды
      break;
  }
}
