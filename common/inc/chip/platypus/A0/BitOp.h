//////
/// don't edit! auto-generated by docc: BitOp.h
////////////////////////////////////////////////////////////
#ifndef BitOp_h
#define BitOp_h (){}


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
/// $INTERFACE BitOpX4                 biu              (4,4)
///     ###
///     * Operator format for BitOp extension
///     ###
///     # # ----------------------------------------------------------
///     @ 0x00000                      (P)
///               %unsigned 6  idx
///                                    ###
///                                    * rS[idx] or xT[idx] is used as source bit
///                                    * In case src==rS, idx[5:4] specifies the operation mode:
///                                    * idx[5:4]==2'b0x: output is from rS
///                                    * idx[5:4]==2'b10: output bit is forced to 0
///                                    * idx[5:4]==2'b11: output bit is forced to 1
///                                    ###
///               %unsigned 1  src
///                                    : xT                        0x0
///                                    : rS                        0x1
///                                                 ###
///                                                 * If src=rS, the source is from 16-bit input
///                                                 * If src=xT, the source is from 64-bit input
///                                                 ###
///               %unsigned 1  mode
///                                    : copy                      0x0
///                                    : inv                       0x1
///                                                 ###
///                                                 * copy: target bit copied from source bit
///                                                 * inv: target bit copied from source bit then inverted
///                                                 * Not used when src==rS & idx[5]==1'b1
///                                                 ###
///               %unsigned 6  idx1
///               %unsigned 1  src1
///               %unsigned 1  mode1
///               %unsigned 6  idx2
///               %unsigned 1  src2
///               %unsigned 1  mode2
///               %unsigned 6  idx3
///               %unsigned 1  src3
///               %unsigned 1  mode3
///                                    ###
///                                    * End BitOpX4
///                                    ###
///     # # ----------------------------------------------------------
/// $ENDOFINTERFACE  # size:       4B, bits:      32b, padding:     0B
////////////////////////////////////////////////////////////
#ifndef h_BitOpX4
#define h_BitOpX4 (){}

    #define     BA_BitOpX4_idx                                 0x0000
    #define     B16BitOpX4_idx                                 0x0000
    #define   LSb32BitOpX4_idx                                    0
    #define   LSb16BitOpX4_idx                                    0
    #define       bBitOpX4_idx                                 6
    #define   MSK32BitOpX4_idx                                    0x0000003F

    #define     BA_BitOpX4_src                                 0x0000
    #define     B16BitOpX4_src                                 0x0000
    #define   LSb32BitOpX4_src                                    6
    #define   LSb16BitOpX4_src                                    6
    #define       bBitOpX4_src                                 1
    #define   MSK32BitOpX4_src                                    0x00000040
    #define        BitOpX4_src_xT                                           0x0
    #define        BitOpX4_src_rS                                           0x1

    #define     BA_BitOpX4_mode                                0x0000
    #define     B16BitOpX4_mode                                0x0000
    #define   LSb32BitOpX4_mode                                   7
    #define   LSb16BitOpX4_mode                                   7
    #define       bBitOpX4_mode                                1
    #define   MSK32BitOpX4_mode                                   0x00000080
    #define        BitOpX4_mode_copy                                        0x0
    #define        BitOpX4_mode_inv                                         0x1

    #define     BA_BitOpX4_idx1                                0x0001
    #define     B16BitOpX4_idx1                                0x0000
    #define   LSb32BitOpX4_idx1                                   8
    #define   LSb16BitOpX4_idx1                                   8
    #define       bBitOpX4_idx1                                6
    #define   MSK32BitOpX4_idx1                                   0x00003F00

    #define     BA_BitOpX4_src1                                0x0001
    #define     B16BitOpX4_src1                                0x0000
    #define   LSb32BitOpX4_src1                                   14
    #define   LSb16BitOpX4_src1                                   14
    #define       bBitOpX4_src1                                1
    #define   MSK32BitOpX4_src1                                   0x00004000

    #define     BA_BitOpX4_mode1                               0x0001
    #define     B16BitOpX4_mode1                               0x0000
    #define   LSb32BitOpX4_mode1                                  15
    #define   LSb16BitOpX4_mode1                                  15
    #define       bBitOpX4_mode1                               1
    #define   MSK32BitOpX4_mode1                                  0x00008000

    #define     BA_BitOpX4_idx2                                0x0002
    #define     B16BitOpX4_idx2                                0x0002
    #define   LSb32BitOpX4_idx2                                   16
    #define   LSb16BitOpX4_idx2                                   0
    #define       bBitOpX4_idx2                                6
    #define   MSK32BitOpX4_idx2                                   0x003F0000

    #define     BA_BitOpX4_src2                                0x0002
    #define     B16BitOpX4_src2                                0x0002
    #define   LSb32BitOpX4_src2                                   22
    #define   LSb16BitOpX4_src2                                   6
    #define       bBitOpX4_src2                                1
    #define   MSK32BitOpX4_src2                                   0x00400000

    #define     BA_BitOpX4_mode2                               0x0002
    #define     B16BitOpX4_mode2                               0x0002
    #define   LSb32BitOpX4_mode2                                  23
    #define   LSb16BitOpX4_mode2                                  7
    #define       bBitOpX4_mode2                               1
    #define   MSK32BitOpX4_mode2                                  0x00800000

    #define     BA_BitOpX4_idx3                                0x0003
    #define     B16BitOpX4_idx3                                0x0002
    #define   LSb32BitOpX4_idx3                                   24
    #define   LSb16BitOpX4_idx3                                   8
    #define       bBitOpX4_idx3                                6
    #define   MSK32BitOpX4_idx3                                   0x3F000000

    #define     BA_BitOpX4_src3                                0x0003
    #define     B16BitOpX4_src3                                0x0002
    #define   LSb32BitOpX4_src3                                   30
    #define   LSb16BitOpX4_src3                                   14
    #define       bBitOpX4_src3                                1
    #define   MSK32BitOpX4_src3                                   0x40000000

    #define     BA_BitOpX4_mode3                               0x0003
    #define     B16BitOpX4_mode3                               0x0002
    #define   LSb32BitOpX4_mode3                                  31
    #define   LSb16BitOpX4_mode3                                  15
    #define       bBitOpX4_mode3                               1
    #define   MSK32BitOpX4_mode3                                  0x80000000
    ///////////////////////////////////////////////////////////

    typedef struct SIE_BitOpX4 {
    ///////////////////////////////////////////////////////////
    #define   GET32BitOpX4_idx(r32)                            _BFGET_(r32, 5, 0)
    #define   SET32BitOpX4_idx(r32,v)                          _BFSET_(r32, 5, 0,v)
    #define   GET16BitOpX4_idx(r16)                            _BFGET_(r16, 5, 0)
    #define   SET16BitOpX4_idx(r16,v)                          _BFSET_(r16, 5, 0,v)

    #define   GET32BitOpX4_src(r32)                            _BFGET_(r32, 6, 6)
    #define   SET32BitOpX4_src(r32,v)                          _BFSET_(r32, 6, 6,v)
    #define   GET16BitOpX4_src(r16)                            _BFGET_(r16, 6, 6)
    #define   SET16BitOpX4_src(r16,v)                          _BFSET_(r16, 6, 6,v)

    #define   GET32BitOpX4_mode(r32)                           _BFGET_(r32, 7, 7)
    #define   SET32BitOpX4_mode(r32,v)                         _BFSET_(r32, 7, 7,v)
    #define   GET16BitOpX4_mode(r16)                           _BFGET_(r16, 7, 7)
    #define   SET16BitOpX4_mode(r16,v)                         _BFSET_(r16, 7, 7,v)

    #define   GET32BitOpX4_idx1(r32)                           _BFGET_(r32,13, 8)
    #define   SET32BitOpX4_idx1(r32,v)                         _BFSET_(r32,13, 8,v)
    #define   GET16BitOpX4_idx1(r16)                           _BFGET_(r16,13, 8)
    #define   SET16BitOpX4_idx1(r16,v)                         _BFSET_(r16,13, 8,v)

    #define   GET32BitOpX4_src1(r32)                           _BFGET_(r32,14,14)
    #define   SET32BitOpX4_src1(r32,v)                         _BFSET_(r32,14,14,v)
    #define   GET16BitOpX4_src1(r16)                           _BFGET_(r16,14,14)
    #define   SET16BitOpX4_src1(r16,v)                         _BFSET_(r16,14,14,v)

    #define   GET32BitOpX4_mode1(r32)                          _BFGET_(r32,15,15)
    #define   SET32BitOpX4_mode1(r32,v)                        _BFSET_(r32,15,15,v)
    #define   GET16BitOpX4_mode1(r16)                          _BFGET_(r16,15,15)
    #define   SET16BitOpX4_mode1(r16,v)                        _BFSET_(r16,15,15,v)

    #define   GET32BitOpX4_idx2(r32)                           _BFGET_(r32,21,16)
    #define   SET32BitOpX4_idx2(r32,v)                         _BFSET_(r32,21,16,v)
    #define   GET16BitOpX4_idx2(r16)                           _BFGET_(r16, 5, 0)
    #define   SET16BitOpX4_idx2(r16,v)                         _BFSET_(r16, 5, 0,v)

    #define   GET32BitOpX4_src2(r32)                           _BFGET_(r32,22,22)
    #define   SET32BitOpX4_src2(r32,v)                         _BFSET_(r32,22,22,v)
    #define   GET16BitOpX4_src2(r16)                           _BFGET_(r16, 6, 6)
    #define   SET16BitOpX4_src2(r16,v)                         _BFSET_(r16, 6, 6,v)

    #define   GET32BitOpX4_mode2(r32)                          _BFGET_(r32,23,23)
    #define   SET32BitOpX4_mode2(r32,v)                        _BFSET_(r32,23,23,v)
    #define   GET16BitOpX4_mode2(r16)                          _BFGET_(r16, 7, 7)
    #define   SET16BitOpX4_mode2(r16,v)                        _BFSET_(r16, 7, 7,v)

    #define   GET32BitOpX4_idx3(r32)                           _BFGET_(r32,29,24)
    #define   SET32BitOpX4_idx3(r32,v)                         _BFSET_(r32,29,24,v)
    #define   GET16BitOpX4_idx3(r16)                           _BFGET_(r16,13, 8)
    #define   SET16BitOpX4_idx3(r16,v)                         _BFSET_(r16,13, 8,v)

    #define   GET32BitOpX4_src3(r32)                           _BFGET_(r32,30,30)
    #define   SET32BitOpX4_src3(r32,v)                         _BFSET_(r32,30,30,v)
    #define   GET16BitOpX4_src3(r16)                           _BFGET_(r16,14,14)
    #define   SET16BitOpX4_src3(r16,v)                         _BFSET_(r16,14,14,v)

    #define   GET32BitOpX4_mode3(r32)                          _BFGET_(r32,31,31)
    #define   SET32BitOpX4_mode3(r32,v)                        _BFSET_(r32,31,31,v)
    #define   GET16BitOpX4_mode3(r16)                          _BFGET_(r16,15,15)
    #define   SET16BitOpX4_mode3(r16,v)                        _BFSET_(r16,15,15,v)

            UNSG32 u_idx                                       :  6;
            UNSG32 u_src                                       :  1;
            UNSG32 u_mode                                      :  1;
            UNSG32 u_idx1                                      :  6;
            UNSG32 u_src1                                      :  1;
            UNSG32 u_mode1                                     :  1;
            UNSG32 u_idx2                                      :  6;
            UNSG32 u_src2                                      :  1;
            UNSG32 u_mode2                                     :  1;
            UNSG32 u_idx3                                      :  6;
            UNSG32 u_src3                                      :  1;
            UNSG32 u_mode3                                     :  1;
    ///////////////////////////////////////////////////////////
    } SIE_BitOpX4;

    ///////////////////////////////////////////////////////////
     SIGN32 BitOpX4_drvrd(SIE_BitOpX4 *p, UNSG32 base, SIGN32 mem, SIGN32 tst);
     SIGN32 BitOpX4_drvwr(SIE_BitOpX4 *p, UNSG32 base, SIGN32 mem, SIGN32 tst, UNSG32 *pcmd);
       void BitOpX4_reset(SIE_BitOpX4 *p);
     SIGN32 BitOpX4_cmp  (SIE_BitOpX4 *p, SIE_BitOpX4 *pie, char *pfx, void *hLOG, SIGN32 mem, SIGN32 tst);
    #define BitOpX4_check(p,pie,pfx,hLOG) BitOpX4_cmp(p,pie,pfx,(void*)(hLOG),0,0)
    #define BitOpX4_print(p,    pfx,hLOG) BitOpX4_cmp(p,0,  pfx,(void*)(hLOG),0,0)

