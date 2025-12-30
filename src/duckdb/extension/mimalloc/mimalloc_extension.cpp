#include "mimalloc_extension.hpp"

#include "duckdb/common/allocator.hpp"
#include "duckdb/common/helper.hpp"

#ifdef USE_MIMALLOC

#include <cstdio>
#include <atomic>
#include <cstddef>  // for size_t

// For static linking, we declare mimalloc functions directly without including mimalloc.h
// This avoids the __declspec(dllimport) declarations that the header always uses on Windows
// We only declare the functions we actually use
#pragma warning(push)
#pragma warning(disable: 4005) // disable 'macro redefinition' warning

// Declare mimalloc functions for static linking
// These match the signatures from mimalloc.h but without __declspec attributes
extern "C" {
    void* __cdecl mi_malloc(size_t size);
    void __cdecl mi_free(void* p);
    void* __cdecl mi_realloc(void* p, size_t newsize);
    void __cdecl mi_collect(bool force);
    int __cdecl mi_version(void);  // for version logging
}

#pragma warning(pop)

namespace duckdb {

// Static variable to track if mimalloc has been initialized/logged
static std::atomic<bool> mimalloc_initialized{false};

data_ptr_t MimallocExtension::Allocate(PrivateAllocatorData *private_data, idx_t size) {
	(void)private_data; // unused
	
	// Log mimalloc initialization on first allocation
	if (!mimalloc_initialized.exchange(true)) {
		fprintf(stderr, "[MIMALLOC] Extension initialized - mimalloc will be used for memory allocation\n");
		// Try to get mimalloc version if available
		#if defined(mi_version)
		int version_int = mi_version();
		// mi_version() returns an integer: major * 100 + minor (e.g., 224 = version 2.24)
		int major = version_int / 100;
		int minor = version_int % 100;
		fprintf(stderr, "[MIMALLOC] Mimalloc version: %d.%d (encoded: %d)\n", major, minor, version_int);
		#elif defined(MI_VERSION)
		fprintf(stderr, "[MIMALLOC] Mimalloc version: %d\n", MI_VERSION);
		#else
		fprintf(stderr, "[MIMALLOC] Mimalloc version: unknown\n");
		#endif
		fprintf(stderr, "[MIMALLOC] First allocation via mimalloc: %llu bytes\n", (unsigned long long)size);
		fflush(stderr);
	}
	
	auto result = mi_malloc(size);
	if (!result) {
		throw std::bad_alloc();
	}
	return data_ptr_cast(result);
}

void MimallocExtension::Free(PrivateAllocatorData *private_data, data_ptr_t pointer, idx_t size) {
	(void)private_data; // unused
	(void)size; // unused
	mi_free(pointer);
}

data_ptr_t MimallocExtension::Reallocate(PrivateAllocatorData *private_data, data_ptr_t pointer, idx_t old_size,
                                         idx_t size) {
	(void)private_data; // unused
	(void)old_size; // unused
	return data_ptr_cast(mi_realloc(pointer, size));
}

void MimallocExtension::ThreadFlush(idx_t threshold) {
	(void)threshold; // unused
	// mimalloc automatically manages thread-local caches
	// We can force a collection if needed
	mi_collect(false); // false = not force, true = force
}

void MimallocExtension::FlushAll() {
	// Force collection of all memory
	mi_collect(true); // true = force collection
}

} // namespace duckdb

#else // !USE_MIMALLOC

namespace duckdb {

// Stub implementations when mimalloc is not available
// These should never be called since allocator.cpp checks USE_MIMALLOC before calling them

data_ptr_t MimallocExtension::Allocate(PrivateAllocatorData *private_data, idx_t size) {
	(void)private_data;
	(void)size;
	throw std::runtime_error("MimallocExtension::Allocate called but USE_MIMALLOC is not defined");
}

void MimallocExtension::Free(PrivateAllocatorData *private_data, data_ptr_t pointer, idx_t size) {
	(void)private_data;
	(void)pointer;
	(void)size;
	throw std::runtime_error("MimallocExtension::Free called but USE_MIMALLOC is not defined");
}

data_ptr_t MimallocExtension::Reallocate(PrivateAllocatorData *private_data, data_ptr_t pointer, idx_t old_size,
                                         idx_t size) {
	(void)private_data;
	(void)pointer;
	(void)old_size;
	(void)size;
	throw std::runtime_error("MimallocExtension::Reallocate called but USE_MIMALLOC is not defined");
}

void MimallocExtension::ThreadFlush(idx_t threshold) {
	(void)threshold;
	// No-op when mimalloc is not available
}

void MimallocExtension::FlushAll() {
	// No-op when mimalloc is not available
}

} // namespace duckdb

#endif // USE_MIMALLOC

