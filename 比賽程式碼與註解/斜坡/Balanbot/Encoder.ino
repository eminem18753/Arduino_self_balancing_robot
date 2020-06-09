void Encoder_L()   //讀取左輪的Encoder值
{
  if(digitalRead(SPD_INT_L2) == HIGH) 
  {
    encoderPosL = encoderPosL + 1; //正轉
  }
  else
  {
    encoderPosL = encoderPosL - 1; //反轉
  }
}

void Encoder_R()   //讀取右輪的Encoder值
{
  if(digitalRead(SPD_INT_R2) == LOW)
  {
    encoderPosR = encoderPosR + 1; //正轉
  }
  else
  {
    encoderPosR = encoderPosR - 1; //反轉
  }  
}
