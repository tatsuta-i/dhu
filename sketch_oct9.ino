#include <Wire.h>
#include <ESP32Servo.h>
#include <ESP32Encoder.h>
#include <string.h>
#include <stdlib.h>
#define OLED_ADRS 0x3C //SA0=L(SA0=H の場合は 0x3D)
#define WAIT_TIM 100 //100msウェイト

////////////////////文字列////////////////
char text1[20];//1行目の文字列
char text2[20];//2行目の文字列
String gettext;
String send_string;
char music_list_1[20] = "music1           ";
char music_list_2[20] = "music2           ";
char music_list_3[20] = "music3           ";
char video_list_1[20] = "video1           ";
char video_list_2[20] = "video2           ";
char video_list_3[20] = "video3           ";
char music_title[20] = "title";
////////////////////数字////////////////
int flag = 0; //loop内の分岐を決める
int idx;//好きに使っていいやつ
////////////////////３色LED(時間)//////////////////////
const int PIN_led_time_g = 15;  //緑
const int PIN_led_time_b = 2;  //青
const int PIN_led_time_r = 0;  //赤
////////////////////３色LED(電源)//////////////////////
const int PIN_led_power_g = 4;  //緑
const int PIN_led_power_b = 16;  //青
const int PIN_led_power_r = 17;  //赤
////////////////////サーボ/////////////////////////////
const int PIN_servo = 5;
Servo myServo;
////////////////////OLED//////////////////////////////
int DisplayON = 0x0F, ClearDisplay = 0x01, ReturnHome = 0x02;
const int PIN_OLED_SCL = 22;
const int PIN_OLED_SDA = 21;
////////////////////タクトスイッチ//////////////////////////////
const int PIN_tact = 23;
int flag_tact = 0;
  void IRAM_ATTR btnPushed() {
    flag_tact++;
  }
////////////////////ロータリーエンコーダー////////////////
const int PIN_rotaryencoder_A = 13;  // ロータリーエンコーダーA入力
const int PIN_rotaryencoder_B = 12;  // ロータリーエンコーダーB入力
const int ROTARYMIN = 5;    // ロータリーエンコーダー最小値
const int ROTARYMAX = 90;  // ロータリーエンコーダー最大値
int newPos = 10;
int lastPos = 10;       // ロータリーエンコーダー前回値
int encoderCount = 10;   // ロータリーエンコーダー現在値
ESP32Encoder REncoder;
////////////////////通信用////////////////
int ModeCode = -1;
int ListCode = -1;
int TimeCode = -1;
int SendCode[3] = {-1,-1,-1};
int array_length = sizeof(SendCode) / sizeof(SendCode[0]);
/////////////////////////////////////////////////////

void setup() {
  flag = 0;
  /////////////////////////////////////////////////////
  //E2 通信確立
  Serial.begin(115200);//115200bpsでシリアル通信を初期化する
  ////////////////////３色LED(時間)//////////////////////
  pinMode(PIN_led_power_r, OUTPUT_OPEN_DRAIN);
  pinMode(PIN_led_power_g, OUTPUT_OPEN_DRAIN);
  pinMode(PIN_led_power_b, OUTPUT_OPEN_DRAIN);
  digitalWrite(PIN_led_power_r, HIGH);//😎
  digitalWrite(PIN_led_power_g, HIGH);//🫠
  digitalWrite(PIN_led_power_b, HIGH);//🫠
  ////////////////////３色LED(電源)//////////////////////
  pinMode(PIN_led_time_r, OUTPUT_OPEN_DRAIN);
  pinMode(PIN_led_time_g, OUTPUT_OPEN_DRAIN);
  pinMode(PIN_led_time_b, OUTPUT_OPEN_DRAIN);
  digitalWrite(PIN_led_power_r, HIGH);//😎
  digitalWrite(PIN_led_power_g, HIGH);//🫠
  digitalWrite(PIN_led_power_b, HIGH);//🫠
  ////////////////////サーボ/////////////////////////////
  myServo.attach(PIN_servo,510,2400);
  ////////////////////ロータリーエンコーダー////////////////
  // Enable the weak pull up resistors
  ESP32Encoder::useInternalWeakPullResistors=UP;
  // use pin
  REncoder.attachSingleEdge(PIN_rotaryencoder_A, PIN_rotaryencoder_B);
  // set starting count value after attaching
  REncoder.setCount(10);
  ////////////////////タクトスイッチ//////////////////////////////
  pinMode(PIN_tact, INPUT_PULLUP);
  attachInterrupt(PIN_tact, btnPushed, FALLING);
  ////////////////////OLED//////////////////////////////
  Wire.begin(); //Wire ライブラリを初期化し、I2C マスタとしてバスに接続
  init_oled(); 
  delay(100);
}

