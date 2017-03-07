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
#define ORG "kwxqcy" // ��� �����������
#define DEVICE_TYPE "SmartTemperature" // ��� ����������
//#define DEVICE_ID "C2MSmartCooler2" // ID ���������� IRV123 -�� ������������, Device ID �������� �� ������������� ����������
#define TOKEN "Qwerty12345" // - ������� � IOT ����
#define COOLER_ID "C2MCOOLER2"


char mqttserver[] = ORG ".messaging.internetofthings.ibmcloud.com"; // ������������ � Blumix
char topic[] = "iot-2/evt/status/fmt/json";
char restopic[] = "iot-2/cmd/rele/fmt/json";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
//char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID; // �������������� ����, ����� ���������� Device ID �� EEPROM
String clientID = "d:" ORG ":" DEVICE_TYPE ":";
char  cID[100]; // ������������ ��� ������������ ��������� Client ID

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
