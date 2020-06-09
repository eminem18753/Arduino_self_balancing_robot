void Car_Control()
{  
  digitalWrite(STBY, HIGH);
  pwm_ll = pwm_l;
  pwm_rr = pwm_r;
  if (pwm_l<0)
  {
    digitalWrite(DIR_L1, LOW);
    digitalWrite(DIR_L2, HIGH);
    pwm_l =- pwm_l;  //change to positive
  }
  else
  {
    digitalWrite(DIR_L1, HIGH);
    digitalWrite(DIR_L2, LOW);
  }
  
  if (pwm_r<0)
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
  if( Angle_Car > 45 || Angle_Car < -45 )
  {
    pwm_l = 0;
    pwm_r = 0;
  }

  if(Speed_Diff_ALL>3&&DIR_L1==HIGH&&DIR_L2==LOW)//backward
  {
      pwm_l=pwm_l-10;//當兩輪速度差大於一定值時，若兩輪車在後退，則將左輪減速(因為後退時左輪速度較快)
  }
  if(Speed_Diff_ALL>3&&DIR_L1==LOW&&DIR_L2==HIGH)//forward
  {
      pwm_l=pwm_l-15;//當兩輪速度差大於一定值時，若兩輪車在前進，則將左輪減速(因為前進時左輪速度較快)
  }
  
  if(DIR_L1==LOW&&DIR_L2==HIGH)  //forward
  {
    if(pwm_l<30)//進行wheel synchronization，pwm太小時，便將其設為0。因為左輪比較快，所以pwm_l<30時，皆設為0
    {
        pwm_l=0;
    }
    if(pwm_r<23)
    {
       pwm_r=0;
    }
  }
  else if(DIR_L1==HIGH&&DIR_L2==LOW)  //backward
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
  /*if(a==1 || a==3)  //forward
  {
    pwm_l=pwm_l+1;
  }*/
  if(a==4 || a==5)
  {
    pwm_r=pwm_r+5;//當a=4或a=5時，增加右輪的pwm，使得右輪加速，以校正兩輪的速度差
  }
  analogWrite(PWM_L, pwm_l>255? 255:pwm_l);
  analogWrite(PWM_R, pwm_r>255? 255:pwm_r);
}
