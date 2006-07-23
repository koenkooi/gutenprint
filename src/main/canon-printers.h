/*
 *   Print plug-in CANON BJL driver for the GIMP.
 *
 *   Copyright 1997-2000 Michael Sweet (mike@easysw.com),
 *	Robert Krawitz (rlk@alum.mit.edu) and
 *      Andy Thaller (thaller@ph.tum.de)
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *   for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* This file contains the capabilities of the various canon printers
*/

#ifndef GUTENPRINT_INTERNAL_CANON_PRINTERS_H
#define GUTENPRINT_INTERNAL_CANON_PRINTERS_H


static const canon_cap_t canon_model_capabilities[] =
{
  /* default settings for unknown models */

  {   -1, 17*72/2,842,180,180,20,20,20,20, CANON_INK_K, CANON_SLOT_ASF1, 0 },

  /* ******************************** */
  /*                                  */
  /* tested and color-adjusted models */
  /*                                  */
  /* ******************************** */




  /* ************************************ */
  /*                                      */
  /* tested models w/out color-adjustment */
  /*                                      */
  /* ************************************ */

  { /* Canon S200x *//* heads: BC-24 */
    4202, 3,
    618, 936,       /* 8.58" x 13 " */
    180, 2880, 2880, 4,
    10, 10, 9, 20,
    CANON_INK_CMYK | CANON_INK_CMY | CANON_INK_K,
    CANON_SLOT_ASF1,
    CANON_CAP_STD1 | CANON_CAP_rr | CANON_CAP_WEAVE,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
/*   2880dpi Resolutions: TBD */
/* 180x180 360x360 720x720 1440x720 1440x1440 2880x2880 */
    {-1,      0,      0,       0,       0,        -1},
/*-------  360x360 720x720 1440x720  1440x1440 ---------*/
    { 1,     2,       1,     0.5,     0.3,        0.2},
    CANON_INK(canon_ink_standard),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },

  { /* Canon  BJ 30   *//* heads: BC-10 */
    30, 1,
    9.5*72, 14*72,
    90, 360, 360, 2,
    11, 9, 10, 18,
    CANON_INK_K,
    CANON_SLOT_ASF1,
    CANON_CAP_STD0 | CANON_CAP_a,
    CANON_MODES(canon_modes_30),
#ifndef EXPERIMENTAL_STUFF
    {-1,0,0,0,-1,-1}, /*090x090 180x180 360x360 720x360 720x720 1440x1440*/
    {1,1,1,1,1,1},    /*------- 180x180 360x360 720x360 ------- ---------*/
    CANON_INK(canon_ink_standard),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },

  { /* Canon  BJC 85  *//* heads: BC-20 BC-21 BC-22 */
    85, 1,
    9.5*72, 14*72,
    90, 720, 360, 2,
    11, 9, 10, 18,
    CANON_INK_K | CANON_INK_CMYK | CANON_INK_CcMmYK,
    CANON_SLOT_ASF1,
    CANON_CAP_STD0 | CANON_CAP_a | CANON_CAP_DMT,
    CANON_MODES(canon_modes_85),
#ifndef EXPERIMENTAL_STUFF
    {-1,-1,1,0,-1,-1},/*090x090 180x180 360x360 720x360 720x720 1440x1440*/
    {1,1,1,1,1,1},    /*------- ------- 360x360 720x360 ------- ---------*/
    CANON_INK(canon_ink_standardphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },

  { /* Canon BJC 4300 *//* heads: BC-20 BC-21 BC-22 BC-29 */
    4300, 1,
    618, 936,      /* 8.58" x 13 " */
    180, 1440, 720, 2,
    11, 9, 10, 18,
    CANON_INK_CMYK | CANON_INK_CcMmYK,
    CANON_SLOT_ASF1 | CANON_SLOT_MAN1,
    CANON_CAP_STD0 | CANON_CAP_DMT,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {-1,1,0,0,-1,-1}, /*180x180 360x360 720x720 1440x720 1440x1440 2880x2880*/
    {1,1,1,1,1,1},    /*------- 360x360 720x720 1440x720 --------- ---------*/
    CANON_INK(canon_ink_standardphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },

  { /* Canon BJC 4400 *//* heads: BC-20 BC-21 BC-22 BC-29 */
    4400, 1,
    9.5*72, 14*72,
    90, 720, 360, 2,
    11, 9, 10, 18,
    CANON_INK_K | CANON_INK_CMYK | CANON_INK_CcMmYK,
    CANON_SLOT_ASF1,
    CANON_CAP_STD0 | CANON_CAP_a | CANON_CAP_DMT,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {-1,-1,0,0,-1,-1},/*090x090 180x180 360x360 720x360 720x720 1440x1440*/
    {1,1,1,1,1,1},    /*------- ------- 360x360 720x360 ------- ---------*/
    CANON_INK(canon_ink_standardphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },

  { /* Canon BJC 6000 *//* heads: BC-30/BC-31 BC-32/BC-31 */
    6000, 3,
    618, 936,      /* 8.58" x 13 " */
    180, 1440, 720, 2,
    11, 9, 10, 18,
    CANON_INK_CMYK | CANON_INK_CcMmYK,
    CANON_SLOT_ASF1 | CANON_SLOT_MAN1,
    CANON_CAP_STD1 | CANON_CAP_DMT | CANON_CAP_ACKSHORT,
    CANON_MODES(canon_modes_6x00),
#ifndef EXPERIMENTAL_STUFF
    {-1,1,0,0,-1,-1}, /*180x180 360x360 720x720 1440x720 1440x1440 2880x2880*/
    {1,1.8,1,0.5,1,1},/*------- 360x360 720x720 1440x720 --------- ---------*/
    CANON_INK(canon_ink_standardphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },

  { /* Canon BJC 6200 *//* heads: BC-30/BC-31 BC-32/BC-31 */
    6200, 3,
    618, 936,      /* 8.58" x 13 " */
    180, 1440, 720, 2,
    11, 9, 10, 18,
    CANON_INK_CMYK | CANON_INK_CcMmYK,
    CANON_SLOT_ASF1 | CANON_SLOT_MAN1,
    CANON_CAP_STD1 | CANON_CAP_DMT | CANON_CAP_ACKSHORT,
    CANON_MODES(canon_modes_6x00),
#ifndef EXPERIMENTAL_STUFF
    {-1,1,0,0,-1,-1}, /*180x180 360x360 720x720 1440x720 1440x1440 2880x2880*/
    {0,1.8,1,.5,0,0}, /*------- 360x360 720x720 1440x720 --------- ---------*/
    CANON_INK(canon_ink_standardphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },

  { /* Canon BJC 6500 *//* heads: BC-30/BC-31 BC-32/BC-31 */
    6500, 3,
    842, 17*72,
    180, 1440, 720, 2,
    11, 9, 10, 18,
    CANON_INK_CMYK | CANON_INK_CcMmYK,
    CANON_SLOT_ASF1 | CANON_SLOT_MAN1,
    CANON_CAP_STD1 | CANON_CAP_DMT,
    CANON_MODES(canon_modes_6x00),
#ifndef EXPERIMENTAL_STUFF
    {-1,1,0,0,-1,-1}, /*180x180 360x360 720x720 1440x720 1440x1440 2880x2880*/
    {0,1.8,1,.5,0,0}, /*------- 360x360 720x720 1440x720 --------- ---------*/
    CANON_INK(canon_ink_standardphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },

  { /* Canon BJC 8200 *//* heads: BC-50 */
    8200, 3,
    842, 17*72,
    150, 1200,1200, 4,
    11, 9, 10, 18,
    CANON_INK_CMYK, /*  | CANON_INK_CcMmYK */
    CANON_SLOT_ASF1,
    CANON_CAP_STD1 | CANON_CAP_r | CANON_CAP_DMT | CANON_CAP_ACKSHORT,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {-1,0,0,-1,0,-1}, /*150x150 300x300 600x600 1200x600 1200x1200 2400x2400*/
    {1,1,1,1,1,1},    /*------- 300x300 600x600 -------- 1200x1200 ---------*/
    CANON_INK(canon_ink_superphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },


  /* *************** */
  /*                 */
  /* untested models */
  /*                 */
  /* *************** */


  { /* Canon BJC 210 *//* heads: BC-02 BC-05 BC-06 */
    210, 1,
    618, 936,      /* 8.58" x 13 " */
    90, 720, 360, 2,
    11, 9, 10, 18,
    CANON_INK_K | CANON_INK_CMY,
    CANON_SLOT_ASF1 | CANON_SLOT_MAN1,
    CANON_CAP_STD0,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {0,0,0,0,-1,-1},/*180x180 360x360 720x720 1440x720 1440x1440 2880x2880*/
    {1,1,1,1,1,1},    /*180x180 360x360 ------- -------- --------- ---------*/
    CANON_INK(canon_ink_standard),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },
  { /* Canon BJC 240 *//* heads: BC-02 BC-05 BC-06 */
    240, 1,
    618, 936,      /* 8.58" x 13 " */
    90, 720, 360, 2,
    11, 9, 10, 18,
    CANON_INK_K | CANON_INK_CMY,
    CANON_SLOT_ASF1 | CANON_SLOT_MAN1,
    CANON_CAP_STD0 | CANON_CAP_DMT,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {0,0,1,0,-1,-1},/*180x180 360x360 720x720 1440x720 1440x1440 2880x2880*/
    {1,1,1,1,1,1},    /*180x180 360x360 ------- -------- --------- ---------*/
    CANON_INK(canon_ink_oldphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },
  { /* Canon BJC 250 *//* heads: BC-02 BC-05 BC-06 */
    250, 1,
    618, 936,      /* 8.58" x 13 " */
    90, 720, 360, 2,
    11, 9, 10, 18,
    CANON_INK_K | CANON_INK_CMY,
    CANON_SLOT_ASF1 | CANON_SLOT_MAN1,
    CANON_CAP_STD0 | CANON_CAP_DMT,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {0,0,1,0,-1,-1},/*180x180 360x360 720x720 1440x720 1440x1440 2880x2880*/
    {1,1,1,1,1,1},    /*180x180 360x360 ------- -------- --------- ---------*/
    CANON_INK(canon_ink_oldphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },
  { /* Canon BJC 1000 *//* heads: BC-02 BC-05 BC-06 */
    1000, 1,
    842, 17*72,
    90, 720, 360, 2,
    11, 9, 10, 18,
    CANON_INK_K | CANON_INK_CMY,
    CANON_SLOT_ASF1,
    CANON_CAP_STD0 | CANON_CAP_DMT | CANON_CAP_a,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {0,0,1,0,-1,-1},  /*180x180 360x360 720x720 1440x720 1440x1440 2880x2880*/
    {1,1,1,1,1,1},    /*180x180 360x360 ------- -------- --------- ---------*/
    CANON_INK(canon_ink_oldphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },
  { /* Canon BJC 2000 *//* heads: BC-20 BC-21 BC-22 BC-29 */
    2000, 1,
    842, 17*72,
    180, 720, 360, 2,
    11, 9, 10, 18,
    CANON_INK_CMYK,
    CANON_SLOT_ASF1,
    CANON_CAP_STD0 | CANON_CAP_a,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {0,0,-1,-1,-1,-1},/*180x180 360x360 720x720 1440x720 1440x1440 2880x2880*/
    {1,1,1,1,1,1},    /*180x180 360x360 ------- -------- --------- ---------*/
    CANON_INK(canon_ink_standard),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },
  { /* Canon BJC 3000 *//* heads: BC-30 BC-33 BC-34 */
    3000, 3,
    842, 17*72,
    180, 1440, 720, 2,
    11, 9, 10, 18,
    CANON_INK_CMYK | CANON_INK_CcMmYK,
    CANON_SLOT_ASF1,
    CANON_CAP_STD0 | CANON_CAP_a | CANON_CAP_DMT, /*FIX? should have _r? */
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {-1,1,0,0,-1,-1}, /*180x180 360x360 720x720 1440x720 1440x1440 2880x2880*/
    {1,1,1,1,1,1},    /*------- 360x360 720x720 1440x720 --------- ---------*/
    CANON_INK(canon_ink_standardphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },
  { /* Canon BJC 6100 *//* heads: BC-30/BC-31 BC-32/BC-31 */
    6100, 3,
    842, 17*72,
    180, 1440, 720, 2,
    11, 9, 10, 18,
    CANON_INK_CMYK | CANON_INK_CcMmYK,
    CANON_SLOT_ASF1,
    CANON_CAP_STD1 | CANON_CAP_a | CANON_CAP_r | CANON_CAP_DMT,
    CANON_MODES(canon_modes_6x00),
#ifndef EXPERIMENTAL_STUFF
    {-1,1,0,0,-1,-1}, /*180x180 360x360 720x720 1440x720 1440x1440 2880x2880*/
    {1,1,1,1,1,1},    /*------- 360x360 720x720 1440x720 --------- ---------*/
    CANON_INK(canon_ink_standardphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },
  { /* Canon BJC 7000 *//* heads: BC-60/BC-61 BC-60/BC-62   ??????? */
    7000, 3,
    842, 17*72,
    150, 1200, 600, 2,
    11, 9, 10, 18,
    CANON_INK_CMYK | CANON_INK_CcMmYyK,
    CANON_SLOT_ASF1,
    CANON_CAP_STD1,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {-1,0,0,0,-1,-1}, /*150x150 300x300 600x600 1200x600 1200x1200 2400x2400*/
    {1,3.5,1.8,1,1,1},/*------- 300x300 600x600 1200x600 --------- ---------*/
    CANON_INK(canon_ink_standardphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },
  { /* Canon BJC 7100 *//* heads: BC-60/BC-61 BC-60/BC-62   ??????? */
    7100, 3,
    842, 17*72,
    150, 1200, 600, 2,
    11, 9, 10, 18,
    CANON_INK_CMYK | CANON_INK_CcMmYyK,
    CANON_SLOT_ASF1,
    CANON_CAP_STD0,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {-1,0,0,0,-1,-1}, /*150x150 300x300 600x600 1200x600 1200x1200 2400x2400*/
    {1,1,1,1,1,1},    /*------- 300x300 600x600 1200x600 --------- ---------*/
    CANON_INK(canon_ink_standardphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },

  /*****************************/
  /*                           */
  /*  extremely fuzzy models   */
  /* (might never work at all) */
  /*                           */
  /*****************************/

  { /* Canon BJC 5100 *//* heads: BC-20 BC-21 BC-22 BC-23 BC-29 */
    5100, 1,
    17*72, 22*72,
    180, 1440, 720, 2,
    11, 9, 10, 18,
    CANON_INK_CMYK | CANON_INK_CcMmYK,
    CANON_SLOT_ASF1,
    CANON_CAP_STD0 | CANON_CAP_DMT,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {-1,1,0,0,-1,-1}, /*180x180 360x360 720x720 1440x720 1440x1440 2880x2880*/
    {1,1,1,1,1,1},    /*------- 360x360 720x720 1440x720 --------- ---------*/
    CANON_INK(canon_ink_standardphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },
  { /* Canon BJC 5500 *//* heads: BC-20 BC-21 BC-29 */
    5500, 1,
    22*72, 34*72,
    180, 720, 360, 2,
    11, 9, 10, 18,
    CANON_INK_CMYK | CANON_INK_CcMmYK,
    CANON_SLOT_ASF1,
    CANON_CAP_STD0 | CANON_CAP_a,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {0,0,-1,-1,-1,-1},/*180x180 360x360 720x720 1440x720 1440x1440 2880x2880*/
    {1,1,1,1,1,1},    /*180x180 360x360 ------- -------- --------- ---------*/
    CANON_INK(canon_ink_standardphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },
  { /* Canon BJC 6500 *//* heads: BC-30/BC-31 BC-32/BC-31 */
    6500, 3,
    17*72, 22*72,
    180, 1440, 720, 2,
    11, 9, 10, 18,
    CANON_INK_CMYK | CANON_INK_CcMmYK,
    CANON_SLOT_ASF1,
    CANON_CAP_STD1 | CANON_CAP_a | CANON_CAP_DMT,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {-1,1,0,0,-1,-1}, /*180x180 360x360 720x720 1440x720 1440x1440 2880x2880*/
    {1,1,1,1,1,1},    /*------- 360x360 720x720 1440x720 --------- ---------*/
    CANON_INK(canon_ink_standardphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },
  { /* Canon BJC 8500 *//* heads: BC-80/BC-81 BC-82/BC-81 */
    8500, 3,
    17*72, 22*72,
    150, 1200,1200, 2,
    11, 9, 10, 18,
    CANON_INK_CMYK | CANON_INK_CcMmYK,
    CANON_SLOT_ASF1,
    CANON_CAP_STD0,
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {-1,0,0,-1,0,-1}, /*150x150 300x300 600x600 1200x600 1200x1200 2400x2400*/
    {1,1,1,1,1,1},    /*------- 300x300 600x600 -------- 1200x1200 ---------*/
    CANON_INK(canon_ink_standardphoto),
#endif
    &canon_default_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },
  { /* Canon PIXMA iP4000 */
    4000, 3,          /*model, model_id*/
    842, 17*72,       /* max paper width and height */
    150, 600, 600, 2, /*base resolution,max_xdpi,max_ydpi,max_quality */
    11, 9, 10, 18,    /*border_left, border_right, border_top, border_bottom */
    CANON_INK_CMYK /*| CANON_INK_CcMmYyK*/, /*canon inks */
    CANON_SLOT_ASF1,  /*paper slot */
    CANON_CAP_STD0|CANON_CAP_extended_t|CANON_CAP_5pixelin1byte|CANON_CAP_DUPLEX,  /*features */
    CANON_MODES(canon_nomodes),
#ifndef EXPERIMENTAL_STUFF
    {-1,-1,0,-1,-1,-1}, /*150x150 300x300 600x600 1200x600 1200x1200 2400x2400*/
    {1,1,1,1,1,1},    /*------- 300x300 600x600 1200x600 --------- ---------*/
    CANON_INK(canon_ink_standard_pixma),
#endif
    &canon_PIXMA_iP4000_paperlist,
    standard_lum_adjustment,
    standard_hue_adjustment,
    standard_sat_adjustment
  },
};

#endif