void loop() {
  switch(flag){
    case 0://E1 起動動作
      digitalWrite(PIN_led_power_r, LOW);//😎
      digitalWrite(PIN_led_power_g, HIGH);//🫠
      digitalWrite(PIN_led_power_b, HIGH);//🫠
      myServo.write(0);//回転角を指定
      delay(500);
      strcpy(text1,"NOW LOADING        ");
      strcpy(text2,"...                ");
      writeCommand(0x02);
      for(idx = 0; idx < 20; idx++) { 
      writeData(text1[idx]);
      }
      writeCommand(0x20+0x80); //2行目の先頭
      for(idx = 0; idx < 20; idx++) { 
        writeData(text2[idx]);
      }
      contrast_max(); //輝度を最大に設定 
      for(idx=0;idx<=180;idx+=1){
        switch(idx%3){
          case 0:
            digitalWrite(PIN_led_power_r, HIGH);
            digitalWrite(PIN_led_power_g, LOW);
            digitalWrite(PIN_led_power_b, HIGH);
          break;
          case 1:
            digitalWrite(PIN_led_power_r, HIGH);
            digitalWrite(PIN_led_power_g, HIGH);
            digitalWrite(PIN_led_power_b, LOW);
          break;
          case 2:
            digitalWrite(PIN_led_power_r, LOW);
            digitalWrite(PIN_led_power_g, HIGH);
            digitalWrite(PIN_led_power_b, HIGH);
          break;
        }
        myServo.write(idx); //回転角を指定
        delay(WAIT_TIM); //回転角を指定するまでの待機時間
      }
      digitalWrite(PIN_led_power_r, HIGH);
      digitalWrite(PIN_led_power_g, LOW);
      digitalWrite(PIN_led_power_b, HIGH);

      delay(1000);
      writeCommand(ClearDisplay); // Clear Display 
      flag = 1;
      break;
///////////////////////////////////////////////////////////////////////////////////////////////////
    case 1://E3再生リスト名取得
    //凍結
    flag = 2;
    //delay(1000);
      break;
///////////////////////////////////////////////////////////////////////////////////////////////////
    case 2://E4再生/更新
      strcpy(text1,"1.PLAY 2.UPDATE    ");
      writeCommand(0x02);
      for(idx = 0; idx < 20; idx++) { 
        writeData(text1[idx]);
      }
      writeCommand(0x20+0x80); //2行目の先頭
      newPos = REncoder.getCount();
      if(newPos - lastPos > 0){
        strcpy(text1,"->1.PLAY         ");
        for(idx = 0; idx < 20; idx++) { 
          writeData(text1[idx]);
        }
        ModeCode = 3;
      }
      if(newPos - lastPos < 0){
        strcpy(text1,"->2.UPDATE       ");
        for(idx = 0; idx < 20; idx++) { 
          writeData(text1[idx]);
        }
        ModeCode = 1;
      }
      if(flag_tact != 0){
        writeCommand(ClearDisplay); // Clear Display 
        encoderCount = 10;
        if(ModeCode == 1){
          TimeCode = 0;
          flag = 3;
        }
        if(ModeCode == 3){
          flag = 4;
        }
        flag_tact =0;
        delay(1000);
      }
      delay(100);
      break;
///////////////////////////////////////////////////////////////////////////////////////////////////
    case 3://E4更新-音楽/動画
      strcpy(text1,"1.MUSIC 2.VIDEO    ");
      writeCommand(0x02);
      for(idx = 0; idx < 20; idx++) { 
        writeData(text1[idx]);
      }
      writeCommand(0x20+0x80); //2行目の先頭
      newPos = REncoder.getCount();
      if(newPos - lastPos > 0){
        strcpy(text1,"->1.MUSIC         ");
        for(idx = 0; idx < 20; idx++) { 
          writeData(text1[idx]);
        }
        ModeCode = 1;
      }
      if(newPos - lastPos < 0){
        strcpy(text1,"->2.VIDEO         ");
        for(idx = 0; idx < 20; idx++) { 
          writeData(text1[idx]);
        }
        ModeCode = 2;
      }
      if(flag_tact != 0){
        writeCommand(ClearDisplay); // Clear Display 
        encoderCount = 10;
        if(ModeCode == 1){
          flag = 6;
        }
        if(ModeCode == 2){
          flag = 7;
        }
        flag_tact =0;
        delay(1000);
      }
      delay(100);
      break;
///////////////////////////////////////////////////////////////////////////////////////////////////
    case 4://E4再生-音楽/動画
      strcpy(text1,"1.MUSIC 2.VIDEO    ");
      writeCommand(0x02);
      for(idx = 0; idx < 20; idx++) { 
        writeData(text1[idx]);
      }
      writeCommand(0x20+0x80); //2行目の先頭
      newPos = REncoder.getCount();
      if(newPos - lastPos > 0){
        strcpy(text1,"->1.MUSIC         ");
        for(idx = 0; idx < 20; idx++) { 
          writeData(text1[idx]);
        }
        ModeCode = 3;
      }
      if(newPos - lastPos < 0){
        strcpy(text1,"->2.VIDEO         ");
        for(idx = 0; idx < 20; idx++) { 
          writeData(text1[idx]);
        }
        ModeCode = 4;
      }
      if(flag_tact != 0){
        writeCommand(ClearDisplay); // Clear Display 
        encoderCount = 10;
        flag = 5;
        flag_tact =0;
        delay(1000);
      }
      delay(100);
      break;
///////////////////////////////////////////////////////////////////////////////////////////////////
    case 5://E4時間入力
      strcpy(text1,"SET TIME          ");
      writeCommand(0x02);
      for(idx = 0; idx < 20; idx++) { 
        writeData(text1[idx]);
      }
      writeCommand(0x20+0x80); //2行目の先頭
      newPos = REncoder.getCount();
      if (lastPos != newPos){
        if(newPos - lastPos > 0){
          encoderCount--;
        }
        if(newPos - lastPos < 0){
          encoderCount++;
        }
        if(encoderCount > ROTARYMAX){
          encoderCount = ROTARYMAX;
        }
        if(encoderCount < ROTARYMIN){
          encoderCount = ROTARYMIN;
        }
        lastPos = newPos;
      }
      sprintf(text2, "Time[min] = %d      ", encoderCount);
      for(idx = 0; idx < 20; idx++) { 
        writeData(text2[idx]);
      }
      contrast_max(); //輝度を最大に設定 
      if(flag_tact != 0){
        TimeCode = encoderCount*60;
        encoderCount = 10;
        if(ModeCode == 3){
          flag = 6;
        }
        if(ModeCode == 4){
          flag = 7;
        }
        flag_tact =0;
        delay(1000);
      }
      delay(100);
      break;
///////////////////////////////////////////////////////////////////////////////////////////////////
    case 6://音楽-再生リスト入力
      strcpy(text1,"SELECT LIST       ");
      writeCommand(0x02);
      for(idx = 0; idx < 20; idx++) { 
        writeData(text1[idx]);
      }
      writeCommand(0x20+0x80); //2行目の先頭
      newPos = REncoder.getCount();
      if (lastPos != newPos){
        if(newPos - lastPos > 0){
          encoderCount++;
        }
        if(newPos - lastPos < 0){
          encoderCount--;
        }
        if(encoderCount > 11){
          encoderCount = 11;
        }
        if(encoderCount < 9){
          encoderCount = 9;
        }
        lastPos = newPos;
      }

      if(encoderCount>10){
        for(idx = 0; idx < 20; idx++) { 
          writeData(music_list_1[idx]);
        }
        ListCode = 11;
      }
      if(encoderCount==10){
        for(idx = 0; idx < 20; idx++) { 
          writeData(music_list_2[idx]);
        }
        ListCode = 12;
      }
      if(encoderCount<10){
        for(idx = 0; idx < 20; idx++) { 
          writeData(music_list_3[idx]);
        }
        ListCode = 13;
      }
      
      if(flag_tact != 0){
        writeCommand(ClearDisplay); // Clear Display 
        encoderCount = 10;
        flag = 8;
        flag_tact =0;
        delay(1000);
      }
      delay(100);
      break;
///////////////////////////////////////////////////////////////////////////////////////////////////
    case 7://動画-再生リスト入力
      strcpy(text1,"SELECT LIST       ");
      writeCommand(0x02);
      for(idx = 0; idx < 20; idx++) { 
        writeData(text1[idx]);
      }
      writeCommand(0x20+0x80); //2行目の先頭
      newPos = REncoder.getCount();
      if (lastPos != newPos){
        if(newPos - lastPos > 0){
          encoderCount++;
        }
        if(newPos - lastPos < 0){
          encoderCount--;
        }
        if(encoderCount > 11){
          encoderCount = 11;
        }
        if(encoderCount < 9){
          encoderCount = 9;
        }
        lastPos = newPos;
      }

      if(encoderCount>10){
        for(idx = 0; idx < 20; idx++) { 
          writeData(video_list_1[idx]);
        }
        ListCode = 21;
      }
      if(encoderCount==10){
        for(idx = 0; idx < 20; idx++) { 
          writeData(video_list_2[idx]);
        }
        ListCode = 22;
      }
      if(encoderCount<10){
        for(idx = 0; idx < 20; idx++) { 
          writeData(video_list_3[idx]);
        }
        ListCode = 23;
      }
      
      if(flag_tact != 0){
        writeCommand(ClearDisplay); // Clear Display 
        encoderCount = 10;
        flag = 8;
        flag_tact =0;
        delay(1000);
      }
      delay(100);
      break;
///////////////////////////////////////////////////////////////////////////////////////////////////
    case 8://E5送信
      send_string = intArrayToCSV(SendCode, array_length);
      Serial.println("sendCode:" + send_string);
      flag = 9;//後で変える
      delay(1000);
      break;
///////////////////////////////////////////////////////////////////////////////////////////////////
    case 9://E6リスト作成通知受け取り
      // Check if there is incoming data: 
      if (Serial.available() > 0) { 
      // Read the incoming data as a string: 
      gettext =Serial.readString(); 
      }
      if(gettext == "Listing_completed."){
        if(ModeCode != 3){
          flag = 11;
        }else{
          flag = 10;
        }
      }
      delay(1000);
      break;
///////////////////////////////////////////////////////////////////////////////////////////////////
    case 10://E7再生中
    ;
      break;
///////////////////////////////////////////////////////////////////////////////////////////////////
    case 11://E8終了動作
      digitalWrite(PIN_led_power_r, HIGH);
      digitalWrite(PIN_led_power_g, LOW);
      digitalWrite(PIN_led_power_b, HIGH);
      myServo.write(180);//回転角を指定
      delay(500);
      strcpy(text1,"NOW ENDING         ");
      strcpy(text2,"...                ");
      writeCommand(0x02);
      for(idx = 0; idx < 20; idx++) { 
      writeData(text1[idx]);
      }
      writeCommand(0x20+0x80); //2行目の先頭
      for(idx = 0; idx < 20; idx++) { 
        writeData(text2[idx]);
      }
      contrast_max(); //輝度を最大に設定 
      for(idx=180;idx>=0;idx-=1){
        switch(idx%3){
          case 0:
            digitalWrite(PIN_led_power_r, HIGH);
            digitalWrite(PIN_led_power_g, LOW);
            digitalWrite(PIN_led_power_b, HIGH);
          break;
          case 1:
            digitalWrite(PIN_led_power_r, HIGH);
            digitalWrite(PIN_led_power_g, HIGH);
            digitalWrite(PIN_led_power_b, LOW);
          break;
          case 2:
            digitalWrite(PIN_led_power_r, LOW);
            digitalWrite(PIN_led_power_g, HIGH);
            digitalWrite(PIN_led_power_b, HIGH);
          break;
        }
        myServo.write(idx); //回転角を指定
        delay(WAIT_TIM); //回転角を指定するまでの待機時間
      }
      digitalWrite(PIN_led_power_r, LOW);
      digitalWrite(PIN_led_power_g, HIGH);
      digitalWrite(PIN_led_power_b, HIGH);

      writeCommand(ClearDisplay); // Clear Display 
      Serial.println("shutdown");
      delay(10000);
      break;
  }
}
//----main end----
////////////////////OLEDの関数//////////////////////////////
void writeData(byte t_data) {
  Wire.beginTransmission(OLED_ADRS); 
  Wire.write(0x40);
  Wire.write(t_data);
  Wire.endTransmission();
  delay(1); 
}

void writeCommand(byte t_command) {
  Wire.beginTransmission(OLED_ADRS);
 Wire.write(0x00);
 Wire.write(t_command); 
 Wire.endTransmission();
  delay(10); 
}

void contrast_max(){ 
  writeCommand(0x2a);//RE=1 
  writeCommand(0x79);//SD=1 
  writeCommand(0x81);//コントラストセット 
  writeCommand(0xFF);//輝度MAX 
  writeCommand(0x78);//SD を0にもどす 
  writeCommand(0x28); //2C=高文字 28=ノーマル 
  delay(100);
}

void init_oled(){
  delay(100);
  writeCommand(ClearDisplay); // Clear Display 
  delay(20);
  writeCommand(ReturnHome); // ReturnHome
  delay(2);
  writeCommand(DisplayON); // Send Display on command
  delay(2);
  writeCommand(ClearDisplay); // Clear Display 
  delay(20);
}
//通信用
String intArrayToCSV(int array[], int array_length) {
    String csv_string = "";
    for (int i = 0; i < array_length; i++) {
        csv_string += String(array[i]);
        if (i < array_length - 1) { // 最後の要素でない場合、カンマを追加
            csv_string += ",";
        }
    }
    return csv_string;
}