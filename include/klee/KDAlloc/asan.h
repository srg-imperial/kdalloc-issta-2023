#ifndef KDALLOC_ASAN_H
#define KDALLOC_ASAN_H

namespace __sanitizer {
// from llvm/compiler-rt/lib/sanitizer_common/sanitizer_internal_defs.h
typedef unsigned long uptr;
typedef signed int s32;
typedef unsigned int u32;

// Callback type for iterating over chunks.
typedef void (*ForEachChunkCallback)(uptr chunk, void *arg);
} // namespace __sanitizer

namespace klee {

namespace kdalloc {
class Allocator;
}

class KDAllocAsan {

public:
  static bool inKDAllocAsan;

  // stub, we do not use a cache
  struct AllocatorCache {};

  kdalloc::Allocator *allocator;

  void *Allocate(AllocatorCache *cache, __sanitizer::uptr size,
                 __sanitizer::uptr alignment);

  void Deallocate(AllocatorCache *cache, void *p);

  void *Reallocate(AllocatorCache *cache, void *p, __sanitizer::uptr new_size,
                   __sanitizer::uptr alignment);

  void InitLinkerInitialized(__sanitizer::uptr address, __sanitizer::uptr size,
                             __sanitizer::u32 quarantine);

  __sanitizer::uptr GetActuallyAllocatedSize(void *p);

  void ForceLock() {}

  void ForceUnlock() {}

  void ForEachChunk(__sanitizer::ForEachChunkCallback callback, void *arg);

  __sanitizer::s32 ReleaseToOSIntervalMs() const {
    // ignore for now
    return 0;
  }

  void SetReleaseToOSIntervalMs(__sanitizer::s32 release_to_os_interval_ms) {
    // ignore for now
  }

  void ForceReleaseToOS() {
    // ignore for now
  }

  bool PointerIsMine(const void *p) const;

  bool FromPrimary(const void *p) const { return PointerIsMine(p); }
  void SwallowCache(AllocatorCache *cache) {}

  void *GetBlockBegin(const void *p);
  void *GetBlockBeginFastLocked(void *p) { return GetBlockBegin(p); }

  void PrintStats() {
    // do nothing
  }
};

} // namespace klee

#endif
