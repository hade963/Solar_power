#include <Arduino.h>
#include <SoftwareSerial.h>
#include "DeviceState.h"
#include "HomeDevice.h"
#include "SolarPowerSystem.h"

//////////////////////////////////////////////////////////////////
// إعداد الأجهزة المنزلية                                        //
//////////////////////////////////////////////////////////////////
HomeDevice devices[] = {
  // المنفذ، الاستطاعة، استطاعة الإقلاع، أقل زمن تشغيل، أكبر زمن تشغيل، أقل زمن إطفاء، أكبر زمن إطفاء، الأولوية
  HomeDevice(4, 1500.0f, 2000.0f, 3 * HOUR, 5 * HOUR, HOUR, 2 * HOUR, 1),
  HomeDevice(5, 500.0f, 700.0f, 2 * HOUR, 4 * HOUR, 0.3f * HOUR, 2.5f * HOUR, 3),
  HomeDevice(6, 1000.0f, 1200.0f, 0.5f * HOUR, 2 * HOUR, 0.1f * HOUR, HOUR, 2)
};

const int deviceCount = sizeof(devices) / sizeof(HomeDevice);
SolarPowerSystem solarSystem(devices, deviceCount);

//////////////////////////////////////////////////////////////////
// دوال أردوينو الرئيسية                                         //
//////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  while (!Serial); // انتظر حتى يصبح Serial جاهزًا (خاصة لبعض اللوحات)
  Serial.println("Serial Monitor Initialized.");

  // بدء اتصال البلوتوث التسلسلي (بمعدل الباود الافتراضي 9600)
  // تأكد من أن هذا المعدل يطابق إعدادات وحدة البلوتوث
  solarSystem.beginBluetooth();
}

void loop() {
  solarSystem.update();
} 