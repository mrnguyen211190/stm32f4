
#include <main.h>

int main(void)
{
    myinit();
    //btn_init();
    led_toggle();
    for (;;) {
        HAL_Delay(1000);
        led_toggle();
#if 0
        if(btn_status())
            led_toggle();
#else
#if 0
        if(btn_status())
            led_on();
        else
            led_off();
#endif
#endif
    }
}

