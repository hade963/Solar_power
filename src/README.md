# تتبع تنفيذ نظام إدارة الطاقة الشمسية

يشرح هذا الملف تدفق التنفيذ الأساسي للنظام خلال دورتين من الحلقة الرئيسية `loop()`.

## الإعداد الأولي (Setup)

1. **`main.cpp` (قبل `setup`)**:
   * **إنشاء `devices`**: يتم إنشاء مصفوفة `devices` من نوع `HomeDevice`. لكل جهاز في المصفوفة، يتم استدعاء المنشئ `HomeDevice::HomeDevice` لتمرير القيم الأولية: رقم المنفذ (`pin`)، استطاعة التشغيل (`operationPower`)، استطاعة الإقلاع (`startupPower`)، أزمنة التشغيل والإطفاء الدنيا والقصوى (`min/max On/Off Time`)، والأولوية (`priority`).
     * **داخل `HomeDevice::HomeDevice`**:
       * تُخزَّن القيم المُمرَّرة في المتغيرات الخاصة بالكائن (`_pin`, `_operationPower`, إلخ).
       * تُهيَّأ الحالة الأولية إلى `OFF` (`_state = OFF`, `_prevState = OFF`).
       * تُصفَّر عدادات زمن التشغيل والإطفاء (`_onTime = 0`, `_offTime = 0`).
       * يُعدّ المنفذ الرقمي (`_pin`) كمخرج (`pinMode(_pin, OUTPUT)`).
       * تُضبَط حالة المنفذ الأولية إلى منخفضة (`digitalWrite(_pin, LOW)`), مما يضمن أن الجهاز مطفأ عند البدء.
   * **إنشاء `solarSystem`**: يتم إنشاء كائن `solarSystem` من نوع `SolarPowerSystem`، ويتم استدعاء المنشئ الخاص به.
2. **`SolarPowerSystem::SolarPowerSystem` (المنشئ):**
   * **تهيئة `bluetoothSerial`**: يتم تهيئة كائن `SoftwareSerial` المسمى `bluetoothSerial` لاستخدام المنفذين `bluetoothRxPin` (2) و `bluetoothTxPin` (3) المعرفين كثوابت `static const byte` في `SolarPowerSystem.h`.
   * **تهيئة ثوابت ومتغيرات البطارية**: يتم حساب وتخزين ثوابت البطارية (`BATTERY_FULL`, `BATTERY_MAX`, `BATTERY_MIN`) بناءً على السعة (`BATTERY_CAPACITY_AMP`) والجهد (`BATTERY_VOLTAGE`) المُمرَّرين أو القيم الافتراضية. تُهيَّأ قيم المتغيرات الأخرى مثل `battery` (إلى 50% من `BATTERY_FULL`) و `batteryPercentage`.
   * **تهيئة متغيرات الطاقة**: تُعطى قيم أولية لـ `solarPanelPower` (2000.0f افتراضيًا), `gridPower` (0.0f), `housePower` (0.0f), `batteryPower` (0.0f), `excessPower` (0.0f), `shortfallPower` (0.0f).
   * **تهيئة متغيرات الوقت**: تُصفَّر `previousMillis`, `currentMillis`, `deltaTime`, و `printTimer`.
   * **نسخ مؤشر الأجهزة**: يُخزَّن مؤشر مصفوفة `deviceArray` المُمرَّر في المتغير العضو `devices`. يُخزَّن عدد الأجهزة `count` في `deviceCount`.
   * **تحديث `previousMillis`**: تُقرأ القيمة الحالية لـ `millis()` وتُخزَّن في `previousMillis` لتكون نقطة البداية لحساب `deltaTime` في أول استدعاء لـ `update`.
   * **استدعاء `sortDevicesByPriority()`**:
     * **داخل `SolarPowerSystem::sortDevicesByPriority()`**: يتم تطبيق خوارزمية ترتيب الفقاعات (Bubble Sort). تقارن الحلقة الخارجية والداخلية أولويات الأجهزة المتجاورة (`devices[j].getPriority() > devices[j + 1].getPriority()`). إذا كان الجهاز `j` له أولوية أعلى (رقم أكبر) من الجهاز `j+1`، يتم تبديل موقعيهما في المصفوفة `devices` باستخدام متغير مؤقت `temp`. الهدف هو ترتيب الأجهزة تصاعديًا حسب الأولوية (الأولوية الأقل تأتي أولاً).
     * **طباعة رسائل أولية**: تُطبع رسائل ترحيبية على `Serial Monitor` للإشارة إلى بدء تشغيل النظام.
