/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <common/uuid.h>

/* Return the hex nibble value of a char */
static int8_t hex_val(char hex)
{
	int8_t val = 0;

	if ((hex >= '0') && (hex <= '9')) {
		val = (int8_t)(hex - '0');
	} else if ((hex >= 'a') && (hex <= 'f')) {
		val = (int8_t)(hex - 'a' + 0xa);
	} else if ((hex >= 'A') && (hex <= 'F')) {
		val = (int8_t)(hex - 'A' + 0xa);
	} else {
		val = -1;
	}

	return val;
}

/*
 * Read hex_src_len hex characters from hex_src, convert to bytes and
 * store in buffer pointed to by dest
 */
static int read_hex(uint8_t *dest, char *hex_src, unsigned int hex_src_len)
{
	int8_t nibble;
	uint8_t byte;

	/*
	 * The string length must be a multiple of 2 to represent an
	 * exact number of bytes.
	 */
	assert((hex_src_len % 2U) == 0U);

	for (unsigned int i = 0U; i < (hex_src_len / 2U); i++) {
		nibble = 0;
		byte = 0U;

		nibble = hex_val(hex_src[2U * i]);
		if (nibble < 0) {
			return -1;
		}
		byte = (uint8_t)nibble;
		byte <<= 4U;

		nibble = hex_val(hex_src[(2U * i) + 1U]);
		if (nibble < 0) {
			return -1;
		}
		byte |= (uint8_t)nibble;

		*dest = byte;
		dest++;
	}

	return 0;
}

/* Parse UUIDs of the form aabbccdd-eeff-4099-8877-665544332211 */
int read_uuid(uint8_t *dest, char *uuid)
{
	int err;
	uint8_t *dest_start = dest;

	/* Check that we have enough characters */
	if (strnlen(uuid, UUID_STRING_LENGTH) != UUID_STRING_LENGTH) {
		WARN("UUID string is too short\n");
		return -EINVAL;
	}

	/* aabbccdd */
	err = read_hex(dest, uuid, 8);
	uuid += 8;
	dest += 4;

	/* Check for '-' */
	err |= ((*uuid == '-') ? 0 : -1);
	uuid++;

	/* eeff */
	err |= read_hex(dest, uuid, 4);
	uuid += 4;
	dest += 2;

	/* Check for '-' */
	err |= ((*uuid == '-') ? 0 : -1);
	uuid++;

	/* 4099 */
	err |= read_hex(dest, uuid, 4);
	uuid += 4;
	dest += 2;

	/* Check for '-' */
	err |= ((*uuid == '-') ? 0 : -1);
	uuid++;

	/* 8877 */
	err |= read_hex(dest, uuid, 4);
	uuid += 4;
	dest += 2;

	/* Check for '-' */
	err |= ((*uuid == '-') ? 0 : -1);
	uuid++;

	/* 665544332211 */
	err |= read_hex(dest, uuid, 12);
	uuid += 12;
	dest += 6;

	if (err < 0) {
		WARN("Error parsing UUID\n");
		/* Clear the buffer on error */
		memset((void *)dest_start, '\0', UUID_BYTES_LENGTH * sizeof(uint8_t));
		return -EINVAL;
	}

	return 0;
}

/*
 * Helper function to check if 2 UUIDs match.
 */
bool uuid_match(uint32_t *uuid1, uint32_t *uuid2)
{
	return !memcmp(uuid1, uuid2, sizeof(uint32_t) * 4);
}

/*
 * Helper function to copy from one UUID struct to another.
 */
void copy_uuid(uint32_t *to_uuid, uint32_t *from_uuid)
{
	to_uuid[0] = from_uuid[0];
	to_uuid[1] = from_uuid[1];
	to_uuid[2] = from_uuid[2];
	to_uuid[3] = from_uuid[3];
}

bool is_null_uuid(uint32_t *uuid)
{
	return (uuid[0] == 0 && uuid[1] == 0 &&
		uuid[2] == 0 && uuid[3] == 0);
}
