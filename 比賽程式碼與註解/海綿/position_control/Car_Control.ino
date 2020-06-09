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
  analogWrite(PWM_L, pwm_l>255? 255:pwm_l);
  analogWrite(PWM_R, pwm_r>255? 255:pwm_r);
}
