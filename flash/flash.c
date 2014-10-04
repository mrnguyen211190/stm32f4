
#include <main.h>
#include "flash.h"

#ifndef HAL_FLASH_TIMEOUT_VALUE
#define HAL_FLASH_TIMEOUT_VALUE   ((uint32_t)50000)/* 50 s */
#endif

static uint32_t flash_fsz(uint32_t *start, uint32_t *end)
{
    static uint32_t start_addr = 0;
    if(start_addr == 0)
        start_addr = (uint32_t)start;
    if((end - start) <= 4)
    {
        uint8_t *ptr8 = (uint8_t*)end;
        while(--ptr8 > start)
        {
            if(*(ptr8-1) != 0xFF)
                break;
        }
        end = ptr8 - start_addr;
        start_addr = 0;
        return end;
    }
    uint32_t *mid = (uint32_t*)(((uint32_t)start + (uint32_t)end)/2);
    if((*mid == 0xFFFFFFFF) && (*(end-1) == 0xFFFFFFFF))
        return flash_fsz(start, mid);
    else
        return flash_fsz(mid, end);
}

uint32_t flash_fsz0(void)
{
    return flash_fsz(USER_FLASH_START_ADDR, USER_FLASH_MID_ADDR);
}

uint32_t flash_fsz1(void)
{
    return flash_fsz(USER_FLASH_MID_ADDR, USER_FLASH_END_ADDR);
}

#if 1
uint32_t flash_write(uint32_t addr, uint32_t data)
{
    if((USER_FLASH_START_ADDR <= addr) && (addr <= (USER_FLASH_END_ADDR - 4)))
    {
        uint32_t status = HAL_FLASH_Program(TYPEPROGRAM_WORD, addr & 0xFFFFFFFC, data);
        //if(HAL_FLASH_Program(TYPEPROGRAM_WORD, addr & 0xFFFFFFFC, data) != HAL_OK)
        if(status != HAL_OK)
        {
            uart_send_hex2("flash_write.error", status);
            uart_send_hex2("flash_write.addr", addr);
            return 1;
        }
    }
    else
        return 1;
    return 0;
}
#endif

uint32_t flash_erase1(void)
{
    uint32_t sz = flash_fsz1();
    if(!sz)
        return 0;
    HAL_FLASH_Unlock();
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
    uart_send_int2("flash_erase_init.NbSectors", flash_erase_init.NbSectors);

    if(HAL_FLASHEx_Erase(&flash_erase_init, &SectorError) != HAL_OK)
        return SectorError;
    uart_send_int2("flash_erase_init.erased", 0);
    if(FLASH_WaitForLastOperation((uint32_t)HAL_FLASH_TIMEOUT_VALUE) != HAL_OK)
        return 1;
    uart_send_int2("flash_erase_init.lock", 0);
    HAL_FLASH_Lock();
    uart_send_int2("flash_erase_init.ok", 0);

    return 0;
}

uint32_t flash_write_array(uint32_t addr, uint8_t *data, uint16_t sz)
{
    uart_send_hex2("flash_write_array.sz", sz);
#if 0
    if(addr & 3)
    {
        uart_send_hex2("flash_write_array.return", addr);
        return 0;
    }
#endif
    uint16_t i, j;
    uint32_t *ptr, dataw;
#if 0
    for(i = 0; i < sz; i += 4)
    {
        ptr = (uint32_t*)&data[i];
        dataw = *ptr;
        if(HAL_FLASH_Program(TYPEPROGRAM_WORD, addr + i, *ptr) != HAL_OK)
            return 1;
    }
#endif
    for(i = 0; i < sz; i++)
    {
        if(HAL_FLASH_Program(TYPEPROGRAM_BYTE, addr + i, data[i]) != HAL_OK)
            return 1;
    }
    return 0;
}

uint32_t revbit(uint32_t data)
{
  asm("rbit r0,r0");
  return data;
};

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

    asm("NOP");asm("NOP");asm("NOP");//delay for hardware ready

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

uint32_t flash_crc1()
{
    uint32_t sz = flash_fsz1();
    if(!sz)
        return 0;
    return flash_crc(USER_FLASH_MID_ADDR, sz);
}

__attribute__ ((long_call, section (".ramtext"))) uint32_t flash_copy10(void)
{
    uint32_t *ptr0, *ptr1, i;
    for(i = 0; i < USER_FLASH_SZ/2; i += 4)
    {
        ptr1 = (uint32_t*)(USER_FLASH_MID_ADDR + i);
        ptr0 = (uint32_t*)(USER_FLASH_START_ADDR + i);
        FLASH->CR &= CR_PSIZE_MASK;
        FLASH->CR |= FLASH_PSIZE_WORD;
        FLASH->CR |= FLASH_CR_PG;
        *ptr0 = *ptr1;
        while(FLASH->SR & FLASH_FLAG_BSY);
    }
    return 123;
}
