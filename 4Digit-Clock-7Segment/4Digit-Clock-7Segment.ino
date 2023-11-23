/*  Pinout
 *  2 = Dot Led
 *  3 = Latch
 *  4 = Data
 *  5 = CLK
 */
#include <Wire.h>
#include <RtcDS3231.h>

/* 7Segment Program */
const int pin_dot = 2;
const int pin_latch = 3;
const int pin_data = 4;
const int pin_clock = 5;

/*
 *     5
 *    _ _
 * 7 | 8 | 6
 *    - -
 * 3 |_ _| 1 .4
 *     2
 */
 
/* Reversed
 *     2
 *    _ _
 * 1 | 8 | 3
 *    - -
 * 6 |_ _| 7 .4?
 *     5
 */

 const byte chars[] = {
  0b00010001, // 0
  0b01111011, // 1
  0b10010010, // 2
  0b00110010, // 3
  0b01111000, // 4
  0b00110100, // 5
  0b00010100, // 6
  0b01110011, // 7
  0b00010000, // 8
  0b00110000, // 9
  0b11110000, // °
  0b10010101, // C
 };
 const byte chars_rv[] = {
  0b00010001, // 0
  0b11011101, // 1
  0b10010010, // 2
  0b10010100, // 3
  0b01011100, // 4
  0b00110100, // 5
  0b00110000, // 6
  0b10011101, // 7
  0b00010000, // 8
  0b00010100, // 9
  0b00001110, // °
  0b00110010, // C
 };
 enum CHARA {
  deg= 10,
  C
 };

/* RTC Program*/
RtcDS3231<TwoWire> Rtc(Wire);

bool rtc_wasError(const char *errorTopic = "") {
  byte error = Rtc.LastError();
  if (error) {
    Serial.print("[");
    Serial.print(errorTopic);
    Serial.print("] Wire communication error(");
    Serial.print(error);
    Serial.print(") : ");

    switch (error) {
      case Rtc_Wire_Error_None:
          Serial.println("(none?!)");
          break;
      case Rtc_Wire_Error_TxBufferOverflow:
          Serial.println("transmit buffer overflow");
          break;
      case Rtc_Wire_Error_NoAddressableDevice:
          Serial.println("no device responded");
          break;
      case Rtc_Wire_Error_UnsupportedRequest:
          Serial.println("device doesn't support request");
          break;
      case Rtc_Wire_Error_Unspecific:
          Serial.println("unspecified error");
          break;
      case Rtc_Wire_Error_CommunicationTimeout:
          Serial.println("communications timed out");
          break;
    }
    return true;
  }

  return false;
}
/* Display Program */
void display_jam() {
  static unsigned long lastM_blink = 0;
  static bool blinking = 0;
  if (millis() - lastM_blink >= 1000) {
    lastM_blink = millis();
    digitalWrite(pin_dot, blinking = !blinking); 
  }
  
  RtcDateTime now = Rtc.GetDateTime();
  int jam1,jam2, menit1,menit2;
  if (now.Hour() < 10) {
    jam1 = 0;
    jam2 = now.Hour();
  } else {
    jam1 = now.Hour() / 10;
    jam2 = now.Hour() - (jam1 * 10);
  }
  if (now.Minute() < 10) {
    menit1 = 0;
    menit2 = now.Minute();
  } else {
    menit1 = now.Minute() / 10;
    menit2 = now.Minute() - (menit1 * 10);
  }

  digitalWrite(pin_latch, 0);
  shiftOut(pin_data, pin_clock, MSBFIRST, chars[jam1]);
  shiftOut(pin_data, pin_clock, MSBFIRST, chars[jam2]);
  shiftOut(pin_data, pin_clock, MSBFIRST, chars_rv[menit1]);
  shiftOut(pin_data, pin_clock, MSBFIRST, chars[menit2]);
  digitalWrite(pin_latch, 1);
}

void display_suhu() {
  RtcTemperature get_temp = Rtc.GetTemperature();
  const int temp = get_temp.AsFloatDegC();
  const int temp1 = (temp < 10) ? 0 : temp / 10;
  const int temp2 = (temp < 10) ? temp : temp - (temp1 * 10);
  
  digitalWrite(pin_latch, 0);
  shiftOut(pin_data, pin_clock, MSBFIRST, chars[temp1]);
  shiftOut(pin_data, pin_clock, MSBFIRST, chars[temp2]);
  shiftOut(pin_data, pin_clock, MSBFIRST, chars_rv[CHARA::deg]);
  shiftOut(pin_data, pin_clock, MSBFIRST, chars[CHARA::C]);
  digitalWrite(pin_latch, 1);
}

/* Global Program */
const int pin_output_length = 4;
const int *pin_output[] = {
  &pin_dot,
  &pin_latch,
  &pin_clock,
  &pin_data
};
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  for (int i=0; i < pin_output_length; i++) {
    pinMode( *pin_output[i], OUTPUT );
  }

  Rtc.Begin();
  if (!Rtc.GetIsRunning()) {
    if (!rtc_wasError("setup GetIsRunning")) {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
    }
  }
  Rtc.Enable32kHzPin(false);
  rtc_wasError("setup Enable32kHzPin");
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
  rtc_wasError("setup SetSquareWavePin");
}

void loop() {
  // put your main code here, to run repeatedly:
  RtcDateTime now = Rtc.GetDateTime();
  const int second = now.Second();
  if (second > 40) display_jam();
  else if (second > 30) display_suhu();
  else if (second > 10) display_jam(); 
  else display_suhu();

}
