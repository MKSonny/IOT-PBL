#include <SoftwareSerial.h>
#include "DHT.h"
#include "SNIPE.h"
#include "string.h"

#define DHTTYPE DHT11

#define PING  1
#define PONG  2

#define CODE  PING    /* Please define PING or PONG */

#define TXpin 11
#define RXpin 10
#define ATSerial Serial

// fan
#define FAN_POWER 128

// 사용하는 아날로그핀
#define LIGHT_A2 A2 // 조도 센서
#define SOIL_PIN A3 // 토양 센서

// 사용하는 디지털핀 
#define DHTPIN 3
#define WATER_PIN5 5
#define WATER_PIN6 6
#define LED_PIN 8
#define FAN_PIN9 9
#define FAN_PIN7 7

float fanEdit = 30.0f;
float soilEdit = 30.0f;

// 토양센서 500이 최대

DHT dht(DHTPIN, DHTTYPE);

//16byte hex key
String lora_app_key = "44 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 00";  

SoftwareSerial DebugSerial(RXpin,TXpin);
SNIPE SNIPE(ATSerial);

//팬모터가 x방향으로 회전
void TurnA(int x, int y){
  digitalWrite(x,LOW);
  digitalWrite(y,HIGH);
}

//팬모터가 y방향으로 회전
void TurnB(int x, int y){
  digitalWrite(x,HIGH);
  digitalWrite(y,LOW);
}

void SetUpFan(){
  pinMode(FAN_PIN9, OUTPUT);  // AA를 출력 핀으로 설정
  //pinMode(b, OUTPUT);  // AB를 출력 핀으로 설정
  pinMode(FAN_PIN7, OUTPUT);
}

void SetUpWater() {
  pinMode(WATER_PIN5, OUTPUT);  // AA를 출력 핀으로 설정
  pinMode(WATER_PIN6, OUTPUT);  // AB를 출력 핀으로 설정
}

void WaterStart() {
  digitalWrite(WATER_PIN5, HIGH);  // 정방향으로 모터 회전
  digitalWrite(WATER_PIN6, LOW);
  //water_status = 1;
  delay(2000);  // 5초동안 상태 유지(1000ms = 5s)
}

void WaterStop() {
  digitalWrite(WATER_PIN5, LOW);  // 모터 정지
  digitalWrite(WATER_PIN6, LOW);
  //water_status = 0;
  delay(5000);  // 5초동안 상태 유지(1000ms = 5s)
}

void StartFan() {
  digitalWrite(FAN_PIN9, LOW);
  digitalWrite(FAN_PIN7, HIGH); 
  // fan_status = 1;
}

void StopFan() {
  digitalWrite(FAN_PIN9,HIGH); // 전체 정지
  digitalWrite(FAN_PIN7,HIGH);
  // fan_status = 0;
  //delay(5000);
}

int Photoresistor(){ //조도센서를 통해 값을 받는 함수
  int light_val = analogRead(LIGHT_A2); //아날로그 A0로 값을 받아오고 변수 a에 저장
  return light_val; //조도센서 값 리턴
}

String sendSensorData() {
  
}

bool fan_status = 0;
bool water_status = 0;

void setup() {
  SetUpWater();
  ATSerial.begin(115200); 
  dht.begin();
  SetUpFan();
  StopFan();
  // 온습도 센서 시작
  // dht.begin();

  // FAN 설정하기
  // SetUpFan(LOW);

  // put your setup code here, to run once:
  while(ATSerial.read()>= 0) {}
  while(!ATSerial);

  DebugSerial.begin(115200);

  /* SNIPE LoRa Initialization */
  if (!SNIPE.lora_init()) {
    DebugSerial.println("SNIPE LoRa Initialization Fail!");
    while (1);
  }

  /* SNIPE LoRa Set Appkey */
  if (!SNIPE.lora_setAppKey(lora_app_key)) {
    DebugSerial.println("SNIPE LoRa app key value has not been changed");
  }
  
  /* SNIPE LoRa Set Frequency */
  if (!SNIPE.lora_setFreq(LORA_CH_2)) {
    DebugSerial.println("SNIPE LoRa Frequency value has not been changed");
  }

  /* SNIPE LoRa Set Spreading Factor */
  if (!SNIPE.lora_setSf(LORA_SF_7)) {
    DebugSerial.println("SNIPE LoRa Sf value has not been changed");
  }

  /* SNIPE LoRa Set Rx Timeout 
   * If you select LORA_SF_12, 
   * RX Timout use a value greater than 5000  
  */
  if (!SNIPE.lora_setRxtout(5000)) {
    DebugSerial.println("SNIPE LoRa Rx Timout value has not been changed");
  }  
    
  DebugSerial.println("SNIPE LoRa PingPong Test");
}



