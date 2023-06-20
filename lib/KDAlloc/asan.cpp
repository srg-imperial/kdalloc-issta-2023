#include "klee/KDAlloc/asan.h"

#include "klee/KDAlloc/kdalloc.h"

#include <cassert>
#include <cstdio>

namespace klee {

bool KDAllocAsan::inKDAllocAsan = false;

void *KDAllocAsan::Allocate(AllocatorCache *cache, __sanitizer::uptr size,
                            __sanitizer::uptr alignment) {
  size = std::max(size, alignment);

  inKDAllocAsan = true;
  auto res = allocator->allocate(size);
  inKDAllocAsan = false;

  return res;
}

void KDAllocAsan::Deallocate(AllocatorCache *cache, void *p) {
  inKDAllocAsan = true;
  allocator->free(p);
  inKDAllocAsan = false;
}

void *KDAllocAsan::Reallocate(AllocatorCache *cache, void *p,
                              __sanitizer::uptr new_size,
                              __sanitizer::uptr alignment) {
  new_size = std::max(new_size, alignment);

  inKDAllocAsan = true;
  auto result = allocator->allocate(new_size);
  auto old_size = allocator->getSize(p);
  std::memcpy(result, p, std::min(new_size, old_size));
  allocator->free(p);
  inKDAllocAsan = false;

  return result;
}

void KDAllocAsan::InitLinkerInitialized(__sanitizer::uptr address,
                                        __sanitizer::uptr size,
                                        __sanitizer::u32 quarantine) {

  inKDAllocAsan = true;
  allocator = new kdalloc::Allocator(
      klee::kdalloc::AllocatorFactory(address, size, quarantine)
          .makeAllocator());
  inKDAllocAsan = false;
}

__sanitizer::uptr KDAllocAsan::GetActuallyAllocatedSize(void *p) {
  inKDAllocAsan = true;
  auto res = allocator->getSize(p);
  inKDAllocAsan = false;
  return res;
}

void KDAllocAsan::ForEachChunk(__sanitizer::ForEachChunkCallback callback,
                               void *arg) {
  assert(0 && "not supported");
}

bool KDAllocAsan::PointerIsMine(const void *p) const {
  // only calling getters, so we can probably omit setting `inKDAllocAsan`
  auto const &mapping = allocator->getMapping();
  auto base = mapping.getBaseAddress();
  auto size = mapping.getSize();

  return base <= p && p < (reinterpret_cast<char *>(base) + size);
}

/// return start of allocation `p` points into, `nullptr` otherwise
void *KDAllocAsan::GetBlockBegin(const void *p) {
  inKDAllocAsan = true;
  auto res = allocator->locationInfo(p, 1).getBaseAddress();
  inKDAllocAsan = false;
  return res;
}

} // namespace klee
