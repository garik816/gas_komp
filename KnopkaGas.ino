// v. 2.0

#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 7, 6, 5, 4); //Создаём объект LCD-дисплея, указываем, к каким пинам Arduino подключены выводы дисплея: RS, E, DB4, DB5, DB6, DB7
#include <Wire.h> //Уменьшить буфер!!!!!
#include <EEPROM.h>

#define VoltagePin    A0  //Пин подключения датчика напряжения
#define EngineTempPin A1  //Пин подключения датчика температуры
#define GasLevelPin   A2  //Пин подключения датчика уровня газа
#define BenzLevelPin  A3  //Пин подключения датчика уровня бензина
#define ButonsPin     A6  //Пин подключения кнопок управления компьютером
//#define ButonsPin     10  //Пин подключения кнопок управления компьютером
#define GasButonPin   0  //Пин подключения кнопки "газ"
#define BenzButonPin  1  //Пин подключения кнопки "бензин"
#define GasPin        11 //Пин подключения газового клапана
#define BenzPin       12 //Пин подключения бензонасоса

#define GasSwitchTemp       20     //Температура переключения на газ (C)
#define DeltaTempCalckTime  2000   //Период измерения температуры (мс)
#define DeltaGas            500    //Длительность подачи газа после остановки двигателя (0.5сек)
#define DeltaBenz           1000   //Длительность работы бензонасоса после остановки двигателя (1 сек)
//#define DeltaFromGasToBenz  0   //Длительность закачки бензина в карбюратор (милисекунд)-------------------------------------------------проверить------------------------------------------------------
#define LCDDelta            400    //Период обноления информации на дисплее (мс)
#define FuerLevelDelta      600000  //Период вычисления уровня топлива (мс) (раз в 10 мин)
#define StartOnBenz         15000  //Время начальной закачки бензина в карбюратор (мс)

#define CurentFuer_EEPROM_adres         0  //0     (1 байт)  адресс EEPROM для хранения значения текущего топлива
#define Odometr_EEPROM_adres            1  //1-4   (4 байта) одометра
#define Odometr_Gas_EEPROM_adres        5  //5-8   (4 байта) пробега на газу
#define Odometr_Benz_EEPROM_adres       9  //9-12  (4 байта) пробега на бензине
#define Odometr_Time_Gas_EEPROM_adres   13 //13-16 (4 байта) времени работы двигателя на газу
#define Odometr_Time_Benz_EEPROM_adres  17 //17-20 (4 байта) времени работы двигателя на бензине
#define Volume_Benz_EEPROM_adres        21 //21    (1 байт)  объема бензина для контрольной точки
#define CheckPoint_Benz_EEPROM_adres    22 //22-25 (4 байта) пробега на бензине для контрольной точки
#define Volume_Gas_EEPROM_adres         26 //26    (1 байт)  объема газа для контрольной точки
#define CheckPoint_Gas_EEPROM_adres     27 //27-30 (4 байта) пробега на газу для контрольной точки
#define GasLevel_EEPROM_adres           31 //31    (1 байт) текущий уровень газа
#define BenzLevel_EEPROM_adres          32 //32    (1 байт) текщий уровень бензина



//long Speed_of_program; //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


char output_string_1[17]; //Для вывода на дисплей
char output_string_2[17]; //Для вывода на дисплей

word Voltage;     //Напряжение бортовой сети (20В = 1000 единиц с АЦП)
boolean LowVoltage = 0 ; //Флаг низкого напряжения
int  EngineTemp;  //Температура двигателя (10В = 1000 единиц с АЦП)
unsigned long TempCalckTime = 0; //Для запоминания времени, когда вычисляли тепературу
boolean CurentFuer;  //Текущее топливо: false - бензин, true - газ








//Заменить!!!

boolean Led_Status; //Состояние светодиода


//на


//digitalWrite(13, digitalRead(13)==1?0:1); delay(1);










byte Speed; //Переменная для хранения значения скорости (км/ч)

