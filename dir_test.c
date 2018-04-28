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

#define DIRPIN1 21 //40ピン(物理)
int pi;
extern int pi;

int main(int argc, char const *argv[])
{
  pi = pigpio_start("localhost",  //引数１：pigpioのデーモンが動いているホストもしくはIPアドレス
            "8888");
  set_mode(pi, DIRPIN1, PI_OUTPUT);
  gpio_write(pi, DIRPIN1, PI_LOW);
  time_sleep(10);
  gpio_write(pi, DIRPIN1, PI_HIGH);
  time_sleep(10);
  gpio_write(pi, DIRPIN1, PI_LOW);
  set_mode(pi, DIRPIN1, PI_INPUT);
}