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

#define LIMIT 200   //下限[Hz]
#define HALF 500000 //duty50[%]

#define FORWARD  PI_LOW  //正転
#define REVERSAL PI_HIGH  //逆転

//command用
#define DRIVE 0
#define BACK  1
#define LEFT  2
#define RIGHT 3

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
    {FORWARD,  FORWARD }, //前進
    {REVERSAL, REVERSAL}, //後退
    {FORWARD,  REVERSAL}, //左回転
    {REVERSAL, FORWARD}  //右回転
  };
  static int command_speed[4][2] = {
    {300, 300}, //前進
    {300, 300}, //後退
    {200, 400}, //左回転
    {400, 200}  //右回転
  };

  int command;
  int dir[2];
  int speed[2] = {LIMIT, LIMIT};
  int current_dir[2];
  int current_speed[2] = {0, 0};
  bool runnning_flg = false;
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

    dir[0]   = command_dir[command][0];
    dir[1]   = command_dir[command][1];
    speed[0] = command_speed[command][0];
    speed[1] = command_speed[command][1];

    if(runnning_flg == false){
      current_dir[0] = dir[0];
      current_dir[1] = dir[1];
      runnning_flg = true;
    }

    gpio_write(pi, dirpin[0], dir[0]);
    gpio_write(pi, dirpin[1], dir[1]);
    hardware_PWM(pi, pwmpin[0], speed[0], HALF);
    hardware_PWM(pi, pwmpin[1], speed[1], HALF);

    current_dir[0]   = dir[0];
    current_dir[1]   = dir[1];
    current_speed[0] = speed[0];
    current_speed[1] = speed[1];

    printf("speed[0]=%d, speed[1]=%d\n", speed[0],speed[1]);
    printf("dir[0]=%d, dir[1]=%d\n", dir[0],dir[1]);

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