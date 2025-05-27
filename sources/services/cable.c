//******************************************************************************
// Имя файла    :       cable.c
// Заголовок    :
// Автор        :       Сосновских А.А.
// Дата         :       20.03.2019
//
//------------------------------------------------------------------------------
/**
Контроль кабелей портов Ethernet
**/

#include "cable.h"
#include "sw_reg.h"

struct sCableTestResult gCableTestResult;

//Запись в регистр свитча
void WriteSwReg(struct sIntPort * cpIntPort, uint8_t iReg, uint8_t iValue) {
  uint8_t cWriteValue = iValue;
  ksz8895fmq_write_registers(cpIntPort->sw, iReg, &cWriteValue, 1);
}

//Запись в регистр порта
void WriteSwPortReg(struct sIntPort * cpIntPort, uint8_t iReg, uint8_t iValue) {
  uint8_t cValue = iValue;
  ksz8895fmq_write_registers(cpIntPort->sw, (cpIntPort->port * 0x10) + iReg, &cValue, 1);
}

bool ReadSwPortBit(struct sIntPort * cpIntPort, uint8_t iReg, uint8_t iBit) {
  uint8_t cValue;
  ksz8895fmq_read_registers(cpIntPort->sw, (cpIntPort->port * 0x10) + iReg, &cValue, 1);
  return ((cValue & (1 << iBit)) != 0);
}

//Функция запуска теста - чтения результата

//Дополнительные коэффициенты коррекции (по конкретному кабелю)
#define CABLE_COEF_SUB (6) //Вклад внутренней схемы МКПШ-10
//Фактическое смещение = 6 (а не 26 как в KSZ8895 LinkMD)
#define CABLE_COEF_MUL (0.44204)  //Множитель для вычисления длины в метрах
#define CABLE_COEF_ADD (0.4648)   //Смещение для вычисления длины в метрах
/* Исходные данные:
  Длины кабеля: 40,28   159,14    199,42
  Измерения:    92      362
  Коэффициенты расчетные: MUL = 1,7648; ADD = 0,44204
Принятые значения:
  Длины кабеля: 39,78   159,14
                (-0,50м)
  Коэффициенты: MUL = 0,4648; ADD = 0,44204
                      (-1,3)
*/
//#include <cmath> // для round

