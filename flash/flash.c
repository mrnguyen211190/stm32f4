
#pragma long_calls

#include <main.h>
#include "flash.h"

void flash_unlock(void)
{
    HAL_FLASH_Unlock();
    if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_PGSERR) != RESET)
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGSERR);
    if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_PGPERR) != RESET)
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGPERR);
}

void flash_erase_img1(void)
{
    uint32_t sz1 = flash_fsz1();
    if(!sz1)
        return;
    uint32_t crc1 = flash_crc1(0);
    uint32_t crc0 = flash_crc0(0);
    if(crc1 == crc0)
        flash_erase1();
}

static uint32_t flash_fsz(uint32_t *start, uint32_t *end)
{
    //dbg_send_hex2("flash_start", (uint32_t)start);
    //dbg_send_hex2("flash_end", (uint32_t)end);
    static uint32_t start_addr = 0;
    if(start_addr == 0)
        start_addr = (uint32_t)start;
    if((end - start) <= 4)
    {
        //dbg_send_hex2("start", (uint32_t)start);
        //dbg_send_hex2("end", (uint32_t)end);
        uint8_t *ptr8 = (uint8_t*)end + 1;
        //dbg_send_hex2("ptr8", (uint32_t)ptr8);
        while(--ptr8 > (uint8_t*)start)
        {
            if(*(ptr8-1) != 0xFF)
                break;
        }
        //dbg_send_hex2("ptr8", (uint32_t)ptr8);
        uint32_t ret = (uint32_t)ptr8 - start_addr;
        start_addr = 0;
        return ret;
    }
    uint32_t *mid = (uint32_t*)(((uint32_t)start + (uint32_t)end)/2);
    if((*mid == 0xFFFFFFFF) && (*(end-1) == 0xFFFFFFFF))
        return flash_fsz(start, mid);
    else
        return flash_fsz(mid, end);
}

uint32_t flash_fsz0(void)
{
    return flash_fsz((uint32_t*)USER_FLASH_START_ADDR, (uint32_t*)USER_FLASH_MID_ADDR - 0x100);
}

uint32_t flash_fsz1(void)
{
    //dbg_send_str3("flash_fsz1", 1);
    return flash_fsz((uint32_t*)USER_FLASH_MID_ADDR, (uint32_t*)USER_FLASH_END_ADDR - 0x100);
}

#if 1
uint32_t flash_write(uint32_t addr, uint32_t data)
{
    flash_unlock();
    if((USER_FLASH_START_ADDR <= addr) && (addr <= (USER_FLASH_END_ADDR - 4)))
        data = HAL_FLASH_Program(TYPEPROGRAM_WORD, addr & 0xFFFFFFFC, data);
    return data;
}
#endif

uint32_t flash_erase1(void)
{
    uint32_t sz = flash_fsz1();
    if(!sz)
        return 0;
    flash_unlock();
    uint32_t SectorError = 0;
    FLASH_EraseInitTypeDef flash_erase_init;
    flash_erase_init.TypeErase = TYPEERASE_SECTORS;
    flash_erase_init.Sector = FLASH_SECTOR_8;
    if((ADDR_FLASH_SECTOR_8 + sz) < ADDR_FLASH_SECTOR_9)
        flash_erase_init.NbSectors = 1;
    else if((ADDR_FLASH_SECTOR_8 + sz) < ADDR_FLASH_SECTOR_10)
        flash_erase_init.NbSectors = 2;
    else if((ADDR_FLASH_SECTOR_8 + sz) < ADDR_FLASH_SECTOR_11)
        flash_erase_init.NbSectors = 3;
    else
        flash_erase_init.NbSectors = 4;
    flash_erase_init.VoltageRange = VOLTAGE_RANGE_3;

    if(HAL_FLASHEx_Erase(&flash_erase_init, &SectorError) != HAL_OK)
        return SectorError;
    if(FLASH_WaitForLastOperation((uint32_t)HAL_FLASH_TIMEOUT_VALUE) != HAL_OK)
        return HAL_FLASH_TIMEOUT_VALUE;

    return flash_erase_init.NbSectors;
}

uint16_t flash_write_data(uint32_t addr, uint8_t *data, uint16_t sz)
{
    flash_unlock();
    if((addr < USER_FLASH_START_ADDR) || (addr > USER_FLASH_END_ADDR))
    {
        dbg_send_hex2("bad addr", addr);
        return 0;
    }
    uint16_t i;
    uint32_t *data32 = (uint32_t*)data;
    for(i = 0; (i + 4) <= sz; i += 4)
    {
        if(HAL_FLASH_Program(TYPEPROGRAM_WORD, addr + i, data32[i/4]) != HAL_OK)
            return i;
    }
    if(i == sz)
        return i;
    i -= 4;
    for(; i < sz; i++)
    {
        if(HAL_FLASH_Program(TYPEPROGRAM_BYTE, addr + i, data[i]) != HAL_OK)
            return i;
    }
    return sz;
}

