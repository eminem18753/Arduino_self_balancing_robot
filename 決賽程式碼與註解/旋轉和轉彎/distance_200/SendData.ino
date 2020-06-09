void SendData(int interval_send){
    //write data to Android via BT
    if( (millis() - SendTimer) > interval_send)
    {
        SendTimer = millis();
        //Serial.println("Send");
        String total_sending_text = String("[") 
                                    + KA_P + String(";") + KA_I + String(";") + KA_D + String(";")
                                    + KP_P + String(";") + KP_I + String(";") + KP_D + String(";")
                                    + Angle_Car_Adjustment + String(";") 
                                    + Speed_LR + String(";") + Angle_Car + String(";]");
        char* buf = (char*)malloc(sizeof(char)*total_sending_text.length()+1);
        total_sending_text.toCharArray(buf, total_sending_text.length()+1);
        BTSerial.write(buf); //傳起始點給手機端
        free(buf);
    }
}
