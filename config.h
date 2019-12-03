/**
 * @file config.h
 * @brief Configuration du module AX25.
 * @author Pascal JEAN <pjean@btssn.net>
 *          @copyright 2014 GNU Lesser General Public License version 3
 *          <http://www.gnu.org/licenses/lgpl.html>
 * @version $Id$
 * Revision History ---
 *    20120519 - Initial version
 */

#ifndef _RADIO_CONFIG_H_
#define _RADIO_CONFIG_H_

/**
 * Module logging level.
 */
#define AX25_LOG_LEVEL      LOG_LVL_WARN
//#define AX25_LOG_LEVEL      LOG_LVL_INFO

/**
 * Module logging format.
 */
#define AX25_LOG_FORMAT     LOG_FMT_TERSE

/**
 * Enable access to the physical layer using streams
 * This option is not yet functional !
 */
#define AX25_CFG_USE_STREAM 0

#endif /* _RADIO_CONFIG_H_ */
