bool CheckIfJoystick()
{
        //資料傳輸部分[joy:<direction>,<speed>;]
        if(str_receive[0] == '[' && str_receive[1] == 'j' && str_receive[2] == 'o' && str_receive[3] == 'y' && str_receive[4] == ':')
        {
                char* speed_cmd = (char*)malloc(5);
                char* dir_cmd = (char*)malloc(7);
                int speed_cmd_len=0, dir_cmd_len=0;
                bool speed_flag=false, dir_flag=true;
                for(int i=5;i<len_receive;i++)
                {
                    if(str_receive[i] == ',')dir_flag=false;
                    if(dir_flag == true)
                    {
                        dir_cmd[dir_cmd_len++] = str_receive[i];
                        //Serial.print(dir_cmd[dir_cmd_len-1]);
                    }
                    if(str_receive[i] == ';')speed_flag = false;
                    if(speed_flag == true)
                    {
                        speed_cmd[speed_cmd_len++] = str_receive[i];
                        //Serial.print(speed_cmd[speed_cmd_len-1]);
                    }
                    if(str_receive[i] == ',')speed_flag = true;
                    if(str_receive[i] == ']')break;
                }
                float ttt = atof(dir_cmd);
                if(ttt>0)
                    Speed_Need = atof(speed_cmd)*10.0;
                else
                    Speed_Need = -1*atof(speed_cmd)*10.0;
                if(Speed_Need == 0)
                    Turn_Need = 0;
                else
                    Turn_Need = ttt - 90;
                free(speed_cmd);
                free(dir_cmd);
                
                //Serial.print(Speed_Need);
                //Serial.print(" ");
                //Serial.println(Turn_Need);

                return true; //if joystick
        }
        else
            return false;
}
