#ifndef MINTOSH_XCM_USI_TOKENS_H
#define MINTOSH_XCM_USI_TOKENS_H

#include <stddef.h>
#include <stdint.h>
#include "protocol.h"

#define XCM_USI_STREAM_MAGIC UINT32_C(0x49535558)
#define XCM_USI_STREAM_VERSION 1U
#define XCM_USI_MAX_TOKENS 256U
#define XCM_USI_MAX_DEPTH 32U
#define XCM_USI_MAX_TEXT 1024U

enum xcm_usi_token_type {
    XCM_USI_TEXT = 1,
    XCM_USI_ROW_BEGIN = 2,
    XCM_USI_ROW_END = 3,
    XCM_USI_COLUMN_BEGIN = 4,
    XCM_USI_COLUMN_END = 5,
    XCM_USI_SPACER = 6,
    XCM_USI_BUTTON = 7,
    XCM_USI_IMAGE_RESOURCE = 8
};

struct xcm_usi_stream_header {
    uint32_t magic;
    uint16_t version;
    uint16_t token_count;
    uint32_t total_size;
    uint32_t reserved;
};

struct xcm_usi_token_header {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
};

struct xcm_usi_button_prefix {
    uint64_t action_id;
    uint16_t label_size;
    uint16_t reserved;
    uint32_t reserved2;
};

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(struct xcm_usi_stream_header) == 16, "USI stream ABI changed");
_Static_assert(sizeof(struct xcm_usi_token_header) == 8, "USI token ABI changed");
_Static_assert(sizeof(struct xcm_usi_button_prefix) == 12, "USI button ABI changed");
#endif

/* Returns zero only for a complete, bounded, structurally valid presentation stream. */
int xcm_usi_validate(const void *data, size_t size);

#endif
