//////
/// don't edit! auto-generated by docc: J2A.h
////////////////////////////////////////////////////////////
#ifndef J2A_h
#define J2A_h (){}


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
/// $INTERFACE AHB_DATA                biu              (4,4)
///     ###
///     * format definition for the AHB_DATA register
///     * [00:31]
///     ###
///     # # ----------------------------------------------------------
///     @ 0x00000 data                 (P)
///               %unsigned 32 val                       
///                                    ###
///                                    * content of the read/write data registers
///                                    * [32:63]
///                                    ###
///     @ 0x00004 flag                 (P)
///               %unsigned 1  rd                        
///                                    ###
///                                    * outgoing: read data is valid or not;
///                                    * incoming: a new AHB read should be triggered
///                                    ###
///               %unsigned 1  wr                        
///                                    ###
///                                    * outgoing: 0
///                                    * incoming: a new AHB write should be triggered
///                                    ###
///               %%        30         # Stuffing bits...
///     # # ----------------------------------------------------------
/// $ENDOFINTERFACE  # size:       8B, bits:      34b, padding:     0B
////////////////////////////////////////////////////////////
#ifndef h_AHB_DATA
#define h_AHB_DATA (){}

    #define     RA_AHB_DATA_data                               0x0000

    #define     BA_AHB_DATA_data_val                           0x0000
    #define     B16AHB_DATA_data_val                           0x0000
    #define   LSb32AHB_DATA_data_val                              0
    #define   LSb16AHB_DATA_data_val                              0
    #define       bAHB_DATA_data_val                           32
    #define   MSK32AHB_DATA_data_val                              0xFFFFFFFF
    ///////////////////////////////////////////////////////////
    #define     RA_AHB_DATA_flag                               0x0004

    #define     BA_AHB_DATA_flag_rd                            0x0004
    #define     B16AHB_DATA_flag_rd                            0x0004
    #define   LSb32AHB_DATA_flag_rd                               0
    #define   LSb16AHB_DATA_flag_rd                               0
    #define       bAHB_DATA_flag_rd                            1
    #define   MSK32AHB_DATA_flag_rd                               0x00000001

    #define     BA_AHB_DATA_flag_wr                            0x0004
    #define     B16AHB_DATA_flag_wr                            0x0004
    #define   LSb32AHB_DATA_flag_wr                               1
    #define   LSb16AHB_DATA_flag_wr                               1
    #define       bAHB_DATA_flag_wr                            1
    #define   MSK32AHB_DATA_flag_wr                               0x00000002
    ///////////////////////////////////////////////////////////

    typedef struct SIE_AHB_DATA {
    ///////////////////////////////////////////////////////////
    #define   GET32AHB_DATA_data_val(r32)                      _BFGET_(r32,31, 0)
    #define   SET32AHB_DATA_data_val(r32,v)                    _BFSET_(r32,31, 0,v)

    #define     w32AHB_DATA_data                               {\
            UNSG32 udata_val                                   : 32;\
          }
    union { UNSG32 u32AHB_DATA_data;
            struct w32AHB_DATA_data;
          };
    ///////////////////////////////////////////////////////////
    #define   GET32AHB_DATA_flag_rd(r32)                       _BFGET_(r32, 0, 0)
    #define   SET32AHB_DATA_flag_rd(r32,v)                     _BFSET_(r32, 0, 0,v)
    #define   GET16AHB_DATA_flag_rd(r16)                       _BFGET_(r16, 0, 0)
    #define   SET16AHB_DATA_flag_rd(r16,v)                     _BFSET_(r16, 0, 0,v)

    #define   GET32AHB_DATA_flag_wr(r32)                       _BFGET_(r32, 1, 1)
    #define   SET32AHB_DATA_flag_wr(r32,v)                     _BFSET_(r32, 1, 1,v)
    #define   GET16AHB_DATA_flag_wr(r16)                       _BFGET_(r16, 1, 1)
    #define   SET16AHB_DATA_flag_wr(r16,v)                     _BFSET_(r16, 1, 1,v)

    #define     w32AHB_DATA_flag                               {\
            UNSG32 uflag_rd                                    :  1;\
            UNSG32 uflag_wr                                    :  1;\
            UNSG32 RSVDx4_b2                                   : 30;\
          }
    union { UNSG32 u32AHB_DATA_flag;
            struct w32AHB_DATA_flag;
          };
    ///////////////////////////////////////////////////////////
    } SIE_AHB_DATA;

    typedef union  T32AHB_DATA_data
          { UNSG32 u32;
            struct w32AHB_DATA_data;
                 } T32AHB_DATA_data;
    typedef union  T32AHB_DATA_flag
          { UNSG32 u32;
            struct w32AHB_DATA_flag;
                 } T32AHB_DATA_flag;
    ///////////////////////////////////////////////////////////

    typedef union  TAHB_DATA_data
          { UNSG32 u32[1];
            struct {
            struct w32AHB_DATA_data;
                   };
                 } TAHB_DATA_data;
    typedef union  TAHB_DATA_flag
          { UNSG32 u32[1];
            struct {
            struct w32AHB_DATA_flag;
                   };
                 } TAHB_DATA_flag;

    ///////////////////////////////////////////////////////////
     SIGN32 AHB_DATA_drvrd(SIE_AHB_DATA *p, UNSG32 base, SIGN32 mem, SIGN32 tst);
     SIGN32 AHB_DATA_drvwr(SIE_AHB_DATA *p, UNSG32 base, SIGN32 mem, SIGN32 tst, UNSG32 *pcmd);
       void AHB_DATA_reset(SIE_AHB_DATA *p);
     SIGN32 AHB_DATA_cmp  (SIE_AHB_DATA *p, SIE_AHB_DATA *pie, char *pfx, void *hLOG, SIGN32 mem, SIGN32 tst);
    #define AHB_DATA_check(p,pie,pfx,hLOG) AHB_DATA_cmp(p,pie,pfx,(void*)(hLOG),0,0)
    #define AHB_DATA_print(p,    pfx,hLOG) AHB_DATA_cmp(p,0,  pfx,(void*)(hLOG),0,0)

#endif
//////
/// ENDOFINTERFACE: AHB_DATA
////////////////////////////////////////////////////////////



#ifdef __cplusplus
  }
#endif
#pragma  pack()

#endif
//////
/// ENDOFFILE: J2A.h
////////////////////////////////////////////////////////////

