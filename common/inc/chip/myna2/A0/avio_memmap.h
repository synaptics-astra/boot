//////
/// don't edit! auto-generated by docc: avio_memmap.h
////////////////////////////////////////////////////////////
#ifndef avio_memmap_h
#define avio_memmap_h (){}


#include "ctypes.h"

#pragma pack(1)
#ifdef __cplusplus
  extern "C" {
#endif

#ifndef _DOCC_H_BITOPS_
#define _DOCC_H_BITOPS_ (){}

    #define _bSETMASK_(b)                                      ((b)<32 ? (1<<((b)&31)) : 0)
    #define _NSETMASK_(msb,lsb)                                (_bSETMASK_((msb)+1)-_bSETMASK_(lsb))
    #define _bCLRMASK_(b)                                      (~_bSETMASK_(b))
    #define _NCLRMASK_(msb,lsb)                                (~_NSETMASK_(msb,lsb))
    #define _BFGET_(r,msb,lsb)                                 (_NSETMASK_((msb)-(lsb),0)&((r)>>(lsb)))
    #define _BFSET_(r,msb,lsb,v)                               do{ (r)&=_NCLRMASK_(msb,lsb); (r)|=_NSETMASK_(msb,lsb)&((v)<<(lsb)); }while(0)

#endif



//////
/// 
/// $INTERFACE AVIO_MEMMAP                              (4,4)
///     # # ----------------------------------------------------------
///               : VPP128B_DHUB_REG_BASE 0x0
///                            ###
///                            * Base address of VIO 128 bit DHUB control Registers
///                            * 64KB
///                            ###
///               : VPP128B_DHUB_REG_SIZE 0x10000
///                            ###
///                            * Size of VIO 128bit DHUB Registers memory mapping
///                            ###
///               : VPP128B_DHUB_REG_DEC_BIT 0x10
///                            ###
///                            * 64 KB has a 16 bits offset
///                            ###
///               : AIO64B_DHUB_REG_BASE 0x10000
///                            ###
///                            * Base address of AIO 64 bit DHUB control Registers
///                            * 64 KB
///                            ###
///               : AIO64B_DHUB_REG_SIZE 0x10000
///                            ###
///                            * Size of AIO 64bit DHUB Registers memory mapping
///                            ###
///               : AIO64B_DHUB_REG_DEC_BIT 0x10
///                            ###
///                            * 64 KB has a 16 bits offset
///                            ###
///               : AVIO_LCDC1_REG_BASE 0x20000
///                            ###
///                            * Base address for LCDC1 registers
///                            * 64 KB
///                            ###
///               : AVIO_LCDC1_REG_SIZE 0x10000
///                            ###
///                            * Size of LCDC1 registers
///                            ###
///               : AVIO_LCDC1_REG_DEC_BIT 0x10
///                            ###
///                            * 64KB has a 16 bits offset
///                            ###
///               : AVIO_LCDC2_REG_BASE 0x40000
///                            ###
///                            * Base address for LCDC2 registers
///                            * 64 KB
///                            ###
///               : AVIO_LCDC2_REG_SIZE 0x10000
///                            ###
///                            * Size of LCDC2 registers
///                            ###
///               : AVIO_LCDC2_REG_DEC_BIT 0x10
///                            ###
///                            * 64KB has a 16 bits offset
///                            ###
///               : AVIO_GBL_BASE      0x60000
///                            ###
///                            * Base address for AVIO Global Registers
///                            * 16KB
///                            ###
///               : AVIO_GBL_SIZE      0x4000
///                            ###
///                            * Size of AVIO Global Registers
///                            ###
///               : AVIO_GBL_DEC_BIT   0xE
///                            ###
///                            * 16 KB has a 14 bits offset
///                            ###
///               : AVIO_DAIF_REG_BASE 0x64000
///                            ###
///                            * Base address of DAIF Registers
///                            * 2KB
///                            ###
///               : AVIO_DAIF_REG_SIZE 0x800
///                            ###
///                            * Size of DAIF registers
///                            ###
///               : AVIO_DAIF_REG_DEC_BIT 0xB
///                            ###
///                            * 2KB has a 11 bits offset
///                            ###
///               : AVIO_DRT_REG_BASE  0x64800
///                            ###
///                            * Base address of DRT Registers
///                            * 512B
///                            ###
///               : AVIO_DRT_REG_SIZE  0x200
///                            ###
///                            * Size of DRT registers
///                            ###
///               : AVIO_DRT_REG_DEC_BIT 0x9
///                            ###
///                            * 512B has a 9 bits offset
///                            ###
///               : AVIO_DCLS_REG_BASE 0x64C00
///                            ###
///                            * Base address of DCLS Registers
///                            * 128B
///                            ###
///               : AVIO_DCLS_REG_SIZE 0x80
///                            ###
///                            * Size of DCLS Registers memory mapping
///                            ###
///               : AVIO_DCLS_REG_DEC_BIT 0x7
///                            ###
///                            * 128B has a 7 bits offset
///                            ###
///               : AVIO_BCM_REG_BASE  0x65000
///                            ###
///                            * Base address for AVIO BCM Registers
///                            * 512B
///                            ###
///               : AVIO_BCM_REG_SIZE  0x200
///                            ###
///                            * Size of AVIO BCM Registers memory mapping
///                            ###
///               : AVIO_BCM_REG_DEC_BIT 0x9
///                            ###
///                            * 512B has a 9 bits offset
///                            ###
///               : AVIO_PTRACK1_REG_BASE 0x65400
///                            ###
///                            * Base address for AVIO PTRACK1 Registers
///                            * 256B
///                            ###
///               : AVIO_PTRACK1_REG_SIZE 0x100
///                            ###
///                            * Size of AVIO PTRACK1 Registers memory mapping
///                            ###
///               : AVIO_PTRACK1_REG_DEC_BIT 0x8
///                            ###
///                            * 256B has a 8 bits offset
///                            ###
///               : AVIO_PTRACK2_REG_BASE 0x65800
///                            ###
///                            * Base address of AVIO PTRACK2 Registers
///                            * 256B
///                            ###
///               : AVIO_PTRACK2_REG_SIZE 0x100
///                            ###
///                            * Size of AVIO PTRACK2 registers
///                            ###
///               : AVIO_PTRACK2_REG_DEC_BIT 0x8
///                            ###
///                            * 256B has a 8 bits offset
///                            ###
///               : AVIO_I2S_REG_BASE  0x65C00
///                            ###
///                            * Base address for I2S Registers
///                            * 1KB
///                            ###
///               : AVIO_I2S_REG_SIZE  0x400
///                            ###
///                            * Size of I2S registers
///                            ###
///               : AVIO_I2S_REG_DEC_BIT 0xA
///                            ###
///                            * 1KB has a 10 bits offset
///                            ###
///               : AVIO_RESERVED0_REG_BASE 0x80000
///                            ###
///                            * 512KB AVIO reserved space
///                            * 512KB
///                            ###
///               : AVIO_RESERVED0_REG_SIZE 0x80000
///                            ###
///                            * 512KB reserved0 size
///                            ###
///               : AVIO_RESERVED0_REG_DEC_BIT 0x13
///                            ###
///                            * 512KB has a 19 bits offset
///                            ###
///               : AVIO_RESERVED1_REG_BASE 0x100000
///                            ###
///                            * 1MB space reserved
///                            * 1MB
///                            ###
///               : AVIO_RESERVED1_REG_SIZE 0x100000
///                            ###
///                            * Size of reserved1 registers
///                            ###
///               : AVIO_RESERVED1_REG_DEC_BIT 0x14
///                            ###
///                            * 1MB has a 20 bits offset
///                            ###
///     # # ----------------------------------------------------------
/// $ENDOFINTERFACE  # size:       0B, bits:       0b, padding:     0B
////////////////////////////////////////////////////////////
#ifndef h_AVIO_MEMMAP
#define h_AVIO_MEMMAP (){}

    #define        AVIO_MEMMAP_VPP128B_DHUB_REG_BASE           0x0
    #define        AVIO_MEMMAP_VPP128B_DHUB_REG_SIZE           0x10000
    #define        AVIO_MEMMAP_VPP128B_DHUB_REG_DEC_BIT        0x10
    #define        AVIO_MEMMAP_AIO64B_DHUB_REG_BASE            0x10000
    #define        AVIO_MEMMAP_AIO64B_DHUB_REG_SIZE            0x10000
    #define        AVIO_MEMMAP_AIO64B_DHUB_REG_DEC_BIT         0x10
    #define        AVIO_MEMMAP_AVIO_LCDC1_REG_BASE             0x20000
    #define        AVIO_MEMMAP_AVIO_LCDC1_REG_SIZE             0x10000
    #define        AVIO_MEMMAP_AVIO_LCDC1_REG_DEC_BIT          0x10
    #define        AVIO_MEMMAP_AVIO_LCDC2_REG_BASE             0x40000
    #define        AVIO_MEMMAP_AVIO_LCDC2_REG_SIZE             0x10000
    #define        AVIO_MEMMAP_AVIO_LCDC2_REG_DEC_BIT          0x10
    #define        AVIO_MEMMAP_AVIO_GBL_BASE                   0x60000
    #define        AVIO_MEMMAP_AVIO_GBL_SIZE                   0x4000
    #define        AVIO_MEMMAP_AVIO_GBL_DEC_BIT                0xE
    #define        AVIO_MEMMAP_AVIO_DAIF_REG_BASE              0x64000
    #define        AVIO_MEMMAP_AVIO_DAIF_REG_SIZE              0x800
    #define        AVIO_MEMMAP_AVIO_DAIF_REG_DEC_BIT           0xB
    #define        AVIO_MEMMAP_AVIO_DRT_REG_BASE               0x64800
    #define        AVIO_MEMMAP_AVIO_DRT_REG_SIZE               0x200
    #define        AVIO_MEMMAP_AVIO_DRT_REG_DEC_BIT            0x9
    #define        AVIO_MEMMAP_AVIO_DCLS_REG_BASE              0x64C00
    #define        AVIO_MEMMAP_AVIO_DCLS_REG_SIZE              0x80
    #define        AVIO_MEMMAP_AVIO_DCLS_REG_DEC_BIT           0x7
    #define        AVIO_MEMMAP_AVIO_BCM_REG_BASE               0x65000
    #define        AVIO_MEMMAP_AVIO_BCM_REG_SIZE               0x200
    #define        AVIO_MEMMAP_AVIO_BCM_REG_DEC_BIT            0x9
    #define        AVIO_MEMMAP_AVIO_PTRACK1_REG_BASE           0x65400
    #define        AVIO_MEMMAP_AVIO_PTRACK1_REG_SIZE           0x100
    #define        AVIO_MEMMAP_AVIO_PTRACK1_REG_DEC_BIT        0x8
    #define        AVIO_MEMMAP_AVIO_PTRACK2_REG_BASE           0x65800
    #define        AVIO_MEMMAP_AVIO_PTRACK2_REG_SIZE           0x100
    #define        AVIO_MEMMAP_AVIO_PTRACK2_REG_DEC_BIT        0x8
    #define        AVIO_MEMMAP_AVIO_I2S_REG_BASE               0x65C00
    #define        AVIO_MEMMAP_AVIO_I2S_REG_SIZE               0x400
    #define        AVIO_MEMMAP_AVIO_I2S_REG_DEC_BIT            0xA
    #define        AVIO_MEMMAP_AVIO_RESERVED0_REG_BASE         0x80000
    #define        AVIO_MEMMAP_AVIO_RESERVED0_REG_SIZE         0x80000
    #define        AVIO_MEMMAP_AVIO_RESERVED0_REG_DEC_BIT      0x13
    #define        AVIO_MEMMAP_AVIO_RESERVED1_REG_BASE         0x100000
    #define        AVIO_MEMMAP_AVIO_RESERVED1_REG_SIZE         0x100000
    #define        AVIO_MEMMAP_AVIO_RESERVED1_REG_DEC_BIT      0x14
    ///////////////////////////////////////////////////////////

#endif
//////
/// ENDOFINTERFACE: AVIO_MEMMAP
////////////////////////////////////////////////////////////



#ifdef __cplusplus
  }
#endif
#pragma  pack()

#endif
//////
/// ENDOFFILE: avio_memmap.h
////////////////////////////////////////////////////////////

