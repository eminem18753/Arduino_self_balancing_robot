void PWM_Calculate()
{  
  Speed_L = (encoderPosL - encoderL_past)/CPR*PI*2*TIRE_RADIUS/(millis()-SpeedTimer)*1000; //cm/sec
  Speed_R = (encoderPosR - encoderR_past)/CPR*PI*2*TIRE_RADIUS/(millis()-SpeedTimer)*1000; //cm/sec

  SpeedTimer = millis();

  encoderR_past = encoderPosR;
  encoderL_past = encoderPosL;
  
  Speed_LR = (Speed_L + Speed_R) * 0.5;
    
  Speed_Diff = Speed_L - Speed_R;
  Speed_Diff_ALL += Speed_Diff;

 
    double temp;
    double Pt = 0, It = 0, Dt = 0;
    
    Pt = KA_P * Angle_Car;
    It = KA_I * (Et_total);
    Dt = KA_D * Gyro_Car;
    Et_total += Angle_Car*(micros()-angle_dt)*0.000001;//micros --> sec
    
    angle_dt = micros();
    
    temp = Pt+It+Dt;
    pwm = int(temp);
    //Serial.println(pwm);
    //Speed_Need  //Turn_Need
    pwm_r = pwm;
    pwm_l = pwm;
  
}
