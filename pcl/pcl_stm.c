
#include "pcl_stm.h"

#ifdef ENABLE_FLASH
extern uint16_t pcl_load(picolInterp *i, uint32_t addr);

COMMAND(flash) {
    ARITY(argc >= 2, "flash cmd [addr]");
    uint32_t sz = 0;
#if 1
    if(argc >= 3)
    {
        uint32_t *ptr = (uint32_t*)str2int(argv[2]);
        if(SUBCMD1("mr"))
            return picolSetHex4Result(i, *ptr);
        if(argc >= 4)
        {
            if(SUBCMD1("mw"))
                return picolSetIntResult(i, flash_write((uint32_t)ptr, str2int(argv[3])));
        }
    }
    if(SUBCMD1("unlock"))
        return picolSetHexResult(i, HAL_FLASH_Unlock());
    if(SUBCMD1("lock"))
        return picolSetHexResult(i, HAL_FLASH_Lock());
    if(SUBCMD1("fsz0"))
        return picolSetIntResult(i, flash_fsz0());
    if(SUBCMD1("fsz1"))
        return picolSetIntResult(i, flash_fsz1());
    if(SUBCMD1("crc0"))
    {
        if(argc >= 3)
            sz = str2int(argv[2]);
        return picolSetHex4Result(i, flash_crc0(sz));
    }
    if(SUBCMD1("crc1"))
    {
        if(argc >= 3)
            sz = str2int(argv[2]);
        return picolSetHex4Result(i, flash_crc1(sz));
    }
    if(SUBCMD1("erase1"))
        return picolSetIntResult(i, flash_erase1());
    if(SUBCMD1("pclupd"))
        return picolSetHex4Result(i, pcl_load(i, USER_FLASH_MID_ADDR));
#endif
    return PICOL_ERR;
}
#endif

COMMAND(mw) {
    ARITY(argc >= 3, "mw addr data");
    uint32_t *ptr = (uint32_t*)str2int(argv[1]);
    *ptr = str2int(argv[2]);
    return picolSetResult(i, argv[2]);
}

#ifdef ENABLE_GPIO
static GPIO_TypeDef *get_gpio_instance(char *a)
{
    char x = to_upper(a[0]);
    if(x == 'A')
        return GPIOA;
    if(x == 'B')
        return GPIOB;
    if(x == 'C')
        return GPIOC;
    if(x == 'D')
        return GPIOD;
    if(x == 'E')
        return GPIOE;
    if(x == 'F')
        return GPIOF;
    if(x == 'G')
        return GPIOG;
    if(x == 'H')
        return GPIOH;
    return 0;
}

