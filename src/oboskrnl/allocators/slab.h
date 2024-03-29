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

			bool Initialize(void* allocBase, size_t allocSize, bool findAddress = true, size_t initialNodeCount = 0, size_t padding = 0x10, uintptr_t mapFlags = 0);
			bool AddRegion(void* base, size_t regionSize);

			void* Allocate(size_t size) override;
			void* ReAllocate(void* base, size_t newSize) override;
			void Free(void* base, size_t ignored) override;

			size_t GetAllocationSize() override { return m_allocationSize; }

			size_t QueryObjectSize(const void* base) override;

			void OptimizeAllocator() override;

			const SlabRegionList& GetRegionList() const { return m_regionNodes; }

			~SlabAllocator();
		private:
			void ImplOptimizeList(SlabList& list);
			SlabNode* LookForNode(SlabList& list, const void* addr);
			void CombineContinuousNodes(SlabList& list);
			void* AllocateFromRegion(SlabRegionNode* region, size_t size);
			SlabRegionList m_regionNodes;
			size_t m_allocationSize = 0;
			size_t m_stride = 0;
			size_t m_padding = 0;
			void* m_allocBase = nullptr;
			static constexpr size_t m_maxEmptyRegionNodesAllowed = 4;
		};
	}
}