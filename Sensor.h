/****
 * 각종 센서 파일
 * 
 * 1. 내부, 외부 온습도 측정
 * 2. 이산화탄소 농도 측정
 * 3. 인체감지
 */

//타이머 파일
#include "Timer.h"

//내부 온도 내장 파일
#include <DHT12.h>

//외부 온도 내장 파일
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>

//이산화탄소 농도 측정 파일
#include <ccs811.h>

DHT12 dht12;         //온습도 센서 객체 생성
CCS811 ccs811;      //공기 질 센서 객체 생성
const int HumanSensor = D6;     //인체감지 센서

double temp_In;     //내부 온도 변수
double humid_In;    //내부 습도 변수
String text_TempIn; //내부 온도에 따른 문자

double temp_Out;        //외부온도 변수
double humid_Out;       //외부습도 변수
double press_Out;       //외부기압 변수
double wind_Out;        //외부풍속 변수
String text_TempOut;    //외부온도에 따른 문자

uint16_t eco2, etvoc, errstat, raw; //공기 질 측정 변수 
String text_CO2;                    //이산화탄소 농도, 공기 질에 따른 문자

String openWeatherMapApiKey = "bd2c147ec445786b9517bba2893e8402";   //제작자의 Api 번호
String lat = "37.2839";     //외부 온습도를 가져올 장소의 위도
String lon = "126.9896";    //외부 온습도를 가져올 장소의 경도
String jsonBuffer;

bool checker;   //인체감지 여부
bool b_checker = false;     //갱신여부
bool find_human;            //최종 인체감지 여부
bool human_measure;         //인체감지 측정여부(1: 측정, 0: 미측정)
unsigned long Hdect_Day;                   //미감지 스톱워치-일
unsigned long Hdect_Hour;                  //미감지 스톱워치-시
unsigned long Hdect_Minute;                //미감지 스톱워치-분
unsigned long Hdect_Second;                //미감지 스톱워치-초
int cnt_notfound;                           //인체 미감지 시간

String human_text;  //인체감지 결과에 따른 문자

void ReadPrint_TempHumidIn();   //내부 온습도 측정

void ReadPrint_TempHumidOut();                  //외부 온습도 가져오는 함수
String httpGETRequest(const char* serverName);  //http 관련

void Setup_CO2();       //CCS811 준비
void ReadPrint_CO2();   //공기 질, 이산화탄소 농도 측정

void Read_Human(void);      //인체감지 함수
void SetHTime(unsigned long Timer);     //시간, 분 저장 함수

void ReadPrint_TempHumidIn(){
    temp_In = dht12.readTemperature();  //온도측정
    humid_In = dht12.readHumidity();    //습도측정

    if (isnan(temp_In)) {
        Serial.println("Failed to read from DHT12 sensor!");
    }else{
        Serial.print("Temperature_Inside: ");
        Serial.print(temp_In);
        Serial.println(" °C");
    }

    //내부 온도 값에 따라 문자를 저장한다
    if(temp_In >= 27) text_TempIn = "온도가 높습니다. 에어컨이나 선풍기를 틀고 생활하세요";
    else if(temp_In >= 26.5 && temp_In < 27) text_TempIn = "적절한 온도입니다";
    else text_TempIn = "온도가 낮습니다. 난방 온도를 높이시는 것을 권장드립니다";
}

void ReadPrint_TempHumidOut(){
    if(WiFi.status() == WL_CONNECTED){  //Wifi 연결 성공 시 아래 코드를 실행
        String serverPath = "http://api.openweathermap.org/data/2.5/weather?lat=" +
        lat + "&lon=" + lon + "&units=metric&APPID=" + openWeatherMapApiKey;
        jsonBuffer = httpGETRequest(serverPath.c_str());
        Serial.println(jsonBuffer);
        JSONVar myObject = JSON.parse(jsonBuffer);
        if(JSON.typeof(myObject) == "undefined"){
            Serial.println("Parsing input failed!");
        }
        //시리얼 출력
        Serial.print("JSON object = ");
        Serial.println(myObject);
        Serial.print("Temperature: ");
        Serial.print(myObject["main"]["temp"]);
        Serial.println(" °C");
        Serial.print("Pressure: ");
        Serial.println(myObject["main"]["pressure"]);
        Serial.print("Humidity: ");
        Serial.println(myObject["main"]["humidity"]);
        Serial.println(" %");
        Serial.print("Wind Speed: ");
        Serial.println(myObject["wind"]["speed"]);

        temp_Out = myObject["main"]["temp"];
        humid_Out = myObject["main"]["humidity"];
        press_Out = myObject["main"]["pressure"];
        wind_Out = myObject["wind"]["speed"];

        //외부온도에 따른 문자를 저장한다
        if(temp_Out > 33){
            text_TempOut = "폭염이니 주의하시기 바랍니다";
        } else if(temp_Out > 28){
            text_TempOut = "바깥이 더우니 주의하시기 바랍니다";
        } else if(temp_Out <= 28 && temp_Out >= 20){
            text_TempOut = "정상적인 온도입니다";
        } else if(temp_Out < 20){
            text_TempOut = "살짝 쌀쌀합니다";
        } else if(temp_Out < 0){
            text_TempOut = "추우니 주의하시기 바랍니다";
        } else if(temp_Out < -15){
            text_TempOut = "한파이니 주의하시기 바랍니다";
        }
    } else {    //WiFi 연결 실패 시 아래 코드를 실행
        Serial.println("WiFi Disconnected");
    }
}

