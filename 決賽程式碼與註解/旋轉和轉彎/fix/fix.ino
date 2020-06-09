#include <SoftwareSerial.h>
#include <Wire.h>
#include "Kalman.h" 
#include "I2C.h"
#include "MPU6050.h"
MPU6050 Gyro6050;

//bluetooth receive text
#define MAX_BTCMDLEN 128
char str_receive[MAX_BTCMDLEN]; // received 128 bytes from an Android system
byte str_send[MAX_BTCMDLEN]; // sent 128 bytes from an Arduino system
int len_send = 0; //要傳送的字串長度
int len_receive = 0; //要接收的字串長度

SoftwareSerial BTSerial(A0,13); // Arduino RX/TX    //to Phone
SoftwareSerial BTSerial2(A2,A1); // Arduino RX/TX   //to PC

Kalman kalmanX; // Create the Kalman instances
Kalman kalmanY;

/* IMU Data */
double accX, accY, accZ;
double gyroX, gyroY, gyroZ;
int16_t tempRaw;

double gyroXangle, gyroYangle; // Angle calculate using the gyro only
double compAngleX, compAngleY; // Calculated angle using a complementary filter
double kalAngleX, kalAngleY; // Calculated angle using a Kalman filter
double rotate_angle_l=0;
double rotate_angle_r=0;
uint32_t timer;
uint8_t i2cData[14]; // Buffer for I2C data

//motor  define
#define PWM_L 10  //PWMA
#define PWM_R 5   //PWMB
#define DIR_L1 4
#define DIR_L2 12
#define DIR_R1 7
#define DIR_R2 8
#define STBY 6 //standby

//encoder define
#define SPD_INT_R 3   //interrupt 
#define SPD_INT_L 2   //interrupt
#define SPD_INT_R2 11
#define SPD_INT_L2 9

#define CPR 374.0 //count per round
#define TIRE_RADIUS 3.45 //3.45cm

int Speed_L,Speed_R;
int pwm,pwm_l,pwm_r;
int pwm_ll, pwm_rr;
double distance=0;
double Angle_Car; //車子傾斜角
double Angle_Car_Adjustment;
double Gyro_Car;
double Gyro_Car_Adjustment;
double temp=0;
float Speed_LR;
boolean flag_pwm=false;
boolean flag_stop=false;

double KA_P,KA_I,KA_D;
double KP_P,KP_I,KP_D;
double ANG_OFFSET;
double Et_total;
double x;
int c=0;
int d=0;
int e=0;

float Speed_Need,Turn_Need;
int Speed_Diff,Speed_Diff_ALL;
boolean encoder_flag=false;
boolean encoder_stop=false;
boolean encoder_pause=false;
boolean encoder_rest=false;
uint32_t angle_dt;
uint32_t SendTimer;
uint32_t ReceiveTimer;
uint32_t SendToPCTimer;
uint32_t SpeedTimer;
uint32_t t=0;
uint32_t z=0;

