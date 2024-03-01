/*
	oboskrnl/allocators/slab.h

	Copyright (c) 2024 Omar Berrow
*/

#pragma once

#include <allocators/allocator.h>
#include <allocators/slab_structs.h>

#include <locks/spinlock.h>

namespace obos
{
	namespace allocators
	{
		class SlabAllocator final : public Allocator
		{
		public:
			SlabAllocator() = default;

			bool Initialize(void* allocBase, size_t allocSize, size_t initialNodeCount = 0, size_t padding = 0x10);

			void* Allocate(size_t size) override;
			void* ReAllocate(void* base, size_t newSize) override;
			void Free(void* base, size_t ignored) override;

			size_t GetAllocationSize() override { return m_allocationSize; }

			size_t QueryObjectSize(void* base) override;

			void OptimizeAllocator() override;

			~SlabAllocator();
		private:
			void ImplOptimizeAllocator();
			void* m_base = nullptr;
			size_t m_regionSize = 0;
			size_t m_allocationSize = 0;
			size_t m_stride = 0;
			size_t m_padding = 0;
			SlabList m_freeNodes = {};
			SlabList m_allocatedNodes = {};
			locks::SpinLock m_lock;
		};
	}
}