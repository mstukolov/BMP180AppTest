#include <ESP8266WiFi.h>
#include <MQTTClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <EEPROM.h>

#include <Wire.h>
#include <Adafruit_BMP085.h> //

// -For BlueMix connect----
#define ORG "kwxqcy" // ��� �����������
#define DEVICE_TYPE "SmartCooler" // ��� ����������
//#define DEVICE_ID "C2MSmartCooler2" // ID ���������� IRV123 -�� ������������, Device ID �������� �� ������������� ����������
#define TOKEN "Qwerty12345" // - ������� � IOT ����
#define COOLER_ID "C2MCOOLER2"

// EEPROM - ���������� ������� ������

#define ModeoffSet 0 // ��������� ����: 0 ���� - ����� �������� ����������
#define SizeMode 1 // - ������ ������ Mode ���� -1
#define SSIDoffSet 1 // ��������� ����: SSID
#define SizeSSID 32 // ������ ������ SSID ���� -32
#define PasswordoffSet 33 //  ��������� ���� Password
#define SizePassword 32 //  ������ ������   Password
#define DeviceIDoffset 66 // ��������� ���� Device ID
#define SizeDeviceID 32 // ������ ������ Device ID ����
#define DeviceTypeoffset 99 // ��������� ���� Device ID
#define SizeDeviceType 32 // ������ ������ Device ID ����
#define OrgIDoffset 132 // ��������� ���� Device ID
#define SizeOrgID 32 // ������ ������ Device ID ����
#define DWoffSet 165 //  ��������� ����: 65-75 ����- �������� ������ �����
#define SizeDW  10 // ������ ������ DW
#define FWoffSet 175 //  ��������� ����:  �������� ������� �����
#define SizeFW  10 // ������ ������ ��� �������
#define BoffSet 185 //  ��������� ����: ��� �������
#define SizeB  5 // ������ ������ ������ ���


char mqttserver[] = ORG ".messaging.internetofthings.ibmcloud.com"; // ������������ � Blumix
char topic[] = "iot-2/evt/status/fmt/json";
char restopic[] = "iot-2/cmd/rele/fmt/json";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
//char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID; // �������������� ����, ����� ���������� Device ID �� EEPROM
String clientID = "d:" ORG ":" DEVICE_TYPE ":";
char  cID[100]; // ������������ ��� ������������ ��������� Client ID
float dryWeight;

//------ ������� �����------

int DOUT = 5;   // HX711.DOUT ������ � ������� ����
int SCK1 = 4;  // HX711.PD_SCK ������������ ������� ����
int RELE = 14; // RELE �� D05  � ������ ������ - ��������
int BUTTON = 12; // ������ Setup
long massa = 0;
long oldmassa = 500;

Adafruit_BMP085 bmp; // ������� ���������� ��� ������ � ������������� BMP180

char ssid[32];
char password[32];
char* cfg_ssid = "co12"; // SSID ������������ � ������ AP �������
char* cfg_password = "87654321"; // Password ��� ������� � Cooler

long lastMsg = 0;
long starttime; // ������ �������� � ����� 0

String content;
String dwEPRPM;
String stip;
String stpsw;
String DeviceID;
String IDclient;
String Bottle;
char   dvid[32];
char  dwRead[10];
char  fwRead[10];
char  BRead[5];
int statusCode;

int MODE; // ����� ��������

ESP8266WebServer server(80);
WiFiClientSecure net; // sec
MQTTClient client;
unsigned long lastMillis = 0;

void myconnect(); // <- predefine connect() for setup()
				  //void APSetup(); // <- predefine APSetup for setup()
void EEread(char* eeprval, byte offSet, byte Size);
void EEwrite(String indata, byte offSet, byte Size);
void myreboot();