volatile unsigned long TahometrTimer[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //Масив для хранения значений таймера (для прерывания от катушки)
volatile byte t = 0; //Указатель для масива значения таймера

volatile unsigned long OdometrTimer[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //Масив для хранения значений таймера (для датчика скорости)
volatile byte o = 0; //Указатель для масива значения таймера
volatile word OdometrVar = 0; //Счетчик импульсов с датчика скорости (считывается при выводе информации на дисплей)

byte BenzLevelArray[10]; //Масив для хранения значений уровня бензина
byte BenzLevel; //значение уровня бензина, л * 4
byte b = 0; //Указатель для масива значений уровня топлива
unsigned long FuerLevelTime = 0; //Переменная для сохранения значения таймера для вычисления уровня топлива
byte GasLevelArray[10]; //Масив для хранения значений уровня газа
byte GasLevel; //значение уровня газа

word Prognoz; //прогноз пробега на остатке топлива, км
word Rashod;  //расход топлива, л/1000км


unsigned long Odometr; //Переменная для хранения значения одометра (метров);
unsigned long Odometr_Gas; //Переменная для хранения значения одометра пробега на газу (метров);
unsigned long Odometr_Benz; //Переменная для хранения значения одометра пробега на бензине (метров);
unsigned long Odometr_Time_Gas; //Переменная для хранения значения времени работы двигателя на газу с нулевой скоростью (милисек);
unsigned long Odometr_Time_Benz; //Переменная для хранения значения времени работы двигателя на бензине с нулевой скоростью (милисек);

boolean StartOnBenzFlag = false;   //Флаг запуска двигателя на бензине
boolean FromBenzToGasFlag = false; //Флаг процесса перехода с бензина на газ
//boolean FromGasToBenzFlag = false; //Флаг процесса перехода с газа на бензин
unsigned long Delta;               // Для процедур перехода бензин-газ

byte ButonTimer = 0; //Для устранения дребезга контактов кнопок

unsigned long LCDTimer = 0; // Переменная для сохранения значения таймера (для отображения информации на дисплее)
byte LCD_Page = 0; // Перемення для хранения текущего значения номера страницы отображаемой на дисплее
byte Butons_Timer = 0; //Счетчик для клавиатуры

void setup()
{
  pinMode(GasPin, OUTPUT);    //инициализаруем пин управления газовым клапаном
  digitalWrite(GasPin, LOW);  //отключаем газ
  pinMode(BenzPin, OUTPUT);   //инициализаруем пин управления бензонасосом
  digitalWrite(BenzPin, LOW); //отключаем бензин
  pinMode(GasButonPin, INPUT_PULLUP);      //инициализаруем пин кнопки газа, включаем внутрение подтягивающие резисторы
  pinMode(BenzButonPin, INPUT_PULLUP);     //инициализаруем пин кнопки бензина, включаем внутрение подтягивающие резисторы
  //  pinMode(ButonsPin, INPUT_PULLUP);    //инициализаруем пин кнопки управления компьютером, включаем внутрение подтягивающие резисторы
  pinMode(13, OUTPUT);    //инициализаруем пин управления светодиодом
  digitalWrite(13, HIGH);  //включаем светодиод

  attachInterrupt(0, _TahometrInterrupt, RISING); //инициализируем обработчик импульсов с катушки
  attachInterrupt(1, _OdometrInterrupt, RISING);  //инициализируем обработчик импульсов с датчика скорости

  EEPROM.get(CurentFuer_EEPROM_adres, CurentFuer); //считываем сохраненное значение текущего топлива
  EEPROM.get(Odometr_EEPROM_adres, Odometr);                      // одометра
  EEPROM.get(Odometr_Gas_EEPROM_adres, Odometr_Gas);              // пробега на газу
  EEPROM.get(Odometr_Benz_EEPROM_adres, Odometr_Benz);            // пробега на бензине
  EEPROM.get(Odometr_Time_Gas_EEPROM_adres, Odometr_Time_Gas);    // времени работы двигателя на газу
  EEPROM.get(Odometr_Time_Benz_EEPROM_adres, Odometr_Time_Benz);  // времени работы двигателя на бензине

  Wire.begin(); // join i2c bus (address optional for master)
  lcd.begin(16, 2); //Инициализируем дисплей: 2 строки по 16 символов

  //  if (!digitalRead(ButonsPin)) Set_Time_and_Date(); //если нажата клавиша управления - запускаем процедуру настроек
  if (analogRead(ButonsPin) < 100) Set_Time_and_Date(); //если нажата клавиша управления - запускаем процедуру настроек

  lcd.print("      Wait");
  do
  {
    Voltage = analogRead(VoltagePin);
  } while (Voltage < 400); //ждем пока напряжение не поднимется до 8В

  boolean InStable;

  do //ждем пока установится напряжение на датчиках
  {
    InStable = false;
    EngineTemp = analogRead(EngineTempPin);
    Voltage = analogRead(VoltagePin);
    delay(200);
    if ( EngineTemp != analogRead(EngineTempPin) ) InStable = true;
    if ( Voltage != analogRead(VoltagePin) ) InStable = true;
  } while (InStable);  // Ждем пока установится напряжение на датчиках
  delay(200);

  for (byte i = 0; i < 10; i++) Calc_Fuer_Level(); //заполняем массив текущим уровнем топлива

  EEPROM.get(GasLevel_EEPROM_adres, Speed); // считываем уровень газа
  EngineTemp = GasLevel - Speed; //вычисляем разницу между текущим и ранее сохраненным уровнем
  if (EngineTemp > 32) //заправка газа >8 литров
  {
    lcd.clear();                    //вывод сообщения о заправке газом
    lcd.print("GAS       l");
    lcd.setCursor(7, 0);
    lcd.print(EngineTemp / 4);
    lcd.setCursor(0, 1);
    lcd.print("Сheckpoint creat");
    EEPROM.put(Volume_Gas_EEPROM_adres, GasLevel); //сохраняем текущий объем газа для контрольной точки
    EEPROM.put(CheckPoint_Gas_EEPROM_adres, Odometr_Gas); //сохраняем текущее значение пробега на газу для контрольной точки
    delay(3000);
  }
  else GasLevel = Speed;  //присваиваем считанное значение текущему

  EEPROM.get(BenzLevel_EEPROM_adres, Speed);  // считываем уровень бензина
  EngineTemp = BenzLevel - Speed; //вычисляем разницу между текущим и ранее сохраненным уровнем
  if (EngineTemp > 32) //заправка бензином >8 литров
  {
    lcd.clear();                    //вывод сообщения о заправке бензином
    lcd.print("BENZ      l");
    lcd.setCursor(7, 0);
    lcd.print(EngineTemp / 4);
    lcd.setCursor(0, 1);
    lcd.print("Сheckpoint creat");
    EEPROM.put(Volume_Benz_EEPROM_adres, BenzLevel); //сохраняем текущий объем бензина для контрольной точки
    EEPROM.put(CheckPoint_Benz_EEPROM_adres, Odometr_Benz); //сохраняем текущее значение пробега на бензине для контрольной точки
    delay(3000);
  }
  else BenzLevel = Speed;  //присваиваем считанное значение текущему

  EngineTemp = 300;
  _Temp(); //Измеряем температуру двигателя

  if (!digitalRead(GasButonPin)) goto SkipBenz; //если нажата кнопка ГАЗ - бензин не закачиваем
  if (!digitalRead(BenzButonPin)) goto Benz; //если нажата кнопка Бензин - бензин закачиваем

  if (EngineTemp < GasSwitchTemp || !CurentFuer) //сравниваем с температурой переключения или мотор был заглушен на бензине
  {
Benz:
    CurentFuer = false; // топливо - бензин
    StartOnBenzFlag = true;
    Delta = millis();
    digitalWrite(BenzPin, HIGH); //включаем бензин
  }
SkipBenz:
  digitalWrite(13, LOW);  //выключаем светодиод
}

//Функции и процедуры--------------------------------------------------------------------------
//Процедура настройки времени и даты----------------------------------------
void Set_Time_and_Date()
{

  byte Second, Minute, Hour, Day, Wday, Month, Year;
  boolean flag;
  byte Cursor = 0;
  byte key;

  lcd.cursor();
  lcd.blink();

Loop:
  //блок считывания текушего времени и даты
  Wire.beginTransmission(0x68);
  Wire.write((byte)0);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 7);
  //    if (Wire.available() < 7) return;
  Second = bcd2dec(Wire.read() & 0x7f);
  Minute = bcd2dec(Wire.read() );
  Hour =   bcd2dec(Wire.read() & 0x3f);  // mask assumes 24hr clock
  Wday = Wire.read();
  Day = bcd2dec(Wire.read() );
  Month = bcd2dec(Wire.read() );
  Year = bcd2dec(Wire.read() ); //отсчет с 2000 года

  //блок вывода на дисплей
  lcd.setCursor(4, 0);
  if (Hour < 10) lcd.print(' ');
  lcd.print(Hour);
  lcd.print(':');
  if (Minute < 10) lcd.print('0');
  lcd.print(Minute);
  lcd.print(':');
  if (Second < 10) lcd.print('0');
  lcd.print(Second);
  lcd.setCursor(3, 1);
  if (Day < 10) lcd.print(' ');
  lcd.print(Day);
  lcd.print('/');
  if (Month < 10) lcd.print('0');
  lcd.print(Month);
  lcd.print('/');
  lcd.print(Year + 2000);

  //считываем нажатия кнопок
  key = 0;
  if (!digitalRead(GasButonPin)) key = 2;
  if (!digitalRead(BenzButonPin)) key = 3;
  //  if (!digitalRead(ButonsPin)) key = 1;
  if (analogRead(ButonsPin) < 100) key = 1;

  //коректируем время/дату
  switch (Cursor) {
    case 0: //часы
      switch (key) {
        case 1: //следующая
          Cursor++;
          if (Cursor > 5) Cursor = 0;
          break;
        case 2: // -
          if (Hour > 0) Hour--;
          break;
        case 3: // +
          if (Hour < 23) Hour++;
          break;
      }
      lcd.setCursor(4, 0);
      break;

    case 1: //минуты
      switch (key) {
        case 1: //следующая
          Cursor++;
          if (Cursor > 5) Cursor = 0;
          break;
        case 2: // -
          if (Minute > 0) Minute--;
          break;
        case 3: // +
          if (Minute < 60) Minute++;
          break;
      }
      lcd.setCursor(7, 0);
      break;

    case 2: //секунды
      switch (key) {
        case 1: //следующая
          Cursor++;
          if (Cursor > 5) Cursor = 0;
          break;
        case 2: // -
          if (Second > 0) Second--;
          break;
        case 3: // +
          if (Second < 60) Second++;
          break;
      }
      lcd.setCursor(10, 0);
      break;

    case 3: //день
      switch (key) {
        case 1: //следующая
          Cursor++;
          if (Cursor > 5) Cursor = 0;
          break;
        case 2: // -
          if (Day > 1) Day--;
          break;
        case 3: // +
          if (Day < 31) Day++;
          break;
      }
      lcd.setCursor(3, 1);
      break;

    case 4: //месяц
      switch (key) {
        case 1: //следующая
          Cursor++;
          if (Cursor > 5) Cursor = 0;
          break;
        case 2: // -
          if (Month > 1) Month--;
          break;
        case 3: // +
          if (Month < 12) Month++;
          break;
      }
      lcd.setCursor(6, 1);
      break;

    case 5: //год
      switch (key) {
        case 1: //следующая
          Cursor++;
          if (Cursor > 5) Cursor = 0;
          break;
        case 2: // -
          if (Year > 16) Year--;
          break;
        case 3: // +
          if (Year < 250) Year++;
          break;
      }
      lcd.setCursor(9, 1);
      break;
  }

  //записываем время если были нажаты клавиши + или -
  if (key > 1)
  {
    Second |= 0x80;  // stop the clock
    Wire.beginTransmission(0x68);
    Wire.write((uint8_t)0x00); // reset register pointer
    Wire.write(dec2bcd(Second)) ;
    Wire.write(dec2bcd(Minute));
    Wire.write(dec2bcd(Hour));      // sets 24 hour format
    Wire.write(Wday);
    Wire.write(dec2bcd(Day));
    Wire.write(dec2bcd(Month));
    Wire.write(dec2bcd(Year));
    Wire.endTransmission();

    Second &= 0x7f;  // start the clock
    Wire.beginTransmission(0x68);
    Wire.write((uint8_t)0x00); // reset register pointer
    Wire.write(dec2bcd(Second)) ;
    Wire.endTransmission();
  }
  delay(500);

  goto Loop; //зацикливаем настройку, выход из настройки - выключением
}

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t dec2bcd(uint8_t num)
{
  return ((num / 10 * 16) + (num % 10));
}

//Измерение температуры двигателя--------------------------------------------------------------------------------------переделать------------------------------------------------------------------------------------------------------------
void _Temp()
{
  long V;
  long T; //long - обязательно
  float k;

  T = analogRead(EngineTempPin);
  V = analogRead(VoltagePin);
  //  T = ((690 - V) * T) / V + T; //коректировка на заниженое напряжение питания (690АЦП = 13.8В)-------------------------------проверить----------------------------------
  T = ((690 - V) * T) / (V + V - 690) + T; //коректировка на заниженое напряжение питания и падение напрядения на проводах при включении нагрузки(690АЦП = 13.8В)-------------------------------проверить----------------------------------
  if (T < 675) //Температура >70 С
  {
    T = 197 - int(T * 0.185); //Формула для вычисления температуры в диапазоне >70 С
  }
  else //Температура <70 С
  {
    T = 610 - T;
    k = T * 0.025;
    k = k * k * k;
    T = int(k) + 76; //Формула для вычисления температуры в диапазоне <70 С
  }
  if ( T > EngineTemp ) EngineTemp++; //защита от бросков измеренных данных
  if ( T < EngineTemp ) EngineTemp--; //защита от бросков измеренных данных
  if ( EngineTemp > 150 ) EngineTemp = T; //начальная установка температуры
}

//Езда на бензине-----------------------------------------------------------
void _Benz()
{
  //Прерывание записывает текущее значение millis в TahometrTimer
  if (millis() - TahometrTimer[t] > DeltaBenz) digitalWrite(BenzPin, LOW); //Импульсов от катушки нет
  else digitalWrite(BenzPin, HIGH); // Импульсы от катушки есть
}

//Езда на газе--------------------------------------------------------------
void _Gas()
{
  //Прерывание записывает текущее значение millis в TahometrTimer
  if (millis() - TahometrTimer[t] > DeltaGas) digitalWrite(GasPin, LOW); //Импульсов от катушки нет
  else digitalWrite(GasPin, HIGH); // Импульсы от катушки есть
}

//Контроль температуры и переключение видов топлива-------------------------
void Control_Temp_And_Swith_Fuer()
{
  //блок начальной закачки бензина в карбюратор
  if (StartOnBenzFlag)                                    //проверяем надо ли накачать бензин перед запуском двигателя
    if (millis() - Delta > StartOnBenz) StartOnBenzFlag = false; //если прошло StartOnBenz сек сбрасываем флаг
    else return;                 //ждем StartOnBenz сек

  //блок перехода с бензина на газ
  if (FromBenzToGasFlag)
  {
    short int k;
    byte i;
    word tah;

Loop:
    i = t;              //сохраняем текущее значение указателя массива
    tah = _Tahometr();  // и обороты
    if ( tah == 0 ) return; //нет импульсов с катушки больше 0.4 сек (< 75 оборотов в минуту)
    switch (i) { //вычисляем производную функции изменения оборотов двигателя. >0 - обороты падают, <0 - растут, =0 - обороты не изменяются, величина производной показывает скорость изменения оборотов
      case 0:
        k = (TahometrTimer[i] - TahometrTimer[10]) - (TahometrTimer[2] - TahometrTimer[1]);
        break;
      case 9:
        k = (TahometrTimer[i] - TahometrTimer[8]) - (TahometrTimer[0] - TahometrTimer[10]);
        break;
      case 10:
        k = (TahometrTimer[i] - TahometrTimer[9]) - (TahometrTimer[1] - TahometrTimer[0]);
        break;
      default:
        k = (TahometrTimer[i] - TahometrTimer[i - 1]) - (TahometrTimer[i + 2] - TahometrTimer[i + 1]);
    }
    if ( i != t ) goto Loop;              //если изменилось значение указателя массива - пересчитываем заново
    if (tah > 700)                       //если обороты <700 - включаем газ, нужно для прогретого двигателя - там обороты падают медленно
      if (k < 4 || tah > 1100 || EngineTemp > 70) return;   //иначе проверяем скорость падения оборотов и текущие обороты  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!проверить!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    digitalWrite(GasPin, HIGH);                             //включаем газ, дублируем здесь команду включаения газа для ускорения открытия клапана.
    CurentFuer = true;
    FromBenzToGasFlag = false;
  }
  /*
    //блок перехода с газа на бензин
    if (FromGasToBenzFlag)
      if (millis() - Delta > DeltaFromGasToBenz) //ждем пока насос накачает бензин
      {
        CurentFuer = false;
        FromGasToBenzFlag = false;
        digitalWrite(GasPin, LOW); //выключаем газ
      }
      else return;                 //ждем DeltaFromGasToBenz
  */
  //блок контроля температуры и переключения топлива
  if (EngineTemp > GasSwitchTemp)             //сравниваем с температурой переключения
    if (CurentFuer) _Gas();                   //текущее топливо - газ?, продолжаем ехать на газу
    else {
      FromBenzToGasFlag = true;
      Delta = millis();
      digitalWrite(BenzPin, LOW);             //выключаем бензин
    }
  else                                        //температура меньше GasSwitchTemp
    if (CurentFuer) _Gas();                   //текущее топливо - газ?, продолжаем ехать на газу
    else _Benz();                             //продолжаем ехать на бензине
}

  /*
  if (EngineTemp > GasSwitchTemp)             //сравниваем с температурой переключения
    if (CurentFuer) _Gas();                   //текущее топливо - газ?, продолжаем ехать на газу
    else {
      FromBenzToGasFlag = true;
      Delta = millis();
      digitalWrite(BenzPin, LOW);             //выключаем бензин
    }
  else _Benz();                             //продолжаем ехать на бензине
  else                                        //температура меньше GasSwitchTemp
    if (!CurentFuer) _Benz();                 //текущее топливо - бензин?, продолжаем ехать на бензине
    else if (EngineTemp + 10 < GasSwitchTemp) //10 градусоов гистерезис вниз ------------------------------------ проверить !!!!!!!! ---------------------------------------------------------------------------------------------
    {
      //        FromGasToBenzFlag = true;
      Delta = millis();
      digitalWrite(BenzPin, HIGH);            //включаем бензин
    }
    else _Gas();                              //продолжаем ехать на газу
}
*/
//Функция вычисления оборотов двигателя-------------------------------------
word _Tahometr()
{
  unsigned long k;
  byte i;
Loop:
  i = t; //сохраняем текущее значение указателя масива
  if ( millis() - TahometrTimer[i] > LCDDelta ) return 0; //нет импульсов с катушки больше 0.4 сек (< 75 оборотов в минуту)
  if ( i == 10 ) k = TahometrTimer[i] - TahometrTimer[0];
  else k = TahometrTimer[i] - TahometrTimer[i + 1];
  if ( i != t ) goto Loop; //если изменилось значение указателя массива - пересчитываем заново
  k = 300000 / k;
  return k; //Обороты (об/мин)
}

//процедура вычисления пройденого пути и скорости ------------------------------
void Odometr_and_Speedometr()
{
  unsigned long k;
  byte i;
  //блок вычисления скорости
Loop:
  i = o; //сохраняем текущее значение указателя масива
  if ( millis() - OdometrTimer[i] > LCDDelta ) Speed = 0; //нет импульсов с датчика скорости больше 0.4 сек (< 1.5 км/ч)
  else
  {
    if ( i == 10 ) k = OdometrTimer[i] - OdometrTimer[0];
    else k = OdometrTimer[i] - OdometrTimer[i + 1];
    if ( i != o ) goto Loop;  //если изменилось значение указателя массива - пересчитываем заново
    Speed = 6000 / k; //Скорость (км/час)
  }

  //блок вычисления пройденого пути
  k = OdometrVar / 6;               //Датчик дает 6 импульсов на 1 метр
  OdometrVar = OdometrVar - k * 6;  //Отнимаем от счетчика целое кол-во метров
  Odometr += k;                     //Пройденое расстояние хранится в метрах
  if (Speed)
  {
    if (CurentFuer) Odometr_Gas += k; //едем на газу (м)
    else Odometr_Benz += k;           //едем на бензине (м)
  }
  else
  {
    if (_Tahometr())    //если двигатель заглушен - время не считаем
      if (CurentFuer) Odometr_Time_Gas += LCDDelta + 1; //греем машину на газу (мс)
      else Odometr_Time_Benz += LCDDelta + 1;           //греем машину на бензине (мс)
  }
}

//Процедура вывода символов в строку--------------------------------------------
void Print_Digits(char *str, byte pos, byte len, char ch, long num) //(указать на строку, начальная позиция, количество символов на число, заполняющий символ, число для вывода)
{
  byte i = 0; // позиция выводимого символа в строке
  boolean Flag = false; //признак отрицатального числа

  if (num < 0) {     //если число отрицательное:
    num = abs(num); //преобразовываем в положительное
    Flag = true;    //устанавливаем флаг
  }
  str += pos + len; //устанавливаем указатель строки на младший разряд числа
  do {
    *--str = num % 10 + '0'; //преобразуем число в строку начиная с младшего разряда
    num /= 10;
    i++;
  }  while (num != 0);
  if (Flag) { //если число отрицательное выводим знак минус
    *--str = '-';
    i++;
  }
  while (i < len) {//заполняем место перед числом заполняющими символами
    *--str = ch;
    i++;
  }
}

//процедура вывода на дисплей данных счетчика времени---------------------------
void Calc_Time(unsigned long k)
{
  byte D, H, M, S;
  boolean ZeroFlag = false;

  D = k / 86400;  //дни
  k %= 86400;
  H = k / 3600;   //часы
  k %= 3600;
  M = k / 60;     //минуты
  S = k % 60;     //секунды

  output_string_2[13] = 'c';//s
  output_string_2[14] = 'e';//e
  output_string_2[15] = 0xBA;//c
  if ( D )
  {
    Print_Digits(output_string_2, 0, 2, ' ', D); //(указать на строку, начальная позиция, количество симвволов на число, заполняющий символ, число для вывода)
    output_string_2[2] = 0xE3; //d
    ZeroFlag = true;
  }
  if ( H || ZeroFlag)
  {
    if (ZeroFlag) Print_Digits(output_string_2, 4, 2, '0', H);
    else Print_Digits(output_string_2, 4, 2, ' ', H);
    output_string_2[6] = ':';
    ZeroFlag = true;
  }
  if ( M || ZeroFlag )
  {
    if (ZeroFlag) Print_Digits(output_string_2, 7, 2, '0', M);
    else Print_Digits(output_string_2, 7, 2, ' ', M);
    output_string_2[9] = ':';
    ZeroFlag = true;
  }
  if (ZeroFlag) Print_Digits(output_string_2, 10, 2, '0', S);
  else Print_Digits(output_string_2, 10, 2, ' ', S);
}

// Convert Binary Coded Decimal (BCD) to Decimal--------------------------------
byte bcd2dec(byte num)
{
  return ((num / 16 * 10) + (num % 16));
}

//процедура вычисления среднего расхода топлива----------------------------------
void Fuel_Consumption(byte Fuer_Volume_EEPROM_adres, byte CheckPoint_EEPROM_adres, byte Fuer_Level, unsigned long _Odometr)
{
  byte Vol_Fuer; //сохраненая метка кол-ва топлива, л
  unsigned long Check_Point; //сохраненая метка километража, км

  EEPROM.get(Fuer_Volume_EEPROM_adres, Vol_Fuer); //считываем сохраненный объем топлива
  EEPROM.get(CheckPoint_EEPROM_adres, Check_Point); //считываем сохраненное значение километража

  if (Vol_Fuer > Fuer_Level && _Odometr > Check_Point) {// защита от неправильно измерения уровня топлива
    Vol_Fuer -= Fuer_Level;                     //вычисляем сколько топлива израсходовано, литров * 4
    _Odometr -= Check_Point;                   //вычисляем пройденое растояние, метров
    Rashod = ((unsigned long)Vol_Fuer * 250000) / _Odometr; //расход на 1000 км (для того чтоб 1 цифра после запятой вошла в целую часть) //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11 проверить !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
    Prognoz = ((word)Fuer_Level * 250) / Rashod;        //прогноз пробега на остатке топлива, км                                          //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11 проверить !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
  }
  else {
    Rashod = 0;
    Prognoz = 0;
  }
}

//процедура вычисления уровня топлива-------------------------------------------
void Calc_Fuer_Level()
{
  long k, h, u;
  k = analogRead(BenzLevelPin);
  h = analogRead(GasLevelPin);
  u = analogRead(VoltagePin);
  BenzLevelArray[b] = (((690 - u) * k) / u + k) / 4; //вычисляем поправку на изменение напряжения, результат делим на 4, чтоб в 1 байт влезло и заносим в массив
  GasLevelArray[b] = h / 4; //делим на 4, чтоб в 1 байт влезло и заносим в массив
  b++;
  if (b == 10) b = 0;
  k = 0;
  h = 0;
  for (byte i = 0; i < 10; i++) {
    k += BenzLevelArray[i];
    h += GasLevelArray[i];
  }
  BenzLevel = 172 - k / 15; //вычисляем остаток в баке, литров * 4   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! передалать сразу на латры
  GasLevel = h / 8; //вычисляем остаток в баке, литров
}


//Основная программа-------------------------------------------------------------
void loop() {
  //блок вычисления напряжения и сохранения данных в ПЗУ
  Voltage = analogRead(VoltagePin);
  if (Voltage < 350 && !LowVoltage) {   //Если напряжение меньше 7В и флаг не поднят - сохраняем данные
    EEPROM.put(CurentFuer_EEPROM_adres, CurentFuer); //если текущее попливо отличается от сохранненого - сохраняем
    EEPROM.put(Odometr_EEPROM_adres, Odometr);                      // одометра
    EEPROM.put(Odometr_Gas_EEPROM_adres, Odometr_Gas);              // пробега на газу
    EEPROM.put(Odometr_Benz_EEPROM_adres, Odometr_Benz);            // пробега на бензине
    EEPROM.put(Odometr_Time_Gas_EEPROM_adres, Odometr_Time_Gas);    // времени работы двигателя на газу
    EEPROM.put(Odometr_Time_Benz_EEPROM_adres, Odometr_Time_Benz);  // времени работы двигателя на бензине
    EEPROM.put(GasLevel_EEPROM_adres, GasLevel);                    // уровень газа
    EEPROM.put(BenzLevel_EEPROM_adres, BenzLevel);                  // уровеь бензина

    digitalWrite(13, HIGH);
    lcd.clear();
    lcd.print("      OFF");
    delay(500);
    digitalWrite(13, LOW);
    LowVoltage = HIGH;
  }
  else {
    LowVoltage = LOW; //Сбрасываем флаг низкого напряжения
  }

  // Блок вывода информации на дисплей и запуска процедур по таймеру
  if ( millis() - LCDTimer > LCDDelta ) {
    LCDTimer = millis(); // Запоминаем время вывода информации на дисплей
    Odometr_and_Speedometr(); //вычисляем скорость и пройденный путь

    //блок измерения температуры двигателя
    if (millis() - TempCalckTime > DeltaTempCalckTime) { //измеряем раз в 2 сек
      TempCalckTime = millis();
      _Temp();

      //блок измерения уровня топлива
      if (millis() - FuerLevelTime > FuerLevelDelta) { //измеряем раз в 1 мин
        FuerLevelTime = millis();
        Calc_Fuer_Level();
      }
    }

    for (byte i = 0; i < 16; i++) {
      output_string_1[i] = ' '; //заполняем строку пробелами
      output_string_2[i] = ' '; //заполняем строку пробелами
    }
    output_string_1[16] = 0; //символ конца строки
    output_string_2[16] = 0; //символ конца строки

    //Блок проверки выхода параметров за допустимые диапазоны
    if (Voltage < 600 && _Tahometr() > 650) { //Если напряжение меньше 12.0В и обороты > 650 об/мин - выводим сообщее об отсутствии зарядки
      sprintf(output_string_1, "  NO CHARGING");
      Print_Digits(output_string_2, 4, 2, ' ', Voltage / 50); //(указать на строку, начальная позиция, количество симвволов на число, заполняющий символ, число для вывода)
      //      Print_Digits(output_string_2, 7, 1, ' ', (Voltage % 50) / 5);
      output_string_2[7] = (Voltage % 50) / 5 + '0';
      output_string_2[6] = '.';
      output_string_2[8] = 'B';
      Led_Status = !Led_Status;
      digitalWrite(13, Led_Status);
      goto LCD_print;
    }
    if (Voltage > 775) { //Если напряжение больше 15.5В - выводим сообщее об перезарядке
      sprintf(output_string_1, "  OVERCHARGING");
      Print_Digits(output_string_2, 4, 2, ' ', Voltage / 50); //(указать на строку, начальная позиция, количество символов на число, заполняющий символ, число для вывода)
      //      Print_Digits(output_string_2, 7, 1, ' ', (Voltage % 50) / 5);
      output_string_2[7] = (Voltage % 50) / 5  + '0';
      output_string_2[6] = '.';
      output_string_2[8] = 'B';
      Led_Status = !Led_Status;
      digitalWrite(13, Led_Status);
      goto LCD_print;
    }
    if (EngineTemp > 105) { //Если температура больше 105С - выводим сообщее об перегреве
      sprintf(output_string_1, "   HIGH TEMP");
      Print_Digits(output_string_2, 6, 3, ' ', EngineTemp); //(указать на строку, начальная позиция, количество симвволов на число, заполняющий символ, число для вывода)
      output_string_2[9] = 0xDF;//значок градуса
      output_string_2[10] = 'C';
      Led_Status = !Led_Status;
      digitalWrite(13, Led_Status);
      goto LCD_print;
    }
    if (Led_Status) { //гасим светодиод
      Led_Status = LOW;
      digitalWrite(13, Led_Status);
    }

    //Блок проверки состояния кнопки управления компьютером
    //    if (!digitalRead(ButonsPin)) Butons_Timer++; //проверка нажатия кнопки
    if (analogRead(ButonsPin) < 100) Butons_Timer++; //проверка нажатия кнопки
    else {
      if (Butons_Timer < 10 && Butons_Timer > 0) { //Краткое нажатие переключает страницы
        LCD_Page++;
        if (LCD_Page > 6) LCD_Page = 0;
      }
      Butons_Timer = 0;
    }
    if (Butons_Timer > 10) //Длительное нажатие - сброс показаний
      switch (LCD_Page) {
        case 0:
          Odometr = 0;
          break;
        case 1:
          Odometr_Gas = 0;
          Odometr_Time_Gas = 0;
          EEPROM.put(Volume_Gas_EEPROM_adres, GasLevel); //сохраняем текущий объем газа
          EEPROM.put(CheckPoint_Gas_EEPROM_adres, Odometr_Gas); //сохраняем текущее значение пробега на газу
          break;
        case 2:
          EEPROM.put(Volume_Gas_EEPROM_adres, GasLevel); //сохраняем текущий объем газа
          EEPROM.put(CheckPoint_Gas_EEPROM_adres, Odometr_Gas); //сохраняем текущее значение пробега на газу
          break;
        case 3:
          Odometr_Benz = 0;
          Odometr_Time_Benz = 0;
          EEPROM.put(Volume_Benz_EEPROM_adres, BenzLevel); //сохраняем текущий объем бензина
          EEPROM.put(CheckPoint_Benz_EEPROM_adres, Odometr_Benz); //сохраняем текущее значение пробега на бензине
          break;
        case 4:
          EEPROM.put(Volume_Benz_EEPROM_adres, BenzLevel); //сохраняем текущий объем бензина
          EEPROM.put(CheckPoint_Benz_EEPROM_adres, Odometr_Benz); //сохраняем текущее значение пробега на бензине
          break;
      }

    //блок считывания текушего времени и даты
    byte Second, Minute, Hour, Day, Month, Year;
    Wire.beginTransmission(0x68);
    Wire.write((byte)0);
    Wire.endTransmission();
    Wire.requestFrom(0x68, 7);
    //    if (Wire.available() < 7) return;
    Second = bcd2dec(Wire.read() & 0x7f);
    Minute = bcd2dec(Wire.read() );
    Hour =   bcd2dec(Wire.read() & 0x3f);  // mask assumes 24hr clock
    Wire.read();
    Day = bcd2dec(Wire.read() );
    Month = bcd2dec(Wire.read() );
    Year = bcd2dec(Wire.read() ); //отсчет с 2000 года

    //Подготовка информации для вывода на дисплей
    switch (LCD_Page) {
      case 0:    // Страница 0  -  Основная страница
        Print_Digits(output_string_1, 0, 3, ' ', EngineTemp); //(указатель на строку, начальная позиция, количество симвволов на число, заполняющий символ, число для вывода)
        Print_Digits(output_string_1, 4, 2, '0', Hour);
        output_string_1[6] = ':';
        Print_Digits(output_string_1, 7, 2, '0', Minute);
        Print_Digits(output_string_1, 10, 4, ' ', analogRead(GasLevelPin));  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1



        Print_Digits(output_string_1, 15, 2, ' ', GasLevel);







        /*   if (Odometr > 9999999) Print_Digits(output_string_1, 9, 7, ' ', Odometr / 1000);
           else {
             if (Odometr > 99999) {
               Print_Digits(output_string_1, 10, 4, ' ', Odometr / 1000);
               output_string_1[14] = '.';
               output_string_1[15] = (Odometr % 1000) / 100 + '0';
             }
             else {
               Print_Digits(output_string_1, 10, 2, ' ', Odometr / 1000);
               output_string_1[12] = '.';
               Print_Digits(output_string_1, 13, 3, '0', Odometr % 1000);
             }
           }  */
        Print_Digits(output_string_2, 0, 4, ' ', _Tahometr());
        Print_Digits(output_string_2, 5, 2, ' ', Voltage / 50);
        output_string_2[7] = '.';
        //Print_Digits(output_string_2, 8, 1, ' ', (Voltage % 50) / 5);
        output_string_2[8] = (Voltage % 50) / 5 + '0'; //экономим 14 байт и время
        output_string_2[9] = 'B';
        Print_Digits(output_string_2, 11, 2, ' ', BenzLevel / 4);
        output_string_2[13] = 0xA7;//L
        //        Print_Digits(output_string_2, 11, 5, ' ', Speed_of_program);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //        Speed_of_program = 0;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        break;
      case 1:    // Страница 1  -  Газ: одометр / время
        if (Odometr_Gas > 99999999) Print_Digits(output_string_1, 4, 9, ' ', Odometr_Gas / 1000);
        else {
          Print_Digits(output_string_1, 4, 5, ' ', Odometr_Gas / 1000);
          Print_Digits(output_string_1, 10, 3, '0', Odometr_Gas % 1000);
        }
        output_string_1[0] = 0xA1; //G
        output_string_1[1] = 'a';  //a
        output_string_1[2] = 0xB7; //z
        output_string_1[9] = '.';  //
        output_string_1[14] = 0xBA;//k
        output_string_1[15] = 0xBC;//m

        Calc_Time(Odometr_Time_Gas / 1000);
        break;
      case 2:    // Страница 2  -  Газ: расход / прогноз пробега до заправки
        Fuel_Consumption(Volume_Gas_EEPROM_adres, CheckPoint_Gas_EEPROM_adres, GasLevel, Odometr_Gas); //вычисляем расход топлива, и прогноз пробега
        Print_Digits(output_string_1, 10, 3, ' ', Rashod / 10);
        //Print_Digits(output_string_1, 14, 1, ' ', Rashod % 10);
        output_string_1[14] = Rashod % 10 + '0';
        Print_Digits(output_string_2, 10, 4, ' ', Prognoz);

        output_string_1[0] = 0xA1; //g
        output_string_1[1] = 'a';  //a
        output_string_1[2] = 0xB7; //z
        output_string_1[5] = 'p';  //r
        output_string_1[6] = 'a';  //a
        output_string_1[7] = 'c';  //s
        output_string_1[8] = 'x';  //h
        output_string_1[9] = '.';  //
        output_string_1[13] = '.'; //
        output_string_1[15] = 0xBB;//l
        output_string_2[4] = 0xE3; //d
        output_string_2[5] = 'o';  //o
        output_string_2[7] = 'A';  //A
        output_string_2[8] = 0xA4; //Z
        output_string_2[9] = 'C';  //S
        output_string_2[14] = 0xBA;//k
        output_string_2[15] = 0xBC;//m
        break;
      case 3:    // Страница 3  -  Бензин: одометр / время
        if (Odometr_Benz > 99999999) Print_Digits(output_string_1, 4, 9, ' ', Odometr_Benz / 1000);
        else
        {
          Print_Digits(output_string_1, 4, 5, ' ', Odometr_Benz / 1000);
          Print_Digits(output_string_1, 10, 3, '0', Odometr_Benz % 1000);
        }
        output_string_1[0] = 0xA0; //B
        output_string_1[1] = 'e';  //e
        output_string_1[2] = 0xBD; //n
        output_string_1[3] = 0xB7; //z
        output_string_1[9] = '.';  //
        output_string_1[14] = 0xBA;//k
        output_string_1[15] = 0xBC;//m

        Calc_Time(Odometr_Time_Benz / 1000);
        break;
      case 4:    // Страница 4  -  Бензин: расход / прогноз пробега до заправки
        Fuel_Consumption(Volume_Benz_EEPROM_adres, CheckPoint_Benz_EEPROM_adres, BenzLevel, Odometr_Benz); //вычисляем расход топлива, и прогноз пробега
        Print_Digits(output_string_1, 10, 3, ' ', Rashod / 10);
        //Print_Digits(output_string_1, 14, 1, ' ', Rashod % 10);
        output_string_1[14] = Rashod % 10 + '0';
        Print_Digits(output_string_2, 10, 4, ' ', Prognoz);

        output_string_1[0] = 0xA0; //B
        output_string_1[1] = 'e';  //e
        output_string_1[2] = 0xBD; //n
        output_string_1[3] = 0xB7; //z
        output_string_1[5] = 'p';  //r
        output_string_1[6] = 'a';  //a
        output_string_1[7] = 'c';  //s
        output_string_1[8] = 'x';  //h
        output_string_1[9] = '.';  //
        output_string_1[13] = '.'; //
        output_string_1[15] = 0xBB;//l
        output_string_2[4] = 0xE3; //d
        output_string_2[5] = 'o';  //o
        output_string_2[7] = 'A';  //A
        output_string_2[8] = 0xA4; //Z
        output_string_2[9] = 'C';  //S
        output_string_2[14] = 0xBA;//k
        output_string_2[15] = 0xBC;//m
        break;
      case 5:    // Страница 5  -  Время / Дата
        Print_Digits(output_string_1, 4, 2, ' ', Hour);
        output_string_1[6] = ':';
        Print_Digits(output_string_1, 7, 2, '0', Minute);
        output_string_1[9] = ':';
        Print_Digits(output_string_1, 10, 2, '0', Second);

        Print_Digits(output_string_2, 3, 2, ' ', Day);
        output_string_2[5] = '/';
        Print_Digits(output_string_2, 6, 2, '0', Month);
        output_string_2[8] = '/';
        Print_Digits(output_string_2, 9, 4, '0', Year + 2000);
        break;
      case 6:    // Страница 6  -  Значения АЦП
        Print_Digits(output_string_1, 0, 4, ' ', analogRead(VoltagePin)); //(указатель на строку, начальная позиция, количество симвволов на число, заполняющий символ, число для вывода)
        output_string_1[5] = 'V';
        Print_Digits(output_string_1, 7, 4, ' ', analogRead(EngineTempPin)); //(указатель на строку, начальная позиция, количество симвволов на число, заполняющий символ, число для вывода)
        output_string_1[12] = 'T';
        //        Print_Digits(output_string_2, 0, 4, ' ', analogRead(BenzLevelPin)); //(указатель на строку, начальная позиция, количество симвволов на число, заполняющий символ, число для вывода)
        //        output_string_2[5] = 'B';
        //        Print_Digits(output_string_2, 7, 4, ' ', analogRead(GasLevelPin)); //(указатель на строку, начальная позиция, количество симвволов на число, заполняющий символ, число для вывода)
        //        output_string_2[12] = 'G';
        //температура
        Print_Digits(output_string_2, 0, 3, ' ', EngineTemp); //(указатель на строку, начальная позиция, количество симвволов на число, заполняющий символ, число для вывода)


        break;
    }
    //Вывод на дисплей
LCD_print:
    lcd.clear();
    lcd.print(output_string_1);
    lcd.setCursor(0, 1);
    lcd.print(output_string_2);
  }

  //Блок проверки состояния кнопок Газ/Бензин
  if (StartOnBenzFlag) Control_Temp_And_Swith_Fuer(); //Проверка начальной закачки бензина
  else if (!digitalRead(BenzButonPin))              //проверка кнопки "бензин"
    if (ButonTimer > 10) {                             //защита от дребезга контактов: 10 проверок подряд должны показать что кнопка нажата
      FromBenzToGasFlag = false;  //сбрасываем флаг процесса перехода с бензина на газ
      //        FromGasToBenzFlag = false;  //сбрасываем флаг процесса перехода с газа на бензин
      digitalWrite(GasPin, LOW);  //выключаем газ
      CurentFuer = false;         //Устанавливаем текущее топливо бензин
      _Benz();                    //включаем бензин
    }
    else ButonTimer++;
  else if (!digitalRead(GasButonPin)) //проверка кнопки "газ"
    if (ButonTimer > 10) {             //защита от дребезга контактов: 10 проверок подряд должны показать что кнопка нажата
      FromBenzToGasFlag = false;  //сбрасываем флаг процесса перехода с бензина на газ
      //          FromGasToBenzFlag = false;  //сбрасываем флаг процесса перехода с газа на бензин
      digitalWrite(BenzPin, LOW); //выключаем бензин
      CurentFuer = true;          //Устанавливаем текущее топливо газ
      _Gas();                     //включаем газ
    }
    else ButonTimer++;
  else {
    ButonTimer = 0;                 //кнопки не нажаты, сбрасывем счетчик нажатий
    Control_Temp_And_Swith_Fuer();  //функция контроля температуры и переключения видов топлива
  }









  //  Speed_of_program++;




} //конец программы

//Прерывания----------------------------------------------------------------
void _TahometrInterrupt()
{
  t++;
  if ( t == 11 ) t = 0;
  TahometrTimer[t] = millis();
}

void _OdometrInterrupt()
{
  o++;
  if ( o == 11 ) o = 0;
  OdometrTimer[o] = millis(); // Для вычисления скорости
  OdometrVar++; // Для вычисления пройденого расстояния
}
