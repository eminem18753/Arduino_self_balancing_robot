#include <SoftwareSerial.h>
#include <Wire.h>
#include "Kalman.h" 
#include "I2C.h"
#include "MPU6050.h"
MPU6050 Gyro6050;

//bluetooth receive text
#define MAX_BTCMDLEN 128
char str_receive[MAX_BTCMDLEN];  // received 128 bytes from an Android system
byte str_send[MAX_BTCMDLEN];  // sent 128 bytes from an Arduino system
int len_send = 0; //要傳送的字串長度
int len_receive = 0; //要接收的字串長度

SoftwareSerial BTSerial(A0,13);  // Arduino RX/TX   //to Phone
SoftwareSerial BTSerial2(A2,A1);  // Arduino RX/TX  //to PC

// Create the Kalman instances
Kalman kalmanX; 
Kalman kalmanY;

/* IMU Data */
double accX, accY, accZ;  //三軸加速規值
double gyroX, gyroY, gyroZ;  //三軸陀螺儀值
double distance=0;
int16_t tempRaw;

double gyroXangle, gyroYangle;  // Angle calculate using the gyro only
double compAngleX, compAngleY;  // Calculated angle using a complementary filter(這是另一種濾波器方法，有需要的話也可以用，不過在這邊是沒用到的)
double kalAngleX, kalAngleY;  // Calculated angle using a Kalman filter

uint32_t timer;
uint8_t i2cData[14];  // Buffer for I2C data

//腳位設定
//motor  define
#define PWM_L 10
#define PWM_R 5
#define DIR_L1 4
#define DIR_L2 12
#define DIR_R1 7
#define DIR_R2 8
#define STBY 6  //standby

//encoder define
#define SPD_INT_R 3  //interrupt 
#define SPD_INT_L 2  //interrupt
#define SPD_INT_R2 11
#define SPD_INT_L2 9

#define CPR 374.0  //count per round
#define TIRE_RADIUS 3.45  //輪子半徑

int Speed_L,Speed_R;  //左右輪速度
int pwm,pwm_l,pwm_r;
int t=0;
int x=0;
int s=0;
double Angle_Car;  //車子傾斜角(有Kalman)
double Angle_Car_Adjustment; //初始化調整值
double Gyro_Car;  //車子傾斜角(無Kalman)
double Gyro_Car_Adjustment;  //初始化調整值
float Speed_LR;  //車子整體速度

double KA_P,KA_I,KA_D;  //角度控制PID
double KP_P,KP_I,KP_D;  //位置控制PID
double ANG_OFFSET;  
double Et_total;  //PI累積值

float Speed_Need,Turn_Need;
int Speed_Diff,Speed_Diff_ALL;

uint32_t angle_dt;
uint32_t SendTimer;
uint32_t ReceiveTimer;
uint32_t SendToPCTimer;
uint32_t SpeedTimer;

int encoderPosL, encoderPosR;
int encoderR_past, encoderL_past;

bool First_Time = true;

