#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd (0x27,16,2); 

#define RED_LED_PIN 8


RF24 radio(9, 10); // Пины подключения модуля NRF24L01

const byte address[6] = "00101";  // Адрес для передачи данных


int xAxisPin = A2; // Пин для оси X джойстика
int yAxisPin = A1; // Пин для оси Y джойстика
int buttonLeftPin = 1; // Пин кнопки для движения влево боком
int buttonRightPin = 2; // Пин кнопки для движения вправо боком
int buttonTowerLeft = 3; // Пин кнопки для поворота башни влево
int buttonTowerRight = 4; // Пин кнопки для поворота башни вправо
int buttonWhiteLed = 5; 
int buttonRGBLed = 6; 
int buttonRGBLedChange = 7;

int prevButtonStateLeft = HIGH;
int prevButtonStateRight = HIGH;
int prevButtonStateTowerLeft = HIGH;
int prevButtonStateTowerRight = HIGH;
int prevButtonStateWhiteLed = HIGH;
int prevButtonStateRgbLed = HIGH;
int prevButtonStateRgbLedChange = HIGH;

int xAxisValue, yAxisValue; // Переменные для хранения значений осей
bool moveLeft = false;
bool moveRight = false;
int driveNum=0;
int towerRotate=0;
int whiteLedOn=0;
int rgbLedOn=0;
int rgbColor=1;


void setup() {
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW); // Выбор мощности передачи (можно изменить)
  radio.stopListening();

  pinMode(buttonLeftPin, INPUT_PULLUP);
  pinMode(buttonRightPin, INPUT_PULLUP);
  pinMode(buttonTowerLeft, INPUT_PULLUP);
  pinMode(buttonTowerRight, INPUT_PULLUP);
  pinMode(buttonWhiteLed, INPUT_PULLUP);
  pinMode(buttonRGBLed, INPUT_PULLUP);
  pinMode(buttonRGBLedChange, INPUT_PULLUP);
  pinMode(RED_LED_PIN, OUTPUT); // Установка пина красного светодиода как выхода

  digitalWrite(RED_LED_PIN, HIGH); // Включение красного светодиода при запуске
  
  lcd.begin(16, 2);

   lcd.init();    // инициализация LCD дисплея
   lcd.backlight(); // включение подсветки дисплея

   lcd.blink(); // включение мигающего курсора
   delay(500);
   lcd.noBlink();
   
   lcd.setCursor(1, 0);    // ставим курсор на 1 символ первой строки
   lcd.print("DEVICE");   // печатаем сообщение на первой строке
   lcd.setCursor(8, 1);        // ставим курсор на 1 символ второй строки
   lcd.print("IS ON");  // печатаем сообщение на второй строке
   delay(500);

   lcd.cursor(); // включение текущей позиции курсора
   delay(500);
   lcd.noCursor();
}

void loop() {

  displayInfoOnLCD();

  // Отправка данных на машинку
  
  checkButtons();
  drive();
  send();


  delay(50); // Задержка для стабилизации передачи данных
}

// Функция для отправки данных через радио модуль NRF24L01
void send() {
  int dataToSend[5] = {driveNum, towerRotate, whiteLedOn, rgbLedOn, rgbColor};
  radio.write(&dataToSend, sizeof(dataToSend));
}


void drive() {

    // Чтение значений с джойстика
  xAxisValue = analogRead(xAxisPin);
  yAxisValue = analogRead(yAxisPin);

  if (digitalRead(buttonLeftPin) == LOW) {
    moveLeft = true;
    moveRight = false;
  } else if (digitalRead(buttonRightPin) == LOW) {
    moveRight = true;
    moveLeft = false;
  } else {
    moveLeft = false;
    moveRight = false;
  }

  // Преобразование значений диапазона оси X (0-1023) в диапазон (-255, 255)
  int x = map(xAxisValue, 0, 1023, -255, 255);
  // Преобразование значений диапазона оси Y (0-1023) в диапазон (-255, 255)
  int y = map(yAxisValue, 0, 1023, -255, 255);



  if (y<50 && y>-50 && x<50 && x>-50 && moveLeft && !moveRight) {
    driveNum=10;
  } else if (y<50 && y>-50 && x<50 && x>-50 && moveRight && !moveLeft) {
    driveNum=9;
  } else if (y>=50 && x<50 && x>-50) {
    driveNum=2;
  } else if (y<=-50 && x<50 && x>-50) {
    driveNum=1;
  } else if (y>=50 && x<=-50) {
    driveNum=6;
  } else if (y>=50 && x>=50) {
    driveNum=5;
  } else if (y<=-50 && x<=-50) {
    driveNum=4;
  } else if (y<=-50 && x>=50) {
    driveNum=3;
  } else if (y>-50 && y<50 && x<=-50) {
    driveNum=8;
  } else if (y>-50 && y<50 && x>=50) {
    driveNum=7;
  } else {
    driveNum=0;
  }

}