COMMAND(gpio) {
    ARITY(argc >= 2, "gpio a|b|c... num [val]");
    GPIO_TypeDef *gpiox = get_gpio_instance(argv[1]);
    if(gpiox == 0)
        return PICOL_ERR;
    ARITY(gpiox, "gpio a|b|c... num [val]");
    uint32_t pin = 0, value = 0;
    char op = 0;
    if(argc >= 3)
    {
        volatile uint32_t *preg = gpio_get_reg_ptr(gpiox, argv[2]);
        if(preg)
        {
            if(argc == 4)
                *preg = str2int(argv[3]);
            return picolSetHex4Result(i,*preg);
        }
    }
    if(('0' <= argv[1][1]) && (argv[1][1] <= '9'))
    {
        pin = str2int(&(argv[1][1]));
        if(argc == 2)
            op = 'r';
        else
        {
            op = 'w';
            value = str2int(argv[2]);
        }
    }
    else if (argc >= 3)
    {
        if(('0' <= argv[2][0]) && (argv[2][0] <= '9'))
        {
            pin = str2int(argv[2]);
            if(argc == 3)
                op = 'r';
            else
            {
                op = 'w';
                value = str2int(argv[3]);
            }
        }
    }
    if(op == 'r')
        value = HAL_GPIO_ReadPin(gpiox, 1 << pin);
    else if(op == 'w')
    {
        if(value)
            HAL_GPIO_WritePin(gpiox, 1 << pin, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(gpiox, 1 << pin, GPIO_PIN_RESET);
    }
    else
        return PICOL_ERR;
    return picolSetHexResult(i,value);
}

COMMAND(btn) {
    ARITY(argc >= 3, "btn [a n ...] ...");
    uint8_t j;
    uint32_t pin;
    GPIO_TypeDef *gpiox = 0;
    for(j = 1; j < argc; j++)
    {
        if(gpiox && ('0' <= argv[j][0]) && (argv[j][0] <= '9'))
            btn_irq_init(gpiox, str2int(argv[j]));
        else
            gpiox = get_gpio_instance(argv[j]);
    }
    return PICOL_OK;
}
#endif

#ifdef ENABLE_ADC
COMMAND(adc) {
    ARITY((argc >= 2), "adc start");
    if(SUBCMD1("start"))
    {
        if(argc == 2)
            adc_start();
        else
            adc_start_sz(str2int(argv[2]));
        return PICOL_OK;
    }
    if(SUBCMD1("stop"))
    {
        adc_stop();
        return PICOL_OK;
    }
    return PICOL_ERR;
}
#endif

#ifdef ENABLE_DAC
COMMAND(dac) {
    ARITY((argc >= 2), "dac start|stop");
    if(SUBCMD1("start"))
    {
        dac_start_sin();
        return PICOL_OK;
    }
    if(SUBCMD1("stop"))
    {
        dac_stop();
        return PICOL_OK;
    }
    return PICOL_ERR;
}
#endif

#ifdef ENABLE_UART
COMMAND(uart) {
    ARITY((argc >= 2), "uart 1|2|3... str");
    char buf[IO_BUF_SZ];
    uint32_t value = 0, j = 0;
    static uint32_t timeout = 500;
    if(SUBCMD1("timeout"))
    {
        if(argc == 3)
            timeout = str2int(argv[2]);
        return picolSetIntResult(i, timeout);
    }
    value = str2int(argv[1]);
    ARITY((0 < value) && (value <= 6) && (argc >= 3), "uart 1|2|3... str");
    UART_HandleTypeDef huart;
    uart_get_handle(&huart, value, HAL_UART_STATE_READY);
    if(SUBCMD2("init"))
    {
        ARITY((argc >= 4), "uart 1|2|3... init baudrate [8n1]");
        HAL_UART_DeInit(&huart);
        huart.Init.BaudRate = str2int(argv[3]);
        return picolSetIntResult(i, (HAL_UART_DeInit(&huart) == HAL_OK) ? 0 : 1);
    }
    volatile uint32_t *reg_ptr = uart_get_reg_ptr(huart.Instance, argv[2]);
    if(reg_ptr)
    {
        if(argc == 3)
            value = uart_get_reg(huart.Instance, argv[2]);
        else if(argc == 4)
            value = uart_set_reg(huart.Instance, argv[2], str2int(argv[3]));
        return picolSetHexResult(i, value);
    }
    buf[0] = 0;
    for(j = 2; j < argc; j++)
    {
        if(!mystrncmp(argv[j], "\\n", 2))
        {
            mystrncat(buf, "\n", IO_BUF_SZ);
            continue;
        }
        if(!mystrncmp(argv[j], "\\r", 2))
        {
            mystrncat(buf, "\r", IO_BUF_SZ);
            continue;
        }
        if(j > 2)
            mystrncat(buf, " ", IO_BUF_SZ);
        mystrncat(buf, argv[j], IO_BUF_SZ);
    }
    HAL_UART_Transmit(&huart, (uint8_t*)buf, mystrnlen(buf, IO_BUF_SZ), timeout);
    mymemset(buf, 0, IO_BUF_SZ);
    HAL_UART_Receive(&huart, (uint8_t*)buf, IO_BUF_SZ, timeout);
    return picolSetResult(i, buf);
}
#endif

#ifdef ENABLE_I2C
COMMAND(eeprom) {
    ARITY(argc > 3, "eeprom read|write addr ...");
    uint16_t addr = str2int(argv[2]);
    uint8_t buf[MAXSTR];
    uint8_t buf1[32];
    uint16_t sz = 0, j, k;
    if(SUBCMD1("read")) {
        sz = str2int(argv[3]);
        if(eeprom_read_data(addr, buf1, sz) == 0)
            return PICOL_ERR;
        for(j = 0, k = 0; j < sz; j++)
        {
            if(j)
                buf[k++] = ' ';
            itoh(buf1[j], (char*)&(buf[k]), 1);
            k += 4;
        }
        return picolSetResult(i, (char*)buf);
    }
    if(SUBCMD1("write")) {
        sz = argc - 3;
        for(j = 0, k = 0; j < sz; j++)
            buf1[j] = str2int(argv[j + 3]);
        if(eeprom_write_data(addr, buf1, sz) == 0)
            return PICOL_ERR;
        return PICOL_OK;
    }
    return PICOL_ERR;
}
#endif

#ifdef ENABLE_SPI
COMMAND(spi) {
    ARITY(argc >= 3, "spi nspi data ...");
    uint8_t nspi = 0;
    if(argv[1][0] == '1') nspi = 1;
    if(argv[1][0] == '2') nspi = 2;
    if(argv[1][0] == '3') nspi = 3;
    uint8_t buf[MAXSTR];
    uint32_t tmp = 0;
    if(nspi && (argv[1][1] == '.') && (mystrnlen(argv[1], 8) >= 4))
    {
        GPIO_TypeDef *csgpiox = get_gpio_instance(&(argv[1][2]));
        if(!csgpiox)
            return PICOL_ERR;
        uint8_t csgpion = str2int(&(argv[1][3]));
        uint16_t len = str2bytes(argv[2], buf, MAXSTR);
        tmp = spi_send(nspi, csgpiox, csgpion, buf, len);
        return picolSetHex4Result(i, tmp);

    }
    if(nspi)
    {
        tmp = spi_get_reg(nspi, argv[2]);
        return picolSetHex4Result(i, tmp);
    }
    return PICOL_ERR;
}
#endif

void pcl_stm_init(picolInterp *i)
{
#ifdef ENABLE_GPIO
    picolRegisterCmd(i, "gpio", picol_gpio, 0);
    picolRegisterCmd(i, "btn", picol_btn, 0);
#endif
#ifdef ENABLE_ADC
    picolRegisterCmd(i, "adc", picol_adc, 0);
#endif
#ifdef ENABLE_DAC
    picolRegisterCmd(i, "dac", picol_dac, 0);
#endif
#ifdef ENABLE_FLASH
    picolRegisterCmd(i, "flash", picol_flash, 0);
#endif
#ifdef ENABLE_I2C
    picolRegisterCmd(i, "eeprom", picol_eeprom, 0);
#endif
#ifdef ENABLE_SPI
    picolRegisterCmd(i, "spi", picol_spi, 0);
#endif
#ifdef ENABLE_UART
    picolRegisterCmd(i, "uart", picol_uart, 0);
#endif
}

