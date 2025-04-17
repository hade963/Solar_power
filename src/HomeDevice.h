#ifndef HOME_DEVICE_H
#define HOME_DEVICE_H

#include <Arduino.h>
#include "DeviceState.h"

//////////////////////////////////////////////////////////////////
// كلاس يمثل الأجهزة المنزلية                                    //
//////////////////////////////////////////////////////////////////
class HomeDevice {
  private:
    // خصائص الجهاز
    int _pin;                   // منفذ التحكم بالجهاز
    int _priority;              // أولوية تشغيل الجهاز
    float _operationPower;      // الاستطاعة التي يستهلكها عند التشغيل
    float _startupPower;        // الاستطاعة التي يستهلكها عند الإقلاع
    
    // حدود التشغيل
    unsigned long _minOnTime;   // زمن التشغيل الأقل
    unsigned long _maxOnTime;   // زمن التشغيل الأطول
    unsigned long _minOffTime;  // زمن الإطفاء الأقل
    unsigned long _maxOffTime;  // زمن الإطفاء الأطول
    
    // متغيرات الحالة
    DeviceState _state;         // حالة الجهاز الحالية
    DeviceState _prevState;     // حالة الجهاز السابقة
    unsigned long _onTime;      // زمن التشغيل الحالي
    unsigned long _offTime;     // زمن الإيقاف الحالي
    
  public:
    // منشئ الكلاس
    HomeDevice(int pin, float operationPower, float startupPower, 
              unsigned long minOnTime, unsigned long maxOnTime, 
              unsigned long minOffTime, unsigned long maxOffTime, int priority);
    
    // تحديث حالة الجهاز إذا كان مطفأ
    void updateIfOff(float availablePower, float &totalConsumption);
    
    // تحديث حالة الجهاز إذا كان قيد التشغيل
    void updateIfOn(float availablePower, float &totalConsumption);
    
    // تنفيذ حالة الجهاز
    void run();
    
    // إضافة الوقت المنقضي
    void addTime(unsigned long deltaTime);
    
    // الحصول على أولوية الجهاز
    int getPriority() const;
    
    // الحصول على استطاعة الجهاز
    float getPower() const;
    
    // الحصول على حالة الجهاز
    DeviceState getState() const;
};

#endif // HOME_DEVICE_H 