3. **`setup()` في `main.cpp`:**
   * **`Serial.begin(9600)`**: تُهيِّئ الاتصال التسلسلي مع الكمبيوتر (Serial Monitor) بمعدل باود 9600.
   * **`while (!Serial);`**: حلقة انتظار اختيارية تتأكد من أن منفذ `Serial` جاهز تمامًا قبل المتابعة (مهم لبعض لوحات Arduino مثل Leonardo).
   * **`Serial.println(...)`**: تُطبع رسالة تأكيد تهيئة `Serial Monitor`.
   * **استدعاء `solarSystem.beginBluetooth()`**:
     * **داخل `SolarPowerSystem::beginBluetooth(long baudRate = 9600)`**:
       * **`bluetoothSerial.begin(baudRate)`**: تبدأ الاتصال التسلسلي باستخدام كائن `bluetoothSerial` (على المنفذين 2 و 3) بمعدل الباود المحدد (الافتراضي 9600). **مهم:** يجب أن يتطابق هذا المعدل مع إعدادات وحدة البلوتوث HC-05/HC-06.
       * **`Serial.print(...) / Serial.println(...)`**: تُطبع رسائل على `Serial Monitor` لتأكيد بدء تشغيل البلوتوث ومعدل الباود المستخدم، وتذكير المستخدم بإقران الوحدة.

## الحلقة الرئيسية (Loop) - الدورة الأولى

1. **`loop()` في `main.cpp`:**
   * **استدعاء `solarSystem.update()`**: هذه هي الوظيفة الرئيسية التي تُستدعى بشكل متكرر لتحديث حالة النظام بأكمله.
