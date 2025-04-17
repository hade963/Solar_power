#ifndef SOLAR_POWER_SYSTEM_H
#define SOLAR_POWER_SYSTEM_H

#include <Arduino.h>
#include "HomeDevice.h"
#include <SoftwareSerial.h>

//////////////////////////////////////////////////////////////////
// كلاس يمثل نظام الطاقة الشمسية                                 //
//////////////////////////////////////////////////////////////////
class SolarPowerSystem {
private:
  // --- تعريفات وحدة البلوتوث ---
  static const byte bluetoothRxPin = 2; // يتصل بـ TXD الخاص بوحدة البلوتوث
  static const byte bluetoothTxPin = 3; // يتصل بـ RXD الخاص بوحدة البلوتوث
  SoftwareSerial bluetoothSerial;      // <-- إضافة جديدة
  
  // ثوابت البطارية
  const float BATTERY_CAPACITY_AMP;  // سعة البطارية بالأمبير
  const float BATTERY_VOLTAGE;       // جهد البطارية بالفولت
  const float BATTERY_FULL;          // الطاقة الكاملة للبطارية
  const float BATTERY_MAX;           // الحد الأقصى للبطارية
  const float BATTERY_MIN;           // الحد الأدنى للبطارية

  // متغيرات مصادر الطاقة
  float solarPanelPower;             // الاستطاعة المقدمة من الألواح 
  float gridPower;                   // الاستطاعة المقدمة من الكهرباء العامة
  float housePower;                  // الاستطاعة المستهلكة في المنزل
  
  // متغيرات البطارية
  float batteryPower;                // الاستطاعة الداخلة أو الخارجة من البطارية
  float battery;                     // سعة البطارية الحالية
  float batteryPercentage;           // نسبة امتلاء البطارية
  
  // متغيرات الطاقة الإضافية
  float excessPower;                 // الطاقة الزائدة التي لم تستخدم
  float shortfallPower;              // الطاقة المطلوبة التي لم تتوفر

  // متغيرات الوقت
  unsigned long previousMillis;
  unsigned long currentMillis;
  unsigned long deltaTime;
  unsigned long printTimer;

  // مصفوفة للأجهزة المنزلية
  HomeDevice* devices;
  int deviceCount;

  // تحديث حالة الأجهزة حسب الطاقة المتوفرة والأولوية
  void updateDevices();
  
  // تحديث حالة البطارية
  void updateBatteryPower(unsigned long deltaTime);
  
  // ترتيب الأجهزة حسب الأولوية
  void sortDevicesByPriority();

public:
  // منشئ الكلاس
  SolarPowerSystem(HomeDevice* deviceArray, int count, 
                  float batteryCapacityAmp = 250.0f, 
                  float batteryVoltage = 24.0f);
  
  // --- إضافة جديدة: دالة لبدء البلوتوث ---
  void beginBluetooth(long baudRate = 9600);
  
  // تحديث حالة النظام
  void update();
  
  // طباعة معلومات النظام
  void printSystemInfo();
  
  // دوال للوصول للمتغيرات وتعديلها
  void setSolarPanelPower(float power);
  void setGridPower(float power);
  float getBatteryPercentage() const;
  float getSolarPanelPower() const;
  float getHousePower() const;
};

#endif // SOLAR_POWER_SYSTEM_H 