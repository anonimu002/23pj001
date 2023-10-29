/****
 * LCD 측정 파일
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 4);    //LCD(주소, 16열, 4행)

//LCD 글자
byte h_nae[8]           ={0b00001, 0b00101, 0b10101, 0b10101, 0b10111, 0b11101, 0b00101, 0b00001};    //내
byte h_wae[8]           ={0b01001, 0b10101, 0b10101, 0b01001, 0b00001, 0b01001, 0b11111, 0b00001};    //외
byte h_bu[8]            ={0b10001, 0b11111, 0b10001, 0b11111, 0b00000, 0b11111, 0b00100, 0b00100};    //부
byte h_temp[8]          ={0b00100, 0b01010, 0b01010, 0b01010, 0b01010, 0b10001, 0b11111, 0b01110};    //온도 그림
byte h_humi[8]          ={0b10001, 0b10001, 0b10001, 0b10001, 0b10101, 0b11111, 0b11111, 0b11111};    //습도 그림
byte h_tunit[8]       ={0b10000, 0b10110, 0b01001, 0b01000, 0b01000, 0b01001, 0b00110, 0b00000};      //°C
byte h_cha[8]           ={0b01010, 0b01010, 0b11110, 0b00110, 0b01011, 0b10110, 0b00010, 0b00010};    //차
byte h_i[8]             ={0b00001, 0b01001, 0b10101, 0b10101, 0b10101, 0b10101, 0b01001, 0b00001};    //이

double tempDif, humidDif;   //온습도 차이 값 변수
int b_print = 0;            //출력 갱신 여부
int printInterval = 0;      //출력 종류(0: 내부, 외부 온습도+차이, 1: 공기질, 2: 인체감지+경과시간)

void SetupLCD(bool flag_Ready);                                                                                        //LCD '준비 중', '준비 완료' 출력
void FlagLCD(bool flag_start, String IP);                                                               //WiFi 연결 성공 여부 출력
void Print_TempHumidIn_LCD(double tempIn, double humidIn);                                              //내부 온습도 출력
void Print_TempHumidOut_LCD(double tempOut, double humidOut);                                           //외부 온습도 출력
void Print_TempHumidDif_LCD(double tempIn, double humidIn, double tempOut, double humidOut);            //내부, 외부 온습도 차이 출력
void Print_CO2_LCD(double co2, double air);                                                             //이산화탄소, 공기 질 출력
void Print_Human_LCD(int type);                                                                         //인체감지 결과 출력
void Print_TimerHuman_LCD(unsigned int Hour, unsigned int Minute);                                      //인체감지 스톱워치 출력(시간, 분)

void SetupLCD(bool flag_Ready){
    if(flag_Ready == false){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Activating...");
    } else {
        lcd.setCursor(0,1);
        lcd.print("Ready.....");
    }
}

void FlagLCD(bool flag_start, String IP){
    lcd.clear();
    lcd.setCursor(0,0);
    if(flag_start == true){     //연결 성공 시 아래 코드 실행
        lcd.print("Success!");
        lcd.setCursor(0,1);
        lcd.print("IP is...");
        lcd.setCursor(-4,2);
        lcd.print(IP);
        lcd.setCursor(-4,3);
        lcd.print("Starting...");
    } else {                    //연결 실패 시 아래 코드 실행
        lcd.print("Connecting...");
    }
}

void Print_TempHumidIn_LCD(double tempIn, double humidIn){
    //내부온도
    lcd.setCursor(0,0);
    lcd.write(1);
    lcd.write(3);
    lcd.print(":");
    lcd.write(4);
    lcd.print(tempIn,1);
    lcd.write(6);

    //내부습도
    lcd.setCursor(10,0);
    lcd.write(5);
    lcd.print(humidIn,1);
    lcd.print("%");
}

void Print_TempHumidOut_LCD(double tempOut, double humidOut){
    //외부온도
    lcd.setCursor(0,1);
    lcd.write(2);
    lcd.write(3);
    lcd.print(":");
    lcd.write(4);
    lcd.print(tempOut, 1);
    lcd.write(6);

    //외부습도
    lcd.setCursor(10,1);
    lcd.write(5);
    lcd.print(humidOut, 1);
    lcd.print("%");
}

void Print_TempHumidDif_LCD(double tempIn, double humidIn, double tempOut, double humidOut){
    //온습도 차이 변수 계산
    if(tempIn > tempOut) tempDif = tempIn - tempOut;
    else tempDif = tempOut - tempIn;

    if(humidIn > humidOut) humidDif = humidIn - humidOut;
    else humidDif = humidOut - humidIn;

    //온도차이 출력
    lcd.setCursor(-4,3);
    lcd.write(7);
    lcd.write(8);
    lcd.print(":");
    lcd.write(4);
    lcd.print(tempDif,1);
    lcd.write(6);

    //습도차이 출력
    lcd.setCursor(6,3);
    lcd.write(5);
    lcd.print(humidDif,1);
    lcd.print("%");
}

void Print_CO2_LCD(double co2, double air){
    lcd.setCursor(0,0);
    lcd.print("CO2 : ");
    lcd.print(co2,0);
    lcd.print(" ppm   ");
    lcd.setCursor(0,1);
    lcd.print("Air : ");
    lcd.print(air,0);
    lcd.print(" ppb  ");

    lcd.setCursor(-4,3);
    lcd.print("State:");
    if(eco2 > 1600 && etvoc > 300){
        lcd.print("Bad    ");
    } else if(eco2 > 1600 || etvoc > 300){
        lcd.print("Normal   ");
    }else{
        lcd.print("Good    ");
    }
}

void Print_TimerHuman_LCD(bool flagH, unsigned long Day, unsigned long Hour, unsigned long Minute, unsigned long Second){
    String DayLine, HourLine, MinuteLine, SecondLine;
    (Day >= 10) ? DayLine ="Day" : DayLine = " Day";
    (Hour >= 10) ? HourLine = " " : HourLine = " 0";
    (Minute >= 10) ? MinuteLine = ":" : MinuteLine = ":0";
    (Second >= 10) ? SecondLine = ":" : SecondLine = ":0";
    lcd.setCursor(0,0);
    lcd.print("Human Time");
    lcd.setCursor(0, 1);
    (flagH == true) ? lcd.print("Detected  ") : lcd.print("Not found  ");
    lcd.setCursor(-4,2);
    lcd.print(Day);
    lcd.print(DayLine);
    lcd.setCursor(-1,3);
    lcd.print(HourLine);
    lcd.print(Hour);
    lcd.setCursor(2,3);
    lcd.print(MinuteLine);
    lcd.print(Minute);
    lcd.setCursor(5, 3);
    lcd.print(SecondLine);
    lcd.print(Second);
}
