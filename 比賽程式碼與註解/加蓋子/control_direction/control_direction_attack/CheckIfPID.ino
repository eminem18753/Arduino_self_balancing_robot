bool CheckIfPID()
{
    if(str_receive[0] == '[' && str_receive[1] == 'p' && str_receive[2] == 'i' && str_receive[3] == 'd' && str_receive[4] == ':')
        {
                char* Ka_p = (char*)malloc(7);
                char* Ka_i = (char*)malloc(7);
                char* Ka_d = (char*)malloc(7);
                char* Kp_p = (char*)malloc(7);
                char* Kp_i = (char*)malloc(7);
                char* S = (char*)malloc(7);
                char* Ang_offset = (char*)malloc(7);
                int Ka_p_len=0, Ka_i_len=0, Ka_d_len=0, Kp_p_len=0, Kp_i_len=0, S_len=0, Ang_offset_len=0, Spd_len=0, Ang_len=0;
                int count_which=0;
                for(int i=5;i<len_receive;i++)
                {
                    if(str_receive[i] == ';')count_which++;
                    else if(str_receive[i] == ']')break;
                    else
                    {
                        switch(count_which)
                        {
                            case 0:
                            {
                              Ka_p[Ka_p_len++] = str_receive[i];
                              break;
                            }
                            case 1:
                            {
                              Ka_i[Ka_i_len++] = str_receive[i];
                              break;
                            }
                            case 2:
                            {
                              Ka_d[Ka_d_len++] = str_receive[i];
                              break;
                            }
                            case 3:
                            {
                              Ang_offset[Ang_offset_len++] = str_receive[i];
                              break;
                            }
                            case 4:
                            {
                              Kp_p[Kp_p_len++] = str_receive[i];
                              break;
                            }
                            case 5:
                            {
                              Kp_i[Kp_i_len++] = str_receive[i];
                              break;
                            }
                            case 6:
                            {
                              S[S_len++] = str_receive[i];
                              break;
                            }
                        }
                    }
                }
                KA_P=atof(Ka_p);
                KA_I=atof(Ka_i);
                KA_D=atof(Ka_d);
                KP_P=atof(Kp_p);
                KP_I=atof(Kp_i);
                s=atof(S);
                ANG_OFFSET=atof(Ang_offset);
                
                free(Ka_p);
                free(Ka_i);
                free(Ka_d);
                free(Kp_p);
                free(Kp_i);
                free(S);
                free(Ang_offset);

                return true; //if PID
        }
        else
            return false;
}
