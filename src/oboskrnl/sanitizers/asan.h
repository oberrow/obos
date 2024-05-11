/*
 * oboskrnl/sanitizers/asan.h
 *
 * Copyright (c) 2024 Omar Berrow
*/

#pragma once

#include <int.h>
#include <klog.h>
#include <memmanip.h>

#include <allocators/basic_allocator.h>

#define OBOS_CROSSES_PAGE_BOUNDARY(base, size) (((uintptr_t)(base) & ~0xfff) == ((((uintptr_t)(base) + (size)) & ~0xfff)))

typedef enum
{
	ASAN_InvalidType = 0,
	ASAN_InvalidAccess,
	ASAN_ShadowSpaceAccess,
	ASAN_UseAfterFree,
} asan_violation_type;
// Used to index into OBOS_ASANPoisonValues.
enum
{
	ASAN_POISON_ALLOCATED,
	ASAN_POISON_FREED,
	ASAN_POISON_MAX = ASAN_POISON_FREED,
};
extern const uint8_t OBOS_ASANPoisonValues[ASAN_POISON_MAX + 1];