#include <ch32fun.h>
#include <ch32v20xhw.h>

#define MAIN_CODE_ADDR (0x400)

#define MAIN_CODE_FLASH_ADDR (FLASH_BASE + 0x400)
#define NEW_FW_ADDR (FLASH_BASE + 0x20000) // 128K
#define NEW_FW_LENGTH (0x20000 - MAIN_CODE_ADDR) // 128K - bootloader length

extern "C" void __libc_init_array(void);

#define FLASH_WAIT() while(FLASH->STATR & SR_BSY) {}

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
       //FLASH->CTLR &= ~CR_PAGE_ER;

       if((addr / 2048) % 2)
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

// Write 256 byte page
void writeFlash256(uint32_t pageAddress, uint32_t* data)
{   
    FLASH->CTLR |= CR_PAGE_PG;
    while(FLASH->STATR & SR_BSY);
    while(FLASH->STATR & SR_WR_BSY);

    uint32_t size = 64;

    while(size)
    {
        *(uint32_t *)pageAddress = *(uint32_t *)data;
        pageAddress += 4;
        data += 1;
        size -= 1;
        while(FLASH->STATR & SR_WR_BSY);
    }

    FLASH->CTLR |= CR_PG_STRT;
    while(FLASH->STATR & SR_BSY);
    FLASH->CTLR &= ~CR_PAGE_PG;
}

//Copy new to main by 256 byte pages
void copyNewToMain256(uint32_t mainStart, uint32_t newStart, uint32_t length)
{
    static bool flag = true;

    /* Authorize the FPEC of Bank1 Access */
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;

    /* Fast mode unlock */
    FLASH->MODEKEYR = FLASH_KEY1;
    FLASH->MODEKEYR = FLASH_KEY2;

    uint32_t newAddr = newStart;
    for(uint32_t addr = mainStart; addr < mainStart + length; addr += 256)
    {
        writeFlash256(addr, (uint32_t *)newAddr);
        newAddr += 256;

        if(flag)
        {
            funDigitalWrite(PC0, FUN_HIGH);
            funDigitalWrite(PC1, FUN_HIGH);
        }
        else
        {
            funDigitalWrite(PC0, FUN_LOW);
            funDigitalWrite(PC1, FUN_LOW);
        }
        flag = !flag;
    }
    // Lock flash back
    FLASH->CTLR |= CR_FAST_LOCK_Set;
    FLASH->CTLR |= CR_LOCK_Set;
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

    // Lock flash back
    FLASH->CTLR |= CR_LOCK_Set;
}

int main()
{ 
    SystemInit();

    SysTick->SR   = 0;
    SysTick->CMP  = DELAY_MS_TIME; // 1 ms
    SysTick->CNT  = 0; 
    SysTick->CTLR |= SYSTICK_CTLR_STE | SYSTICK_CTLR_STIE | SYSTICK_CTLR_STCLK ;
//
    funGpioInitC();
    funPinMode(PC0, GPIO_Speed_2MHz | GPIO_CNF_OUT_PP);
	funPinMode(PC1, GPIO_Speed_2MHz | GPIO_CNF_OUT_PP);
    Delay_Ms(3000);

    volatile uint32_t* rd = (uint32_t*)NEW_FW_ADDR;
    if(*rd != 0xe339e339)
    {
        funDigitalWrite(PC1, FUN_HIGH);
        funDigitalWrite(PC0, FUN_HIGH);
        //Delay_Ms(500);
        eraseFlash(MAIN_CODE_FLASH_ADDR, NEW_FW_LENGTH);
        copyNewToMain(MAIN_CODE_FLASH_ADDR, NEW_FW_ADDR, NEW_FW_LENGTH);
        eraseFlash(NEW_FW_ADDR, NEW_FW_LENGTH);
        PFIC->SCTLR = 1<<31; // reboot
    }
    //eraseFlash(MAIN_CODE_FLASH_ADDR, NEW_FW_LENGTH);

    //copyNewToMain256(MAIN_CODE_FLASH_ADDR, NEW_FW_ADDR, NEW_FW_LENGTH);
    //copyNewToMain(MAIN_CODE_FLASH_ADDR, NEW_FW_ADDR, NEW_FW_LENGTH);

    void (*jumpMainFW)(void) ;

    jumpMainFW = (void (*)(void))MAIN_CODE_ADDR;

    jumpMainFW(); 
}

