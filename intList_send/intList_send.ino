int modeCode = 1;
int listCode = 2;
int timeCode = 3;
int sendCode[3] = {modeCode,listCode,timeCode};
int array_length = sizeof(sendCode) / sizeof(sendCode[0]);

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

void setup() {
  // Initialize serial communication at 115200 bps:
  Serial.begin(115200);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(3000);// 3 second
  String csv_string = intArrayToCSV(sendCode, array_length);
  Serial.println("sendCode:" + csv_string);
  while (Serial.available() > 0) {
    String incomingData = Serial.readString();
    Serial.println("Received by ESP32:" + incomingData);
  }
}
