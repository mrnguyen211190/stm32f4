
#include <main.h>

#ifdef TIMx

#pragma message "TIM: TIM" STR(TIMn)
#pragma message "TIM: TIM_INTERVAL[ms] = " STR(TIMx_INTERVAL)

TIM_HandleTypeDef htim;
extern void Error_Handler(void);
extern uint32_t SystemCoreClock;

void tim_init(void)
{
#if 0
    htim.Instance = TIMx;
    htim.Init.Period = 10*TIMx_INTERVAL - 1;
    //htim.Init.Prescaler = (uint32_t) ((SystemCoreClock / 20000) - 1);
    htim.Init.Prescaler = tim_get_prescaler(TIMx);
    htim.Init.ClockDivision = 0;
    htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    if(HAL_TIM_Base_Init(&htim) != HAL_OK)
    { 
        Error_Handler();
    }

    if(HAL_TIM_Base_Start_IT(&htim) != HAL_OK)
    { 
        Error_Handler();
    }
#else
    htim.Instance = TIMx;
    htim.Init.Period = 999999;
    //htim.Init.Prescaler = (uint32_t) ((SystemCoreClock / 20000) - 1);
    htim.Init.Prescaler = tim_get_prescaler(TIMx)/100;
    htim.Init.ClockDivision = 0;
    htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    if(HAL_TIM_OC_Init(&htim) != HAL_OK)
    { 
        Error_Handler();
    }
    TIM_OC_InitTypeDef oc_init;
    oc_init.OCMode     = TIM_OCMODE_PWM1;
    oc_init.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc_init.Pulse = 1000;
    if(HAL_TIM_OC_ConfigChannel(&htim, &oc_init, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    if(HAL_TIM_OC_Start(&htim, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
#endif
}

void tim_deinit(void)
{
    HAL_TIM_Base_DeInit(&htim);
}
#else
void tim_init(void)
{
}
void tim_deinit(void)
{
}
#endif

uint32_t tim_get_prescaler(TIM_TypeDef *tim)
{
    RCC_ClkInitTypeDef clk_init;
    uint32_t latency;
    HAL_RCC_GetClockConfig(&clk_init, &latency);
    uint32_t prescaler = 1000;
    uint32_t clk = SystemCoreClock;
    if(clk_init.AHBCLKDivider == RCC_SYSCLK_DIV2)
        clk /= 2;
    if((tim == TIM2) || (tim == TIM3) || (tim == TIM4) || (tim == TIM5) || (tim == TIM6) || (tim == TIM7))
    {
        if(clk_init.APB1CLKDivider == RCC_HCLK_DIV2)
            clk /= 2;
        if(clk_init.APB1CLKDivider == RCC_HCLK_DIV4)
            clk /= 4;
        if(clk_init.APB1CLKDivider == RCC_HCLK_DIV8)
            clk /= 8;
        if(clk_init.APB1CLKDivider != RCC_HCLK_DIV1)
            clk *= 2;
        prescaler = clk/10000;
    }
    return prescaler - 1;
}

