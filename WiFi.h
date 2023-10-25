/****
 * WiFi 파일
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include "Sensor.h"         //각종 센서 파일
#include "Lcd.h"            //LCD 파일
#include "StartActBuz.h"    //시작버튼 파일

WiFiClient espClient;
PubSubClient client(espClient);
WiFiManager wm;

const char* mqtt_server = "mqtt.easylab.kr";            //MQTT 주소
const char* topic_tempIn = "cms/temp_In";               //내부 온도
const char* topic_humidIn = "cms/humid_In";             //내부 습도
const char* topic_tempOut = "cms/temp_Out";             //외부 온도
const char* topic_humidOut = "cms/humid_Out";           //외부 습도
const char* topic_tempDif = "cms/temp_Difference";      //내부, 외부의 온도 차이
const char* topic_humidDif = "cms/humid_Difference";    //내부, 외부의 습도 차이
const char* topic_ReadCO2ppm = "cms/co2ppm";            //이산화탄소 농도(CCS-811)
const char* topic_ReadCO2pbm = "cms/co2pbm";            //공기 질(CCS-811)
const char* topic_mesTempIn = "cms/temp_InText";        //문자(내부온도)
const char* topic_mesTempOut = "cms/temp_outText";      //문자(내부 온도)
const char* topic_mesCO2 = "cms/CO2_Text";              //문자(공기 질)
const char* topic_textH = "cms/Htext";                  //문자(인체감지)
const char* topic_Hmeasured = "cms/Human_Detect";       //인체감지 측정여부
const char* topic_DayH = "cms/HdetectDay";              //인체감지 경과시간(일)
const char* topic_HourH = "cms/HdetectHour";            //인체감지 경과시간(시)
const char* topic_MinuteH = "cms/HdetectMinute";        //인체감지 경과시간(분)
const char* topic_SecondH = "cms/HdetectSecond";        //인체감지 경과시간(초)

String IP;  //IP를 String으로 저장하기 위해 만들었다

//메인에 있는 함수
void MQTT_TempHumid();
void MQTT_AirCO2();
void MQTT_Human();

//WiFi.h 내 함수
void setup_wifi();
void callback(char* topic, unsigned char* payload, unsigned int length);
void reconnect();
void Check_autoConnect(void);

void setup_wifi() {
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WiFi.SSID());

    WiFi.mode(WIFI_STA);
    WiFi.begin(WiFi.SSID(), WiFi.psk());
    Buzzer_OnOff(16, 1, 1, 80);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Buzzer_OnOff(16, 1, 2, 80);
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP8266Client-";
        clientId += String(ESP.getChipId(), HEX);   //random(0xffff
        Serial.println(clientId);
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");      
            client.publish(topic_tempIn, String(temp_In).c_str(), true);            //내부온도
            client.publish(topic_humidIn, String(humid_In).c_str(), true);          //내부습도
            client.publish(topic_tempOut, String(temp_Out).c_str(), true);          //외부온도
            client.publish(topic_humidOut, String(humid_Out).c_str(), true);        //외부습도
            client.publish(topic_tempDif, String(tempDif).c_str(), true);           //내부온도와 외부온도의 차이
            client.publish(topic_humidDif, String(humidDif).c_str(), true);         //내부습도와 외부습도의 차이
            client.publish(topic_ReadCO2ppm, String(eco2).c_str(), true);           //이산화탄소 농도(ppm)
            client.publish(topic_ReadCO2pbm, String(etvoc).c_str(), true);          //이산화탄소 농도(ppb)
            client.publish(topic_mesTempIn, String(text_TempIn).c_str(), true);     //내부온도 문자
            client.publish(topic_mesTempOut, String(text_TempOut).c_str(), true);   //외부온도 문자
            client.publish(topic_mesCO2, String(text_CO2).c_str(), true);           //이산화탄소 농도 문자
            client.publish(topic_textH, String(human_text).c_str(), true);          //인체감지 문자
            client.publish(topic_Hmeasured, String(human_measure).c_str(), true);   //인체감지 여부
            client.publish(topic_DayH, String(Hdect_Day).c_str(), true);            //인체감지 경과시간(일)
            client.publish(topic_HourH, String(Hdect_Hour).c_str(), true);          //인체감지 경과시간(시간)
            client.publish(topic_MinuteH, String(Hdect_Minute).c_str(), true);      //인체감지 경과시간(분)
            client.publish(topic_SecondH, String(Hdect_Second).c_str(), true);      //인체감지 경과시간(초)
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            Buzzer_OnOff(16, 1, 4, 80);
            delay(5000);
        }
    }
}

void Check_autoConnect(void){
    WiFi.mode(WIFI_STA);
    bool res = wm.autoConnect("AutoConnectAP","12345678");
    if(!res) Serial.println("Failed to connect");
    else{        
        Serial.println("WiFi connected successfully");  
        Serial.printf("Chip ID: %06X\n", ESP.getChipId());
        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());   
        Serial.print("PSK: ");
        Serial.println(WiFi.psk());    
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
}