void setup() {
  
  Serial.begin(115200);
  BTSerial.begin(38400);  // NANO 沒辦法同時處理兩個BT UART傳輸  //bt1
  //BTSerial2.begin(38400);  //bt2
  Init();
  Wire.begin();
  TWBR = ((F_CPU / 400000L) - 16) / 2;  // Set I2C frequency to 400kHz

  while (i2cWrite(0x19, i2cData, 4, false));  // Write to all four registers at once
  while (i2cWrite(0x6B, 0x01, true));  // PLL with X axis gyroscope reference and disable sleep mode
  while (i2cRead(0x75, i2cData, 1));
  
  if (i2cData[0] != 0x68) {  // Read "WHO_AM_I" register
    Serial.print(F("Error reading sensor"));
    while (1);
  }
  delay(200); // Wait for sensor to stabilize

  /* Set kalman and gyro starting angle */
  while (i2cRead(0x3B, i2cData, 6));
  
  accX = (i2cData[0] << 8) | i2cData[1];
  accY = (i2cData[2] << 8) | i2cData[3];
  accZ = (i2cData[4] << 8) | i2cData[5];

  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // It is then converted from radians to degrees
  #ifdef RESTRICT_PITCH  // Eq. 25 and 26
    double roll  = atan2( accY, accZ) * RAD_TO_DEG;
    double pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
  #else  // Eq. 28 and 29
    double roll  = atan( accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
    double pitch = atan2(-accX, accZ) * RAD_TO_DEG;
  #endif

  kalmanX.setAngle(roll);  // Set starting angle
  kalmanY.setAngle(pitch);
  gyroXangle = roll;
  gyroYangle = pitch;
  compAngleX = roll;
  compAngleY = pitch;
 
  attachInterrupt(0, Encoder_L, RISING);  //用中斷來算左輪Encoder值，用來做位置控制
  attachInterrupt(1, Encoder_R, RISING);  //用中斷來算右輪Encoder值，用來做位置控制
  
  timer = micros();
  angle_dt = micros();
  SendTimer = millis();
  ReceiveTimer = millis();
  SendToPCTimer = millis();
  SpeedTimer = millis();
}

void loop() {

  double DataAvg[3];
  double AngleAvg = 0;

  DataAvg[0]=0; DataAvg[1]=0; DataAvg[2]=0;
       
  while(1)
  {
    //按下Reset之後，把目前車子傾角預設為平衡點
    if(UpdateAttitude())
    {
      DataAvg[2] = DataAvg[1];
      DataAvg[1] = DataAvg[0];
      DataAvg[0] = Angle_Car;
      AngleAvg = (DataAvg[0]+DataAvg[1]+DataAvg[2])/3;
      
      if(First_Time)
      {
        Angle_Car_Adjustment = -17.5;//讓兩輪車在斜坡上慢慢向下移動(給一個往上的車傾角(斜坡約傾斜18度)以抵銷往下的速度)
        DataAvg[2] = 0;
        DataAvg[1] = 0;
        DataAvg[0] = 0;
        Angle_Car = 0;
        First_Time = false;
      }
      //之後如果平衡點沒有超過45度，則執行PID的計算
      else if(AngleAvg < 45 || AngleAvg > -45){  
        distance=distance+Speed_LR*0.01;//計算從斜坡頂上開始往下移動了多少距離
        PWM_Calculate();
        Car_Control();
        if(distance<-100&&t==0&&x==0)//當往forward方向移動了100公分後，慢慢讓速度增加(因為斜坡的底部比較平緩)
        {
            Angle_Car_Adjustment=-17.2;//將傾斜角度降低，使兩輪車能以更快的速度往下滑(因為抵銷向下移動的力量變小了)
        }
        if(pwm_l>150&&x==0)//當向下移動的速度太快時，改變兩輪車的車傾角使其減速
        {
           Angle_Car_Adjustment=-19;//將傾斜角度增加，以抵銷兩輪車下降的速度
           t=1;
        }
        if(pwm_l>200&&x==0)//當向下移動的速度更快時，加強改變兩輪車的車傾角使其速度減得更慢
        {
           Angle_Car_Adjustment=-20;//將傾斜角度增加，以抵銷兩輪車下降的速度
           t=1;
        }
        if(pwm_l>250&&x==0)//當向下移動的速度又更快時，加強改變兩輪車的車傾角使其速度減得更慢
        {
           Angle_Car_Adjustment=-20.5;//將傾斜角度增加，以抵銷兩輪車下降的速度
           t=1;
        }
        if(pwm_l<80&&t==1)//當向下移動的速度變慢時，使兩輪車能以更快的速度往下滑(因為抵銷向下移動的力量變小了)
        {
          Angle_Car_Adjustment=-17.2;//將傾斜角度降低，使兩輪車能以更快的速度往下滑(因為抵銷向下移動的力量變小了)
          t=2;
        }
         if(distance<-140&&x==0)//當向下移動的距離大於140公分時，使其慢慢把傾斜角度改小
        {
            Angle_Car_Adjustment=-15;//將傾斜角度降低，使兩輪車能逐漸回到原地平衡的狀態
        }
         if(distance<-150&&x==0)//當向下移動的距離大於150公分時，使其繼續把傾斜角度改小
        {
            Angle_Car_Adjustment=-10;//將傾斜角度降低，使兩輪車能逐漸回到原地平衡的狀態
        }
         if(distance<-155&&x==0)//當向下移動的距離大於155公分時，若速度還太快時，使其繼續把傾斜角度改小
        {
            Angle_Car_Adjustment=-5;//將傾斜角度降低，使兩輪車能逐漸回到原地平衡的狀態
        }
        if(distance<-155&&x==0&&pwm_l<30)//當向下移動的距離大於155公分時，且已經開始停止時，使其把傾斜角度改為原地平衡時的角度
        {
            Angle_Car_Adjustment=4;//使兩輪車開始平衡
            x=1;//使兩輪車進入平衡狀態
        }
         /*if(distance<-145)
        {
            Angle_Car_Adjustment=2.85;
        }*/
      }
    }
    SendData(250);  //250ms     // NANO 沒辦法同時處理兩個BT UART傳輸  //bt1
    ReceiveData(200);  //200ms  // NANO 沒辦法同時處理兩個BT UART傳輸  //bt1

    //SendToPC(100);  // NANO 沒辦法同時處理兩個BT UART傳輸  //bt2
  }
}