int encoderPosL, encoderPosR;
int encoderR_past, encoderL_past;
int encoder_temp=0;
bool First_Time = true;
bool pwm_flag=false;
void setup() {
  
  Serial.begin(115200);
  BTSerial.begin(38400); // NANO 沒辦法同時處理兩個BT UART傳輸  //bt1
  //BTSerial2.begin(38400);   //bt2
  Init();
  Wire.begin();
  TWBR = ((F_CPU / 400000L) - 16) / 2; // Set I2C frequency to 400kHz

  while (i2cWrite(0x19, i2cData, 4, false)); // Write to all four registers at once
  while (i2cWrite(0x6B, 0x01, true)); // PLL with X axis gyroscope reference and disable sleep mode
  while (i2cRead(0x75, i2cData, 1));
  
  if (i2cData[0] != 0x68) { // Read "WHO_AM_I" register
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
  #ifdef RESTRICT_PITCH // Eq. 25 and 26
    double roll  = atan2(accY, accZ) * RAD_TO_DEG;
    double pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
  #else // Eq. 28 and 29
    double roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
    double pitch = atan2(-accX, accZ) * RAD_TO_DEG;
  #endif

  kalmanX.setAngle(roll); // Set starting angle
  kalmanY.setAngle(pitch);
  gyroXangle = roll;
  gyroYangle = pitch;
  compAngleX = roll;
  compAngleY = pitch;
 
  attachInterrupt(0, Encoder_L, RISING);
  attachInterrupt(1, Encoder_R, RISING);
  
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
    if(UpdateAttitude())
    {
      DataAvg[2] = DataAvg[1];
      DataAvg[1] = DataAvg[0];
      DataAvg[0] = Angle_Car;
      AngleAvg = (DataAvg[0]+DataAvg[1]+DataAvg[2])/3;
      
      if(First_Time)
      {
        Angle_Car_Adjustment = -AngleAvg;
        DataAvg[2] = 0;
        DataAvg[1] = 0;
        DataAvg[0] = 0;
        //Angle_Car = 0;
        First_Time = false;
        x=-AngleAvg;
        //temp=Angle_Car_Adjustment;
      }
      
      else if(AngleAvg < 45 || AngleAvg > -45){
        //Angle_Car_Adjustment=1.25;
        
        //if(Angle_Car<0)
        //Angle_Car=Angle_Car+0.3;
        if(millis()>5000)//讓兩輪車先在原地平衡5秒
        {
          z=millis();//z是一個計時器，用來判斷執行完動作的平衡時間(使兩輪車更加穩定)
          distance=distance+Speed_LR*0.01;//開始計算距離
          rotate_angle_l=(encoderPosL-encoder_temp)/CPR*PI*2*TIRE_RADIUS/16.2/PI*180;//計算左輪造成兩輪車的轉動角度(即兩輪車向左轉動的角度)
          rotate_angle_r=(encoderPosR-encoder_temp)/CPR*PI*2*TIRE_RADIUS/16.2/PI*180;//計算右輪造成兩輪車的轉動角度(即兩輪車向右轉動的角度)
          if(c==0&&d==0)   //forward 1m
          {
            if(pwm>=180)//當pwm值太大時，使車子能盡量保持平衡狀態
            {
                Angle_Car_Adjustment=x;//把angle_car_adjustment調整為初始的x值，使其平衡
                t=millis();//計算所需要的平衡時間
            }
            else
            {
              if((z-t)>=1000)//當原地平衡了1秒之後，便可以開始繼續做動作
              {
                if(millis()%1000<100)//每1秒當中有0.1秒是以比較慢的速度在前進
                {
                  Angle_Car_Adjustment=x-1;//當angle_car_adjustment為x-1時，兩輪車會朝backward方向前進
                }
                else if(millis()%1000>=100)//每1秒當中有0.9秒是在以比較快的速度在後退
                {
                  //Angle_Car_Adjustment=x-3.5;
                  Angle_Car_Adjustment=-1;//當angle_car_adjustment為-1時，兩輪車會朝backward方向前進
                }
              }
            }
          }
          if(distance>60&&c==0&&d==0)//當到達移動的距離時先原地平衡1.5秒，等待執行下一個動作
          {
            Angle_Car_Adjustment=x;//把angle_car_adjustment調整為初始的x值，使其平衡
            distance=0;//當到達移動的距離時先原地平衡1.5秒，等待執行下一個動作
            c=1;//準備執行下一個動作
            d=1;//已經平衡1.5秒，開始可以執行下一個動作
          }

        if(c==1&&d==0)   //rotate left 90 degree
          {
              if(millis()-t>2990&&millis()-t<3010)//原地平衡3秒
             {
                encoder_temp=encoderPosL;//記錄左輪的encoder值(用來計算左輪接下來要旋轉的角度)
                rotate_angle_l=0;//把左輪轉角度設為0
             }
             if(millis()-t>3000&&encoder_flag==false)//原地平衡3秒
              {
              Angle_Car_Adjustment=0.5;//把angle_car_adjustment調整為0.5，使其向後移動
              if(rotate_angle_l>80)
              {
                Angle_Car_Adjustment=x;//把angle_car_adjustment調整為初始的x值，使其平衡
                encoder_flag=true;//判斷已經完成此動作
                c=2;//準備執行下一個動作
                d=1;//已經平衡1.5秒，開始可以執行下一個動作
                distance=0;//重設distance為0
              }
            }
          }
          if(c==2&&d==0)  // forward 1m
          {
              if(pwm>=180)//當pwm值太大時，使車子能盡量保持平衡狀態
            {
                Angle_Car_Adjustment=x;//把angle_car_adjustment調整為初始的x值，使其平衡
                t=millis();//計算所需要的平衡時間
            }
            else
            {
              if((z-t)>=1000)//先讓兩輪車平衡1秒(以維持穩定)
              {
                if(millis()%1000<100)//每1秒當中有0.1秒是以比較慢的速度在後退
                {
                  Angle_Car_Adjustment=x-1;;//當angle_car_adjustment為x-1時，兩輪車會後退
                }
                else if(millis()%1000>=100)//每1秒當中有0.9秒是在以比較快的速度在後退
                {
                  //Angle_Car_Adjustment=x-3.5;
                  Angle_Car_Adjustment=-1;當angle_car_adjustment為-1時，會以比較快的速度向前後退
                }
              }
            }
          }
          if(distance>60&&c==2&&d==0)
          {
            Angle_Car_Adjustment=x;//判斷是否達到移動的目標
            distance=0;
            c=3;
            d=1;
          }
          if(c==3&&d==0)   //rotate left 90 degree
          {
              if(millis()-t>2990&&millis()-t<3010)//在原地平衡3秒
             {
                encoder_temp=encoderPosL;//紀錄左輪的encoder值
                rotate_angle_l=0;//將左輪的轉動角度歸零
             }
             if(millis()-t>3000&&encoder_stop==false)//在原地平衡5秒
              {
              Angle_Car_Adjustment=0.5;//把angle_car_adjustment調整為0.5，使其向後移動
              if(rotate_angle_l>80)//判斷是否已經完成轉動
              {
                Angle_Car_Adjustment=x;//把angle_car_adjustment調整為初始的x值，使其平衡
                encoder_stop=true;//可用以判斷已經執行完動作
                c=4;//準備執行下一個動作
                d=1; d=1;//已經平衡1.5秒，開始可以執行下一個動作
                distance=0;//將distance歸零
              }
            }
          }
          
          if(c==4&&d==0)  // forward 1m//開始執行下一個動作
          {
              if(pwm>=180)//當pwm值太大時，使車子能盡量保持平衡狀態
            {
                Angle_Car_Adjustment=x;//把angle_car_adjustment調整為初始的x值，使其平衡
                t=millis();//計算所需要的平衡時間
            }
            else
            {
              if((z-t)>=1000)//先讓兩輪車平衡1秒(以維持穩定)
              {
                if(millis()%1000<100)//每1秒當中有0.1秒是以比較慢的速度後退
                {
                  Angle_Car_Adjustment=x-1;//當angle_car_adjustment為x-1時，兩輪車會朝backward方向前進
                }
                else if(millis()%1000>=100)//當angle_car_adjustment為x-1時，兩輪車會朝backward方向前進
                {
                  //Angle_Car_Adjustment=x-3.5;
                  Angle_Car_Adjustment=-1;//當angle_car_adjustment為-1時，會以比較快的速度後退
                }
              }
            }
          }
          if(distance>60&&c==4&&d==0)//判斷已經抵達目標
          {
            Angle_Car_Adjustment=x;//把angle_car_adjustment調整為初始的x值，使其平衡
            distance=0;//將distance重設為0
            c=5;//準備執行下一個動作
            d=1;//原地平衡1.5秒
          }

          if(c==5&&d==0)   //rotate right 135 degree
          {
              if(millis()-t>2990&&millis()-t<3010)//原地平衡3秒
             {
                encoder_temp=encoderPosR;//記錄右輪的encoder值
                rotate_angle_r=-0.5;//讓兩輪車向右旋轉
             }
             if(millis()-t>3000&&encoder_pause==false)//原地平衡3秒
              {
              Angle_Car_Adjustment=0.75;//當angle_car_adjustment為0.75時，會開始向右轉動
              if(rotate_angle_r>120)//判斷轉動角度是否大於120度
              {
                Angle_Car_Adjustment=x;//把angle_car_adjustment調整為初始的x值，使其平衡
                encoder_pause=true;
                c=6;//準備執行下一個動作
                d=1;//已經平衡1.5秒，開始可以執行下一個動作
                distance=0;//將distance歸零
              }
            }
          }
          
          if(c==6&&d==0)  // forward 1m
          {
              if(pwm>=180)//當pwm值太大時，使車子能盡量保持平衡狀態
            {
                Angle_Car_Adjustment=x;//把angle_car_adjustment調整為初始的x值，使其平衡
                t=millis();//計算所需要的平衡時間
            }
            else
            {
              if((z-t)>=1000)//先讓兩輪車平衡1秒(以維持穩定)
              {
                if(millis()%1000<100)//每1秒當中有0.1秒是以比較慢的速度後退
                {
                  Angle_Car_Adjustment=x-1;/當angle_car_adjustment為x-1時，兩輪車會朝backward方向前進
                }
                else if(millis()%1000>=100)//當angle_car_adjustment為x-1時，兩輪車會朝backward方向前進
                {
                  //Angle_Car_Adjustment=x-3.5;
                  Angle_Car_Adjustment=-1;//當angle_car_adjustment為-1時，會以比較快的速度後退
                }
              }
            }
          }
          if(distance>40&&c==6&&d==0)//判斷已經抵達目標
          {
            Angle_Car_Adjustment=x;//把angle_car_adjustment調整為初始的x值，使其平衡
            distance=0;//將distance重設為0
            c=7;//完成所有動作
            d=1;//原地平衡1.5秒
          }

          if(c==7&&d==0)   //rotate right 90 degree
          {
              if(millis()-t>2990&&millis()-t<3010)//原地平衡3秒
             {
                encoder_temp=encoderPosR;//記錄右輪的encoder值
                rotate_angle_r=0;//將右輪的轉動角度歸零
             }
             if(millis()-t>3000&&encoder_rest==false)//在原地平衡3秒
              {
              Angle_Car_Adjustment=0.5;//讓兩輪車向右轉動
              if(rotate_angle_r>80)//判斷轉動角度是否大於目標
              {
                Angle_Car_Adjustment=x;//把angle_car_adjustment調整為初始的x值，使其平衡
                encoder_rest=true;//可用以判斷已經執行完動作
                c=8;//準備執行下一個動作
                d=1;//已經平衡1.5秒，開始可以執行下一個動作
                distance=0;//將distance歸零
              }
            }
          }
          if(c==8&&d==0)   //balance
          {
            if(millis()-t>5000)//讓兩輪車原地平衡5秒
            {
               c=9;
               distance=0;
            }
          }
          
          if(c==9&&d==0)  //turn right 135 degree with r=1m
          {
            if(pwm>=200)//當pwm值太大時，使車子能盡量保持平衡狀態
          {
              Angle_Car_Adjustment=x;//把angle_car_adjustment調整為初始的x值，使其平衡
              t=millis();//計算所需要的平衡時間
          }
          else
          {
            if((z-t)>=1000)//先讓兩輪車平衡1秒(以維持穩定)
            {
            if(millis()%1000<100)//每1秒當中有0.1秒是以比較慢的速度後退
            {
               Angle_Car_Adjustment=x-1;//當angle_car_adjustment為x-1時，兩輪車會朝backward方向前進
            }
            else if(millis()%1000>=100)//當angle_car_adjustment為x-1時，兩輪車會朝backward方向前進
            {
               //Angle_Car_Adjustment=x-3.5;
               Angle_Car_Adjustment=-0.5;//當angle_car_adjustment為-0.5時，會以比較快的速度後退
            }
            }
        }
          }
          if(distance>150&&c==9&&d==0)   //160
          {
            Angle_Car_Adjustment=x;//把angle_car_adjustment調整為初始的x值，使其平衡
            distance=0;//將distance重設為0
            c=10;//完成所有動作
            d=1;//原地平衡1.5秒
          }
          
          
          /*if(distance<-100&&c==1&&d==0)
          {
            Angle_Car_Adjustment=x;
            flag_stop=false;
            distance=0;
            c=0;
            d=1;
          }*/

           if(d==1&&e==0)
          {
            t=millis();//計算所需要的平衡時間
            Angle_Car_Adjustment=x;//把angle_car_adjustment調整為初始的x值，使其平衡
            e=1;
          }
          if((z-t)>2000&&d==1)
          {
            d=0;//已經原地平衡1.5秒
            e=0;
            t=millis();//計算所需要的平衡時間
          }
        }
        PWM_Calculate();
        Car_Control();
      }
    }
    SendData(250); //250ms     //bt1
    ReceiveData(200); //200ms  //bt1

    //SendToPC(100);  //bt2
  }
}
