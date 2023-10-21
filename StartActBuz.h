/****
 * 시작버튼 파일
 */

#define pinBUZZER D0
#define pinSTART D3
#define START_KEY (pinMode(pinSTART,INPUT), delay(5),(digitalRead(pinSTART)==0)?1:(pinMode(pinSTART,OUTPUT),0))

void Buzzer_OnOff(int GPIO, int type, int count, int pause);    //부저 함수
void _start(int);

void Buzzer_OnOff(int GPIO, int type, int count, int pause){
    int On, Off;
    switch(GPIO){
        case 16:    //D0 핀
            On = 0;
            Off = 1;
            break;
        default:
            On = 1;
            Off = 0;
            break;
    }
    switch(type){
        case 1:     //Active Buzzer일 때
            for(int i=0;i<count;i++){
                digitalWrite(pinBUZZER, On);
                delay(pause);
                digitalWrite(pinBUZZER, Off);
                delay(pause);
            }
            break;
        case 2:     //Passive Buzzer일 때
            for(int i=0;i<count;i++){
                tone(pinBUZZER,1500,80);
                delay(pause);
                noTone(pinBUZZER);
                delay(pause);
            }    
            if(GPIO == 16) digitalWrite(pinBUZZER,Off); //부저가 D0 핀일 때
            break;
    }
}

void _start(int no){
    pinMode(pinBUZZER, OUTPUT);
    pinMode(pinSTART, INPUT_PULLUP);
    Buzzer_OnOff(16, 1, 1, 100); //시작음을 1회 소리내기

    while(!START_KEY){
        pinMode(pinSTART, OUTPUT);
        digitalWrite(pinSTART, LOW); delay(100);
        digitalWrite(pinSTART, HIGH); delay(50);
        //if(Start_Wait==false) break;
        if(no==0) break;
    }

    Buzzer_OnOff(16, 1, 2, 80); //스타트버튼이 눌리면 시작음 2회 소리내기
}