void displayInfoOnLCD() {
  lcd.clear(); // Очистка дисплея

  // Отображение информации о белых светодиодах
  lcd.setCursor(0, 0);
  lcd.print("r ");
  if (driveNum==0) {
    lcd.print("stop");
  } else if (driveNum==1) {
    lcd.print("vpered");
  } else if (driveNum==2) {
    lcd.print("nazad");
  } else if (driveNum==3) {
    lcd.print("vlevo vpered");
  } else if (driveNum==4) {
    lcd.print("vpavo vpered");
  } else if (driveNum==5) {
    lcd.print("vlevo nazad");
  } else if (driveNum==6) {
    lcd.print("vpravo nazad");
  } else if (driveNum==7) {
    lcd.print("vlevo krug");
  } else if (driveNum==8) {
    lcd.print("vpravo krug");
  } else if (driveNum==9) {
    lcd.print("bokom vlevo");
  } else if (driveNum==10) {
    lcd.print("bokom vpravo");
  } else {
    lcd.print("hz");
  }

  
 /* if (whiteLedOn == 0) {
    lcd.print("Off");
  } else {
    lcd.print("On ");
  }

  // Отображение информации о RGB-светодиоде
  lcd.setCursor(0, 1);
  lcd.print("RGB LED: ");
  if (rgbLedOn == 0) {
    lcd.print("Off");
  } else {
    switch (rgbColor) {
      case 1:
        lcd.print("Red");
        break;
      case 2:
        lcd.print("Green");
        break;
      case 3:
        lcd.print("Blue");
        break;
      default:
        lcd.print("Unknown");
        break;
    }
  } */
}

void checkButtons() {
  int currentButtonStateLeft = digitalRead(buttonLeftPin);
  int currentButtonStateRight = digitalRead(buttonRightPin);
  int currentButtonStateTowerLeft = digitalRead(buttonTowerLeft);
  int currentButtonStateTowerRight = digitalRead(buttonTowerRight);
  int currentButtonStateWhiteLed = digitalRead(buttonWhiteLed);
  int currentButtonStateRgbLed = digitalRead(buttonRGBLed);
  int currentButtonStateRgbLedChange = digitalRead(buttonRGBLedChange);

 /*
  if (currentButtonStateLeft == LOW && prevButtonStateLeft == HIGH) {
    moveLeft = true;
    moveRight = false;
  } else if (currentButtonStateRight == LOW && prevButtonStateRight == HIGH) {
    moveRight = true;
    moveLeft = false;
  } else {
    moveLeft = false;
    moveRight = false;
  }
 */

   if (digitalRead(buttonTowerLeft) == LOW) {
    towerRotate=1;
  } else if (digitalRead(buttonTowerRight) == LOW) {
    towerRotate=2;
  } else {
    towerRotate=0;
  }

  if (currentButtonStateWhiteLed == LOW && prevButtonStateWhiteLed == HIGH) {
    whiteLedOn = (whiteLedOn == 0) ? 1 : 0;
  }

  if (currentButtonStateRgbLed == LOW && prevButtonStateRgbLed == HIGH) {
    rgbLedOn = (rgbLedOn == 0) ? 1 : 0;
  }

  if (currentButtonStateRgbLedChange == LOW && prevButtonStateRgbLedChange == HIGH && rgbLedOn == 1) {
    if (rgbColor == 1) {
      rgbColor = 2;
    } else if (rgbColor == 2) {
      rgbColor = 3;
    } else if (rgbColor == 3) {
      rgbColor = 1;
    }
  }

    if (rgbLedOn == 0) {
    rgbColor = 1;
  }

  // Обновляем предыдущее состояние кнопок
  prevButtonStateLeft = currentButtonStateLeft;
  prevButtonStateRight = currentButtonStateRight;
  prevButtonStateWhiteLed = currentButtonStateWhiteLed;
  prevButtonStateRgbLed = currentButtonStateRgbLed;
  prevButtonStateRgbLedChange = currentButtonStateRgbLedChange;
}



