#include <ch32fun.h>
#include <ch32v20xhw.h>

// Config
#define DELAY_MS 3000
//#define NOBLINK
//#define NOWAIT

#define ERASED_FLASH_CONTENTS (0xe339e339)
#define MAIN_CODE_ADDR        (0x400)
#define MAIN_CODE_FLASH_ADDR  (FLASH_BASE + 0x400)
#define NEW_FW_ADDR           (FLASH_BASE + 0x20000) // 128K
#define NEW_FW_LENGTH         (0x20000 - MAIN_CODE_ADDR) // 128K - bootloader length

#define FLASH_WAIT() while(FLASH->STATR & SR_BSY) {}

#if defined(NOBLINK)
#define blink(x) 
#else
void blink(uint32_t addr)
{
    if((addr / 1024) % 2)
    {
        funDigitalWrite(PC0, FUN_HIGH);
        funDigitalWrite(PC1, FUN_HIGH);
    }
    else
    {
        funDigitalWrite(PC0, FUN_LOW);
        funDigitalWrite(PC1, FUN_LOW);
    }
}
#endif

void eraseFlash(uint32_t start, uint32_t length)
{   
    /* Authorize the FPEC of Bank1 Access */
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;

    /* Fast mode unlock */
    FLASH->MODEKEYR = FLASH_KEY1;
    FLASH->MODEKEYR = FLASH_KEY2;

    FLASH_WAIT();

    // Clear flash by 256 byte pages
    for(uint32_t addr = start; addr < (start + length); addr += 256)
    {
       FLASH->CTLR = CR_PAGE_ER;
       FLASH->ADDR = (intptr_t)addr;
       FLASH->CTLR = CR_STRT_Set | CR_PAGE_ER;
       FLASH_WAIT();

       blink(addr);
    }

    // Lock flash back
    FLASH->CTLR |= CR_FAST_LOCK_Set;
    FLASH->CTLR |= CR_LOCK_Set;
}

void writeFlashWord(uint32_t addr, uint32_t data)
{
    // Wait until not busy
    FLASH_WAIT();

    FLASH->CTLR |= CR_PG_Set;

    // Write first half
    *(__IO uint16_t *)addr = (uint16_t)data;
    FLASH_WAIT();

    // Write second half
    *(__IO uint16_t *)(addr + 2) = data >> 16;
    FLASH_WAIT();

    FLASH->CTLR &= CR_PG_Reset;
}

//Copy new to main by words
void copyNewToMain(uint32_t mainStart, uint32_t newStart, uint32_t length)
{
    /* Authorize the FPEC of Bank1 Access */
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;

    uint32_t newAddr = newStart;
    for(uint32_t addr = mainStart; addr < mainStart + length; addr += 4)
    {
        writeFlashWord(addr, *(uint32_t*)newAddr);
        newAddr += 4;

        blink(addr);
    }

    // Lock flash back
    FLASH->CTLR |= CR_LOCK_Set;
}

int main()
{ 
    SystemInit();

    #if !defined(NOBLINK)
        funGpioInitC();
        funPinMode(PC0, GPIO_Speed_2MHz | GPIO_CNF_OUT_PP);
        funPinMode(PC1, GPIO_Speed_2MHz | GPIO_CNF_OUT_PP);
    #endif

    #if !defined(NOWAIT)
        SysTick->SR   = 0;
        SysTick->CMP  = DELAY_MS_TIME; // 1 ms
        SysTick->CNT  = 0; 
        SysTick->CTLR |= SYSTICK_CTLR_STE | SYSTICK_CTLR_STIE | SYSTICK_CTLR_STCLK ;
        Delay_Ms(DELAY_MS);
    #endif

    uint32_t* rd = (uint32_t*)NEW_FW_ADDR;
    if(*rd != ERASED_FLASH_CONTENTS)
    {
        eraseFlash(MAIN_CODE_FLASH_ADDR, NEW_FW_LENGTH);
        copyNewToMain(MAIN_CODE_FLASH_ADDR, NEW_FW_ADDR, NEW_FW_LENGTH);
        eraseFlash(NEW_FW_ADDR, NEW_FW_LENGTH);
        PFIC->SCTLR = 1<<31; // reboot
    }

    void (*jumpMainFW)(void) ;

    jumpMainFW = (void (*)(void))MAIN_CODE_ADDR;

    jumpMainFW(); 
}

