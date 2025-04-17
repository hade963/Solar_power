#include "SolarPowerSystem.h"
#include "DeviceState.h"
// منشئ الكلاس
SolarPowerSystem::SolarPowerSystem(HomeDevice* deviceArray, int count, 
                                 float batteryCapacityAmp, 
                                 float batteryVoltage) : 
  bluetoothSerial(bluetoothRxPin, bluetoothTxPin),
  BATTERY_CAPACITY_AMP(batteryCapacityAmp),
  BATTERY_VOLTAGE(batteryVoltage),
  BATTERY_FULL(BATTERY_CAPACITY_AMP * BATTERY_VOLTAGE),
  BATTERY_MAX(BATTERY_FULL * 0.8f),
  BATTERY_MIN(BATTERY_FULL * 0.2f),
  solarPanelPower(2000.0f),
  gridPower(0.0f),
  housePower(0.0f),
  batteryPower(0.0f),
  battery(BATTERY_FULL * 0.5f),
  batteryPercentage(50.0f),
  excessPower(0.0f),
  shortfallPower(0.0f),
  previousMillis(0),
  currentMillis(0),
  deltaTime(0),
  printTimer(0)
{
  devices = deviceArray;
  deviceCount = count;
  previousMillis = millis();
  
  // ترتيب الأجهزة حسب الأولوية
  sortDevicesByPriority();
  
  Serial.println("نظام إدارة الطاقة الشمسية المنزلية");
  Serial.println("=================================");
}

// تحديث حالة النظام
void SolarPowerSystem::update() {
  // قراءة استطاعة الألواح الشمسية
  if (Serial.available() > 0) {
    String inputString = Serial.readStringUntil('\n'); // قراءة الإدخال كسلسلة نصية حتى السطر الجديد
    inputString.trim(); // إزالة أي مسافات بيضاء بادئة أو لاحقة

    // محاولة تحويل السلسلة إلى float
    // نستخدم متغير مؤقت للتحقق قبل التحديث الفعلي
    float tempPower = inputString.toFloat();

    // التحقق إذا كان التحويل ناجحًا
    // toFloat() تعيد 0.0 إذا فشل التحويل أو كان الإدخال "0" أو "0.0"
    // نحتاج إلى التحقق من أن السلسلة ليست صفرًا بالفعل إذا كانت النتيجة 0.0
    if (tempPower != 0.0 || inputString.equals("0") || inputString.equals("0.0")) {
        solarPanelPower = tempPower; // تحديث القيمة فقط إذا كان الإدخال رقمًا صالحًا
    } else {
      // تجاهل الإدخال غير الرقمي (لا يتم فعل شيء)
      // يمكنك إضافة طباعة رسالة خطأ هنا إذا أردت
      // Serial.println("Error: Invalid input received.");
    }

    // لا حاجة لتنظيف المخزن المؤقت بشكل منفصل هنا لأن readStringUntil() قامت بذلك
  }

  // حساب الوقت المنقضي
  currentMillis = millis();
  deltaTime = currentMillis - previousMillis;
  previousMillis = currentMillis;
  printTimer += deltaTime;

  // تحديث حالة الأجهزة
  updateDevices();
  
  // تطبيق حالة الأجهزة على المنافذ
  for (int i = 0; i < deviceCount; i++) {
    devices[i].run();
    devices[i].addTime(deltaTime);
  }

  // حساب طاقة البطارية
  updateBatteryPower(deltaTime);
  
  // طباعة معلومات النظام كل 5 ثواني
  if (printTimer >= 5000) {
    printSystemInfo();
    printTimer = 0;
  }
}

// تحديث حالة الأجهزة حسب الطاقة المتوفرة والأولوية
void SolarPowerSystem::updateDevices() {
  // حساب الطاقة المتوفرة (الألواح + الشبكة)
  float availablePower = solarPanelPower + gridPower;
  
  // إعادة حساب الطاقة المستهلكة
  housePower = 0;
  for (int i = 0; i < deviceCount; i++) {
    if (devices[i].getState() == ON) {
      housePower += devices[i].getPower();
    }
  }
  
  // تشغيل الأجهزة المطفأة حسب الأولوية
  for (int i = 0; i < deviceCount; i++) {
    devices[i].updateIfOff(availablePower, housePower);
  }
  
  // إطفاء الأجهزة العاملة بعكس ترتيب الأولوية
  for (int i = deviceCount - 1; i >= 0; i--) {
    devices[i].updateIfOn(availablePower, housePower);
  }
}