void CableTest(uint8_t iPort) {
  //Вычислить свитч и порт
  struct sIntPort * cpIntPort = InternalPort(iPort);

  uint8_t cReg1c, cReg1d, cWriteValue;
  //Сохраним предыдущие значения регистров
  cReg1c = ReadPortField(0, 8, cpIntPort->port, 12, cpIntPort->sw);
  cReg1d = ReadPortField(0, 8, cpIntPort->port, 13, cpIntPort->sw);

//  //1. Disable Auto-Negotiation by writing a ‘1’ to Register 28 (0x1c), bit [7].
//  SetPortKey(cpIntPort->sw, true, cpIntPort->port, 12, 0x80);
//  //2. Disable auto MDI/MDI-X by writing a ‘1’ to Register 29 (0x1d), bit [2] to enable manual
//  //  control over the differential pair used to transmit the LinkMD® pulse.
//  SetPortKey(cpIntPort->sw, true, cpIntPort->port, 13, 0x04);
//  //3. A software sequence set up to the internal registers for LinkMD only, see an example below.

      //The following is a sample procedure for using LinkM® on port 1.
      ////Set Force 100/Full and Force MDIX mode
      ////W is WRITE the register. R is READ register
      //W 1c ff
    WriteSwPortReg(cpIntPort, 0x1c, cReg1c | 0xe0);
      //W 1d 04
    WriteSwPortReg(cpIntPort, 0x1d, 0x04);
      ////Set Internal Registers Temporary Adjustment for LinkMD
      //W 47 b0
    WriteSwReg(cpIntPort, 0x47, 0xb0);
      //W 27 00
    WriteSwReg(cpIntPort, 0x27, 0x00);
      //W 37 03 (03- port 1, 04-port2, 05-port3, 06-port4, 07-port5)
    WriteSwReg(cpIntPort, 0x37, cpIntPort->port + 0x02);
      //W 47 80 (bit7-port1, bit6-port2, bit5-port3, bit4-port4, bit3-port5)
    cWriteValue = (0x80 >> (cpIntPort->port - 1));
    WriteSwReg(cpIntPort, 0x47, cWriteValue);
      //W 27 00
    WriteSwReg(cpIntPort, 0x27, 0x00);
      //W 37 00
    WriteSwReg(cpIntPort, 0x37, 0x00);
    //4. Start cable diagnostic test by writing a ‘1’ to Register 26 (0x1a), bit [4]. This enable bit is self-clearing.
      ////Enable LinkMD Testing with Fault Cable for port [1..5]
      //W 1a 10
    WriteSwReg(cpIntPort, (cpIntPort->port * 0x10) + 0x0a, 0x10);
    //5. Wait (poll) for Register 26 (0x1a), bit [4] to return a ‘0’, and indicating cable diagnostic test is completed.
    while (ReadSwPortBit(cpIntPort, 0x0a, 4)) {

    }
    uint8_t cReg1a, cReg1b;
    int16_t cDistance;
    //6. Read cable diagnostic test results in Register 26 (0x1a), bits [6:5]. The results are as follows:
    //  00 = normal condition (valid test)
    //  01 = open condition detected in cable (valid test)
    //  10 = short condition detected in cable (valid test)
    //  11 = cable diagnostic test failed (invalid test)
    //  The ‘11’ case, invalid test, occurs when the KSZ8895 is unable to shut down the link partner. In this instance, the
    //  test is not run, since it would be impossible for the KSZ8895 to determine if the detected signal is a reflection of
    //  the signal generated or a signal from another source.
      //R 1a
    cReg1a = ReadPortField(0, 8, (cpIntPort->port - 1), 0x0a, cpIntPort->sw);
      //R 1b
    cReg1b = ReadPortField(0, 8, (cpIntPort->port - 1), 0x0b, cpIntPort->sw);
    //7. Get distance to fault by concatenating Register 26 (0x1a), bit [0] and Register 27 (0x1b), bits [7:0]; and
    //   multiplying the result by a constant of 0.4. The distance to the cable fault can be determined by the following
    //   formula:
    //      D (distance to cable fault) = 0.4 x { (Register 26, bit [0]),(Register 27, bits [7:0]) }
      ////Result analysis based on the values of the Register 0x1a and 0x1b for port [1..5]:
      ////The Register 0x1a bits [6-5] are for the open or the short detection.
      ////The Register 0x1a bit [0] + the Register 0x1b bits [7-0] = Vct_Fault [8-0]
      ////The distance to fault is about 0.4 x {Vct_Fault [8-0] – 26}
    cDistance = ((cReg1a & 0x01) << 8) | cReg1b;
    cDistance -= CABLE_COEF_SUB;
    float fDistance = cDistance;
    fDistance *= CABLE_COEF_MUL;
    fDistance += CABLE_COEF_ADD;
    if (fDistance < 0)
      fDistance = 0;
    cDistance = round(fDistance);
    gCableTestResult.Distance = cDistance;
    gCableTestResult.CableState = (enum eCableState)((cReg1a >> 5) & 0x03);



  //After end the testing, set all registers above to their default value,
  //the default values are ‘00’ for the Register (0x37) and the Register (0x47)
      //W 37 00
    WriteSwReg(cpIntPort, 0x37, 0x00);
      //W 47 00
    WriteSwReg(cpIntPort, 0x47, 0x00);

  //Если изменялись Auto-Negotiation и/или auto MDI/MDI-X, то:
//  //8. Enable Auto-Negotiation by writing a ‘0’ to Register 28 (0x1c), bit [7].
  SetPortField(cReg1c, 0, 8, cpIntPort->port - 1, 12, cpIntPort->sw, false);
//  //9. Enable auto MDI/MDI-X by writing a ‘0’ to Register 29 (0x1d), bit [2] to enable manual
//  //  control over the differential pair used to transmit the LinkMD® pulse.
  SetPortField(cReg1d, 0, 8, cpIntPort->port - 1, 13, cpIntPort->sw, false);

}