//******************************************************************************
// Имя файла    :       power_src.c
// Заголовок    :
// Автор        :       Дмитриев И.В.
// Дата         :       24.07.2024
//
//------------------------------------------------------------------------------
/**
Драйвер реле
**/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "DEV_Config.h"
#include "power_src.h"
//#include "mb_new_format_pulsar.h"

// Внешние выводы (PIN) каналов управления

struct sPowerChannelPins aPowerChannelPins[2] = {
    {
        .Pin_rele  = { .GPIO_Port = GPIOD, .PinIdx =  0, },
        .Pin_power_cam  = { .GPIO_Port = GPIOD, .PinIdx =  1, },
};
//
//struct s_DispChannel gaDispChannel[1] = {
//    {
//        .DispPosRele  = { .x = 91, .y = 63 },
//};

struct sPowerChannel gaPowerChannel[2] = {
    {
        .Rele = 0,      //Реле включено  при вкл.
        //.TimeON = 1000,   // время
    },
};

struct sPowerChannel * GetPowerChannel(uint8_t iChannel) {
    return &gaPowerChannel[iChannel];
}


int gCount = 0;

// Состояние реле

float GetChRele(int8_t iChannel) {
    struct sPowerChannel * cpPowerChannel = GetPowerChannel(iChannel);
    if ( pPowerChannel->Rele == true){

        return  1;
    }
    else return 0;

    return 0;
}


enum enrele = {
    "OFF",
    "ON",
    "NC",
};


enum Charge_Pulsar_sataus{
    Discharge,
    Charge,
    Full_charge,
    No_Bat,
    Error
}   ;

extern struct mode_struct_s * GetMode(int iModeIdx);

extern const struct s_MbServerItem MbServerItem_Iip;
void Fill_IIP_MBServer_PulsarFormat() {         // заполнить 7 байт данными

    //Очистить сервер источника
    memset ((uint8_t * )&MbServerItem_Iip.Regs, 0, MbServerItem_Iip.RegNum * 2);

    struct s_MbItem_Iip * cpMbItem_Iip = (struct s_MbItem_Iip *)MbServerItem_Iip.Regs;  // присваиваем адрес глобальнйо струкутуры содерж. указательна маасив адресов модбас структур
    //2 = SR_I1 Регистр состояния источника
    uint8_t cState = 0 ;

    struct s_ModBus_Reg_Pulsar  * cpChannel;
    cpChannel = (struct s_ModBus_Reg_Pulsar *) MbServerItem_Iip.Regs;

    struct sPowerChannel * cpPowerChannel = & gaPowerChannel[0];

    cpChannel->Voltage = cpPowerChannel->ValueU * 1000;
    cpChannel->Current = cpPowerChannel->ValueI * 1000;
    cpChannel ->Charge = GetCharge();
    if (cpChannel ->Charge < 0) cpChannel ->Charge = 0;
    cpChannel->ExtPwrMode = GetPwrMode() ;
    //cpChannel->ZarMode = ZarModeIdx() ;
    cpChannel->ZarMode = GetChargeMode_Pulsar();

    cpChannel->RS_1_Power = ((extio_read_idx(EXTIO_CPU_33_V5)==0) ? 1:0);
    cpChannel->RS_2_Power =  ((extio_read_idx(EXTIO_CPU_33_V4)==0) ? 1:0);
}



struct sPowerCtrl_IN gPowerCtrl_IN = {
    .Pin_K = { .GPIO_Port = GPIOE, .PinIdx = 12 },  //Пин выходного реле
    .DispPosRele = { .x = 91, .y = 107 },  //Позиция "Состояние канала"

    .Rele = 0,   //Реле включено

};

//Инициализация пинов канала
void InitPowerChannelPins(int iIdx) {
    struct sPowerChannelPins * cpChannel = &aPowerChannelPins[iIdx];
    uint32_t cPeriph = PortRCC_Periph((PortRCC_Periph((uint32_t)cpChannel->Pin_K.GPIO_Port);
    RCC_AHB1PeriphClockCmd(cPeriph, ENABLE);

    GPIO_INIT(cpChannel->Pin_K.GPIO_Port , cpChannel->Pin_K.PinIdx , MODE_OUT, OTYPE_PP, OSPEED_LS/*OSPEED_VHS*/, PUPD_N, 0, 0);

}


extern struct sPowerChannelPins  * gpPowerChannelPins;

void Power_task(void * pvParameters) {
    uint32_t cInterval = GetInterval()-1;
    uint64_t cCount;
    InitPowerChannelPins(0);
    vTaskDelay(1000);

    while (1) {
        cCount = CurrTimeInMsecs();
        int cRefresh = gCount % REFRESH_INT;

        if ((cRefresh >= 0) && (cRefresh <= 2)) {
        } else {
            ShowLife();
        }

        if ( (CurrTimeInMsecs() - cInterval ) > 10000)
        {
            //settings_load_serial_mode(2);

            cInterval = CurrTimeInMsecs()- 1;
            vTaskDelay(100);
        }

        ++gCount;
        gCycleTime = CurrTimeInMsecs() - cCount;
    }
}


void Init_Power() {
#ifndef INA260
    InitPowerChannels();
#endif
    //Отдельная задача управление источником
    xTaskCreate (Power_task, (int8_t *) "Power_task", configMINIMAL_STACK_SIZE * 5, NULL, tskIDLE_PRIORITY + 2, NULL);
}



