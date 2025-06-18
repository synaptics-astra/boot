/*
 * Flash-based transactional key-value store
 *
 * Copyright (C) 2010 Google, Inc.
 */
#ifndef _FLASH_TS_H
#define _FLASH_TS_H

#include "priv_type.h"

int flash_ts_init(loff_t start_block, loff_t blocks);

/* Get/set value, returns 0 on success */
int flash_ts_set(const char *key, const char *value);
void flash_ts_get(const char *key, char *value, unsigned int size);

/* Get value as an integer, if missing/invalid return 'default_value' */
int flash_ts_get_int(const char *key, int default_value);

#endif  /* _FLASH_TS_H */
