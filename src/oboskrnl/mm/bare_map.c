/*
 * oboskrnl/mm/bare_map.c
 * 
 * Copyright (c) 2024 Omar Berrow
*/

#include <int.h>
#include <klog.h>

#include <mm/bare_map.h>

#include <locks/spinlock.h>

#define REGION_MAGIC_INT 0x4F424F534253434D
#define REGION_MAGIC "OBOSBSCM"

// Represents an allocated region.
typedef struct region
{
	union
	{
		uint64_t integer;
		const char* signature;
	} magic;
	uintptr_t addr;
	size_t size;
	struct region* next;
	struct region* prev;
} region;
static struct
{
	region* head;
	region* tail;
	size_t nNodes;
} s_regionList;
static spinlock s_regionListLock;
static bool s_regionListLockInitialized;
static irql Lock()
{
	if (!s_regionListLockInitialized)
	{
		s_regionListLock = Core_SpinlockCreate();
		s_regionListLockInitialized = true;
	}
	return Core_SpinlockAcquire(&s_regionListLock);
}
static void Unlock(irql oldIrql)
{
	OBOS_ASSERT(s_regionListLockInitialized);
	Core_SpinlockRelease(&s_regionListLock, oldIrql);
}

void* OBOS_BasicMMAllocatePages(size_t sz, obos_status* status)
{
	sz += sizeof(region);
	sz += (OBOS_PAGE_SIZE - (sz % OBOS_PAGE_SIZE));
	// Find a region.
	region* node = nullptr;
	region* currentNode = nullptr;
	uintptr_t lastAddress = OBOS_KERNEL_ADDRESS_SPACE_BASE;
	uintptr_t found = 0;
	for (currentNode = s_regionList.head; currentNode;)
	{
		uintptr_t currentNodeAddr = currentNode->addr & ~0xfff;
		if ((currentNodeAddr - lastAddress) >= (sz + OBOS_PAGE_SIZE))
		{
			found = lastAddress;
			break;
		}
		lastAddress = currentNodeAddr + currentNode->size;

		currentNode = currentNode->next;
	}
	if (!found)
	{
		region* currentNode = s_regionList.tail;
		if (currentNode)
			found = (currentNode->addr & ~0xfff) + currentNode->size;
		else
			found = OBOS_KERNEL_ADDRESS_SPACE_BASE;
	}
	node = (region*)found;
	for (uintptr_t addr = found; addr < (found + sz); addr += OBOS_PAGE_SIZE)
	{
		obos_status stat = OBOS_STATUS_SUCCESS;
		uintptr_t mem = OBOSS_AllocatePhysicalPages(1, 1, &stat);
		if (stat != OBOS_STATUS_SUCCESS)
		{
			if (status)
				*status = stat;
			return nullptr;
		}
		OBOSS_MapPage_RW_XD((void*)addr, mem);
	}
	OBOSH_BasicMMAddRegion(node, node + 1, sz);
	if (status)
		*status = OBOS_STATUS_SUCCESS;
	return node + 1;
}
obos_status OBOS_BasicMMFreePages(void* base_, size_t sz)
{
	sz += sizeof(region);
	sz += (OBOS_PAGE_SIZE - (sz % OBOS_PAGE_SIZE));
	uintptr_t base = (uintptr_t)base_;
	region* reg = ((region*)base - 1);
	if (((uintptr_t)reg & ~0xfff) != (base & ~0xfff))
		return OBOS_STATUS_MISMATCH;
	if (reg->magic.integer != REGION_MAGIC_INT)
		return OBOS_STATUS_MISMATCH;
	if (reg->size != sz)
		return OBOS_STATUS_INVALID_ARGUMENT;
	irql oldIrql = Lock();
	if (reg->next)
		reg->next->prev = reg->prev;
	if (reg->prev)
		reg->prev->next = reg->next;
	if (s_regionList.head == reg)
		s_regionList.head = reg->next;
	if (s_regionList.tail == reg)
		s_regionList.tail = reg->prev;
	s_regionList.nNodes--;
	Unlock(oldIrql);
	// Unmap the region.
	base &= ~0xfff;
	for (uintptr_t addr = base; addr < (base + sz); addr += OBOS_PAGE_SIZE)
		OBOSS_UnmapPage((void*)addr);
	return OBOS_STATUS_SUCCESS;
}

size_t OBOSH_BasicMMGetRegionSize() { return sizeof(region); }
void OBOSH_BasicMMAddRegion(void* nodeBase, void* base_, size_t sz)
{
	uintptr_t base = (uintptr_t)base_;
	region* node = (region*)nodeBase;
	node->magic.integer = REGION_MAGIC_INT;
	node->addr = base;
	node->size = sz;
	if (s_regionList.tail && s_regionList.tail->addr < base)
	{
		// Append it
		irql oldIrql = Lock();
		if (s_regionList.tail)
			s_regionList.tail->next = node;
		if (!s_regionList.head)
			s_regionList.head = node;
		node->prev = s_regionList.tail;
		s_regionList.tail = node;
		s_regionList.nNodes++;
		Unlock(oldIrql);
		return;
	}
	else if (s_regionList.head && s_regionList.head->addr > base)
	{
		// Prepend it
		irql oldIrql = Lock();
		if (s_regionList.head)
			s_regionList.head->prev = node;
		if (!s_regionList.tail)
			s_regionList.tail = node;
		node->next = s_regionList.head;
		s_regionList.head = node;
		s_regionList.nNodes++;
		Unlock(oldIrql);
		return;
	}
	else
	{
		irql oldIrql = Lock();
		// Find the node that this node should go after.
		region* found = nullptr, *n = s_regionList.head;
		while (n && n->next)
		{
			if (n->addr < base &&
				n->next->addr > node->addr)
			{
				found = n;
				break;
			}

			n = n->next;
		}
		if (!found)
		{
			// Append it
			if (s_regionList.tail)
				s_regionList.tail->next = node;
			if (!s_regionList.head)
				s_regionList.head = node;
			node->prev = s_regionList.tail;
			s_regionList.tail = node;
			s_regionList.nNodes++;
			Unlock(oldIrql);
			return;
		}
		if (found->next)
			found->next->prev = node;
		if (s_regionList.tail == found)
			s_regionList.tail = node;
		node->next = found->next;
		found->next = node;
		node->prev = found;
		s_regionList.nNodes++;
		Unlock(oldIrql);
	}
}