#include <Servo.h>
#include <IRremote.hpp>  
#define DECODE_NEC


Servo leftright;
Servo updown;


int counter;
int angle1 = 90;
int angle2 = 90;
int JS_X = A0;
int JS_Y = A1;
int SW = 4;
int blue = 6, yellow = 8, white = 7;
int receiver = 9;
int shoot = 0;
int mode = 1;
int time;
int width = 950, height = 540;  // total resolution of the video

IRrecv irrecv(receiver);
decode_results results;

typedef struct task {
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);

} task;

const unsigned short tasksNum = 5;
task tasks[tasksNum];

enum SM1_States { SM1_INIT, manual, wireless, automatic};
int Modes(int state1) {
  int selector = map(analogRead(A2), 0, 1023, 1, 4);
  switch (state1) { // State transitions

    case SM1_INIT:
      state1 = manual;
      break;
    case manual:
      if (selector == 1)
        state1 = manual;
      else if (selector == 2)
        state1 = wireless;
      else if (selector >= 3)
        state1 = automatic;
      break;
    case wireless:
      if (selector == 1) {
        state1 = manual;
        leftright.write(90);
        updown.write(90);
      }
      else if (selector == 2)
        state1 = wireless;
      else if (selector >= 3)
        state1 = automatic;
      break;
    case automatic:
      if (selector == 1)
        state1 = manual;
      else if (selector == 2)
        state1 = wireless;
      else if (selector >= 3)
        state1 = automatic;
      break;
  }
  switch (state1) { // State Action
    case SM1_INIT:
      break;
    case manual:
      mode = 1;
      digitalWrite(blue, HIGH);
      digitalWrite(white, LOW);
      digitalWrite(yellow, LOW);
      break;
    case wireless:
      mode = 2;
      digitalWrite(yellow, HIGH);
      digitalWrite(blue, LOW);
      digitalWrite(white, LOW);
      break;
    case automatic:
      mode = 3;
      digitalWrite(white, HIGH);
      digitalWrite(blue, LOW);
      digitalWrite(yellow, LOW);
      break;
  }
  return state1;
}

enum SM2_States { SM2_INIT, off1, on1};
int Manual_Tick(int state2) {
  switch (state2) { // State transitions
    case SM2_INIT:
      state2 = off1;
      break;
    case off1:
      if (mode == 1)
      {
        state2 = on1;
      }
      break;
    case on1:
      int x = analogRead(JS_X);
      int y = analogRead(JS_Y);

      if (digitalRead(SW) == 0)
      {
        shoot = 1;
      }

      if (x < 300)
      {
        //Serial.println("left");
        if (angle1 + 1 >= 180)
        {
          angle1 = 180;
        }
        else
        {
          angle1 += 1;
          leftright.write(angle1);
          //Serial.println(angle1);
        }
      }
      if (x > 800)
      {
        //Serial.println("right");
        if (angle1 - 1 <= 0)
        {
          angle1 = 0;
        }
        else
        {
          angle1 -= 1;
          leftright.write(angle1);
          //Serial.print(angle1);
        }
      }
      if (y < 300)
      {
        
        //Serial.println("up");
        if (angle2 - 2 <= 80)
        {
          angle2 = 80;
        }
        else
        {
          angle2 -= 2;
          updown.write(angle2);
          //Serial.println(angle2);
        }
      }
      if (y > 700)
      {
        
        //Serial.println("down");
        if (angle2 + 2 >= 110)
        {
          angle2 = 110;
        }
        else
        {
          angle2 += 2;
          updown.write(angle2);
          //Serial.println(angle2);
        }
      }
      
      if (mode != 1)
      {
        state2 = off1;
      }
      break;
  }
  return state2;
}

