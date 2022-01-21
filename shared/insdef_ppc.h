
#ifndef __INSDEFPPC_H__
#define __INSDEFPPC_H__


#define PPC_be_NOP           (0x60000000)
#define PPC_be_RFI           (0x4C000064)

// Specific to PowerPC BDM targets
#define PPC_be_MTSPR_DPDR(a)      ((31<<26) | ((a)<<21)  | (723<<11)  |              (467<<1) )
#define PPC_be_MFSPR_DPDR(a)      ((31<<26) | ((a)<<21)  | (723<<11)  |              (339<<1) )

#define PPC_be_MFSPR_R0(a)        ((31<<26)|(((((a)&0x1f)<<5)|(((a)>>5)&0x1f))<<11)| (339<<1) )
#define PPC_be_MTSPR_R0(a)        ((31<<26)|(((((a)&0x1f)<<5)|(((a)>>5)&0x1f))<<11)| (467<<1) )

#define PPC_be_MTMSR_R0           ((31<<26) |                                        (146<<1) )
#define PPC_be_MFMSR_R0           ((31<<26) |                                        ( 83<<1) )

#define PPC_be_LWZ_r1             ((32<<26) |                (1<<16))
#define PPC_be_STW_r1             ((36<<26) |                (1<<16))
#define PPC_be_LHZ_r1             ((40<<26) |                (1<<16))
#define PPC_be_STH_r1             ((44<<26) |                (1<<16))
#define PPC_be_LBZ_r1             ((34<<26) |                (1<<16))
#define PPC_be_STB_r1             ((38<<26) |                (1<<16))

#define PPC_be_LWZU(rD,rA,ofs)    ((33<<26) | ((rD)<<21) | ((rA)<<16) |                 (ofs) )
#define PPC_be_LWZ(rD,rA,ofs)     ((32<<26) | ((rD)<<21) | ((rA)<<16) |                 (ofs) )
#define PPC_be_LHZ(rD,rA,ofs)     ((40<<26) | ((rD)<<21) | ((rA)<<16) |                 (ofs) )
#define PPC_be_LBZ(rD,rA,ofs)     ((34<<26) | ((rD)<<21) | ((rA)<<16) |                 (ofs) )

#define PPC_be_LI(rD,val)         ((14<<26) | ((rD)<<21) |                              (val) )
#define PPC_be_LIS(rD,val)        ((15<<26) | ((rD)<<21) |                              (val) )

#define PPC_be_OR(rA,rS,rB)       ((31<<26) | ((rS)<<21) | ((rA)<<16) | ((rB)<<11) | (444<<1) )
#define PPC_be_OR_(rA,rS,rB)      ((31<<26) | ((rS)<<21) | ((rA)<<16) | ((rB)<<11) | (444<<1)1)
#define PPC_be_ORI(rA,rS,val)     ((24<<26) | ((rS)<<21) | ((rA)<<16) |                 (val) )
#define PPC_be_ORIS(rA,rS,val)    ((25<<26) | ((rS)<<21) | ((rA)<<16) |                 (val) )

#endif
