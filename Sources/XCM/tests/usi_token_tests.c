#include "../include/xcm/usi_tokens.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static size_t append_token(unsigned char *buffer, size_t offset,
                           uint16_t type, const void *payload, uint32_t size)
{
    struct xcm_usi_token_header token = { type, 0, size };
    memcpy(buffer + offset, &token, sizeof(token));
    offset += sizeof(token);
    if (size != 0) memcpy(buffer + offset, payload, size);
    return offset + size;
}

int main(void)
{
    unsigned char stream[256] = {0};
    struct xcm_usi_stream_header header = {
        XCM_USI_STREAM_MAGIC, XCM_USI_STREAM_VERSION, 4, 0, 0
    };
    memcpy(stream, &header, sizeof(header));
    size_t offset = sizeof(header);
    offset = append_token(stream, offset, XCM_USI_ROW_BEGIN, NULL, 0);
    const char text[] = "Mintosh";
    offset = append_token(stream, offset, XCM_USI_TEXT, text, sizeof(text) - 1U);
    struct xcm_usi_button_prefix button = { 7, 2, 0, 0 };
    unsigned char button_data[sizeof(button) + 2U];
    memcpy(button_data, &button, sizeof(button));
    memcpy(button_data + sizeof(button), "OK", 2);
    offset = append_token(stream, offset, XCM_USI_BUTTON, button_data, sizeof(button_data));
    offset = append_token(stream, offset, XCM_USI_ROW_END, NULL, 0);
    header.total_size = (uint32_t)offset;
    memcpy(stream, &header, sizeof(header));
    assert(xcm_usi_validate(stream, offset) == 0);

    unsigned char invalid[256];
    memcpy(invalid, stream, offset);
    struct xcm_usi_stream_header invalid_header;
    memcpy(&invalid_header, invalid, sizeof(invalid_header));
    invalid_header.magic = 0;
    memcpy(invalid, &invalid_header, sizeof(invalid_header));
    assert(xcm_usi_validate(invalid, offset) != 0);

    memcpy(invalid, stream, offset);
    struct xcm_usi_token_header invalid_token;
    memcpy(&invalid_token, invalid + sizeof(header), sizeof(invalid_token));
    invalid_token.flags = 1;
    memcpy(invalid + sizeof(header), &invalid_token, sizeof(invalid_token));
    assert(xcm_usi_validate(invalid, offset) != 0);

    memcpy(invalid, stream, offset);
    memcpy(&invalid_header, invalid, sizeof(invalid_header));
    invalid_header.token_count = 3;
    memcpy(invalid, &invalid_header, sizeof(invalid_header));
    assert(xcm_usi_validate(invalid, offset) != 0);

    memcpy(invalid, stream, offset);
    invalid[sizeof(header) + sizeof(struct xcm_usi_token_header) +
            sizeof(struct xcm_usi_token_header)] = 0xff;
    assert(xcm_usi_validate(invalid, offset) != 0);

    memcpy(invalid, stream, offset);
    memcpy(&invalid_token, invalid + sizeof(header), sizeof(invalid_token));
    invalid_token.type = 999;
    memcpy(invalid + sizeof(header), &invalid_token, sizeof(invalid_token));
    assert(xcm_usi_validate(invalid, offset) != 0);

    assert(xcm_usi_validate(stream, sizeof(header) - 1U) != 0);
    assert(xcm_usi_validate(NULL, 0) != 0);

    puts("XCM USI token validation tests passed.");
    return 0;
}