enum SM4_States { SM4_INIT, off2, on2};
int Automatic_Tick(int state4) {
  switch (state4) { // State transitions
    case SM4_INIT:
      state4 = off2;
    case off2:
      if (mode == 3)
      {
        state4 = on2;
      }
      break;
    case on2:
      if (Serial.available() > 0)
      {
      int x_face, y_face;
      if (Serial.read() == 'X')
      {
        x_face = Serial.parseInt();  
        if (Serial.read() == 'Y')
          y_face = Serial.parseInt(); 
        }

      if (x_face > width / 2)
        angle1 -= 3;
      if (x_face < width / 2)
        angle1 += 3;
      if (y_face < height / 2)
        angle2 += 1;
      if (y_face > height / 2)
        angle2 -= 1;


    
      if (angle1 >= 180)
        angle1 = 180;
      else if (angle1 <= 0)
        angle1 = 0;
      if (angle2 >= 110)
        angle2 = 110;
      else if (angle2 <= 80)
        angle2 = 80;

    leftright.write(angle1);
    updown.write(angle2);
    
    if (distanceBetween(x_face, y_face, 950/2, 540/2) <= 50) {
      counter++;
      if (counter >= 5)
      {
        shoot = 1;
      }
    }
    else {
      counter = 0;
    }
      
     } 
     if (mode != 3)
      {
        state4 = off2;
      }
     break;
  }
  return state4;

}

enum SM3_States { SM3_INIT, shootDetect};
int Shoot(int state3) {
  switch (state3) { // State transitions
    case SM3_INIT:
      state3 = shootDetect;
      break;
    case shootDetect:
      
      if (shoot == 1)
      {
        time++;
        digitalWrite(5, HIGH);
        if (time >= 100)
        {
          shoot = 0;
          digitalWrite(5, LOW);
          time = 0;
        }        
      }
      break;

  }
  return state3;
}



enum SM5_States { SM5_INIT, off3, on3};
int Wireless_Tick(int state5) {
  switch (state5) { // State transitions
    case SM5_INIT:
      state5 = off3;
      break;
    case off3:
      if (mode == 2)
      {
        state5 = on3;
      }
      break;
    case on3:
      if (IrReceiver.decode()) {
        if (IrReceiver.decodedIRData.protocol == NEC)
        {
          
          if (IrReceiver.decodedIRData.command == 0x44) // left
          {

            if (angle1 + 3 >= 180)
            {
              angle1 = 180;
            }
            else
            {
              angle1 += 3;
              leftright.write(angle1);
            }
          }
          else if (IrReceiver.decodedIRData.command == 0x43) //right
          {

            if (angle1 - 3 <= 0)
            {
              angle1 = 0;
            }
            else
            {
              angle1 -= 3;
              leftright.write(angle1);
              //Serial.print(angle1);
            }
          }
          else if (IrReceiver.decodedIRData.command == 0x46) //up
          {

            if (angle2 - 3 <= 80)
            {
              angle2 = 80;
            }
            else
            {
              angle2 -= 3;
              updown.write(angle2);
              //Serial.println(angle2);
            }
          }
          else if (IrReceiver.decodedIRData.command == 0x15) //down
          {

            if (angle2 + 3 >= 110)
            {
              angle2 = 110;
            }
            else
            {
              angle2 += 3;
              updown.write(angle2);
              //Serial.println(angle2);
            }
          }
          else if (IrReceiver.decodedIRData.command == 0x40)
          {
            shoot = 1;
          }
          
        }
        IrReceiver.resume(); // Enable receiving of the next value
      }
      if (mode != 2)
      {
        state5 = off3;
      }
      break;
      }
  
  return state5;

}



void setup() {
  
  leftright.attach(2);
  updown.attach(3);
  leftright.write(angle1);
  updown.write(angle2);
  IrReceiver.begin(receiver, ENABLE_LED_FEEDBACK);

  pinMode(4, INPUT_PULLUP);
  pinMode(5, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(white, OUTPUT);



  // put your setup code here, to run once:
  unsigned char i = 0;
  tasks[i].state = SM1_INIT;
  tasks[i].period = 500;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Modes;
  i++;
  tasks[i].state = SM2_INIT;
  tasks[i].period = 50;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Manual_Tick;
  i++;
  tasks[i].state = SM3_INIT;
  tasks[i].period = 50;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Shoot;
  i++;
  tasks[i].state = SM4_INIT;
  tasks[i].period = 20;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Automatic_Tick;
  i++;
  tasks[i].state = SM5_INIT;
  tasks[i].period = 50;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Wireless_Tick;

  Serial.begin(9600);

}

void loop() {
  unsigned char i;
  for (i = 0; i < tasksNum; ++i) {
    if ( (millis() - tasks[i].elapsedTime) >= tasks[i].period) {
      tasks[i].state = tasks[i].TickFct(tasks[i].state);
      tasks[i].elapsedTime = millis(); // Last time this task was ran
    }
  }
}


int distanceBetween(int x1, int y1, int x2, int y2)
{
  return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
}






