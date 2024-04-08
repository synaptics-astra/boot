//////
/// don't edit! auto-generated by docc: abipll.h
////////////////////////////////////////////////////////////
#ifndef abipll_h
#define abipll_h (){}


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
/// $INTERFACE abipll                  biu              (4,4)
///     # # ----------------------------------------------------------
///     @ 0x00000 ctrlA                (P-)
///               ###
///               * PLL Control
///               ###
///               %unsigned 1  RESET                     0x0
///                                    ###
///                                    * PLL Internal Reset Active High
///                                    ###
///               %unsigned 1  BYPASS                    0x0
///                                    ###
///                                    * PLL Bypass Enable Active High
///                                    ###
///               %unsigned 1  NEWDIV                    0x0
///                                    ###
///                                    * Fractional Divide Input Control
///                                    ###
///               %unsigned 3  RANGE                     0x2
///                                    ###
///                                    * PLL Filter Range
///                                    ###
///               %%        26         # Stuffing bits...
///     @ 0x00004 ctrlB                (P-)
///               %unsigned 4  SSMF                      0x0
///                                    ###
///                                    * Spread Spectrum Modulation Frequency
///                                    ###
///               %unsigned 3  SSMD                      0x0
///                                    ###
///                                    * Spread Spectrum Modulation Depth
///                                    ###
///               %unsigned 1  SSE_RSVD                  0x0
///                                    ###
///                                    * Spread Spectrum Reserve Bit
///                                    ###
///               %unsigned 1  SSE                       0x0
///                                    ###
///                                    * Enables Spread Spectrum Mode Active High
///                                    ###
///               %unsigned 1  SSDS                      0x0
///                                    ###
///                                    * Spread Spectrum Down Spread Center Spread
///                                    ###
///               %%        22         # Stuffing bits...
///     @ 0x00008 ctrlC                (P-)
///               %unsigned 6  DIVR                      0x1
///                                    ###
///                                    * Reference Divider
///                                    ###
///               %%        26         # Stuffing bits...
///     @ 0x0000C ctrlD                (P-)
///               %unsigned 9  DIVFI                     0x1F
///                                    ###
///                                    * Integer Divider
///                                    ###
///               %%        23         # Stuffing bits...
///     @ 0x00010 ctrlE                (P-)
///               %unsigned 24 DIVFF                     0x0
///                                    ###
///                                    * Fractional Divider Control
///                                    ###
///               %%        8          # Stuffing bits...
///     @ 0x00014 ctrlF                (P-)
///               %unsigned 5  DIVQ                      0x1
///                                    ###
///                                    * Output Divider Control for PLLOUT
///                                    ###
///               %%        27         # Stuffing bits...
///     @ 0x00018 ctrlG                (P-)
///               %unsigned 3  DIVQF                     0x3
///                                    ###
///                                    * Output Divider Control for PLLOUTF
///                                    ###
///               %%        29         # Stuffing bits...
///     @ 0x0001C status               (R-)
///               ###
///               * PLL status register
///               ###
///               %unsigned 1  LOCK                      
///                                    ###
///                                    * Output. Lock detection
///                                    ###
///               %unsigned 1  DIVACK                    
///                                    ###
///                                    * Fractional Divide Hand-shake signal
///                                    ###
///               %%        30         # Stuffing bits...
///     # # ----------------------------------------------------------
/// $ENDOFINTERFACE  # size:      32B, bits:      65b, padding:     0B
////////////////////////////////////////////////////////////
#ifndef h_abipll
#define h_abipll (){}

    #define     RA_abipll_ctrlA                                0x0000

    #define     BA_abipll_ctrlA_RESET                          0x0000
    #define     B16abipll_ctrlA_RESET                          0x0000
    #define   LSb32abipll_ctrlA_RESET                             0
    #define   LSb16abipll_ctrlA_RESET                             0
    #define       babipll_ctrlA_RESET                          1
    #define   MSK32abipll_ctrlA_RESET                             0x00000001

    #define     BA_abipll_ctrlA_BYPASS                         0x0000
    #define     B16abipll_ctrlA_BYPASS                         0x0000
    #define   LSb32abipll_ctrlA_BYPASS                            1
    #define   LSb16abipll_ctrlA_BYPASS                            1
    #define       babipll_ctrlA_BYPASS                         1
    #define   MSK32abipll_ctrlA_BYPASS                            0x00000002

    #define     BA_abipll_ctrlA_NEWDIV                         0x0000
    #define     B16abipll_ctrlA_NEWDIV                         0x0000
    #define   LSb32abipll_ctrlA_NEWDIV                            2
    #define   LSb16abipll_ctrlA_NEWDIV                            2
    #define       babipll_ctrlA_NEWDIV                         1
    #define   MSK32abipll_ctrlA_NEWDIV                            0x00000004

    #define     BA_abipll_ctrlA_RANGE                          0x0000
    #define     B16abipll_ctrlA_RANGE                          0x0000
    #define   LSb32abipll_ctrlA_RANGE                             3
    #define   LSb16abipll_ctrlA_RANGE                             3
    #define       babipll_ctrlA_RANGE                          3
    #define   MSK32abipll_ctrlA_RANGE                             0x00000038
    ///////////////////////////////////////////////////////////
    #define     RA_abipll_ctrlB                                0x0004

    #define     BA_abipll_ctrlB_SSMF                           0x0004
    #define     B16abipll_ctrlB_SSMF                           0x0004
    #define   LSb32abipll_ctrlB_SSMF                              0
    #define   LSb16abipll_ctrlB_SSMF                              0
    #define       babipll_ctrlB_SSMF                           4
    #define   MSK32abipll_ctrlB_SSMF                              0x0000000F

    #define     BA_abipll_ctrlB_SSMD                           0x0004
    #define     B16abipll_ctrlB_SSMD                           0x0004
    #define   LSb32abipll_ctrlB_SSMD                              4
    #define   LSb16abipll_ctrlB_SSMD                              4
    #define       babipll_ctrlB_SSMD                           3
    #define   MSK32abipll_ctrlB_SSMD                              0x00000070

    #define     BA_abipll_ctrlB_SSE_RSVD                       0x0004
    #define     B16abipll_ctrlB_SSE_RSVD                       0x0004
    #define   LSb32abipll_ctrlB_SSE_RSVD                          7
    #define   LSb16abipll_ctrlB_SSE_RSVD                          7
    #define       babipll_ctrlB_SSE_RSVD                       1
    #define   MSK32abipll_ctrlB_SSE_RSVD                          0x00000080

    #define     BA_abipll_ctrlB_SSE                            0x0005
    #define     B16abipll_ctrlB_SSE                            0x0004
    #define   LSb32abipll_ctrlB_SSE                               8
    #define   LSb16abipll_ctrlB_SSE                               8
    #define       babipll_ctrlB_SSE                            1
    #define   MSK32abipll_ctrlB_SSE                               0x00000100

    #define     BA_abipll_ctrlB_SSDS                           0x0005
    #define     B16abipll_ctrlB_SSDS                           0x0004
    #define   LSb32abipll_ctrlB_SSDS                              9
    #define   LSb16abipll_ctrlB_SSDS                              9
    #define       babipll_ctrlB_SSDS                           1
    #define   MSK32abipll_ctrlB_SSDS                              0x00000200
    ///////////////////////////////////////////////////////////
    #define     RA_abipll_ctrlC                                0x0008

    #define     BA_abipll_ctrlC_DIVR                           0x0008
    #define     B16abipll_ctrlC_DIVR                           0x0008
    #define   LSb32abipll_ctrlC_DIVR                              0
    #define   LSb16abipll_ctrlC_DIVR                              0
    #define       babipll_ctrlC_DIVR                           6
    #define   MSK32abipll_ctrlC_DIVR                              0x0000003F
    ///////////////////////////////////////////////////////////
    #define     RA_abipll_ctrlD                                0x000C

    #define     BA_abipll_ctrlD_DIVFI                          0x000C
    #define     B16abipll_ctrlD_DIVFI                          0x000C
    #define   LSb32abipll_ctrlD_DIVFI                             0
    #define   LSb16abipll_ctrlD_DIVFI                             0
    #define       babipll_ctrlD_DIVFI                          9
    #define   MSK32abipll_ctrlD_DIVFI                             0x000001FF
    ///////////////////////////////////////////////////////////
    #define     RA_abipll_ctrlE                                0x0010

    #define     BA_abipll_ctrlE_DIVFF                          0x0010
    #define     B16abipll_ctrlE_DIVFF                          0x0010
    #define   LSb32abipll_ctrlE_DIVFF                             0
    #define   LSb16abipll_ctrlE_DIVFF                             0
    #define       babipll_ctrlE_DIVFF                          24
    #define   MSK32abipll_ctrlE_DIVFF                             0x00FFFFFF
    ///////////////////////////////////////////////////////////
    #define     RA_abipll_ctrlF                                0x0014

    #define     BA_abipll_ctrlF_DIVQ                           0x0014
    #define     B16abipll_ctrlF_DIVQ                           0x0014
    #define   LSb32abipll_ctrlF_DIVQ                              0
    #define   LSb16abipll_ctrlF_DIVQ                              0
    #define       babipll_ctrlF_DIVQ                           5
    #define   MSK32abipll_ctrlF_DIVQ                              0x0000001F
    ///////////////////////////////////////////////////////////
    #define     RA_abipll_ctrlG                                0x0018

    #define     BA_abipll_ctrlG_DIVQF                          0x0018
    #define     B16abipll_ctrlG_DIVQF                          0x0018
    #define   LSb32abipll_ctrlG_DIVQF                             0
    #define   LSb16abipll_ctrlG_DIVQF                             0
    #define       babipll_ctrlG_DIVQF                          3
    #define   MSK32abipll_ctrlG_DIVQF                             0x00000007
    ///////////////////////////////////////////////////////////
    #define     RA_abipll_status                               0x001C

    #define     BA_abipll_status_LOCK                          0x001C
    #define     B16abipll_status_LOCK                          0x001C
    #define   LSb32abipll_status_LOCK                             0
    #define   LSb16abipll_status_LOCK                             0
    #define       babipll_status_LOCK                          1
    #define   MSK32abipll_status_LOCK                             0x00000001

    #define     BA_abipll_status_DIVACK                        0x001C
    #define     B16abipll_status_DIVACK                        0x001C
    #define   LSb32abipll_status_DIVACK                           1
    #define   LSb16abipll_status_DIVACK                           1
    #define       babipll_status_DIVACK                        1
    #define   MSK32abipll_status_DIVACK                           0x00000002
    ///////////////////////////////////////////////////////////

    typedef struct SIE_abipll {
    ///////////////////////////////////////////////////////////
    #define   GET32abipll_ctrlA_RESET(r32)                     _BFGET_(r32, 0, 0)
    #define   SET32abipll_ctrlA_RESET(r32,v)                   _BFSET_(r32, 0, 0,v)
    #define   GET16abipll_ctrlA_RESET(r16)                     _BFGET_(r16, 0, 0)
    #define   SET16abipll_ctrlA_RESET(r16,v)                   _BFSET_(r16, 0, 0,v)

    #define   GET32abipll_ctrlA_BYPASS(r32)                    _BFGET_(r32, 1, 1)
    #define   SET32abipll_ctrlA_BYPASS(r32,v)                  _BFSET_(r32, 1, 1,v)
    #define   GET16abipll_ctrlA_BYPASS(r16)                    _BFGET_(r16, 1, 1)
    #define   SET16abipll_ctrlA_BYPASS(r16,v)                  _BFSET_(r16, 1, 1,v)

    #define   GET32abipll_ctrlA_NEWDIV(r32)                    _BFGET_(r32, 2, 2)
    #define   SET32abipll_ctrlA_NEWDIV(r32,v)                  _BFSET_(r32, 2, 2,v)
    #define   GET16abipll_ctrlA_NEWDIV(r16)                    _BFGET_(r16, 2, 2)
    #define   SET16abipll_ctrlA_NEWDIV(r16,v)                  _BFSET_(r16, 2, 2,v)

    #define   GET32abipll_ctrlA_RANGE(r32)                     _BFGET_(r32, 5, 3)
    #define   SET32abipll_ctrlA_RANGE(r32,v)                   _BFSET_(r32, 5, 3,v)
    #define   GET16abipll_ctrlA_RANGE(r16)                     _BFGET_(r16, 5, 3)
    #define   SET16abipll_ctrlA_RANGE(r16,v)                   _BFSET_(r16, 5, 3,v)

    #define     w32abipll_ctrlA                                {\
            UNSG32 uctrlA_RESET                                :  1;\
            UNSG32 uctrlA_BYPASS                               :  1;\
            UNSG32 uctrlA_NEWDIV                               :  1;\
            UNSG32 uctrlA_RANGE                                :  3;\
            UNSG32 RSVDx0_b6                                   : 26;\
          }
    union { UNSG32 u32abipll_ctrlA;
            struct w32abipll_ctrlA;
          };
    ///////////////////////////////////////////////////////////
    #define   GET32abipll_ctrlB_SSMF(r32)                      _BFGET_(r32, 3, 0)
    #define   SET32abipll_ctrlB_SSMF(r32,v)                    _BFSET_(r32, 3, 0,v)
    #define   GET16abipll_ctrlB_SSMF(r16)                      _BFGET_(r16, 3, 0)
    #define   SET16abipll_ctrlB_SSMF(r16,v)                    _BFSET_(r16, 3, 0,v)

    #define   GET32abipll_ctrlB_SSMD(r32)                      _BFGET_(r32, 6, 4)
    #define   SET32abipll_ctrlB_SSMD(r32,v)                    _BFSET_(r32, 6, 4,v)
    #define   GET16abipll_ctrlB_SSMD(r16)                      _BFGET_(r16, 6, 4)
    #define   SET16abipll_ctrlB_SSMD(r16,v)                    _BFSET_(r16, 6, 4,v)

    #define   GET32abipll_ctrlB_SSE_RSVD(r32)                  _BFGET_(r32, 7, 7)
    #define   SET32abipll_ctrlB_SSE_RSVD(r32,v)                _BFSET_(r32, 7, 7,v)
    #define   GET16abipll_ctrlB_SSE_RSVD(r16)                  _BFGET_(r16, 7, 7)
    #define   SET16abipll_ctrlB_SSE_RSVD(r16,v)                _BFSET_(r16, 7, 7,v)

    #define   GET32abipll_ctrlB_SSE(r32)                       _BFGET_(r32, 8, 8)
    #define   SET32abipll_ctrlB_SSE(r32,v)                     _BFSET_(r32, 8, 8,v)
    #define   GET16abipll_ctrlB_SSE(r16)                       _BFGET_(r16, 8, 8)
    #define   SET16abipll_ctrlB_SSE(r16,v)                     _BFSET_(r16, 8, 8,v)

    #define   GET32abipll_ctrlB_SSDS(r32)                      _BFGET_(r32, 9, 9)
    #define   SET32abipll_ctrlB_SSDS(r32,v)                    _BFSET_(r32, 9, 9,v)
    #define   GET16abipll_ctrlB_SSDS(r16)                      _BFGET_(r16, 9, 9)
    #define   SET16abipll_ctrlB_SSDS(r16,v)                    _BFSET_(r16, 9, 9,v)

    #define     w32abipll_ctrlB                                {\
            UNSG32 uctrlB_SSMF                                 :  4;\
            UNSG32 uctrlB_SSMD                                 :  3;\
            UNSG32 uctrlB_SSE_RSVD                             :  1;\
            UNSG32 uctrlB_SSE                                  :  1;\
            UNSG32 uctrlB_SSDS                                 :  1;\
            UNSG32 RSVDx4_b10                                  : 22;\
          }
    union { UNSG32 u32abipll_ctrlB;
            struct w32abipll_ctrlB;
          };
    ///////////////////////////////////////////////////////////
    #define   GET32abipll_ctrlC_DIVR(r32)                      _BFGET_(r32, 5, 0)
    #define   SET32abipll_ctrlC_DIVR(r32,v)                    _BFSET_(r32, 5, 0,v)
    #define   GET16abipll_ctrlC_DIVR(r16)                      _BFGET_(r16, 5, 0)
    #define   SET16abipll_ctrlC_DIVR(r16,v)                    _BFSET_(r16, 5, 0,v)

    #define     w32abipll_ctrlC                                {\
            UNSG32 uctrlC_DIVR                                 :  6;\
            UNSG32 RSVDx8_b6                                   : 26;\
          }
    union { UNSG32 u32abipll_ctrlC;
            struct w32abipll_ctrlC;
          };
    ///////////////////////////////////////////////////////////
    #define   GET32abipll_ctrlD_DIVFI(r32)                     _BFGET_(r32, 8, 0)
    #define   SET32abipll_ctrlD_DIVFI(r32,v)                   _BFSET_(r32, 8, 0,v)
    #define   GET16abipll_ctrlD_DIVFI(r16)                     _BFGET_(r16, 8, 0)
    #define   SET16abipll_ctrlD_DIVFI(r16,v)                   _BFSET_(r16, 8, 0,v)

    #define     w32abipll_ctrlD                                {\
            UNSG32 uctrlD_DIVFI                                :  9;\
            UNSG32 RSVDxC_b9                                   : 23;\
          }
    union { UNSG32 u32abipll_ctrlD;
            struct w32abipll_ctrlD;
          };
    ///////////////////////////////////////////////////////////
    #define   GET32abipll_ctrlE_DIVFF(r32)                     _BFGET_(r32,23, 0)
    #define   SET32abipll_ctrlE_DIVFF(r32,v)                   _BFSET_(r32,23, 0,v)

    #define     w32abipll_ctrlE                                {\
            UNSG32 uctrlE_DIVFF                                : 24;\
            UNSG32 RSVDx10_b24                                 :  8;\
          }
    union { UNSG32 u32abipll_ctrlE;
            struct w32abipll_ctrlE;
          };
    ///////////////////////////////////////////////////////////
    #define   GET32abipll_ctrlF_DIVQ(r32)                      _BFGET_(r32, 4, 0)
    #define   SET32abipll_ctrlF_DIVQ(r32,v)                    _BFSET_(r32, 4, 0,v)
    #define   GET16abipll_ctrlF_DIVQ(r16)                      _BFGET_(r16, 4, 0)
    #define   SET16abipll_ctrlF_DIVQ(r16,v)                    _BFSET_(r16, 4, 0,v)

    #define     w32abipll_ctrlF                                {\
            UNSG32 uctrlF_DIVQ                                 :  5;\
            UNSG32 RSVDx14_b5                                  : 27;\
          }
    union { UNSG32 u32abipll_ctrlF;
            struct w32abipll_ctrlF;
          };
    ///////////////////////////////////////////////////////////
    #define   GET32abipll_ctrlG_DIVQF(r32)                     _BFGET_(r32, 2, 0)
    #define   SET32abipll_ctrlG_DIVQF(r32,v)                   _BFSET_(r32, 2, 0,v)
    #define   GET16abipll_ctrlG_DIVQF(r16)                     _BFGET_(r16, 2, 0)
    #define   SET16abipll_ctrlG_DIVQF(r16,v)                   _BFSET_(r16, 2, 0,v)

    #define     w32abipll_ctrlG                                {\
            UNSG32 uctrlG_DIVQF                                :  3;\
            UNSG32 RSVDx18_b3                                  : 29;\
          }
    union { UNSG32 u32abipll_ctrlG;
            struct w32abipll_ctrlG;
          };
    ///////////////////////////////////////////////////////////
    #define   GET32abipll_status_LOCK(r32)                     _BFGET_(r32, 0, 0)
    #define   SET32abipll_status_LOCK(r32,v)                   _BFSET_(r32, 0, 0,v)
    #define   GET16abipll_status_LOCK(r16)                     _BFGET_(r16, 0, 0)
    #define   SET16abipll_status_LOCK(r16,v)                   _BFSET_(r16, 0, 0,v)

    #define   GET32abipll_status_DIVACK(r32)                   _BFGET_(r32, 1, 1)
    #define   SET32abipll_status_DIVACK(r32,v)                 _BFSET_(r32, 1, 1,v)
    #define   GET16abipll_status_DIVACK(r16)                   _BFGET_(r16, 1, 1)
    #define   SET16abipll_status_DIVACK(r16,v)                 _BFSET_(r16, 1, 1,v)

    #define     w32abipll_status                               {\
            UNSG32 ustatus_LOCK                                :  1;\
            UNSG32 ustatus_DIVACK                              :  1;\
            UNSG32 RSVDx1C_b2                                  : 30;\
          }
    union { UNSG32 u32abipll_status;
            struct w32abipll_status;
          };
    ///////////////////////////////////////////////////////////
    } SIE_abipll;

    typedef union  T32abipll_ctrlA
          { UNSG32 u32;
            struct w32abipll_ctrlA;
                 } T32abipll_ctrlA;
    typedef union  T32abipll_ctrlB
          { UNSG32 u32;
            struct w32abipll_ctrlB;
                 } T32abipll_ctrlB;
    typedef union  T32abipll_ctrlC
          { UNSG32 u32;
            struct w32abipll_ctrlC;
                 } T32abipll_ctrlC;
    typedef union  T32abipll_ctrlD
          { UNSG32 u32;
            struct w32abipll_ctrlD;
                 } T32abipll_ctrlD;
    typedef union  T32abipll_ctrlE
          { UNSG32 u32;
            struct w32abipll_ctrlE;
                 } T32abipll_ctrlE;
    typedef union  T32abipll_ctrlF
          { UNSG32 u32;
            struct w32abipll_ctrlF;
                 } T32abipll_ctrlF;
    typedef union  T32abipll_ctrlG
          { UNSG32 u32;
            struct w32abipll_ctrlG;
                 } T32abipll_ctrlG;
    typedef union  T32abipll_status
          { UNSG32 u32;
            struct w32abipll_status;
                 } T32abipll_status;
    ///////////////////////////////////////////////////////////

    typedef union  Tabipll_ctrlA
          { UNSG32 u32[1];
            struct {
            struct w32abipll_ctrlA;
                   };
                 } Tabipll_ctrlA;
    typedef union  Tabipll_ctrlB
          { UNSG32 u32[1];
            struct {
            struct w32abipll_ctrlB;
                   };
                 } Tabipll_ctrlB;
    typedef union  Tabipll_ctrlC
          { UNSG32 u32[1];
            struct {
            struct w32abipll_ctrlC;
                   };
                 } Tabipll_ctrlC;
    typedef union  Tabipll_ctrlD
          { UNSG32 u32[1];
            struct {
            struct w32abipll_ctrlD;
                   };
                 } Tabipll_ctrlD;
    typedef union  Tabipll_ctrlE
          { UNSG32 u32[1];
            struct {
            struct w32abipll_ctrlE;
                   };
                 } Tabipll_ctrlE;
    typedef union  Tabipll_ctrlF
          { UNSG32 u32[1];
            struct {
            struct w32abipll_ctrlF;
                   };
                 } Tabipll_ctrlF;
    typedef union  Tabipll_ctrlG
          { UNSG32 u32[1];
            struct {
            struct w32abipll_ctrlG;
                   };
                 } Tabipll_ctrlG;
    typedef union  Tabipll_status
          { UNSG32 u32[1];
            struct {
            struct w32abipll_status;
                   };
                 } Tabipll_status;

    ///////////////////////////////////////////////////////////
     SIGN32 abipll_drvrd(SIE_abipll *p, UNSG32 base, SIGN32 mem, SIGN32 tst);
     SIGN32 abipll_drvwr(SIE_abipll *p, UNSG32 base, SIGN32 mem, SIGN32 tst, UNSG32 *pcmd);
       void abipll_reset(SIE_abipll *p);
     SIGN32 abipll_cmp  (SIE_abipll *p, SIE_abipll *pie, char *pfx, void *hLOG, SIGN32 mem, SIGN32 tst);
    #define abipll_check(p,pie,pfx,hLOG) abipll_cmp(p,pie,pfx,(void*)(hLOG),0,0)
    #define abipll_print(p,    pfx,hLOG) abipll_cmp(p,0,  pfx,(void*)(hLOG),0,0)

#endif
//////
/// ENDOFINTERFACE: abipll
////////////////////////////////////////////////////////////



#ifdef __cplusplus
  }
#endif
#pragma  pack()

#endif
//////
/// ENDOFFILE: abipll.h
////////////////////////////////////////////////////////////

