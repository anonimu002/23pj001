/****
 * 작성일 : 2023.10.23
 * 작성자 : 대평중학교 3학년 조민서
 * 제  목 : 프로젝트 ver. 0.1.0.0.3(LCD 갱신 추가)
 * 
 * Beta 버전입니다
 */

#include "WiFi.h"   //WiFi 관련 코드가 들어있는 파일

void setup() {
    Serial.begin(115200);   //시리얼 시작
    Serial.println();
    Serial.println("Activating.....");  //활성화 중이라고 사용자에게 알린다
    pinMode(HumanSensor, INPUT);    //인체감지 센서를 입력 모드로 저장
    Wire.begin();       //I2C 시작
    lcd.begin();        //LCD 시작
    lcd.backlight();    //LCD 백라이트 사용
    lcd.clear();        //이전에 LCD에 출력된 모든 것 지우기
    SetupLCD(false);         //LCD에도 활성화 중이라고 출력
    dht12.begin();  //DHT 12 온습도 센서 시작
    Serial.println("Ready to start....");      //모든 준비 완료->시작해도 된다는 뜻으로 출력
    SetupLCD(true);         //모든 준비를 끝내서 준비가 되었다고 출력
    _start(1);  //0: 바로시작, 1: 시작대기
    Serial.println("Starting...");  //시작 중이라고 출력
    
    //WiFi 연결
    FlagLCD(false, IP);
    Check_autoConnect();
    IP = WiFi.localIP().toString();
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    FlagLCD(true, IP);  //WiFi 무선 연결에 성공, IP를 LCD에 출력
    setup_wifi();   //MQTT, WiFi 연결
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    
    Setup_CO2();    //CCS811 준비
    lcd.clear();
}

void loop() {
    if (!client.connected()) reconnect();
    client.loop();
    
    Button_Func();      //버튼 기능 주관
    LCD_Func();         //LCD 갱신 주관
    MQTT_TempHumid();   //내부, 외부 온습도 측정
    MQTT_AirCO2();      //공기 질, 이산화탄소 측정
    MQTT_Human();       //인체감지 및 경과 시간 계산
}

void Button_Func(){
    if(digitalRead(START_KEY) == LOW) { //버튼을 눌렀을 시 아래 코드를 실행
        Buzzer_OnOff(16, 1, 1, 80); //부저로 눌렀다는 것을 알려줌
        Timer_ButtonPress = 0;
        while(digitalRead(START_KEY) == LOW);
        printInterval++;
        if(printInterval > 2) printInterval = 0;
        Timer_LCDchange = LCDinterval;
    }
}

void LCD_Func(){
    if(Timer_LCDchange > LCDinterval){
        Timer_LCDchange = 0;
        if(b_print != printInterval){
            lcd.clear();
            b_print = printInterval;
        }
        
        switch(printInterval) { //LCD 전환
            case 0:     //내부, 외부 온습도와 그 차이값을 출력
                lcd.createChar(1, h_nae);       //'내' 문자 저장
                lcd.createChar(2, h_wae);       //'외' 문자 저장
                lcd.createChar(3, h_bu);        //'부' 문자 저장
                lcd.createChar(4, h_temp);      //온도그림 저장
                lcd.createChar(5, h_humi);      //습도그림 저장
                lcd.createChar(6, h_tunit);     //'°C' 문자 저장
                lcd.createChar(7, h_cha);       //'차' 문자 저장
                lcd.createChar(8, h_i);         //'이' 문자 저장
                Print_TempHumidIn_LCD(temp_In, humid_In);                       //내부 온습도
                Print_TempHumidOut_LCD(temp_Out, humid_Out);                    //외부 온습도
                Print_TempHumidDif_LCD(temp_In, humid_In, temp_Out, humid_Out); //내부, 외부 온습도 차이
                break;
            case 1:     //이산화탄소 농도, 공기 질 출력
                Print_CO2_LCD(eco2, etvoc);
                break;
            case 2:     //인체감지 시간을 출력
                Print_TimerHuman_LCD(human_measure, Hdect_Day, Hdect_Hour, Hdect_Minute, Hdect_Second);
                break;
        }
    }
}

void MQTT_TempHumid(){  //내부, 외부 온습도(MQTT) 주관 함수
    if(Timer_TempHumid > TempHumidInterval){    //5초마다 내부, 외부 온습도를 측정한다.
        Timer_TempHumid = 0;        //타이머 값 초기화
        ReadPrint_TempHumidIn();    //내부 온습도 측정
        ReadPrint_TempHumidOut();   //외부 온습도를 한 사이트에서 가져온다
        client.publish(topic_tempIn, String(temp_In).c_str(), true);            //내부 온도를 MQTT로 보냄
        client.publish(topic_humidIn, String(humid_In).c_str(), true);          //내부 습도를 MQTT로 보냄
        client.publish(topic_tempOut, String(temp_Out).c_str(), true);          //외부 온도를 MQTT로 보냄
        client.publish(topic_humidOut, String(humid_Out).c_str(), true);        //외부 습도를 MQTT로 보냄
        client.publish(topic_tempDif, String(tempDif).c_str(), true);           //내부, 외부 온도 차이 값을 MQTT로 보냄
        client.publish(topic_humidDif, String(humidDif).c_str(), true);         //내부, 외부 습도 차이 값을 MQTT로 보냄
        client.publish(topic_mesTempIn, String(text_TempIn).c_str(), true);     //내부 온도에 따른 문자를 MQTT로 보냄
        client.publish(topic_mesTempOut, String(text_TempOut).c_str(), true);   //외부 온도에 따른 문자를 MQTT로 보냄
    }
}
void MQTT_AirCO2(){ //공기 질(MQTT) 주관 함수
    if(Timer_Air > AirInterval){    //1초마다 공기 질, 이산화탄소 농도를 측정한다.
        Timer_Air = 0;  //타이머 값 초기화
        ReadPrint_CO2();    //공기 질, 이산화탄소 농도 측정
        client.publish(topic_ReadCO2ppm, String(eco2).c_str(), true);   //이산화탄소 농도 값을 MQTT로 보냄
        client.publish(topic_ReadCO2pbm, String(etvoc).c_str(), true);  //공기 질 값을 MQTT로 보냄
        client.publish(topic_mesCO2, String(text_CO2).c_str(), true);   //두 값에 따른 문자를 MQTT로 보냄
    }
}
void MQTT_Human(){  //인체감지(MQTT) 주관함수
    //인체감지
    if(Timer_Human > HumanInterval){    //0.1초마다 사람을 감지한다
        Timer_Human = 0;
        if(human_measure == false) cnt_notfound++;                      //인체감지 미감지 시간을 주기적으로 카운팅
        Read_Human();
        client.publish(topic_textH, String(human_text).c_str(), true);  //인체감지 측정 결과에 따른 문자를 MQTT로 보냄
    }

    //인체감지 타이머 계산
    if(Timer_Htime > HtimeInterval){    //1초마다 실시간으로 계산한다
        Timer_Htime = 0;
        SetHTime(Timer_HumanDetect);
        client.publish(topic_Hmeasured, String(human_measure).c_str(), true);
        client.publish(topic_DayH, String(Hdect_Day).c_str(), true);
        client.publish(topic_HourH, String(Hdect_Hour).c_str(), true);
        client.publish(topic_MinuteH, String(Hdect_Minute).c_str(), true);
        client.publish(topic_SecondH, String(Hdect_Second).c_str(), true);
    }
}
