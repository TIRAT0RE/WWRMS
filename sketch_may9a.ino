/*
  System for monitoring the condition of a room with wood heating with using Arduino UNO and ESP-01S module.
  Displays temperature, Humidity and approximate values of CO and CO2 on a webpage and LCD2004A I2C Display.

  Before uploading code, you should install required libraries and  set your ESP-01S module UART baudrate to 38400.
  Note that SoftwareSerial may not work properly on other baudrates and other/very high baudrates also can damage your ESP module. Proceed with caution.
  
  Also you need to perform calibration of MQ-135 sensor, using Atmosphere CO2 value  and calibration data.

  Your LCD2004/BME280 might have different I2C adresses.
  If you are unsure about I2C adresses, run I2C scanner.

  If your project is not initialized, double check wiring diagram, sensors pinouts and documentation.
  
  If you're unsure about your Arduino model or its pin cofigurations,please check the documentation at http://www.arduino.cc

  If you're unsure about your ESP model or its pin cofigurations,please check the documentation at http://www.espressif.com
 
  modified May 2023 in Kyiv, Ukraine.
  By Kesil Stanislav
  
  */

  /*
  
  Система контролю стану приміщення з дров'яним опаленням з використанням Arduino UNO і модуля ESP-01S.
  Відображає температуру, вологість і приблизні значення CO і CO2 на веб-сторінці та дисплеї LCD2004A I2C.

  Перед завантаженням коду слід встановити необхідні бібліотеки та встановити для модуля ESP-01 швидкість UART на 38400 бод.
  Зверніть увагу, що SoftwareSerial може не працювати належним чином на інших швидкостях передачі даних та на інших
  і дуже високих швидкостях передачі даних також може пошкодити ваш модуль ESP. Дійте обережно.
  
  Також необхідно виконати калібрування датчика MQ-135, використовуючи значення CO2 в атмосфері та дані калібрування.

  Ваш LCD2004/BME280 може мати інші адреси I2C.
  Якщо ви не впевнені щодо адреси I2C, запустіть сканер I2C.

  Якщо ваш проект не ініціалізовано, ще раз перевірте схему підключення, контакти датчиків та документацію.

  Якщо ви не впевнені щодо своєї моделі Arduino або конфігурації її контактів, перевірте документацію на http://www.arduino.cc
 
  Якщо ви не впевнені щодо своєї моделі ESP або конфігурації її контактів, перевірте документацію на http://www.espressif.com
 
  змінено травень 2023 р., Київ, Україна.
  
  By Kesil Stanislav
  
 */

#include <Adafruit_Sensor.h> // include required libraries
#include <Adafruit_BME280.h>
#include <SoftwareSerial.h>
#include <MQ135.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DEBUG true
#define analogPin A1 // MQ-135
#define analogPin1 A0 // MQ-7

void(* resetFunc) (void) = 0;

SoftwareSerial Serial1(2, 3); // creating objects
LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_BME280 bme;
MQ135 gasSensor = MQ135(analogPin);

unsigned long previousTime = 0;
int speakerCount = 0;

