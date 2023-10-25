/****
 * 타이머 파일
 */

#include <elapsedMillis.h>  //타이머 관련 소스 파일

elapsedMillis Timer_TempHumid;          //온습도 측정 타이머
elapsedMillis Timer_Air;                //공기 질 측정 타이머
elapsedMillis Timer_ButtonPress;        //버튼 눌린 타이머
elapsedMillis Timer_LCDchange;          //LCD 갱신 타이머
elapsedMillis Timer_Human;              //인체감지 타이머 1(인체감지 측정 타이머)
elapsedMillis Timer_HumanDetect;        //인체감지 타이머 2(미감지 스톱워치)
elapsedMillis Timer_Htime;              //인체감지 스톱워치 타이머

#define TempHumidInterval 5000            //온습도 측정 주기(5초)
#define AirInterval 1000                  //공기 질 측정 주기(1초)
#define LCDinterval 1000                  //LCD 갱신 주기(1초)
#define HumanInterval 100                //인체감지 측정 주기(1초)
#define HtimeInterval 1000                 //인체감지 스톱워치 계산 주기(1초)
