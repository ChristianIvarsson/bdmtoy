#ifndef __MPC5566_H
#define __MPC5566_H

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct __attribute__((packed)) {
	uint8_t          MDIS     : 1;  // Module disable. 1 Dis, 0 Ena.
	uint8_t          FRZ      : 1;  // Freeze. Allow FlexCAN to enter Freeze
	uint8_t  pad1: 1;
	uint8_t          HALT     : 1;  // Is the module halted(freeze)? Must be cleared
	uint8_t          NOTRDY   : 1;  // Is nonfunctional?
	uint8_t  pad2: 1;
	uint8_t          SOFTRST  : 1;  // Request reset of internal registers
	uint8_t          FRZACK   : 1;
	uint8_t  pad3: 2;
	uint8_t          WRNEN    : 1;
	uint8_t          MDISACK  : 1;
	uint8_t  pad4: 2;
	uint8_t          SRXDIS   : 1;
	uint8_t          MBFEN    : 1;

	uint16_t pad5:10;
	uint8_t          MAXMB    : 6;
} CANx_MCR_t;


typedef struct __attribute__((packed)) {
	uint8_t          PRESDIV  : 8;
	uint8_t          RJW      : 2;
	uint8_t          PSEG1    : 3;
	uint8_t          PSEG2    : 3;

	uint8_t          BOFMSK   : 1;
	uint8_t          ERRMSK   : 1;
	uint8_t          CLK_SRC  : 1;
	uint8_t          LPB      : 1;
	uint8_t          TWRNMSK  : 1;
	uint8_t          RWRNMSK  : 1;
	uint8_t  pad : 2;
	uint8_t          SMP      : 1;
	uint8_t          BOFFREC  : 1;
	uint8_t          TSYN     : 1;
	uint8_t          LBUF     : 1;
	uint8_t          LOM      : 1;
	uint8_t          PROPSEG  : 3;
} CANx_CR_t;

typedef struct __attribute__((packed)) {
	uint16_t pad1:16;

	uint16_t         TIMER    :16;
} CANx_TIMER_t;


typedef struct __attribute__((packed)) {
	uint8_t  pad : 3;
	uint32_t         MSK      :29;
} CANx_RXGMASK_t;




typedef struct __attribute__((packed)) {

} empty;





typedef struct __attribute__((packed)) {
	CANx_MCR_t    CANx_MCR;

} FlexCAN_t;





#ifdef __cplusplus 
}
#endif
#endif