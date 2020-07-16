/**
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef COMPILER

// Note the __clang__ conditional has to come before the __GNUC__ one because
// clang pretends to be GCC.
#if defined(__clang__)
#define COMPILER "[Clang " __clang_version__ "]"
#elif defined(__GNUC__)
#define COMPILER "[GCC " __VERSION__ "]"
// Generic fallbacks.
#elif defined(__cplusplus)
#define COMPILER "[C++]"
#else
#define COMPILER "[C]"
#endif

const char *getcomp()
{
	return COMPILER;
}

#endif /* !COMPILER */