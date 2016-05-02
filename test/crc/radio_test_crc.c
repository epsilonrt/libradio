/**
 * @file test_crc.c
 * @brief CRC Test
 * 
 * Copyright © 2015 epsilonRT, All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <radio/crc.h>

#define TEST_STR "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define TEST_CRC 0xC452


/* internal public functions ================================================ */
int
main (void) {
  uint16_t usCrc;

  printf ("Test CRC CCITT\n\tMessage: %s\n\tExpected result: 0x%04X\n", TEST_STR, TEST_CRC);
  usCrc = usCrcCcitt (CRC_CCITT_INIT_VAL, TEST_STR, strlen (TEST_STR));
  printf ("Calculated result: 0x%04X\n", usCrc);
  assert (usCrc == TEST_CRC);
  printf ("Success ! Have a nice day !\n");

  return 0;
}

/* ========================================================================== */
