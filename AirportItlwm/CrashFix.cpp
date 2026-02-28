//
//  CrashFix.cpp
//  itlwm
//
//  Created by giurika on 28/2/26.
//  Copyright © 2026 钟先耀. All rights reserved.
//

#include "CrashFix.hpp"

#include <sys/mbuf.h>
#include <sys/kpi_mbuf.h>
#include <IOKit/IOLib.h>

static size_t mbuf_total_len(mbuf_t m)
{
    size_t total = 0;
    for (mbuf_t n = m; n != NULL; n = mbuf_next(n)) {
        total += (size_t)mbuf_len(n);
    }
    return total;
}

static mbuf_t ensure_pkthdr(mbuf_t m)
{
    if (!m) return NULL;

    // Already has pkthdr: just make lengths consistent (defensive)
    if (mbuf_flags(m) & MBUF_PKTHDR) {
        mbuf_pkthdr_setlen(m, mbuf_total_len(m));
        return m;
    }

    // No pkthdr: rebuild safely
    const size_t total = mbuf_total_len(m);
    if (total == 0 || total > (512 * 1024)) { // sanity guard
        mbuf_freem(m);
        return NULL;
    }

    // Copy original payload into a temp buffer
    void* tmp = IOMalloc((vm_size_t)total);
    if (!tmp) {
        mbuf_freem(m);
        return NULL;
    }

    mbuf_copydata(m, 0, total, tmp);

    // Allocate a new packet mbuf with pkthdr
    mbuf_t m2 = NULL;
    if (mbuf_allocpacket(MBUF_DONTWAIT, total, NULL, &m2) != 0 || !m2) {
        IOFree(tmp, (vm_size_t)total);
        mbuf_freem(m);
        return NULL;
    }

    // Copy into the new mbuf chain safely
    // mbuf_copyback signature on macOS KPI: (mbuf_t, int off, int len, const void* data, int how)
    if (mbuf_copyback(m2, 0, (int)total, tmp, MBUF_DONTWAIT) != 0) {
        IOFree(tmp, (vm_size_t)total);
        mbuf_freem(m2);
        mbuf_freem(m);
        return NULL;
    }

    IOFree(tmp, (vm_size_t)total);

    // Ensure pkthdr length is correct
    mbuf_pkthdr_setlen(m2, total);

    // Free original and return rebuilt
    mbuf_freem(m);
    return m2;
}
