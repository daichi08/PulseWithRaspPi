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
//

#include <stdio.h>
#include <math.h>
#include <pigpiod_if2.h>

#define PWMPIN1 18 //12ピン(物理)
#define PWMPIN2 19 //35ピン(物理)
#define DIRPIN1 15 //10ピン(物理)
int pi;
extern int pi;

void delay_ms(unsigned int time_ms){
  time_sleep(((double)time_ms) / 1000.0);
}

int main(int argc, char **argv){
  int hz = 500;
  pi = pigpio_start("localhost","8888");
  set_mode(pi, PWMPIN1, PI_OUTPUT);
  set_mode(pi, PWMPIN2, PI_OUTPUT);
  /*
    hardware_PWM(pi, PWMPIN1, hz, 50*10000);
    hardware_PWM(pi, PWMPIN2, hz, 50*10000);
  */
  for (int i = hz; i < 3000; ++i)
  {
    printf("current speed = %d [Hz]\n", i);
    hardware_PWM(pi, PWMPIN1, i, 50*10000);
    delay_ms(100);
  }
  /*
    frequency: 0(stop) or 1..125000000(125M) [Hz]
    duty: 0..1000000(1M) -> 1M = 100[%]
  */
  // time_sleep(10); //10[s]パルスを発生
  hardware_PWM(pi, PWMPIN1, 0, 0);
  hardware_PWM(pi, PWMPIN2, 0, 0);
  set_mode(pi, PWMPIN1, PI_INPUT);
  set_mode(pi, PWMPIN2, PI_INPUT);
  pigpio_stop(pi);
}