void loop() {

  if (soilEdit > 40.0f) {
    WaterStart();
  }
  else {
    WaterStop();
  }
  //delay(500);
  if (fanEdit > 38.0f) {
    StartFan();
  }
  else {
    StopFan();
  }
  //TurnB(FANPIN_8, FANPIN_9);
  //delay(500);

  //StopFan(FANPIN_8, FANPIN_9);
  delay(100);
  
  // 온습도 센서
   float hf = dht.readHumidity();
   float tf = dht.readTemperature();//읽은 온습도값

  String h = String(hf); // 습도
  String t = String(tf); // 온도

  int soil = analogRead(SOIL_PIN);
  // if (soil > 1000) {
  //   WaterStart();
  // }
  // else {
  //   WaterStop();
  // }
  
#if CODE == PING
      //L은 조도센서를 의미
      String light = 'L' + String(Photoresistor());
      // 온도
      String temp = 'T' + String(tf); //+ "aa";
      // 습도
      String humid = 'H' + String(hf); //+ "bb";
      // 토양 습도
      String soil_str = 'S' + String(soil);
      // 팬이 켜졌는지
      String fan_info = 'F' + String(fan_status);

      String msg = "2," + soil_str + ',' + light + ',' + humid + ',' + temp;

        if (SNIPE.lora_send(msg))
        {
           DebugSerial.println("send success");
           DebugSerial.println("msg => " + msg);
          
          //String ver = SNIPE.lora_recv();
          //DebugSerial.println(ver);

        /*
          if (ver.charAt(0) == 'S')
          {
            DebugSerial.println("recv success");
            //DebugSerial.println(SNIPE.lora_getRssi());
            //DebugSerial.println(SNIPE.lora_getSnr());            
          }
          else
          {
            DebugSerial.println("recv fail");
            delay(500);
          }
        */
        }
       delay(100);     
       
       String ver = SNIPE.lora_recv();
        String tempEdit = ver.substring(1, 3);
        String waterEdit = ver.substring(5, 7);
        //T30,S30
        //String[] ver_arry = ver.split(,); //1218

        // T30,S30
        delay(100);

        DebugSerial.println(ver);
        char testing1 = ver[0];
        char testing2 = ver[4];

        if (testing1 == 'T') {
          fanEdit = tempEdit.toFloat();
          DebugSerial.println(testing1);
        }

        if (testing2 == 'S') {
          soilEdit = waterEdit.toFloat();
          DebugSerial.println(testing2);
        }  
       
#elif CODE == PONG
        String ver = SNIPE.lora_recv();
        String tempEdit = myString.substring(1, 3);
        String waterEdit = myString.substring(5, 7);
        //T30,S30
        //String[] ver_arry = ver.split(,); //1218

        // T30,S30
        delay(100);

        DebugSerial.println(ver);
        String testing1 = ver[0];
        String testing2 = ver[4];

        if (testing1.equals('T')) {
          fanEdit = tempEdit;
        }

        if (testing2.equals('S')) {
          soilEdit = waterEdit;
        }
        
        // 읽은 데이터가 조도센서라면
        // 조도센서인지 다른 센서인지 구분하는 법은 앞의 첫 영문자로
        if (ver.charAt(0) == 'L' )
        {
          DebugSerial.println("reading light sensor");
          //DebugSerial.println(SNIPE.lora_getRssi());
          //DebugSerial.println(SNIPE.lora_getSnr());

          if(SNIPE.lora_send("S")) // 여기서 호연님이 lora한테 보내지 않고 wifi로 firebase 서버로 전송
          // 값을 그대로 복사해서 전송하면 될 듯, 아니면 영문자는 빼고 숫자만 전송할 수도 있습니다.
          {
            DebugSerial.println("send success");
          }
          else
          {
            DebugSerial.println("send fail");
            delay(500);
          }
        }
       delay(100);
      
#endif
}