2. **`SolarPowerSystem::update()`:**
   * **قراءة `Serial`**:
     * **`if (Serial.available() > 0)`**: تتحقق إذا كانت هناك بيانات متاحة للقراءة من منفذ `Serial` (المرتبط بالكمبيوتر).
     * **`String inputString = Serial.readStringUntil('\n')`**: إذا كانت هناك بيانات، تقرأها كسلسلة نصية حتى استقبال حرف السطر الجديد (`\n`).
     * **`inputString.trim()`**: تزيل أي مسافات بيضاء (فراغات، tab، إلخ) من بداية ونهاية السلسلة المقروءة.
     * **`float tempPower = inputString.toFloat()`**: تحاول تحويل السلسلة النصية إلى قيمة `float`.
     * **التحقق من صحة الإدخال**: تتحقق إذا كان التحويل ناجحًا. `toFloat()` تعيد `0.0` في حالة الفشل أو إذا كانت السلسلة "0" أو "0.0". لذا، يتم التحقق أيضًا مما إذا كانت السلسلة الأصلية تساوي "0" أو "0.0" للتفريق بين الإدخال الصحيح بقيمة صفر وفشل التحويل.
     * **`solarPanelPower = tempPower`**: إذا كان الإدخال رقمًا صالحًا، يتم تحديث قيمة `solarPanelPower` بالقيمة الجديدة. إذا لم يكن صالحًا، يتم تجاهل الإدخال.
   * **حساب `deltaTime`**:
     * **`currentMillis = millis()`**: تقرأ الوقت الحالي بالمللي ثانية منذ بدء تشغيل اللوحة.
     * **`deltaTime = currentMillis - previousMillis`**: تحسب الفارق الزمني (بالمللي ثانية) منذ آخر مرة تم فيها استدعاء `update`.
     * **`previousMillis = currentMillis`**: تحدِّث `previousMillis` بالقيمة الحالية استعدادًا للحساب في الدورة التالية.
   * **تحديث عداد الطباعة**: **`printTimer += deltaTime`**: تضيف الوقت المنقضي إلى `printTimer`.
   * **استدعاء `updateDevices()`**:
     * **داخل `SolarPowerSystem::updateDevices()`:**
       * **`float availablePower = solarPanelPower + gridPower`**: تحسب إجمالي الطاقة المتاحة حاليًا من الألواح والشبكة.
       * **إعادة حساب `housePower`**: تُصفِّر `housePower` ثم تمر على جميع الأجهزة. إذا كان الجهاز في حالة `ON` (`devices[i].getState() == ON`)، تضيف استطاعته (`devices[i].getPower()`) إلى `housePower`.
       * **حلقة التشغيل (حسب الأولوية)**: تمر على الأجهزة من `i = 0` إلى `deviceCount - 1` (بالترتيب الذي تم تحديده بواسطة `sortDevicesByPriority`، أي الأقل أولوية أولاً). لكل جهاز، تستدعي `updateIfOff(availablePower, housePower)`:
         * **داخل `HomeDevice::updateIfOff(float availablePower, float &totalConsumption)`**:
           * **`if (_state == OFF && _offTime >= _minOffTime && !(_onTime >= _maxOnTime))`**: تتحقق من الشروط الأساسية للسماح بالتشغيل: هل الجهاز مطفأ حاليًا؟ هل مر الحد الأدنى من زمن الإطفاء؟ هل لم يتم الوصول إلى الحد الأقصى لزمن التشغيل (في دورة التشغيل السابقة، إن وجدت)؟
           * **`if (_offTime > _maxOffTime || _operationPower + totalConsumption <= availablePower)`**: إذا تحققت الشروط الأساسية، تتحقق من سبب التشغيل: هل تجاوز الجهاز الحد الأقصى لزمن الإطفاء (يجب تشغيله بغض النظر عن الطاقة)؟ أو هل هناك طاقة كافية لتشغيله (`_operationPower`) بالإضافة إلى الاستهلاك الحالي (`totalConsumption`)؟
           * **`_state = ON; totalConsumption += _operationPower;`**: إذا كان يجب تشغيل الجهاز، تغيِّر حالته إلى `ON` وتزيد الاستهلاك الكلي `totalConsumption` (المُمرَّر بالمرجعية `&`) بقيمة استطاعة تشغيله.
       * **حلقة الإطفاء (عكس الأولوية)**: تمر على الأجهزة من `i = deviceCount - 1` إلى `0` (عكس ترتيب الأولوية، أي الأعلى أولوية أولاً). لكل جهاز، تستدعي `updateIfOn(availablePower, housePower)`:
         * **داخل `HomeDevice::updateIfOn(float availablePower, float &totalConsumption)`**:
           * **`if (_state == ON && _onTime >= _minOnTime && !(_offTime >= _maxOffTime))`**: تتحقق من الشروط الأساسية للسماح بالإطفاء: هل الجهاز قيد التشغيل حاليًا؟ هل مر الحد الأدنى من زمن التشغيل؟ هل لم يتم الوصول إلى الحد الأقصى لزمن الإطفاء (في دورة الإطفاء السابقة، إن وجدت)؟
           * **`if (_onTime > _maxOnTime || totalConsumption > availablePower)`**: إذا تحققت الشروط الأساسية، تتحقق من سبب الإطفاء: هل تجاوز الجهاز الحد الأقصى لزمن التشغيل (يجب إطفاؤه)؟ أو هل الاستهلاك الكلي الحالي (`totalConsumption`) أكبر من الطاقة المتاحة (`availablePower`) (يجب إطفاء هذا الجهاز ذي الأولوية الأعلى لتقليل الحمل)؟
           * **`_state = OFF; totalConsumption -= _operationPower;`**: إذا كان يجب إطفاء الجهاز، تغيِّر حالته إلى `OFF` وتنقص الاستهلاك الكلي `totalConsumption` (المُمرَّر بالمرجعية `&`) بقيمة استطاعة تشغيله.
   * **تطبيق حالة الأجهزة وإضافة الوقت**:
     * تمر حلقة `for` على جميع الأجهزة (`i = 0` to `deviceCount - 1`).
     * **استدعاء `devices[i].run()`**:
       * **داخل `HomeDevice::run()`**:
         * **`if (_prevState != _state)`**: تتحقق إذا كانت الحالة الحالية (`_state`) تختلف عن الحالة السابقة (`_prevState`). هذا يعني أن الحالة قد تغيرت في استدعاء `updateIfOff` أو `updateIfOn`.
         * **`digitalWrite(_pin, _state == ON ? HIGH : LOW)`**: إذا تغيرت الحالة، يتم تحديث حالة المنفذ الرقمي. إذا كانت الحالة `ON`، يُضبط المنفذ إلى `HIGH`، وإذا كانت `OFF`، يُضبط إلى `LOW`.
         * **`_onTime = 0; _offTime = 0;`**: تتم إعادة تصفير عدادات زمن التشغيل والإطفاء عند كل تغيير في الحالة.
         * **`_prevState = _state;`**: يتم تحديث الحالة السابقة لتطابق الحالة الحالية، استعدادًا للدورة التالية.
     * **استدعاء `devices[i].addTime(deltaTime)`**:
       * **داخل `HomeDevice::addTime(unsigned long deltaTime)`**:
         * **`if (_state == ON)`**: إذا كان الجهاز في حالة `ON`، يتم إضافة `deltaTime` (الوقت المنقضي منذ آخر `update`) إلى عداد زمن التشغيل `_onTime`.
         * **`else`**: إذا كان الجهاز في حالة `OFF`، يتم إضافة `deltaTime` إلى عداد زمن الإطفاء `_offTime`.
   * **استدعاء `updateBatteryPower(deltaTime)`**:
     * **داخل `SolarPowerSystem::updateBatteryPower(unsigned long deltaTime)`**:
       * **`batteryPower = solarPanelPower + gridPower - housePower`**: تحسب صافي الطاقة الداخلة أو الخارجة من البطارية. تكون موجبة عند الشحن (الإنتاج > الاستهلاك) وسالبة عند التفريغ (الاستهلاك > الإنتاج).
       * **`float powerDelta = batteryPower * deltaTime / HOUR`**: تحسب مقدار التغير في طاقة البطارية (بالواط-ساعة أو جزء منها). يتم ضرب `batteryPower` (بالواط) في `deltaTime` (بالمللي ثانية) ثم القسمة على 3,600,000 (عدد المللي ثانية في الساعة) لتحويل الواط-مللي ثانية إلى واط-ساعة.
       * **`battery += powerDelta`**: تضيف التغير المحسوب `powerDelta` إلى مستوى طاقة البطارية الحالي `battery`.
       * **التحقق من حدود البطارية**:
         * **`if (battery < BATTERY_MIN)`**: إذا انخفض مستوى البطارية عن الحد الأدنى، يتم تسجيل النقص (`shortfallPower += (BATTERY_MIN - battery)`) وتُثبَّت قيمة `battery` عند `BATTERY_MIN` لمنعها من الانخفاض أكثر.
         * **`else if (battery > BATTERY_MAX)`**: إذا زاد مستوى البطارية عن الحد الأقصى، يتم تسجيل الفائض (`excessPower += (battery - BATTERY_MAX)`) وتُثبَّت قيمة `battery` عند `BATTERY_MAX` لمنعها من الارتفاع أكثر.
       * **`batteryPercentage = (battery / BATTERY_FULL) * 100.0f`**: تعيد حساب النسبة المئوية للبطارية بناءً على القيمة المحدثة لـ `battery` والسعة الكلية `BATTERY_FULL`.
   * **التحقق من مؤقت الطباعة**:
     * **`if (printTimer >= 5000)`**: تتحقق إذا كان الوقت المتراكم في `printTimer` قد وصل إلى 5000 مللي ثانية (5 ثوانٍ) أو أكثر.
     * **استدعاء `printSystemInfo()`**:
       * **داخل `SolarPowerSystem::printSystemInfo()`**:
         * **`String dataString = "";`**: تُهيِّئ سلسلة نصية فارغة.
         * **`dataString += String(..., 1); dataString += ","; ...`**: تبني السلسلة النصية بتنسيق CSV (Comma Separated Values). يتم تحويل كل قيمة (`batteryPercentage`, `excessPower`, إلخ) إلى سلسلة نصية باستخدام `String(...)` مع تحديد دقة رقم عشري واحد (`1`)، وتُضاف فاصلة (`,`) بعد كل قيمة باستثناء الأخيرة.
         * **`bluetoothSerial.println(dataString)`**: ترسل السلسلة النصية المنسقة عبر منفذ البلوتوث التسلسلي. `println` تضيف تلقائيًا حرف سطر جديد (`\n`) في النهاية، والذي يمكن استخدامه كمحدد لنهاية الرسالة في التطبيق المستقبل (مثل Flutter).
         * **`Serial.print("Sending via BT: "); Serial.println(dataString);`**: تطبع نفس السلسلة على `Serial Monitor` لأغراض التصحيح والمراقبة.
     * **`printTimer = 0;`**: تعيد تصفير `printTimer` لبدء العد من جديد لفترة الخمس ثوانٍ التالية.

