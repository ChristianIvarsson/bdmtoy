#ifndef __BDMOPRIVATE_H__
#define __BDMOPRIVATE_H__

// Run CPU
#define BDM_GO      0x0c00
// Reset peripherals
#define BDM_RESET   0x0400

// D/A reg
#define R_DREG_BDM  0x2180
#define W_DREG_BDM  0x2080
// 0-7 = Data Reg
// 8-F = Address Reg

// Sysreg
#define R_SREG_BDM  0x2580
#define W_SREG_BDM  0x2480
// 0 = PC
// 1 = PCC?

// 8 = Tempreg A
// 9 = Fault Address Reg
// A = Vector Base reg
// B = Status reg
// C = User Stack Pointer
// D = Supervisor Stack Pointer
// E = SFC
// F = DFC

// Misc..
#define NULL_BDM    0x0000
#define PATCH_USRC  0x0800

// Read
#define READ8_BDM   0x1900
#define READ16_BDM  0x1940
#define READ32_BDM  0x1980
// Read/Increment address
#define DUMP8_BDM   0x1D00
#define DUMP16_BDM  0x1D40
#define DUMP32_BDM  0x1D80

// Write
#define WRITE8_BDM  0x1800
#define WRITE16_BDM 0x1840
#define WRITE32_BDM 0x1880
// Write/Increment address
#define FILL8_BDM   0x1C00
#define FILL16_BDM  0x1C40
#define FILL32_BDM  0x1C80

// Run CPU
#define BDMOLD_GO       0x0c00
// Reset peripherals
#define BDMOLD_RESET    0x0400


#define BDMOLD_W_DREG   0x2080
#define BDMOLD_W_AREG   BDMOLD_W_DREG + 8
#define BDMOLD_W_SREG   0x2480

#define BDMOLD_R_DREG   0x2180
#define BDMOLD_R_AREG   BDMOLD_R_DREG + 8
#define BDMOLD_R_SREG   0x2580

// 0 = PC
// 1 = PCC?

// 8 = Tempreg A
// 9 = Fault Address Reg
// A = Vector Base reg
// B = Status reg
// C = User Stack Pointer
// D = Supervisor Stack Pointer
// E = SFC
// F = DFC

// Misc..
#define BDMOLD_NULL       0x0000
#define BDMOLD_PATCH_USRC 0x0800
// Read
#define BDMOLD_READ8      0x1900
#define BDMOLD_READ16     0x1940
#define BDMOLD_READ32     0x1980
// Read/Increment address
#define BDMOLD_DUMP8      0x1D00
#define BDMOLD_DUMP16     0x1D40
#define BDMOLD_DUMP32     0x1D80

// Write
#define BDMOLD_WRITE8     0x1800
#define BDMOLD_WRITE16    0x1840
#define BDMOLD_WRITE32    0x1880
// Write/Increment address
#define BDMOLD_FILL8      0x1C00
#define BDMOLD_FILL16     0x1C40
#define BDMOLD_FILL32     0x1C80

#endif
