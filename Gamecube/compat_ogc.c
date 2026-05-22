/*
 * compat_ogc.c
 * Compatibility shim: provides symbols present in old libogc (v1.x) that were
 * removed or renamed in libogc2.
 *
 * WHY THIS FILE EXISTS:
 *   deps/opengx/gc_gl.c is compiled against the devkitPPC system libogc headers
 *   (in /opt/devkitpro/libogc/include) which declare PPCDCacheFlushAsync() as an
 *   external function. libogc2 replaced it with the synchronous DCFlushRange().
 *   This file provides the missing symbol so the linker is satisfied.
 *
 *   Using the synchronous version is safe: the "async" behaviour just means the
 *   CPU doesn't stall waiting for the cache line to finish writing. For correctness
 *   a synchronous flush is always valid (just slightly slower).
 */
#include <gctypes.h>
#include <ogc/cache.h>

void PPCDCacheFlushAsync(void *startaddress, u32 len)
{
    DCFlushRange(startaddress, len);
}
