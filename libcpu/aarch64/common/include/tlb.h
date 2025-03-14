/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-11-28     WangXiaoyao  the first version
 */
#ifndef __TLB_H__
#define __TLB_H__

#include <rtthread.h>
#include <stddef.h>
#include <stdint.h>
#include "mm_aspace.h"
#include "mmu.h"

#define TLBI_ARG(addr, asid)                                                   \
    ({                                                                         \
        rt_ubase_t arg = (rt_ubase_t)(addr) >> ARCH_PAGE_SHIFT;                \
        arg &= (1ull << 44) - 1;                                               \
        arg |= (rt_ubase_t)(asid) << MMU_ASID_SHIFT;                           \
        (void *)arg;                                                           \
    })

static inline void rt_hw_tlb_invalidate_all(void)
{
    __asm__ volatile(
        // ensure updates to pte completed
        "dsb ishst\n"
        "tlbi vmalle1is\n"
        "dsb ish\n"
        // after tlb in new context, refresh inst
        "isb\n" ::
            : "memory");
}

static inline void rt_hw_tlb_invalidate_all_local(void)
{
    __asm__ volatile(
        // ensure updates to pte completed
        "dsb nshst\n"
        "tlbi vmalle1is\n"
        "dsb nsh\n"
        // after tlb in new context, refresh inst
        "isb\n" ::
            : "memory");
}

static inline void rt_hw_tlb_invalidate_aspace(rt_aspace_t aspace)
{
#ifdef ARCH_USING_ASID
    __asm__ volatile(
        // ensure updates to pte completed
        "dsb nshst\n"
        "tlbi aside1is, %0\n"
        "dsb nsh\n"
        // after tlb in new context, refresh inst
        "isb\n" ::"r"(TLBI_ARG(0ul, aspace->asid))
            : "memory");
#else
    rt_hw_tlb_invalidate_all();
#endif
}

static inline void rt_hw_tlb_invalidate_page(rt_aspace_t aspace, void *start)
{
    start = TLBI_ARG(start, 0);
    __asm__ volatile(
        "dsb ishst\n"
        "tlbi vaae1is, %0\n"
        "dsb ish\n"
        "isb\n" ::"r"(start)
        : "memory");
}

static inline void rt_hw_tlb_invalidate_range(rt_aspace_t aspace, void *start,
                                              size_t size, size_t stride)
{
    if (size <= ARCH_PAGE_SIZE)
    {
        rt_hw_tlb_invalidate_page(aspace, start);
    }
    else
    {
        rt_hw_tlb_invalidate_aspace(aspace);
    }
}

#endif /* __TLB_H__ */
