// Arduino  D
#include "EasyNextionLibrary.h"  
#include <string.h>              
#include <SoftwareSerial.h>      

#define PREPAIRING -1
#define WAITING 0
#define BATTLE_START 1

int state = PREPAIRING;

EasyNex myNex(Serial); 

SoftwareSerial mySerial(5, 6);  // для передачи по blt

int rx = 6;
int tx = 5;
const int REFRESH_TIME = 1000;           
unsigned long refresh_timer = millis();  

String stringFromNextion;
bool flag = 1, state1 = 0;

const int N = 4;  // Размер для матрицы
int ArrayOur[N][N];
int ArrayEnemy[16];
int k = 0, c = 0;

// Переменный для счетчика при получении массива.
int jCount = 0;
int iCount = 0;
// Счетчик проверки полученного массива.
int Check = 0;
// Счетчик для переключения актов.
int Count = 0;

int Act = 1;  // Акты нужны для распределения ролей при передаче массива.

void setup() {
  pinMode(tx, INPUT);   // было rx
  pinMode(rx, OUTPUT);  // было tx
  pinMode(13, OUTPUT);
  pinMode(3, INPUT_PULLUP);

 // Для монитора порта 
  Serial.begin(9600); 
  // Для экранчика
  myNex.begin(9600);
  Serial1.begin(9600);
  // Для передачи данных по blt
  mySerial.begin(9600);

}

void loop() {

  myNex.NextionListen();

  switch (state) {
      //case PREPAIRING:

    case WAITING:
      //действия при состоянии WAITING:
      // Act 1 - Отправляем наш массив врагу.
      // Act 2 - Получаем вражеский массив.
      // Act 3 - Обмен массивами завершён.
      // Act 4 - Прорисовываем своё поле на page01 в Nextion.
      // Act 5 - Меняем состояние на BATTLE_START, Act зануляем.

      if (Act == 1) {
        for (int i = 0; i < 4; i++) {
          for (int j = 0; j < 4; j++) {
            char symbol = ArrayOur[i][j] + '0';
            Count++;
            mySerial.print(symbol);
          }
        }
        if (Count == 16) {
          Serial.println("Our array Sent");
          Count = 0;
          Act = 2;
        }
        delay(1000);
      }
      if (Act == 2) {
        Serial.println("Act 2");
        if (mySerial.available()) {
          char message = mySerial.read();
          ArrayEnemy[Count] = message - '0';
          Serial.println(ArrayEnemy[Count]);
          Count++;
        }
        if (Count == 16) {  // Было Count == 15
          Serial.println("Enemy array received");
          Count = 0;
          Act = 3;
          Serial.println("Enemy array proverka:");
        }
      }

      if (Act == 3) {
        // Проверка на успешность принятия вражеского массива.
        for (int i = 0; i < 16; i++) {
          Serial.print(ArrayEnemy[i]);
          if (ArrayEnemy[i] != 0) Check++;
        }
        if (Check == 5) {
          Serial.println();
          Serial.println("Array exchange completed.");
          Check = 0;
          Act = 4;
        }
      }
      if (Act == 4) {
        Serial.println("Obmen srabotal");
        for (int i = 0; i < 16; i++) {
          Serial.print(ArrayEnemy[i]);
          Count++;
        }

        if (Count == 16) {
          Count = 0;
          state = BATTLE_START;
        }
      }

      break;

    case BATTLE_START:

    //while (myNex.currentPageId != 1)
   // {
      //myNex.writeStr("page 1");
      myNex.writeStr("page page01");
      myNex.writeStr("t16.txt", "9");
      //Serial.println();

    //}
    //Serial.println("loop end");
      break;
  }
}

void trigger0() {
  if (state1 == 0) {
    Serial.println("111");
    for (int i = 0; i < 16; i++) {
      String myString = String(i);
      String sendString = "b" + myString + ".txt";
      String txt = myNex.readStr(sendString);
      int a = txt.toInt();
      ArrayOur[k][c] = a;
      c += 1;
      if (c % 4 == 0) {
        k += 1;
        c = 0;
      }
    }

    Serial.println("\nmartix out");

    // Если двухпалубный корабль стоит в начале массива.
    int CountNumberTwo = 0;
    int CountNumberOne = 0;

    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        if (ArrayOur[i][j] == 2) CountNumberTwo++;
        if (ArrayOur[i][j] == 1) CountNumberOne++;
      }
    }
    if (CountNumberTwo != 2) ArrayOur[0][0] = 2;
    else if (CountNumberOne != 3) ArrayOur[0][0] = 1;

    // Вывод полученного массива в монитор порта.
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        Serial.println(ArrayOur[i][j]);
      }
    }

    state = WAITING;
    state1 = 1;
  }
}
