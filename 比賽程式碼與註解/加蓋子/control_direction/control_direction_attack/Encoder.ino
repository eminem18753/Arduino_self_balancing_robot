void Encoder_L()   //car up is positive car down  is negative
{
  if(digitalRead(SPD_INT_L2) == HIGH)
  {
    encoderPosL = encoderPosL + 1; //Forward
  }
  else
  {
    encoderPosL = encoderPosL - 1; //Reverse
  }
  /*
  if(pwm_ll > 0)
  {
     encoderPosL = encoderPosL + 1; //Forward
  }
  else
  {
     encoderPosL = encoderPosL - 1; //Reverse
  }
  */
}

void Encoder_R()    //car up is positive car down  is negative
{
  if(digitalRead(SPD_INT_R2) == LOW)
  {
    encoderPosR = encoderPosR + 1; //Forward
  }
  else
  {
    encoderPosR = encoderPosR - 1; //Reverse
  }
  /*
  if(pwm_rr > 0)
  {
     encoderPosR = encoderPosR + 1; //Forward
  }
  else
  {
     encoderPosR = encoderPosR - 1; //Reverse
  }  
  */
}
