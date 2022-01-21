#ifndef __BDMSPRIVATE_H__
#define __BDMSPRIVATE_H__
/// "Regular" BDMv? commands..
// Hardware command set:
#define BDMHCS12_BACKGROUND     0x90  // none
#define BDMHCS12_READ_BD_BYTE   0xE4  // 16-bit address out, 16-bit data in
#define BDMHCS12_WRITE_BD_BYTE  0xC4  // 16-bit address in, 16-bit data out
#define BDMHCS12_READ_BD_WORD   0xEC  // 16-bit address out, 16-bit data in
#define BDMHCS12_WRITE_BD_WORD  0xCC  // 16-bit address in, 16-bit data out

#define BDMHCS12_READ_BYTE      0xE0  // 16-bit address out, 16-bit data in
#define BDMHCS12_WRITE_BYTE     0xC0  // 16-bit address in, 16-bit data out
#define BDMHCS12_READ_WORD      0xE8  // 16-bit address out, 16-bit data in
#define BDMHCS12_WRITE_WORD     0xC8  // 16-bit address in, 16-bit data out

// Firmware command set:
#define BDMHCS12_READ_NEXT      0x62  // 16-bit data in
#define BDMHCS12_READ_PC        0x63  // 16-bit data in
#define BDMHCS12_READ_D         0x64  // 16-bit data in
#define BDMHCS12_READ_X         0x65  // 16-bit data in
#define BDMHCS12_READ_Y         0x66  // 16-bit data in
#define BDMHCS12_READ_SP        0x67  // 16-bit data in

#define BDMHCS12_WRITE_NEXT     0x42  // 16-bit data out
#define BDMHCS12_WRITE_PC       0x43  // 16-bit data out
#define BDMHCS12_WRITE_D        0x44  // 16-bit data out
#define BDMHCS12_WRITE_X        0x45  // 16-bit data out
#define BDMHCS12_WRITE_Y        0x46  // 16-bit data out
#define BDMHCS12_WRITE_SP       0x47  // 16-bit data out

#define BDMHCS12_GO             0x08  // none
#define BDMHCS12_TRACE1         0x10  // none
#define BDMHCS12_TAGGO          0x18  // none

/// BDMv4 extra
// Additional hardware commands:
#define BDMHCS12_ACK_ENABLE     0xD5  // none
#define BDMHCS12_ACK_DISABLE    0xD6  // none

// Additional firmware commands:
#define BDMHCS12_GO_UNTIL       0x0C  // none

#endif
