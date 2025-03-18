#ifndef CBMC_STUBS_H
#define CBMC_STUBS_H

#ifndef __CPROVER_assume
#define __CPROVER_assume(cond) ((void)0)
#endif

#ifndef __CPROVER_assert
#define __CPROVER_assert(cond, msg) ((void)0)
#endif

#endif // CBMC_STUBS_H
