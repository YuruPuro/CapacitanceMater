/**
 * CapacitanceMater
 *  ATTiny85 With SSD1306(I2C)
 */
#include "DISP7SEG.h"
DISP7SEG disp ;

const int PULSE_PIN = 3;
const int DIGITAL_READ_PIN = 1;
const int ANALOG_READ_PIN = A2;

double P = 1023.0;
const double E = 3.33 ;       // 電圧値(実測値:3.33V)
const double R = 2000000.0;   // 抵抗値(2MΩ)
const double V = E * 0.6322;  // 63.22%の電圧
const int    EP = P * 0.6322; // 63.22%の電圧のAD読み取り値

// ----------------------
// ----- 静電容量表示 -----
// ----------------------
void dispCapacity(double C,long UNIT) {
                    // 01234567 01234567 01234567 01234567
  int  dispSeg[9] ; // xxxx.xuF xxx.xxuF xx.xxxuF -x.xxxuF 
  if (UNIT > 0) {
    // --- 容量値表示
    for (int i=0;i<9;i++) {
      dispSeg[i] = 416 ;  // 空白で埋める
    }
    dispSeg[7] = 14 ; // F
    switch(UNIT) {
      case 1:dispSeg[6] = 11 ; break ;  // uF
      case 1000:dispSeg[6] = 12 ; break ;  // nF
      case 1000000:dispSeg[6] = 13 ; break ;  // pF
    }

    int range1 = C ;
    int range2 = C ;

    int pos1 = 3 ;
    int pos2 = 5 ;
    int pos2l = 1 ;
    dispSeg[4] = 20 ; // DOT
    if (range1 < 100) {
      pos1 = 1 ;
      pos2 = 3 ;
      pos2l = 3;
      dispSeg[2] = 20 ; // DOT
      range2 = C * 1000 ;
      range2 %= 1000 ;
    }
    for (int i=pos1;i>=0;i--) {
      dispSeg[i] = range1 % 10 ;
      range1 /= 10 ;
      if (range1 == 0) break ;
    }
    for (int i=pos2l-1;i>=0;i--) {
      dispSeg[pos2+i] = range2 % 10 ;
      range2 /= 10 ;
    }
  } else {
    // --- 初期画面 8888.8uF 表示
    for (int i=0;i<8;i++) {
      dispSeg[i] = 22 ;
    }
    dispSeg[3] = 20 ;   // DOT
    dispSeg[6] = 11 ;   // u
    dispSeg[7] = 14  ;  // F
    disp.cls() ;
  }

  // --- 表示
  int x = 0 ;
  for (int i=0;i<8;i++) {
    disp.disp7SEG(x,0,dispSeg[i]) ;
    x += (dispSeg[i] == 20) ? 8 : 16 ; // DOTだけ幅を狭める
  } 
}

// ----------------------
// ----- セットアップ -----
// ----------------------
void setup() {
  Wire.begin();
  disp.init() ;
  dispCapacity(0.0,0) ;
  delay(1000) ;

  pinMode(PULSE_PIN, OUTPUT);
  digitalWrite(PULSE_PIN, LOW);
}

void loop() {
  // ----- 静電容量計測 -----
  // ---  放電 --
  digitalWrite(PULSE_PIN, LOW);
  pinMode(DIGITAL_READ_PIN, OUTPUT);
  digitalWrite(DIGITAL_READ_PIN, LOW);
  delay(3000);  // 100uF位あると3秒位待たないと放電しきれないっぽい。
  pinMode(DIGITAL_READ_PIN, INPUT);
  delay(10);

  // --- 充電時間T計測
  double RV = 0;
  digitalWrite(PULSE_PIN, HIGH);
  unsigned long startTime = micros();
  while (analogRead(ANALOG_READ_PIN) < EP) ;
  long T = micros() - startTime;

  // 単位変換
  long UNIT = 1 ;
  // 小数点以下のゼロの数多すぎなんで桁上げする
  if (T < 2500) {
    UNIT = 1000000 ;  // pF
  } else if (T < 50000) {
    UNIT = 1000 ;     // nF
  } else {
    UNIT = 1 ;        // uF
  }
  double C = T / R * UNIT ;

  // --- 表示 ---
  dispCapacity(C,UNIT) ;

  // ---  放電 --
  digitalWrite(PULSE_PIN, LOW);
  pinMode(DIGITAL_READ_PIN, OUTPUT);
  digitalWrite(DIGITAL_READ_PIN, LOW);
  delay(3000);  // 100uF位あると3秒位待たないと放電しきれないっぽい。
  pinMode(DIGITAL_READ_PIN, INPUT);
  delay(10);

  // ----- 計測間隔
  delay(1000) ;
}
