void ReceiveData(int interval_receive) {
    if( (millis() - SendTimer) > interval_receive)
    {
        char val;
        // 若收到藍牙模組的資料，則送到「序列埠監控視窗」
        if (BTSerial.available()) {
          val = BTSerial.read();
          str_receive[len_receive++] = val;
          if(len_receive>=128)len_receive=0;
          if(val == ']')
          {
              //Serial.println(val);
              CheckIfJoystick();
              CheckIfPID();
              len_receive = 0;//歸零
          }
          else
              ;
              //Serial.print(val);
        }
    }
}