String httpGETRequest(const char* serverName){
    WiFiClient client;
    HTTPClient http;
    http.begin(client, serverName);
    int httpResponseCode = http.GET();  //Send HTTP POST request
    String payload = "{}";
    if(httpResponseCode > 0){
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    http.end(); //Free resources
    return payload;
}

void Setup_CO2(){
    //CCS811 준비
    Serial.println("setup: Starting CCS811 basic demo");
    Serial.print("setup: ccs811 lib  version: "); Serial.println(CCS811_VERSION);
    
    // CCS811 시작
    ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
    bool ok= ccs811.begin();
    if( !ok ) Serial.println("setup: CCS811 begin FAILED");
    
    // CCS811 버젼 출력
    Serial.print("setup: hardware    version: "); Serial.println(ccs811.hardware_version(),HEX);
    Serial.print("setup: bootloader  version: "); Serial.println(ccs811.bootloader_version(),HEX);
    Serial.print("setup: application version: "); Serial.println(ccs811.application_version(),HEX);
    
    // 측정 시작
    ok= ccs811.start(CCS811_MODE_1SEC);
    if( !ok ) Serial.println("setup: CCS811 start FAILED");
}

void ReadPrint_CO2(){
    // Read
    ccs811.read(&eco2,&etvoc,&errstat,&raw); //공기 질 등 측정
    if( errstat==CCS811_ERRSTAT_OK ) { 
        //CCS-811에서 측정한 것 출력
        Serial.print("CCS811: ");
        Serial.print("eco2=");  Serial.print(eco2);     Serial.print(" ppm  ");     //이산화탄소 농도
        Serial.print("etvoc="); Serial.print(etvoc);    Serial.println(" ppb  ");   //공기 질
        //Serial.print("raw6=");  Serial.print(raw/1024); Serial.print(" uA  "); 
        //Serial.print("raw10="); Serial.print(raw%1024); Serial.print(" ADC  ");
        //Serial.print("R="); Serial.print((1650*1000L/1023)*(raw%1024)/(raw/1024)); Serial.print(" ohm");

        //이산화탄소 농도에 따른 문자를 저장
        if(eco2 >= 2000) text_CO2 = "공기 질이 나쁩니다. 공기청정기를 틀거나 환기를 하시길 권장드립니다";
        else if(eco2 >= 1000 && eco2 < 2000) text_CO2 = "공기 질이 보통입니다";
        else text_CO2 = "공기 질이 좋습니다";
    } else if( errstat==CCS811_ERRSTAT_OK_NODATA ) {
        Serial.println("CCS811: waiting for (new) data");
    } else if( errstat & CCS811_ERRSTAT_I2CFAIL ) { 
        Serial.println("CCS811: I2C error");
    } else {
        Serial.print("CCS811: errstat="); Serial.print(errstat,HEX); 
        Serial.print("="); Serial.println( ccs811.errstat_str(errstat) ); 
    }
}

void Read_Human(void){
    checker = digitalRead(HumanSensor); //1: 사람 감지, 0: 사람 미감지
    if(b_checker != checker){
        if(checker == true){    //사람이 감지되었다면
            cnt_notfound = 0;
            Timer_HumanDetect = 0;
            human_measure = true;
            human_text = "사람이 감지되었습니다. 본인인지 확인해주세요"; //사람이 감지되었다는 문자
            Serial.println("Human detected");
        }else{  //감지되지 않았지만 기준치 시간에 미달일 때
            cnt_notfound = 0;
            Timer_HumanDetect = 0;
            human_text = "사람이 감지되지 않았습니다. 잘 활동하고 계신가요?";
            human_measure = false;
            Serial.println("Human not detected");
        }
        b_checker = checker;
    }
    if(cnt_notfound > 100) human_text = "오랫동안 거주자님이 감지되지 않았습니다";
}

void SetHTime(unsigned long Timer){
    unsigned long sec = Timer/1000;
    Hdect_Second = sec%60;
    Hdect_Minute = (sec/60)%60; 
    Hdect_Hour = (sec/3600)%24;
    Hdect_Day = sec / 86400;

    Serial.print(Hdect_Day);
    Serial.print(" day(s)\t");
    Serial.print(Hdect_Hour);
    Serial.print("hour(s)\t");
    Serial.print(Hdect_Minute);
    Serial.print("min.\t");
    Serial.print(Hdect_Second);
    Serial.println("sec.");
}