void setup() {
	Serial.begin(115200);
	while (!Serial) {
		// wait serial port initialization
	}
	pinMode(BUTTON, INPUT_PULLUP); // ������������ ��� ������ �� ������ ����������� � +
	attachInterrupt(BUTTON, myreboot, HIGH); // ������������� ���������� �� ���
	EEPROM.begin(512);
	MODE = EEPROM.read(0); // ���������� ����� ��������   ������ 0 ���� EEPROM- ���� 0 - ����� ������������ ���� 1 -������� �����
	EEread(dwRead, DWoffSet, SizeDW); //��������� �������� ������ ���� �� EEPROM
	EEread(fwRead, FWoffSet, SizeFW); //��������� �������� �������  ���� �� EEPROM
	EEread(BRead, BoffSet, SizeB); //��������� ��������  ���� ������� �� EEPROM
	EEread(dvid, DeviceIDoffset, SizeDeviceID); // ��������� ��������  Device ID;

	if (MODE == 1) // ���������� ����� ___________________________________________________________
	{
		Serial.println("Mode-1 - Normal-  Start");
		//������ SSID � Password �� EEPROM:++++++++++
		EEread(ssid, SSIDoffSet, SizeSSID);
		EEread(password, PasswordoffSet, SizePassword);
		Serial.println("SSID");
		Serial.println(ssid);
		Serial.println("Password");
		Serial.println(password);
		Serial.println("ID client:");
		//�������������� Client ID � ���������� � mac��, �.�. String MQTTClient �� ���������
		clientID += String(dvid);
		int str_len = clientID.length() + 1;
		clientID.toCharArray(cID, 50);
		Serial.println(cID);
		delay(1);
		WiFi.begin(ssid, password);
		pinMode(RELE, OUTPUT); // ��������� ���� RELE �� �����
		digitalWrite(RELE, LOW); // ���������� ����
		
		
		
		//--maks--ves.begin(DOUT, SCK1, 128); // ������������� ���
		
		
		client.begin(mqttserver, 8883, net); // 8883-sec 1883 -no sec
		myconnect();
	}

	if (MODE == 0)
	{
		Serial.println("Mode-0 -Config mode- Start");
		WiFi.printDiag(Serial);
		
		
		//--maks--ves.begin(DOUT, SCK1, 128);
		
		
		//WiFi.mode(WIFI_AP); // ����� ����� ��������� ����� AP, ���������� IP Gateway Subnet ip ADRESS, ��� ��������� �������� �� ��������� 192.168.4.1

		IPAddress APIP(10, 0, 0, 100);
		IPAddress gateway(10, 0, 0, 1);
		IPAddress subnet(255, 255, 255, 0);
		WiFi.softAPConfig(APIP, gateway, subnet);
		WiFi.softAP(cfg_ssid, cfg_password);
		starttime = millis(); // ��������� ����� �������� � ����� 0, ����� 5 ���
		launchWebAP(0);//OK
					   //return;
	}
}

void myconnect() {
	Serial.print("checking wifi...");
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(1000);
	}
	Serial.print("\nconnecting...");
	while (!client.connect(cID, authMethod, token)) {
		Serial.print("+");
		delay(1000);
	}
	Serial.println("\nconnected!");
	client.subscribe(restopic);
}
// � ����������� ������


void messageReceived(String restopic, String payload, char * bytes, unsigned int length) {


	if (payload == "{\"rel\":1}") {  // �������� ����
		digitalWrite(RELE, HIGH);
		Serial.println("RELE_ON");
	}
	else if (payload == "{\"rel\":0}") { // ��������� ����
		digitalWrite(RELE, LOW);
		Serial.println("RELE_OFF");
	}

	else if (payload == "{\"rel\":8}") { // �������� �� �������
		String uploadfile = String(dvid);
		uploadfile += ".bin";
		//    t_httpUpdate_return ret = ESPhttpUpdate.update("10.0.0.167", 80, uploadfile);
		Serial.println("NO Update OTA");
	}

	else {
		Serial.println("no_action");
	}
}
// � ���������� ������



String outmessage(long massa, char* dwRead, char* DeviceID)
{
	String pl = "{ \"d\" : {\"deviceid\":\"";
	pl += DeviceID;
	pl += "\",\"currentWeight\":\"";
	pl += massa;
	pl += "\",\"dryWeight\":\"";
	pl += dwRead;
	pl += "\"}}";
	return pl;
}


