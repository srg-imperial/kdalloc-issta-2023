#include "klee/KDAlloc/asan.h"

#include "klee/KDAlloc/kdalloc.h"

#include <cassert>
#include <cstdio>

namespace klee {

__sanitizer::uptr KDAllocAsan::_kChunkHeaderSize = 0;
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
                                        __sanitizer::u32 quarantine,
                                        __sanitizer::uptr kChunkHeaderSize) {
  _kChunkHeaderSize = kChunkHeaderSize;
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
  for (auto it = allocator->objects_begin(), ie = allocator->objects_end();
       it != ie; ++it) {
    __sanitizer::uptr chunk =
        reinterpret_cast<__sanitizer::uptr>(*it) - _kChunkHeaderSize;
    callback(chunk, arg);
  }
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
  void *result = nullptr;
  inKDAllocAsan = true;
  auto location_info = allocator->locationInfo(p, 1);
  if (location_info == klee::kdalloc::LocationInfo::LI_AllocatedOrQuarantined) {
    result = location_info.getBaseAddress();
  }
  inKDAllocAsan = false;
  return result;
}

} // namespace klee
