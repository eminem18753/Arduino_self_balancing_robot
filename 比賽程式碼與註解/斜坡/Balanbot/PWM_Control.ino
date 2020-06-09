void PWM_Calculate()
{  
  // 速度 = (目前Encder值 - 前一刻Encder值) / 一圈Encoder數 * 輪子圓周長 / 取樣時間
  Speed_L = (encoderPosL - encoderL_past) / CPR * PI * TIRE_RADIUS * 2 / (millis() - SpeedTimer) * 1000;  // cm/sec
  Speed_R = (encoderPosR - encoderR_past) / CPR * PI * TIRE_RADIUS * 2 / (millis() - SpeedTimer) * 1000;  // cm/sec

  SpeedTimer = millis();

  // 因為此次的Speed已經算完，所以時間要推進至下一刻
  encoderL_past = encoderPosL;
  encoderR_past = encoderPosR;
  
  Speed_LR = (Speed_L + Speed_R) * 0.5;  // 把左右輪速度相加的一半設為車子整體速度
  
  Speed_Diff = Speed_L - Speed_R;
  Speed_Diff_ALL += Speed_Diff;
 
  double temp;
  double Pt = 0, It = 0, Dt = 0;

  // 角度控制
  Pt = KA_P * Angle_Car;
  It = KA_I * (Et_total);
  Dt = KA_D * Gyro_Car;
  Et_total += Angle_Car*(micros()-angle_dt)*0.000001;  // 單位換算:micros --> sec
  
  angle_dt = micros();
    
  temp = Pt+It+Dt;
  pwm = int(temp); // PWM = Pt+It+Dt
  //Serial.println(pwm);
  //Speed_Need  //Turn_Need  
  pwm_r = pwm;
  pwm_l = pwm;  
}
