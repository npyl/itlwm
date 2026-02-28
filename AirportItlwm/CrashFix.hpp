//
//  CrashFix.hpp
//  itlwm
//
//  Created by giurika on 28/2/26.
//  Copyright © 2026 钟先耀. All rights reserved.
//

#ifndef CRASH_FIX_H
#define CRASH_FIX_H

#include <sys/mbuf.h>

static size_t mbuf_total_len(mbuf_t m);

static mbuf_t ensure_pkthdr(mbuf_t m);

#endif // !CRASH_FIX_H
