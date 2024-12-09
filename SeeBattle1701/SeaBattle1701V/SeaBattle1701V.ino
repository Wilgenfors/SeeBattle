// Arduino  V
#include "EasyNextionLibrary.h"
#include <string.h>

#define PREPAIRING -1
#define WAITING 0
#define BATTLE_START 1
#define ENEMY_TURN 2
#define MY_TURN 3
#define TOTAL_PLAY 4
int state = PREPAIRING;
int k1 = 0;
EasyNex myNex(Serial2);

const int REFRESH_TIME = 100;
unsigned long refresh_timer = millis();

String stringFromNextion;
bool flag = 1, state1 = 0;

bool state3 = 0;
int ArrayOur[16];
int ArrayEnemy[16];
int k = 0, c = 0;
bool triggerReady = false;
// Переменный для счетчика при получении массива.
int jCount = 0;
int iCount = 0;
// Счетчик проверки полученного массива.
int Check = 0;
int tryAim;
// Счетчик для переключения актов.
int Count = 0;
int myCount = 0;
int EnemyCount = 0;
int Act = 1;  // Акты нужны для распределения ролей при передаче массива.

void setup() {
  pinMode(13, OUTPUT);
  pinMode(3, INPUT_PULLUP);

  myNex.begin(9600);    // Для экранчика
  Serial.begin(9600);   // Для вывода в монитор порта
  Serial1.begin(9600);  // Для передачи данных по blt
  Serial2.begin(9600);  // Для монитора порта
}

void loop() {

  myNex.NextionListen();

  switch (state) {
      //case PREPAIRING:

    case WAITING:
      // Act 2 - Получаем вражеский массив.

      if (Act == 2) {
        delay(1000);
        for (int i = 0; i < 16; i++) {
          char symbol = ArrayOur[i] + '0';
          Count++;
          Serial1.print(symbol);
        }
        if (Count == 16) {
          Serial.println("Our array Sent");
          Count = 0;
          Act = 3;
        }
      }

      // Act 1 - Отправляем наш массив врагу.
      if (Act == 1) {
        Serial.println("Act 1");
        if (Serial1.available()) {
          char message = Serial1.read();
          ArrayEnemy[Count] = message - '0';
          Serial.println(ArrayEnemy[Count]);
          Count++;
        }
        if (Count == 16) {  // Было Count == 15
          Serial.println("Enemy array received");
          Count = 0;
          Act = 2;
          Serial.println("Enemy array proverka:");
        }
      }

      // Act 3 - Обмен массивами завершён.
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
      myNex.writeStr("page page01");
      for (int i = 0; i < 16; i++) {
        String s = "b" + String(i) + ".bco";
        if (ArrayOur[i] == 1 || ArrayOur[i] == 2) myNex.writeNum(s, 1055);
      }
      state = ENEMY_TURN;
      break;
    case ENEMY_TURN:
      EnemyCount = 0;
      if (Serial1.available()) {
        int message = Serial1.parseInt();
        String s = "b" + String(message);
        if (ArrayOur[message] == 1) {
          myNex.writeNum(s + ".bco", 63488);
        }
        if (ArrayOur[message] == 2) {
          myNex.writeNum(s + ".bco", 1760);
        }
        if (ArrayOur[message] == 0) {
          myNex.writeStr(s + ".txt", "X");
          state = MY_TURN;
          Serial.println("State-MY_TURN");

          for (int i = 16; i < 32; i++) {
            String s = "b" + String(i);
            String txt = myNex.readStr(s + ".txt");
            int color = myNex.readNumber(s + ".bco");
            if (txt != "X" && color != 1760 && color != 63488) {
              myNex.writeStr("tsw " + s + ",1");
            }
          }
          //счетчик для отображения "Твой ход".
          triggerReady = true;
        }

        for (int i = 0; i < 16; i++) {
          String s = "b" + String(i);
          String txt = myNex.readStr(s + ".txt");
          long color = myNex.readNumber(s + ".bco");
          if (color == 1760 || color == 63488) {
            EnemyCount++;
          }
        }
        Serial.println("EnemyCount");
        Serial.println(EnemyCount);
        Serial.println("Start for Trugger");

        if (EnemyCount == 5) {
          state = TOTAL_PLAY;
        }
        if (triggerReady == true) {
          myNex.writeStr("t17.txt", "Твой ход");
          myNex.writeNum("t17.pco", 1760);
          Serial.println("");
          Serial.println("Enemy missed");

          triggerReady = false;
        }
      }

      break;
    case TOTAL_PLAY:
      if (myCount == 5) {
        myNex.writeStr("page page1");
      }
      if (EnemyCount == 5) {
        myNex.writeStr("page page2");
      }
      break;
  }
}

void trigger1() {
  Serial.println("trigger");
  myCount = 0;
  for (int i = 16; i < 32; i++) {
    String s = "b" + String(i) + ".txt";
    String txt = myNex.readStr(s);
    if (txt == "x") {
      Serial.println("x");
      Serial.println(i);

      tryAim = i - 16;
      Serial1.print(tryAim);

      if (ArrayEnemy[i - 16] == 0) {
        myNex.writeStr(s, "X");
        Serial.println("X");
        myNex.writeStr("t17.txt", "Ход соперника");
        myNex.writeNum("t17.pco", 63488);
        state = ENEMY_TURN;
      }
      if (ArrayEnemy[i - 16] == 1) {
        String s1 = "b" + String(i) + ".bco";
        myNex.writeNum(s1, 63488);
        String s = "b" + String(i) + ".txt";
        myNex.writeStr(s, " ");
        k1 = 1;
      }
      if (ArrayEnemy[i - 16] == 2) {
        String s1 = "b" + String(i) + ".bco";
        myNex.writeNum(s1, 1760);
        String s = "b" + String(i) + ".txt";
        myNex.writeStr(s, " ");
        k1 = 1;
      }

      String s2 = "b" + String(i);
      String s3 = "tsw " + s2 + ",0";
      myNex.writeStr(s3);

      for (int i = 16; i < 32; i++) {
        String s = "b" + String(i);
        String txt = myNex.readStr(s + ".txt");
        long color = myNex.readNumber(s + ".bco");
        if (color == 1760 || color == 63488) {
          myCount++;
        }
      }
      Serial.println("myCount");
      Serial.println(myCount);
      if (myCount == 5) {
        state = TOTAL_PLAY;
      }

      if (k1 == 1) {
        for (int i = 16; i < 32; i++) {
          String s = "b" + String(i);
          String txt = myNex.readStr(s + ".txt");
          int color = myNex.readNumber(s + ".bco");
          myNex.writeStr("tsw " + s + ",1");
        }
      }
      k1 = 0;
      break;
    }
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
      ArrayOur[i] = a;
    }

    Serial.println("\nmartix out");

    // Если двухпалубный корабль стоит в начале массива.
    int CountNumberTwo = 0;
    int CountNumberOne = 0;

    for (int i = 0; i < 16; i++) {
      if (ArrayOur[i] == 2) CountNumberTwo++;
      if (ArrayOur[i] == 1) CountNumberOne++;
    }
    if (CountNumberTwo != 2) ArrayOur[0] = 2;
    else if (CountNumberOne != 3) ArrayOur[0] = 1;

    // Вывод полученного массива в монитор порта.
    for (int i = 0; i < 16; i++) {
      Serial.println(ArrayOur[i]);
    }
    state = WAITING;
    state1 = 1;
  }
}
