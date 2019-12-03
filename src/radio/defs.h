/**
 * @file defs.h
 * @brief
 * @version $Id$
 * @author Pascal JEAN <pascal.jean@btssn.net>
 * @copyright GNU Lesser General Public License version 3
 *            <http://www.gnu.org/licenses/lgpl.html>
 *
 * Copyright (c) 2014 Pascal JEAN <pascal.jean@btssn.net>
 *******************************************************************************
 * This program is free software: you can redistribute it and/or modif         *
 *    it under the terms of the GNU Lesser General Public License as published *
 *    by the Free Software Foundation, either version 3 of the License, or     *
 *    (at your option) any later version.                                      *
 *                                                                             *
 *    This program is distributed in the hope that it will be useful,          *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *    GNU Lesser General Public License for more details.                      *
 *                                                                             *
 *    You should have received a copy of the GNU Lesser General Public License *
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *******************************************************************************
 */
#ifndef _RADIO_DEFS_H_
#define _RADIO_DEFS_H_
/* ========================================================================== */

/* macros =================================================================== */
#ifndef _BV
# define _BV(n) (1<<n)
#endif

#if defined(__cplusplus)
# define __BEGIN_C_DECLS  extern "C" {
# define __END_C_DECLS    }
#else
# define __BEGIN_C_DECLS
# define __END_C_DECLS
#endif

/* constants ================================================================ */
#ifndef TRUE
#  define TRUE  (1==1)
#  define FALSE (1==2)
#endif

#ifndef __ASSEMBLER__
__BEGIN_C_DECLS
  /* ==========================Partie Langage C============================== */
# define __need_NULL
# define __need_size_t
# include <stddef.h>
# include <stdint.h>
# include <stdbool.h>
# include <math.h>

/* macros =================================================================== */
#ifndef ABS
# define ABS(n)  (((n) < 0) ? -(n) : (n))
#endif

#ifndef MIN
# define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
# define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef COUNTOF
# define COUNTOF(a)  (sizeof(a) / sizeof(*(a)))
#endif

#ifndef STRUCT_FIELD_OFFSET
# define STRUCT_FIELD_OFFSET(s,f)  ((size_t)&(((s *)0)->f))
#endif

#define __STATIC_ALWAYS_INLINE(func) \
  static __inline__ func __attribute__ ((always_inline));\
  func

#define UNUSED_ARG(type,arg)    __attribute__((__unused__)) type arg

__END_C_DECLS
  /* ========================Fin Partie Langage C============================ */
#endif /* __ASSEMBLER__ not defined */

/* ========================================================================== */
#endif /* _RADIO_DEFS_H_ defined */
