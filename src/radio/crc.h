/**
 * @file crc.h
 * @brief Calcul de CRC (Implémentation)
 * @author Francesco Sacchi <batt@develer.com>
 *          @copyright 2009 GNU General Public License version 2
 *          See the notice below.
 * @author Pascal JEAN <pjean@btssn.net>
 *          @copyright 2014 GNU Lesser General Public License version 3
 *          <http://www.gnu.org/licenses/lgpl.html>
 * @version $Id$
 * Revision History ---
 *    20120519 - Initial version from BeRTOS
 * -----------------------------------------------------------------------------
 * Bertos is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 *
 * Copyright 2009 Develer S.r.l. (http://www.develer.com/)
 */

#ifndef _RADIO_CRC_H_
#define _RADIO_CRC_H_

#include <radio/defs.h>

/* *INDENT-OFF* */
__BEGIN_C_DECLS
/* ========================================================================== */

/* constants ================================================================ */
/** CRC-CCITT init value */
#define CRC_CCITT_INIT_VAL ((uint16_t)0xFFFF)

/* internal public functions ================================================ */
/**
 * This function implements the CRC-CCITT calculation on a buffer.
 *
 * @param crc  Current CRC-CCITT value.
 * @param buf  The buffer to perform CRC calculation on.
 * @param len  The length of the Buffer.
 *
 * @return The updated CRC-CCITT value.
 */
uint16_t usCrcCcitt (uint16_t usCrc, const void *pvBuf, size_t uLen);

/* inline public functions ================================================ */

/* CRC table */
extern const uint16_t usCrcCcittTab[256];

/**
 * @brief Compute the updated CRC-CCITT value for one octet (inline version)
 */
__STATIC_ALWAYS_INLINE (uint16_t
  usCrcCcittUpdate(uint8_t c, uint16_t oldcrc)) {

  return (oldcrc >> 8) ^ usCrcCcittTab[(oldcrc ^ c) & 0xff];
}

/* ========================================================================== */
__END_C_DECLS
/* *INDENT-ON* */
#endif /* _RADIO_CRC_H_ */
