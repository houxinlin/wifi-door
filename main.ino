#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Servo.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#define LED_PIN_NUMBER 2
#define AP_SSID "103门禁"
#define AP_PSSWORD "103abcabc.."
#define SERVO_PIN_NUMBER 14
#define OPEN_VALUE 0
#define CLOSE_VALUE 150
#define BUTTON_PIN 12


bool clickButton = false;
int state = 1;
String SERVER_ADDRESS = "http://houxinlin.com:8082/get/status";


const uint32_t connectTimeoutMs = 5000;
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;
int isStart = 0;
Servo systemServo;
ESP8266WiFiMulti wifiMulti;

ESP8266WebServer server(80);

void responseIndex() {
  server.send(200, "text/html", "<!DOCTYPE html><html lang='en'><head> <meta charset='UTF-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'> <meta name='viewport' content='width=device-width, initial-scale=1.0'><title>103门禁</title></head><body> <a href='/open'>开门</a> <a href='/close'>关门</a></body></html>");
}
void httpOpen() {
  servoRun(OPEN_VALUE);
  server.send(200, "text/html", "ok");

}
void httpGetState() {
  server.send(200, "text/paint", String(state) );
}
void httpClose() {
  servoRun(CLOSE_VALUE);
  server.send(200, "text/html", "ok");
}
void httpRestart() {
  ESP.restart();
  server.send(200, "text/html", "ok");
}
void setup() {
  Serial.begin(115200);
  Serial.println("");
  pinMode(LED_PIN_NUMBER, OUTPUT);
  systemServo.attach(SERVO_PIN_NUMBER);
  wifiMulti.addAP("HXL", "hxl495594");
  wifiMulti.addAP("扫黄重案组", "abc103103!");
  server.on("/", responseIndex);
  server.on("/open", httpOpen);
  server.on("/close", httpClose);
  server.on("/getState", httpGetState);
  server.begin();
  WiFi.softAP(AP_SSID, AP_PSSWORD);
  Serial.println("start.....");
  IPAddress myIP = WiFi.softAPIP();
  Serial.println(myIP);
  servoRun(OPEN_VALUE);

}
void servoRun(int value) {
  Serial.println(value);
  systemServo.write(value);
}
void handlerCode(String code) {
  if (code == "0") {
    if (state == 0) return;
    Serial.println("close");
    state = 0;
    digitalWrite(LED_PIN_NUMBER, HIGH);
    servoRun(CLOSE_VALUE);
  }

  if (code == "1") {
    if (state == 1) return;
    Serial.println("open");
    state = 1;
    digitalWrite(LED_PIN_NUMBER, LOW);
    servoRun(OPEN_VALUE);
  }
  if (code == "3") {
    ESP.restart();
  }
}
void loop() {
  int val = digitalRead(BUTTON_PIN);   // 读取输入引脚

  if (val == HIGH && (!clickButton)) {
    clickButton = true;
  }
  if (val == LOW && clickButton) {
    clickButton = false;
    //1.开门状态 按下键
    if ( systemServo.read() == OPEN_VALUE ) {
      servoRun(CLOSE_VALUE);
    } else if (systemServo.read() == CLOSE_VALUE) { //2开门状态，已经关了门
      servoRun(OPEN_VALUE);
    } else if ( systemServo.read() == CLOSE_VALUE) { //3.关门状态，安下键盘
      servoRun(OPEN_VALUE);
    } else if ( systemServo.read() == OPEN_VALUE) {  //3.关门状态，已经被按过了
      servoRun(CLOSE_VALUE);
    } else {
      servoRun(OPEN_VALUE);
    }

  }

  server.handleClient();
  if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
    digitalWrite(LED_PIN_NUMBER, HIGH);
    if ((millis() - lastTime) > timerDelay) {
      Serial.println(WiFi.SSID());
      if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;
        String serverPath = SERVER_ADDRESS;

        http.begin(client, serverPath.c_str());
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
          String payload = http.getString();
          handlerCode(payload);
        }
        else {

        }
        http.end();
      }
      else {

      }
      lastTime = millis();
    }
  } else {
    digitalWrite(LED_PIN_NUMBER, LOW);
  }


}
