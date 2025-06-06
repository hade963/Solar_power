#include "HomeDevice.h"

// منشئ الكلاس
HomeDevice::HomeDevice(int pin, float operationPower, float startupPower, 
                    unsigned long minOnTime, unsigned long maxOnTime, 
                    unsigned long minOffTime, unsigned long maxOffTime, int priority) {
  _pin = pin;
  _operationPower = operationPower; 
  _minOnTime = minOnTime;
  _maxOnTime = maxOnTime;
  _minOffTime = minOffTime;
  _maxOffTime = maxOffTime;
  _priority = priority;
  
  _state = OFF;
  _prevState = OFF;
  _onTime = 0;
  _offTime = 0;
  
  // إعداد منفذ التحكم بالجهاز
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

// تحديث حالة الجهاز إذا كان مطفأ
void HomeDevice::updateIfOff(float availablePower, float &totalConsumption) {
  
}

// تحديث حالة الجهاز إذا كان قيد التشغيل
void HomeDevice::updateIfOn(float availablePower, float &totalConsumption) {
 
}

// تنفيذ حالة الجهاز
void HomeDevice::run() {
  if (_prevState != _state) {
    digitalWrite(_pin, _state == ON ? HIGH : LOW);
    _onTime = 0;
    _offTime = 0;
  }
  _prevState = _state;
}

// إضافة الوقت المنقضي
void HomeDevice::addTime(unsigned long deltaTime) {
  if (_state == ON) {
    _onTime += deltaTime;
  } else {
    _offTime += deltaTime;
  }
}

// الحصول على أولوية الجهاز
int HomeDevice::getPriority() const { 
  return _priority; 
}

// الحصول على استطاعة الجهاز
float HomeDevice::getPower() const { 
  return _operationPower; 
}

// الحصول على حالة الجهاز
DeviceState HomeDevice::getState() const {
  return _state;
} 