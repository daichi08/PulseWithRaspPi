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

#define PWMPIN1 18 //12ピン(物理)
#define PWMPIN2 19 //35ピン(物理)

#define DIRPIN1 21 //40ピン(物理)

#define LIMIT 200   //下限[Hz]
#define HALF 500000 //duty50[%]

#define FORWARD  PI_LOW  //正転
#define REVERSAL PI_HIGH  //逆転

#define DELAY 5 //[ms]

int pi;
extern int pi;

void delay_ms(unsigned int time_ms){
  time_sleep(((double)time_ms) / 1000.0);
}

int main(int argc, char **argv){
  int hz = 0;
  int current_hz = 0;
  int dir = FORWARD;
  int current_dir = FORWARD;
  char c;
  pi = pigpio_start("localhost","8888");
  set_mode(pi, PWMPIN1, PI_OUTPUT);
  set_mode(pi, DIRPIN1, PI_OUTPUT);

  while(c != 'q'){
    do{
      printf("frequency[Hz]?\n");
      scanf("%d", &hz);
    }while(hz < 0 && 4096 < hz);
    printf("dir? f or r\n");
    do{
      c = getchar();
    }while(c != 'f' && c != 'r' && c != 'q' );

    if (c == 'f'){
      dir = FORWARD;
    }else if(c == 'r'){
      dir = REVERSAL;
    }else{
      break;
    }

    // 回転方向切替時処理
    if (dir != current_dir){
      if (dir == FORWARD) gpio_write(pi, DIRPIN1, REVERSAL);
      else                gpio_write(pi, DIRPIN1, FORWARD);

      for (int i = current_hz; i >= LIMIT; --i){
        printf("%d\n", i);
        hardware_PWM(pi, PWMPIN1, i, HALF);
        delay_ms(DELAY);
      }
      gpio_write(pi, DIRPIN1, dir);
      for (int i = LIMIT; i <= hz; ++i){
        printf("%d\n", i);
        hardware_PWM(pi, PWMPIN1, i, HALF);
        delay_ms(DELAY);
      }
      current_hz = hz;
    }

    // 低速すぎると電圧が不安定になるため
    if (current_hz < LIMIT) current_hz = LIMIT;
    if (hz < LIMIT)                 hz = LIMIT;

    // 通常回転時
    gpio_write(pi, DIRPIN1, dir);
    if (current_hz < hz){
      for (int i = current_hz; i <= hz; ++i){
        printf("%d\n", i);
        hardware_PWM(pi, PWMPIN1, i, HALF);
        delay_ms(DELAY);
      }
    }else if(hz < current_hz){
      for (int i = current_hz; i >= hz; --i){
        printf("%d\n", i);
        hardware_PWM(pi, PWMPIN1, i, HALF);
        delay_ms(DELAY);
      }     
    }
    current_hz = hz;
    current_dir = dir;
  }

  if (c == 'q'){
    for (int i = current_hz; i >= LIMIT; --i){
      printf("%d\n", i);
      hardware_PWM(pi, PWMPIN1, i, HALF);
      delay_ms(DELAY);
    }
  }

  // 出力信号の停止
  hardware_PWM(pi, PWMPIN1, 0, 0);
  gpio_write(pi, DIRPIN1, PI_LOW);

  // PINをOUTからINに
  set_mode(pi, PWMPIN1, PI_INPUT);
  set_mode(pi, DIRPIN1, PI_INPUT);

  // 終了
  pigpio_stop(pi);
}