void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect(cID, authMethod, token)) {
			Serial.println("connected");
			// Once connected, publish an announcement...

			String payload = outmessage(massa, dwRead, dvid);
			client.publish(topic, (char*)payload.c_str());
			// ... and resubscribe
			client.subscribe(restopic);
		}
		else {
			Serial.print("failed, rc=");
			// Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}
// � ���������� ������

//_____________________________________________________________

void launchWebAP(int webtype) {
	createWebServer(webtype);
	server.begin();
}
void createWebServer(int webtype)
{

	if (webtype == 0) {
		server.on("/", []() {
			EEread(ssid, SSIDoffSet, SizeSSID);
			EEread(password, PasswordoffSet, SizePassword);
			EEread(dwRead, DWoffSet, SizeDW);
			EEread(dvid, DeviceIDoffset, SizeDeviceID);
			//_______________________________________________________
			content = "<!DOCTYPE HTML>\r\n<html>  <head> <meta http-equiv=\"Content - Type\" content=\"text / html; charset = utf-8\"> </head> <h1>Smart Cooler ���������</h1> <h2>������� ��������:</h2>";
			content += "SSID=";
			content += ssid;
			content += "  Password:";
			content += password;
			content += "<br>  Dry Weight :";
			content += dwRead;
			content += "  Device ID :";
			content += dvid;
			//_______________________________________________________
			content += " <hr><h2> ������� ����� �������� SSID �  Password </h2>";
			content += "<form method='get' action='setting'>";
			content += "<label>SSID: </label><input name='ip' length=32><br><br>";
			content += "<label>PASSWORD: </label><input name='password' length=32><br><br>";
			content += "<input type='submit' value='��������� SSID/Password'></form>";
			//______________________________________________________
			content += "<hr><h2>������� ����� ������������� ���������� </h2> ";
			content += "<form method='get' action='deviceidsetting'>";
			content += "<label>DeviceID: </label><input name='DeviceID' length=32><br><br>";
			content += "<input type='submit' value='��������� Device ID'></form>";
			//______________________________________________________
			content += "<h2> ���������� ���� �������� ������ </h2>";
			content += "<p> ���������� ������ ����� ����� �� ���������, ������� ������ Save Dry Weight. </p>";
			content += "<p><font  color=\"red\"> �� ��������� ������ ��������� ����� ��� ���� � ������ ���� ����.</font> </p>";
			content += "<p><font  color=\"blue\"> ���� �� �������� ��������� ����� ��� ��� ������� ����: <br> 1)������� ������ � ������,<br> 2)������ ������� ���� �� ������ <br> 3)������� ������ ��������� ����� ���.</font> </p>";
			content += "<form method='get' action='dw'><input type='submit' value='Save Dry Weight'></form>";
			content += "<h2> ���������� ���� �������  ������ </h2>";
			content += "<p> ���������� ������ ������� �  �����, ��������� 10 ������ , ������� ������ Save Full Weight. </p>";
			content += "<form method='get' action='fw'><input type='submit' value='Save Full Weight'></form>";
			//______________________________________________________

			content += "<hr><h3>��������� ���� ������ </h3> ";
			content += "������� ��� ������ :";
			content += BRead;
			content += "<form method='get' action='deviceidsetting'>";
			content += "<label>��� ������: </label><input name='Bottle' length=6><br><br>";
			content += "<input type='submit' value='��������� ��� ������'></form>";
			//______________________________________________________
			content += "<hr><h2> ������������ � ������� ����� </h2>";
			content += "<p><font  color=\"red\"> ����� ������������ ������������� ����������.</font> </p>";
			content += "<form method='get' action='changemode'><input type='submit' value='������������� � ������� �����'></form>";
			content += "</html>";
			server.send(200, "text/html", content);
		});

		server.on("/changemode", []() {
			EEPROM.write(0, 1); // ������������� ����� ���������� ��������
			EEPROM.commit();
			content = "<!DOCTYPE HTML>\r\n<html>";
			content = "<p> \"���������� ���������� � ������� �����, ������������� ����������\"</p></br>}";
			content += "<a href='/'>��������� � ������������</a>";
			content += "</html>";
			server.send(200, "text/html", content);
			ESP.restart();
		});
		server.on("/setting", []() {
			String stip = server.arg("ip");
			String stpsw = server.arg("password");
			if (stip.length() > 0) {
				EEwrite(stip, SSIDoffSet, SizeSSID);
				EEwrite(stpsw, PasswordoffSet, SizePassword);
				content = "<!DOCTYPE HTML>\r\n<html>";
				content = "<p>SSID Password ��������� </p>";
				content += "</br>";
				content += "<a href='/'>��������� � ������������</a>";
				content += "</html>";
				server.send(200, "text/html", content);
				Serial.println("Sending 200 -SSID PASWD-OK");
			}
			else {
				content = "{\"Error\":\"404 not found\"}";
				statusCode = 404;
				Serial.println("Sending 404");
			}
			server.send(statusCode, "application/json", content);

		});

		server.on("/dw", []() {
			
			
			//--maks--dryWeight = ves.read_average(30);
			
			dryWeight = 1000;
			
			dwEPRPM = String(dryWeight);
			EEwrite(dwEPRPM, DWoffSet, SizeDW);
			content = "<!DOCTYPE HTML>\r\n<html>";
			content += "<p>Dry Wight calibrate OK <br> </p>";
			content += "<a href='/'>��������� � ������������</a>";
			content += "</html>";
			server.send(200, "text/html", content);
		});

		server.on("/fw", []() {
			
			//--maks--dryWeight = ves.read_average(30);
			dryWeight = 1000;
			
			dwEPRPM = String(dryWeight);
			EEwrite(dwEPRPM, FWoffSet, SizeFW);
			content = "<!DOCTYPE HTML>\r\n<html>";
			content += "<p>Full Wight calibrate OK <br> </p>";
			content += "<a href='/'>��������� � ������������</a>";
			content += "</html>";
			server.send(200, "text/html", content);

		});

		server.on("/deviceidsetting", []() {
			DeviceID = server.arg("DeviceID");
			EEwrite(DeviceID, DeviceIDoffset, SizeDeviceID);
			content = "<!DOCTYPE HTML>\r\n<html>";
			content += "<p> Deviceid Settings <br>";
			content += "</p>";
			content += "<a href='/'>��������� � ������������</a>";
			content += "</html>";
			server.send(200, "text/html", content);
		});

		server.on("/bottlesetting", []() {
			Bottle = server.arg("Bottle");
			EEwrite(Bottle, BoffSet, SizeB);
			content = "<!DOCTYPE HTML>\r\n<html>";
			content += "<p> ��� ������ �������� <br>";
			content += "</p>";
			content += "<a href='/'>��������� � ������������</a>";
			content += "</html>";
			server.send(200, "text/html", content);

		});
	}
}


void myreboot() // ������������ � ����� ���������������� ��� ������� ������
{
	EEPROM.write(0, 0); // ������������� ����� � 0
	EEPROM.commit();
	Serial.println("Reboot");
	detachInterrupt(BUTTON);
	//ESP.reset();
	ESP.restart();
}


void EEread(char* eeprval, byte offSet, byte Size)
{

	for (int i = 0; i <= Size; ++i)
	{
		eeprval[i] = char(EEPROM.read(i + offSet));
		if (char(EEPROM.read(i + offSet)) == '\0')
		{
			break;
		}
	}

}

void EEwrite(String indata, byte offSet, byte Size) // ������ � EEPROM
{
	if (indata.length() > 0 && Size >= indata.length())

	{

		for (int i = 0; i <= indata.length(); ++i)
		{
			EEPROM.write(i + offSet, indata[i]);
		}
		EEPROM.commit();
	}
}

//_____________________________________________________________

void loop() {


	if (MODE == 1)
	{

		//--maks--massa = ves.read_average(10);


		long now = millis();

		if (massa < oldmassa - 100 || massa > oldmassa + 1000 || now - lastMsg > 10000) // �������� ��������� ��� ��������� ����� ��� ��  ��������
		{

			delay(2000); // ���� ����  ��������� ��������� �������� ���������� ����
			//--maks--massa = ves.read_average(10);


			if (!client.connected()) {
				reconnect();
			}

			lastMsg = now;
			oldmassa = massa;
			String payload = outmessage(massa, dwRead, dvid);
			Serial.print("Publish message: ");
			Serial.println(payload);
			client.publish(topic, (char*)payload.c_str());
			Serial.println("OTAAAA: ");

		}

		client.loop();
		delay(10);
	}
	if (MODE != 1)
	{
		server.handleClient();
		long now = millis();
		if (now - starttime > 300000)  // ����� 5 ��� ���������� � ���������������� ������, ��������������� � �������� �����
		{
			EEPROM.write(0, 1);
			EEPROM.commit();
			Serial.println("Reboot");
			ESP.restart();
		}

	}
}
