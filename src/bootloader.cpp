#include <ch32fun.h>
#include <ch32v20xhw.h>

#define MAIN_CODE_ADDR 0x400;

int main()
{ 
    SystemInit();

    void (*jump_func)(void);

    jump_func = (void (*)(void))MAIN_CODE_ADDR;

    __disable_irq();
    jump_func(); 
}

