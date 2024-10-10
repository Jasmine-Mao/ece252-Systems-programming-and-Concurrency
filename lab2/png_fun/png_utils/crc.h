/**
 * @file: crc.h
 * @brief: crc calculation functions for PNG file
 */

#ifndef CRC_H
#define CRC_H

void make_crc_table(void);
unsigned long update_crc(unsigned long crc, unsigned char *buf, int len);
unsigned long crc(unsigned char *buf, int len);

#endif
