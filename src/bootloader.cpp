#include <ch32fun.h>
#include <ch32v20xhw.h>

#define MAIN_CODE_ADDR (0x400);

int main()
{ 
    SystemInit();

    SysTick->SR   = 0;
    SysTick->CMP  = DELAY_MS_TIME; // 1 ms
    SysTick->CNT  = 0; 
    SysTick->CTLR |= SYSTICK_CTLR_STE | SYSTICK_CTLR_STIE | SYSTICK_CTLR_STCLK ;

    funGpioInitC();
    funPinMode(PC0, GPIO_Speed_50MHz | GPIO_CNF_OUT_PP);
	funPinMode(PC1, GPIO_Speed_50MHz | GPIO_CNF_OUT_PP);
    Delay_Ms(3000);

    void (*jump_func)(void) ;

    jump_func = (void (*)(void))MAIN_CODE_ADDR;

    //__disable_irq();
    //asm volatile(" j 0x400;\n");
    jump_func(); 
}