void setup() {
  Serial.begin(9600);   // Serial monitor
  Serial1.begin(38400); // ESP Baud rate.
  Serial.println("boot");
  pinMode(8,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(10,OUTPUT);
  //int resetPin = 12;
  digitalWrite(8,HIGH);
  delay(1500);
  digitalWrite(8,LOW);
  digitalWrite(10,HIGH);
  digitalWrite(11,LOW);
  pinMode(analogPin, INPUT); // analogPins work mode.
  pinMode(analogPin1, INPUT);

  if (!bme.begin(0x76)) { //Device self-test.
    digitalWrite(10,LOW);
    digitalWrite(11,HIGH);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor (4,0);
    lcd.print("SENSOR ERROR!");
    delay(10000);
    resetFunc();
  }

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor (0,0);
  lcd.print("Preparing Sensors");
  delay(2000);
  lcd.clear();

  lcd.setCursor (0,0);
  lcd.print("Setting up ESP");

  Serial.println("Setting up ESP");

  sendData("AT+RST\r\n", 2000, DEBUG); // reset module
  sendData("AT+CWMODE=1\r\n", 1000, DEBUG); // configure as client mode
  //sendData("AT+CWQAP\r\n", 1000, DEBUG);
  //sendData("AT+CWLAP\r\n", 2000, DEBUG); // list of available access points (optional, for reference)
  //sendData("AT+CWJAP=\"SSID\",\"PASSWORD\"\r\n", 5000, DEBUG); // connect to the specified access point
  sendData("AT+CIPMUX=1\r\n", 1000, DEBUG); // configure for multiple connections
  sendData("AT+CIFSR\r\n", 1000, DEBUG); // get IP address
  sendData("AT+CIPSERVER=1,80\r\n", 1000, DEBUG); // turn on server on port 80

  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP Address: ");
  lcd.setCursor(0, 1);
  lcd.print(getIPAddress());
  lcd.setCursor(0, 2);
  lcd.print("Write it in browser");
  lcd.setCursor(0, 3);
  lcd.print("or your Android app.");
  Serial.println("Device seems to be ready");

  delay(10000);
}

float temperature() { //function to sense temperature.
  int val = int(bme.readTemperature());
  delay(1000);
  return(val);
}

float humidity() { //function to sense humidity.
  int val1 = int(bme.readHumidity());
  delay(1000);
  return(val1);
}

float co2() { //function to sense co2.
  int val2 = int(gasSensor.getPPM());
  delay(1000);
  return(val2);
}

float co() { //function to sense co.
  int val3 = int(analogRead(analogPin1));
  delay(1000);
  return(val3);
}

int connectionId;

void loop() {
  lcd.clear();
  lcd.setCursor (0,0);
  lcd.print("Temperature: ");
  lcd.setCursor (12,0);
  lcd.print(int(bme.readTemperature()));
  lcd.setCursor (14,0);
  lcd.print((char)223);
  lcd.setCursor (15,0);
  lcd.print("C");
  lcd.setCursor (16,0);
  lcd.print(" ");

  lcd.setCursor (0,1);
  lcd.print("Humidity: ");
  lcd.setCursor (12,1);
  lcd.print(int(bme.readHumidity()));
  lcd.setCursor (14,1);
  lcd.print(" ");
  lcd.setCursor (15,1);
  lcd.print("%");
  lcd.setCursor (16,1);
  lcd.print(" ");

  lcd.setCursor (0,2);
  lcd.print("Approx CO2: ");
  lcd.setCursor (12,2);
  lcd.print(int(gasSensor.getPPM()));
  lcd.setCursor (17,2);
  lcd.print("PPM");

  lcd.setCursor (0,3);
  lcd.print("Approx CO: ");
  lcd.setCursor (12,3);
  lcd.print(int(analogRead(analogPin1)));
  lcd.setCursor (17,3);
  lcd.print("PPM");

  delay(2000);

  if (Serial1.available()) {
    //Recieving from web browser to toggle led
    if (Serial1.find("+IPD,")) {
      delay(300);
      connectionId = Serial1.read()-48;
      String data = data;

      if (temperature() != 0) {
        data += String(int(temperature()));
      }

      if (humidity() != 0) {
        if (data.length() > 0) {
          data += ",";
        }
        data += String(int(humidity()));
      }

      if (co2() != 0) {
        if (data.length() > 0) {
          data += ",";
        }
        data += String(int(co2()));
      }

      if (co() != 0) {
        if (data.length() > 0) {
          data += ",";
        }
        data += String(int(co()));
      }

      espsend(data, connectionId);

      String closeCommand = "AT+CIPCLOSE=";  //close the socket connection. esp command 
      closeCommand += connectionId; //append connection id
      closeCommand += "\r\n";
      sendData(closeCommand, 3000, DEBUG);
    }
  }
  //code for speaker to alarm when the CO value hits high number
  if (millis() - previousTime >= 60000) {
    previousTime = millis();
    if (co() > 100) {
      digitalWrite(11,HIGH);
      digitalWrite(10,LOW);

      if (speakerCount < 5) {
        for (int i = 0; i < 5; i++) {
          coAlarm();
          speakerCount++;
        }
      }
    } else {
      speakerCount = 0;
      digitalWrite(10,HIGH);
      digitalWrite(11,LOW);
    }
  }
}

String getIPAddress() {
  String response = sendData("AT+CIFSR\r\n", 1000, DEBUG); //Receiving IP address from ESP
  int index = response.indexOf("STAIP,\"") + 7; // Index of the beginning of IP
  int endIndex = response.indexOf("\"", index); // Index of the end of IP
  String ipAddress = response.substring(index, endIndex);
  return ipAddress;
}

void coAlarm() {
  lcd.clear();
  lcd.setCursor (6,1);
  lcd.print("CO ALARM");
  digitalWrite(8,HIGH);
  delay(2000);
  digitalWrite(8,LOW);
  delay(1000);
}

//sends data from ESP to webpage
void espsend(String d, int connectionId) {
  String httpHeader = "HTTP/1.1 200 OK\r\n";
  httpHeader += "Content-Type: text/plain\r\n";
  httpHeader += "Connection: close\r\n\r\n";

  String response = httpHeader + d;

  String cipSend = "AT+CIPSEND=";
  cipSend += connectionId;
  cipSend += ",";
  cipSend += response.length();
  cipSend += "\r\n";
  sendData(cipSend, 1000, DEBUG);
  sendData(response, 1000, DEBUG);
}

//getting the data from ESP and displaying it in serial monitor
String sendData(String command, const int timeout, boolean debug) {
  String response = "";
  Serial1.print(command);
  long int time = millis();
  while ((time + timeout) > millis()) {
    while (Serial1.available()) {
      char c = Serial1.read(); //read the next character
      response += c;
    }
  }

  if (debug) {
    Serial.print(response); //displays the esp response messages in arduino Serial monitor
  }
  return response;
}
