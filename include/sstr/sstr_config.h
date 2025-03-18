/*
 * Copyright 2025 Asim Ihsan
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

/**
 * SStr Configuration
 *
 * This file contains configuration options for the SStr library.
 * Users can define their own values for these options before including
 * the SStr headers to customize behavior.
 */

#ifndef SSTR_CONFIG_H
#define SSTR_CONFIG_H

/**
 * Default truncation policy for string operations.
 * Can be overridden by the user before including sstr.h.
 */
/* Default truncation policy - changed here to ensure all tests work properly */
#ifndef SSTR_DEFAULT_POLICY
#define SSTR_DEFAULT_POLICY SSTR_ERROR
#endif

/**
 * Maximum allowed size for a single string.
 * This prevents integer overflow in size calculations.
 */
#ifndef SSTR_MAX_SIZE
#define SSTR_MAX_SIZE ((size_t)0x7FFFFFFF)
#endif

/**
 * Define format specifiers to handle.
 */
#ifndef SSTR_ENABLE_FLOAT_FORMAT
#define SSTR_ENABLE_FLOAT_FORMAT 1
#endif

#endif /* SSTR_CONFIG_H */
