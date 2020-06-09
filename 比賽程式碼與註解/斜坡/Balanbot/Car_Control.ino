void Car_Control()
{  
  digitalWrite(STBY, HIGH);

  //決定輪子是正轉還反轉
  if (pwm_l < 0)
  {
    digitalWrite(DIR_L1, LOW);
    digitalWrite(DIR_L2, HIGH);
    pwm_l = -pwm_l;  //change to positive // PWM只能接收正數0~255
  }
  else
  {
    digitalWrite(DIR_L1, HIGH);
    digitalWrite(DIR_L2, LOW);
  }
  
  if (pwm_r < 0)
  {
    digitalWrite(DIR_R1, HIGH);
    digitalWrite(DIR_R2, LOW);
    pwm_r = -pwm_r;
  }
  else
  {
    digitalWrite(DIR_R1, LOW);
    digitalWrite(DIR_R2, HIGH);
  }

  //設定最大傾角，超過這個角度就不再平衡
  if( Angle_Car > 45 || Angle_Car < -45 )
  {
    pwm_l = 0;
    pwm_r = 0;
  }
  if(Angle_Car_Adjustment<-8)
  {
    if(Speed_Diff_ALL>3&&DIR_L1==LOW&&DIR_L2==HIGH)     //forward
    {
      pwm_r=pwm_r-25;//當兩輪速度差大於一定值時，若兩輪車在前進，則將右輪減速(因為前進時右輪速度較快)
    }
    if(Speed_Diff_ALL>3&&DIR_L1==HIGH&&DIR_L2==LOW)    //backward
    {
      pwm_l=pwm_l-18;//當兩輪速度差大於一定值時，若兩輪車在後退，則將左輪減速(因為後退時左輪速度較快)
    }
  }
  if(DIR_L1==LOW&&DIR_L2==HIGH&&Angle_Car_Adjustment<-8)  //forward
  {
    if(pwm_l<27)//進行wheel synchronization，pwm太小而且還在斜坡上半部時，便將其設為0
    {
        pwm_l=0;
    }
    if(pwm_r<27)
    {
       pwm_r=0;
    }
  }
  else if(DIR_L1==LOW&&DIR_L2==HIGH&&Angle_Car_Adjustment>-8)  //forward
  {
    if(pwm_l<23)//進行wheel synchronization，pwm太小而且已經到達斜坡下半部時，便將其設為0
    {
        pwm_l=0;
    }
    if(pwm_r<23)
    {
       pwm_r=0;
    }
  }
  if(DIR_L1==HIGH&&DIR_L2==LOW)  //backward
  {
    if(pwm_l<19)//進行wheel synchronization，pwm太小時，便將其設為0
    {
       pwm_l=0;
    }
    if(pwm_r<19)
    {
       pwm_r=0;
    }
  }
  if(Angle_Car_Adjustment<-8)
    pwm_r=pwm_r-7;//當兩輪車還在斜坡上半部時，將右輪速度降低(因為右輪速度較快)
  //把PWM值固定在0~255
  analogWrite(PWM_L, pwm_l > 255 ? 255:pwm_l);
  analogWrite(PWM_R, pwm_r > 255 ? 255:pwm_r);
}
