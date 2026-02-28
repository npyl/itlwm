//
//  AirportItlwmInterface.cpp
//  AirportItlwm
//
//  Created by qcwap on 2020/9/7.
//  Copyright © 2020 钟先耀. All rights reserved.
//

#include "AirportItlwmInterface.hpp"

#include "CrashFix.hpp"

#define super IO80211Interface
OSDefineMetaClassAndStructors(AirportItlwmInterface, IO80211Interface);

const char* hexdump(uint8_t *buf, size_t len)
{
    ssize_t str_len = len * 3 + 1;
    char *str = (char*)IOMalloc(str_len);
    if (!str)
        return nullptr;
    for (size_t i = 0; i < len; i++)
    snprintf(str + 3 * i, (len - i) * 3, "%02x ", buf[i]);
    str[MAX(str_len - 2, 0)] = 0;
    return str;
}

bool AirportItlwmInterface::
init(IO80211Controller *controller, ItlHalService *halService)
{
    if (!super::init(controller))
        return false;
    this->fHalService = halService;
    return true;
}

UInt32
AirportItlwmInterface::inputPacket(mbuf_t packet, UInt32 length, IOOptionBits options, void *param)
{
    mbuf_t p2 = ensure_pkthdr(packet);
    if (!p2) {
        // Return length to indicate "consumed" even though dropped
        // (prevents potential resend/looping behavior in some paths)
        return length;
    }

    UInt32 newLen = (UInt32)mbuf_total_len(p2);
    return IOEthernetInterface::inputPacket(p2, newLen, options, param);
}
