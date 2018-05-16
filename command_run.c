//--------------------------------------------------------------------------------------------------
//    step_test.c
//
//事前準備(詳しくは"http://abyz.me.uk/rpi/pigpio/pdif2.html"を参照)
//$ sudo pigpiod
//
//
//コンパイル
//$ gcc -Wall -pthread -o 実行ファイル名 プログラム.c ヘッダファイルのＣファイル.c -lpigpiod_if2 -lrt
//
//  -Wall   警告文を出す
//  -pthread  pthreadライブラリの使用 ・・・pigpioのライブラリで使用
//  -lpigpiod_if2 きっとpigpioのライブラリ
//  -lrt    Linuxの高精度タイマ    ・・・コンパイルするのプログラムの後を推奨
//
//実行
//$ ./実行ファイル名
//
//MEMO
//  frequency: 0(stop) or 1..125000000(125M) [Hz]
//  duty: 0..1000000(1M) -> 1M = 100[%]
//

#include <stdio.h>
#include <math.h>
#include <pigpiod_if2.h>
#include <stdbool.h>

//モータの回転方向用
#define FORWARD  PI_LOW  //正転
#define REVERSAL PI_HIGH  //逆転

//モータ用
#define DIR 0
#define SPEED 1

//command用
#define DRIVE 0
#define BACK  1
#define LEFT  2
#define RIGHT 3

//もろもろ
#define LIMIT 250 //下限[Hz]
#define HALF 500000 //duty50[%]
#define DELAY 5 //[ms]


int pi;
extern int pi;

static int pwmpin[2] = {18, 19};
static int dirpin[2] = {21, 20};

void delay_ms(unsigned int time_ms){
  time_sleep(((double)time_ms) / 1000.0);
}

int main(int argc, char **argv){
  static int command_dir[4][2] = {
    {REVERSAL, FORWARD}, //前進
    {FORWARD, REVERSAL}, //後退
    {REVERSAL, FORWARD}, //左回転
    {REVERSAL, FORWARD}  //右回転
  };

  static int command_speed[4][2] = {
    {500, 500}, //前進
    {500, 500}, //後退
    {300, 450}, //左回転
    {450, 300}  //右回転
  };

  //現在の回転方向と回転速度をまとめた配列
  int motor[2][2] = {
    {FORWARD,LIMIT},
    {FORWARD,LIMIT}
  };

  int command;
  int precommand;
  int dir[2];
  int speed[2] = {0, 0};
  bool runnning_flg = false;
  bool inverse_flg = false;
  char c;

  pi = pigpio_start("localhost","8888");
  set_mode(pi, pwmpin[0], PI_OUTPUT);
  set_mode(pi, dirpin[0], PI_OUTPUT);
  set_mode(pi, pwmpin[1], PI_OUTPUT);
  set_mode(pi, dirpin[1], PI_OUTPUT);

  while(c != 'q'){
    printf("Please Enter Command\n");
    printf("Drive(d) or Back(b) or Left(l) or Right(r) or Quit(q)\n");
    do{
      c = getchar();
    }while(c != 'd' && c != 'b' && c != 'l' && c != 'r' && c != 'q');

    switch(c){
      case 'd':
        command = DRIVE;
        break;
      case 'b':
        command = BACK;
        break;
      case 'l':
        command = LEFT;
        break;
      case 'r':
        command = RIGHT;
        break;
      default:
        break;
    }
    if(c != 'q'){
      dir[0] = command_dir[command][0];
      dir[1] = command_dir[command][1];
      speed[0] = command_speed[command][0];
      speed[1] = command_speed[command][1];

      if(runnning_flg == false){
        motor[0][DIR] = dir[0];
        motor[1][DIR] = dir[1];
        precommand = command;
        runnning_flg = true;
      }

      if(precommand == BACK || command == BACK){
        if(precommand != command) inverse_flg = true;
      }

      if(inverse_flg){
        gpio_write(pi, dirpin[0], motor[0][DIR]);
        gpio_write(pi, dirpin[1], motor[1][DIR]);
        while(motor[0][SPEED] > LIMIT || motor[1][SPEED] > LIMIT){
          if(motor[0][SPEED] > LIMIT) motor[0][SPEED] -= 1;
          if(motor[1][SPEED] > LIMIT) motor[1][SPEED] -= 1;

          hardware_PWM(pi, pwmpin[0], motor[0][SPEED], HALF);
          hardware_PWM(pi, pwmpin[1], motor[1][SPEED], HALF);
          delay_ms(50);
          printf("speed[0]=%d, speed[1]=%d\n", motor[0][SPEED],motor[1][SPEED]);
        }
        inverse_flg = false;
      }

      gpio_write(pi, dirpin[0], dir[0]);
      gpio_write(pi, dirpin[1], dir[1]);
      while(motor[0][SPEED] != speed[0] || motor[1][SPEED] != speed[1]){
        if(motor[0][SPEED] < speed[0]){
          motor[0][SPEED] += 1;
        }else if(motor[0][SPEED] > speed[0]){
          motor[0][SPEED] -= 1;
        }

        if(motor[1][SPEED] < speed[1]){
          motor[1][SPEED] += 1;
        }else if(motor[1][SPEED] > speed[1]){
          motor[1][SPEED] -= 1;
        }

        hardware_PWM(pi, pwmpin[0], motor[0][SPEED], HALF);
        hardware_PWM(pi, pwmpin[1], motor[1][SPEED], HALF);
        delay_ms(50);
        
        printf("speed[0]=%d, speed[1]=%d\n", motor[0][SPEED],motor[1][SPEED]);
      }

      motor[0][DIR]   = dir[0];
      motor[1][DIR]   = dir[1];
      motor[0][SPEED] = speed[0];
      motor[1][SPEED] = speed[1];
      precommand = command;
    }else{
      if(motor[0][SPEED] > LIMIT || motor[1][SPEED] > LIMIT){
        while(motor[0][SPEED] > LIMIT || motor[1][SPEED] > LIMIT){
          if(motor[0][SPEED] < LIMIT){
            motor[0][SPEED] += 1;
          }else if(motor[0][SPEED] > LIMIT){
            motor[0][SPEED] -= 1;
          }

          if(motor[1][SPEED] < LIMIT){
            motor[1][SPEED] += 1;
          }else if(motor[1][SPEED] > LIMIT){
            motor[1][SPEED] -= 1;
          }
          hardware_PWM(pi, pwmpin[0], motor[0][SPEED], HALF);
          hardware_PWM(pi, pwmpin[1], motor[1][SPEED], HALF);
          delay_ms(50);
          
          printf("speed[0]=%d, speed[1]=%d\n", motor[0][SPEED],motor[1][SPEED]);
        }
      }
    }
  }

  // 出力信号の停止
  hardware_PWM(pi, pwmpin[0], 0, 0);
  gpio_write(pi, dirpin[0], PI_LOW);
  hardware_PWM(pi, pwmpin[1], 0, 0);
  gpio_write(pi, dirpin[1], PI_LOW);

  // PINOUT -> PININ
  set_mode(pi, pwmpin[0], PI_INPUT);
  set_mode(pi, dirpin[0], PI_INPUT);
  set_mode(pi, pwmpin[1], PI_INPUT);
  set_mode(pi, dirpin[1], PI_INPUT);

  // 終了
  pigpio_stop(pi);
}