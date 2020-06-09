#include <iostream>
using namespace std;
int main()
{
	int pwm=160;
	int flag_pwm=0;
	int flag_stop=0;
	double distance=0;
	double Speed_LR=0;
	
	while(1) 
	{
		Speed_LR=pwm;
		if(pwm>150)
        {
            flag_pwm=1;  
        }
        if(flag_pwm==1)
        {
            distance=distance+Speed_LR*0.01;
			pwm-=10; 
            if(pwm<15)
            {
                flag_pwm=0;
                flag_stop=10;
		    }
        }
        if(flag_stop==10)
        {
        	pwm=-50;
        	Speed_LR=pwm;
          	distance=distance+Speed_LR*0.01;
            if(distance<=0)
            {
                flag_stop=0;
		        printf("pwm:%d distance:%f\n",pwm,distance);
				break;		
            }
        }
        printf("pwm:%d distance:%f\n",pwm,distance);
    }
	return 0;
}