#endif
//////
/// ENDOFINTERFACE: BitOpX4
////////////////////////////////////////////////////////////

//////
///
/// $INTERFACE BITOPRF64               biu              (4,4)
///     ###
///     * Operator format for BitOp extension
///     ###
///     # # ----------------------------------------------------------
///               : BitOp0             0x20
///               : BitOp1             0x21
///               : BitOp2             0x22
///               : BitOp3             0x23
///                            ###
///                            * extension ID definition; must be consistent with vScope_F0A64_BitOp0~3
///                            ###
///     @ 0x00000                      (P)
///     # 0x00000 cmd0
///               $BitOpX4             cmd0              REG       [4]
///     @ 0x00010                      (P)
///     # 0x00010 cmd1
///               $BitOpX4             cmd1              REG       [4]
///     @ 0x00020                      (P)
///     # 0x00020 cmd2
///               $BitOpX4             cmd2              REG       [4]
///     @ 0x00030                      (P)
///     # 0x00030 cmd3
///               $BitOpX4             cmd3              REG       [4]
///                                    ###
///                                    * Four BitOp commands, selected by extension ID
///                                    * End BitOpCtx
///                                    ###
///     # # ----------------------------------------------------------
/// $ENDOFINTERFACE  # size:      64B, bits:     512b, padding:     0B
////////////////////////////////////////////////////////////
#ifndef h_BITOPRF64
#define h_BITOPRF64 (){}

    #define        BITOPRF64_BitOp0                            0x20
    #define        BITOPRF64_BitOp1                            0x21
    #define        BITOPRF64_BitOp2                            0x22
    #define        BITOPRF64_BitOp3                            0x23
    ///////////////////////////////////////////////////////////
    #define     RA_BITOPRF64_cmd0                              0x0000
    ///////////////////////////////////////////////////////////
    #define     RA_BITOPRF64_cmd1                              0x0010
    ///////////////////////////////////////////////////////////
    #define     RA_BITOPRF64_cmd2                              0x0020
    ///////////////////////////////////////////////////////////
    #define     RA_BITOPRF64_cmd3                              0x0030
    ///////////////////////////////////////////////////////////

    typedef struct SIE_BITOPRF64 {
    ///////////////////////////////////////////////////////////
              SIE_BitOpX4                                      ie_cmd0[4];
    ///////////////////////////////////////////////////////////
              SIE_BitOpX4                                      ie_cmd1[4];
    ///////////////////////////////////////////////////////////
              SIE_BitOpX4                                      ie_cmd2[4];
    ///////////////////////////////////////////////////////////
              SIE_BitOpX4                                      ie_cmd3[4];
    ///////////////////////////////////////////////////////////
    } SIE_BITOPRF64;

    ///////////////////////////////////////////////////////////
     SIGN32 BITOPRF64_drvrd(SIE_BITOPRF64 *p, UNSG32 base, SIGN32 mem, SIGN32 tst);
     SIGN32 BITOPRF64_drvwr(SIE_BITOPRF64 *p, UNSG32 base, SIGN32 mem, SIGN32 tst, UNSG32 *pcmd);
       void BITOPRF64_reset(SIE_BITOPRF64 *p);
     SIGN32 BITOPRF64_cmp  (SIE_BITOPRF64 *p, SIE_BITOPRF64 *pie, char *pfx, void *hLOG, SIGN32 mem, SIGN32 tst);
    #define BITOPRF64_check(p,pie,pfx,hLOG) BITOPRF64_cmp(p,pie,pfx,(void*)(hLOG),0,0)
    #define BITOPRF64_print(p,    pfx,hLOG) BITOPRF64_cmp(p,0,  pfx,(void*)(hLOG),0,0)

#endif
//////
/// ENDOFINTERFACE: BITOPRF64
////////////////////////////////////////////////////////////



#ifdef __cplusplus
  }
#endif
#pragma  pack()

#endif
//////
/// ENDOFFILE: BitOp.h
////////////////////////////////////////////////////////////

