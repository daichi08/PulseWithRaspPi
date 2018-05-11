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

#define DELAY 5 //[ms]

int pi;
extern int pi;

static int pwmpin[2] = {18, 19};
static int dirpin[2] = {20, 21};

static int advance[2]    = {FORWARD, REVERSAL};
static int backspace[2]  = {REVERSAL, FORWARD};
static int turn_right[2] = {FORWARD, FORWARD};
static int turn_left[2]  = {REVERSAL, REVERSAL};

void delay_ms(unsigned int time_ms){
  time_sleep(((double)time_ms) / 1000.0);
}

int main(int argc, char **argv){
  int hz[2]          = {0,0};
  int current_hz[2]  = {0,0};
  int dir[2]         = {FORWARD,FORWARD};
  int current_dir[2] = {FORWARD,FORWARD};
  int motor_num;
  bool diff_flg[2] = {false,false};
  char c;

  pi = pigpio_start("localhost","8888");

  set_mode(pi, pwmpin[0], PI_OUTPUT);
  set_mode(pi, dirpin[0], PI_OUTPUT);
  set_mode(pi, pwmpin[1], PI_OUTPUT);
  set_mode(pi, dirpin[1], PI_OUTPUT);
  delay_ms(50);

  while(c != 'q'){
    do{
      printf("which motor? 0 or 1 or 2\n");
      scanf("%d", &motor_num);
    }while(motor_num != 0 && motor_num != 1 && motor_num != 2);

    if(motor_num != 2){
      do{
        printf("frequency[Hz]?(more equal %d[Hz])\n",LIMIT);
        scanf("%d", &hz[motor_num]);
      }while(hz[motor_num] < LIMIT || 4096 < hz[motor_num]);
      
      printf("dir? f or r or q\n");
      do{
        c = getchar();
      }while(c != 'f' && c != 'r' && c != 'q' );

      if (c == 'f') dir[motor_num] = FORWARD;
      else if (c == 'r') dir[motor_num] = REVERSAL;
      else break;

    }else{
      printf("where to go? a or b or r or l or q\n");
      do{
        c = getchar();
      }while(c != 'a' && c != 'b' && c != 'r' && c != 'l' && c != 'q');

      do{
        printf("frequency[Hz]?(more equal %d[Hz])\n",LIMIT);
        scanf("%d", &hz[0]);
      }while(hz[0] < LIMIT || 4096 < hz[0]);
      hz[1] = hz[0];

      if(c == 'a'){
        dir[0] = advance[0];
        dir[1] = advance[1];
      }else if(c == 'b'){
        dir[0] = backspace[0];
        dir[1] = backspace[1];
      }else if(c == 'r'){
        dir[0] = turn_right[0];
        dir[1] = turn_right[1];
      }else if(c == 'l'){
        dir[0] = turn_left[0];
        dir[1] = turn_left[1];
      }else{
        break;
      }
    }

    printf("hz[0] = %d,hz[1] = %d\n", hz[0],hz[1]);
    printf("dir[0] = %d, dir[1] = %d\n", dir[0],dir[1]);
    
    for (int i = 0; i < 2; i++){
      if (dir[i] != current_dir[i]){
        diff_flg[i] = true;
      }
    }
    
    if(diff_flg[0] == true){
      gpio_write(pi, dirpin[0], current_dir[0]);
      hardware_PWM(pi, dirpin[0], current_hz[0], HALF);
      current_hz[0] -= 1;
      if(current_hz[0] <= LIMIT) diff_flg[0] = false;
    }else{
      if(current_hz[0] < hz[0]){
        gpio_write(pi, dirpin[0], dir[0]);
        hardware_PWM(pi, pwmpin[0], current_hz[0], HALF);
        if(current_hz[0] < hz[0]) current_hz[0] += 1;
      }else{
        gpio_write(pi, dirpin[0], dir[0]);
        hardware_PWM(pi, pwmpin[0], current_hz[0], HALF);
        if(current_hz[0] > hz[0]) current_hz[0] -= 1;
      }
    }

    if(diff_flg[1] == true){
      gpio_write(pi, dirpin[1], current_dir[1]);
      hardware_PWM(pi, dirpin[1], current_hz[1], HALF);
      current_hz[1] -= 1;
      if(current_hz[1] <= LIMIT) diff_flg[1] = false;
    }else{
      if(current_hz[1] < hz[1]){
        gpio_write(pi, dirpin[1], dir[1]);
        hardware_PWM(pi, pwmpin[1], current_hz[1], HALF);
        if(current_hz[1] < hz[1]) current_hz[1] += 1;
      }else{
        gpio_write(pi, dirpin[1], dir[1]);
        hardware_PWM(pi, pwmpin[1], current_hz[1], HALF);
        if(current_hz[1] > hz[1]) current_hz[1] -= 1;
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