// تحديث حالة البطارية
void SolarPowerSystem::updateBatteryPower(unsigned long deltaTime) {
  // حساب قوة البطارية (موجبة عند الشحن، سالبة عند التفريغ)
  batteryPower = solarPanelPower + gridPower - housePower;
  
  // تحويل الطاقة إلى تغير في سعة البطارية
  float powerDelta = batteryPower * deltaTime / HOUR; // التحويل من ميلي ثانية إلى ساعة
  battery += powerDelta;

  // التحقق من حدود البطارية
  if (battery < BATTERY_MIN) {
    shortfallPower += (BATTERY_MIN - battery);
    battery = BATTERY_MIN;
  } else if (battery > BATTERY_MAX) {
    excessPower += (battery - BATTERY_MAX);
    battery = BATTERY_MAX;
  }
  
  // حساب نسبة البطارية
  batteryPercentage = (battery / BATTERY_FULL) * 100.0f;
}

// ترتيب الأجهزة حسب الأولوية
void SolarPowerSystem::sortDevicesByPriority() {
  for (int i = 0; i < deviceCount - 1; i++) {
    for (int j = 0; j < deviceCount - i - 1; j++) {
      if (devices[j].getPriority() > devices[j + 1].getPriority()) {
        // تبديل مواقع الأجهزة
        HomeDevice temp = devices[j];
        devices[j] = devices[j + 1];
        devices[j + 1] = temp;
      }
    }
  }
}

// طباعة معلومات النظام
void SolarPowerSystem::printSystemInfo() {
  // 1. تنسيق البيانات في سلسلة واحدة مفصولة بفواصل (CSV format)
  String dataString = "";
  dataString += String(batteryPercentage, 1); // رقم عشري واحد
  dataString += ",";
  dataString += String(excessPower, 1);
  dataString += ",";
  dataString += String(shortfallPower, 1);
  dataString += ",";
  dataString += String(solarPanelPower, 1);
  dataString += ",";
  dataString += String(housePower, 1);
  dataString += ",";
  dataString += String(batteryPower, 1);

  // إضافة حالة الأجهزة (مرتبة حسب الأولوية بالفعل)
  for (int i = 0; i < deviceCount; i++) {
    dataString += ","; // إضافة فاصلة قبل حالة كل جهاز
    dataString += String(devices[i].getState() == ON ? 1 : 0); // إضافة 1 لـ ON و 0 لـ OFF
  }

  // 2. إرسال السلسلة عبر منفذ البلوتوث التسلسلي
  //    استخدام println يضيف حرف سطر جديد (\n) في النهاية،
  //    وهو مفيد كمحدد لنهاية الرسالة في جانب فلاتر.
  bluetoothSerial.println(dataString);

  // (اختياري) طباعة نفس البيانات على Serial Monitor للتصحيح
  Serial.print("Sending via BT: ");
  Serial.println(dataString);
}

// دوال للوصول للمتغيرات وتعديلها
void SolarPowerSystem::setSolarPanelPower(float power) {
  solarPanelPower = power;
}

void SolarPowerSystem::setGridPower(float power) {
  gridPower = power;
}

float SolarPowerSystem::getBatteryPercentage() const {
  return batteryPercentage;
}

float SolarPowerSystem::getSolarPanelPower() const {
  return solarPanelPower;
}

float SolarPowerSystem::getHousePower() const {
  return housePower;
}

// --- دالة جديدة لبدء البلوتوث ---
void SolarPowerSystem::beginBluetooth(long baudRate) {
  bluetoothSerial.begin(baudRate);
  Serial.print("Bluetooth Serial Started at "); // رسالة تصحيح
  Serial.print(baudRate);
  Serial.println(" baud.");
  Serial.println("Make sure Bluetooth module is paired with your phone.");
} 