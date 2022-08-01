#define __APP_UART_H__

#include "context.h"

/* SYSTEM CONTROL REGISTER */
#define SysCtrl  0x0180

/* CALIBRATION PARAMETER REGISTER */
#define PAC     0x00F6
#define PHC     0x00F7
#define PADCC   0x00F8
#define QAC     0x00F9
#define QBC     0x00FA
#define QADCC   0x00FB
#define QBDCC   0x00FC
#define IAC     0x00FD
#define IADCC   0x00FE
#define UC      0x00FF
#define PBC     0x0100
#define PBDCC   0x0101
#define IBC     0x0102
#define IBDCC   0x0103
#define IAADCC  0x0104
#define IBADCC  0x0105
#define UADCC   0x0106
#define BPFPARA 0x0107
#define UDCC    0x0108

/* Checksum Register */
#define CKSUM 0x0109

/* Software Reset Control Register */
#define SFTRST 0x01BF

/* System Status Registers */
#define SysSts    0x00CA
#define SysStsClr 0x019D

/* Metering Control Registers */
#define FREQINST 0x00CB
#define PAINST   0x00CC
#define QINST    0x00CD
#define IAINST   0x00CE
#define UINST    0x00CF
#define PAAVG    0x00D0
#define QAVG     0x00D1
#define FREQAVG  0x00D2
#define IAAVG    0x00D3
#define UAVG     0x00D4
#define PBINST   0x00D5
#define IBINST   0x00D6
#define PBAVG    0x00D7
#define IBAVG    0x00D8
#define UDCINST  0x00D9
#define IADCINST 0x00DA
#define IBDCINST 0x00DB
#define ZXDATREG 0x00DC
#define ZXDAT    0x00DD
#define PHDAT    0x00DE
#define T8BAUD   0x00E0

/* UART Interface */
#define HEAD_BYTE 0x7D
#define READ_BIT  0x01
#define WRITE_BIT 0x02

void power_meter_init(context_t *context);