## الحلقة الرئيسية (Loop) - الدورة الثانية

1. **`loop()` في `main.cpp`:**
   * **استدعاء `solarSystem.update()` مرة أخرى**: تبدأ دورة جديدة.
2. **`SolarPowerSystem::update()`:**
   * **تكرار الخطوات**: تتكرر نفس الخطوات المفصلة في "الدورة الأولى".
     * قراءة `Serial` (قد لا يكون هناك إدخال جديد هذه المرة).
     * حساب `deltaTime` (سيكون قيمة صغيرة تعتمد على سرعة تنفيذ الدورة الأولى).
     * تحديث `printTimer`.
     * استدعاء `updateDevices`: ستعتمد نتيجة `updateIfOff` و `updateIfOn` الآن على قيم `_onTime` و `_offTime` التي تم تحديثها في نهاية الدورة الأولى، بالإضافة إلى `availablePower` الحالية. قد تتغير حالة جهاز أو أكثر بناءً على هذه القيم.
     * استدعاء `run()` و `addTime()` لكل جهاز: لتطبيق أي تغييرات في الحالة وتحديث عدادات الوقت مرة أخرى.
     * استدعاء `updateBatteryPower`: لتحديث مستوى شحن البطارية بناءً على `housePower` المحدث و `deltaTime` لهذه الدورة.
     * التحقق من `printTimer`: على الأغلب لن يكون قد وصل إلى 5 ثوانٍ بعد، لذا لن يتم استدعاء `printSystemInfo` في هذه الدورة (إلا إذا كانت الدورات بطيئة جدًا).

تستمر هذه الدورة، حيث يقوم النظام باستمرار بمراقبة المدخلات (الطاقة الشمسية عبر `Serial`)، وتحديث حالة الأجهزة بناءً على الأولويات والطاقة المتاحة وقيود الوقت المتراكمة، وتحديث حالة البطارية، وإرسال البيانات دوريًا عبر البلوتوث.
