/*
 * Copyright 2025 Asim Ihsan
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * https://mozilla.org/MPL/2.0/.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef CBMC_STUBS_H
#define CBMC_STUBS_H

#ifndef __CPROVER_assume
#define __CPROVER_assume(cond) ((void)0)
#endif

#ifndef __CPROVER_assert
#define __CPROVER_assert(cond, msg) ((void)0)
#endif

#endif // CBMC_STUBS_H