#if 0
uint32_t revbit(uint32_t data)
{
  asm("rbit r0,r0");
  return data;
};
#else
uint32_t revbit(uint32_t data)
{
    int i;
    uint32_t rev = 0;
    for(i=0; i<32; i++)
        rev |= ((data >> i) & 1) << (31 - i);
    return rev;
};
#endif

uint32_t flash_crc(uint8_t *buf, uint32_t sz)
{
    CRC_HandleTypeDef hcrc;
    hcrc.State = HAL_CRC_STATE_RESET;
    hcrc.Instance = CRC;
    uint32_t *buf32 = (uint32_t*)buf;
    if(HAL_CRC_Init(&hcrc) != HAL_OK)
        return 0;

    uint32_t i, j;
    uint32_t ui32;

    CRC->CR=1;

#if 0
    asm("NOP");asm("NOP");asm("NOP");//delay for hardware ready
#else
    HAL_Delay(1);
#endif

    i = sz >> 2;

    /* Process Locked */
    __HAL_LOCK(&hcrc);

    /* Change CRC peripheral state */
    hcrc.State = HAL_CRC_STATE_BUSY;

    /* Reset CRC Calculation Unit */
    __HAL_CRC_DR_RESET(&hcrc);

    /* Enter Data to the CRC calculator */
    for(j = 0; j < i; j++)
    {
        hcrc.Instance->DR = revbit(buf32[j]);
        buf += 4;
    }

    /* Change CRC peripheral state */
    hcrc.State = HAL_CRC_STATE_READY;

    /* Process Unlocked */
    __HAL_UNLOCK(&hcrc);

    /* Return the CRC computed value */
    ui32 = hcrc.Instance->DR;

    ui32=revbit(ui32);//reverse the bit order of output data

    i = sz & 3;

    while(i--)
    {
        ui32 ^= (uint32_t)*buf++;

        for(j=0; j<8; j++)
            if (ui32 & 1)
                ui32 = (ui32 >> 1) ^ 0xEDB88320;
            else
                ui32 >>= 1;
    }

    ui32 ^= 0xffffffff;//xor with 0xffffffff

    return ui32;//now the output is compatible with windows/winzip/winrar
};

uint32_t flash_crc0(uint32_t sz)
{
    if(!sz)
        sz = flash_fsz0();
    if(!sz)
        return 0;
    return flash_crc((uint8_t*)USER_FLASH_START_ADDR, sz);
}

uint32_t flash_crc1(uint32_t sz)
{
    if(!sz)
        sz = flash_fsz1();
    if(!sz)
        return 0;
    return flash_crc((uint8_t*)USER_FLASH_MID_ADDR, sz);
}

#define SECTOR_MASK               ((uint32_t)0xFFFFFF07)

__attribute__ ((long_call, section (".ramtext"))) uint32_t sram_flash_copy10(void)
{
    uint32_t addr0, addr1, i, data;
    uint32_t *led = (uint32_t*)0x40020C14;
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP    | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |\
                                     FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR| FLASH_FLAG_PGSERR);
    for(i = FLASH_SECTOR_0; i < FLASH_SECTOR_8; i++)
    {
        while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);
        FLASH->CR &= CR_PSIZE_MASK;
        FLASH->CR |= FLASH_PSIZE_WORD;
        FLASH->CR &= SECTOR_MASK;
        FLASH->CR |= FLASH_CR_SER | (i << POSITION_VAL(FLASH_CR_SNB));
        FLASH->CR |= FLASH_CR_STRT;
        while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);
        *led = 0x0800;
        FLASH->CR &= (~FLASH_CR_SER);
        FLASH->CR &= SECTOR_MASK;
    }
    for(i = 0; i < USER_FLASH_SZ/2; i += 4)
    {
        addr1 = (uint32_t)(USER_FLASH_MID_ADDR + i);
        addr0 = (uint32_t)(USER_FLASH_START_ADDR + i);
        data = *(volatile uint32_t*)addr1;
        while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);
        FLASH->CR &= CR_PSIZE_MASK;
        FLASH->CR |= FLASH_PSIZE_WORD;
        FLASH->CR |= FLASH_CR_PG;
        *(volatile uint32_t*)addr0 = data;
        while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);
        FLASH->CR &= (~FLASH_CR_PG);
    }
#if 1
    // RESET
    __DSB();
    SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos)      |
            (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
            SCB_AIRCR_SYSRESETREQ_Msk);
    __DSB();
    while(1);
#endif
    return 123;
}

void flash_copy10(void)
{
    dbg_send_hex2("sram_flash_copy10", (uint32_t)sram_flash_copy10);
    sram_flash_copy10();
}

