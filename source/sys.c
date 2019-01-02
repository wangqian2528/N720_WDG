#include "sys.h"
#include "stm8s_clk.h"
#include "stm8s_gpio.h"
#include "stm8s_iwdg.h"
#include "stm8s_tim4.h"
#include "stm8s_uart1.h"
#include "stm8s_exti.h"
#include "stm8s_it.h"

static void Clk_init(void)//16M HSI
{
    CLK_DeInit();
    CLK_HSICmd(ENABLE);
}

static void Gpio_init(void)//PD4:POW_EN PA3:HearBeat 
{
    GPIO_DeInit(GPIOD);
    GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST);
    GPIO_DeInit(GPIOA);
    GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_IN_FL_IT);
    EXTI_DeInit();
    EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOA, EXTI_SENSITIVITY_RISE_FALL);
}

static void Uart1_init(void)
{
    UART1_DeInit();
    UART1_Init((u32)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO, UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_RX_ENABLE);
    UART1_Cmd(ENABLE);
    UART1_ITConfig(UART1_IT_RXNE_OR, ENABLE);
}

static void IWDG_init(void)//200ms WDG
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_64);
    IWDG_SetReload((u8)200);
    IWDG_Enable();
    IWDG_ReloadCounter();
}

static void TIM_init(void)//1ms
{
    TIM4_DeInit();
    TIM4_TimeBaseInit(TIM4_PRESCALER_64, 250);
    TIM4_Cmd(ENABLE);
    TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
}

void Sys_init(void)
{
    sim();
    Clk_init();
    Gpio_init();
    Uart1_init();
    IWDG_init();
    TIM_init();
    rim();
}

unsigned char bPwrDownFlag =0;
void N720_PwrOn(void)
{
    GPIO_WriteLow(GPIOD, GPIO_PIN_4);
    bPwrDownFlag =0;
}


void N720_PwrDown(void)
{
    GPIO_WriteHigh(GPIOD, GPIO_PIN_4);
    bPwrDownFlag = 1;
}

unsigned char flagpacker;       //ȫ�ֱ���    �Ƿ���������һ�����ݰ�  
unsigned char Rxpacker[10];      //ȫ�ֱ���    �������ݰ� 
unsigned char length;           //���ݳ���
INTERRUPT_HANDLER(UART1_RX_IRQHandler, 18)
{
    unsigned char RxBuf;//��ʱ����
    static unsigned char RxData[255];//�������ݻ�����
    static unsigned char count;//���ڽ������ݳ���  
    static unsigned char rec;//�ж��Ƿ����ڽ�������
    UART1_ClearITPendingBit(UART1_IT_RXNE);
    RxBuf = UART1_ReceiveData8();
    if(RxBuf == (0xF0))//֡ͷ
    {
        rec= 1;
        count = 0;
        flagpacker = 0;
        return ;  
    }
    if(RxBuf  == (0xF1))//֡β
    {
        rec= 0;
        //�˴��������У����
        length = count;
        for(unsigned char i = 0; i<count;i++)
        {
            Rxpacker[i] =RxData[i];
        }
        flagpacker = 1;//�ѽ���һ�����������ݰ�
        return ;
    }
    if(rec)//�ж��Ƿ��ڽ���״̬  
    {
        RxData[count++] = RxBuf;
    }
}

unsigned char bHeartBeatFlag = 0;
INTERRUPT_HANDLER(EXTI_PORTA_IRQHandler, 3)
{
    bHeartBeatFlag = 1;
}

unsigned char bN720RstFlag=0;
unsigned char bPwrOnFlag =0;
INTERRUPT_HANDLER(TIM4_UPD_OVF_IRQHandler, 23)
{
    static unsigned int by_Time30s = 0;
    static unsigned int by_Time10s = 0;
    if(by_Time30s > 29999)
    {
        by_Time30s = 0;
        bN720RstFlag=1;
    }
    else by_Time30s++;

    if(bPwrDownFlag)
    {
        if(by_Time10s>9999)
        {
            by_Time10s = 0;
            bPwrOnFlag = 1;
        }
        else by_Time10s++;
    }
}

