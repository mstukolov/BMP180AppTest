#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include <MQTTClient.h>

#include <Wire.h>
#include <Adafruit_BMP085.h>


Adafruit_BMP085 bmp;

// -For BlueMix connect----
#define ORG "kwxqcy" // имя организации
#define DEVICE_TYPE "SmartTemperature" // тип устройства
//#define DEVICE_ID "C2MSmartCooler2" // ID устройства IRV123 -не используется, Device ID задается из конфигуратора устройства
#define TOKEN "Qwerty12345" // - задаешь в IOT хабе
#define COOLER_ID "C2MCOOLER2"


char mqttserver[] = ORG ".messaging.internetofthings.ibmcloud.com"; // подключаемся к Blumix
char topic[] = "iot-2/evt/status/fmt/json";
char restopic[] = "iot-2/cmd/rele/fmt/json";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
//char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID; // переопределили ниже, после считывания Device ID из EEPROM
String clientID = "d:" ORG ":" DEVICE_TYPE ":";
char  cID[100]; // используется для формирования конечного Client ID

WiFiClientSecure net; // sec
MQTTClient client;



void setup()
{
	Serial.begin(9600);
	//Wire.begin (4, 5);
	if (!bmp.begin())
	{
		Serial.println("Could not find BMP180 or BMP085 sensor at 0x77");
		while (1) {}
	}
}

void loop()
{
	Serial.print("Temperature = ");
	Serial.print(bmp.readTemperature());
	Serial.println(" Celsius");

	Serial.print("Pressure MAX = ");
	Serial.print(bmp.readPressure());
	Serial.println(" Pascal");


	Serial.println();
	delay(5000);
}
