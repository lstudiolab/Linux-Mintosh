#include "include/xcm/usi_tokens.h"

#include <stdbool.h>
#include <string.h>

static bool valid_utf8(const unsigned char *text, size_t size)
{
    size_t i = 0;
    while (i < size) {
        unsigned char a = text[i++];
        if (a == 0) return false;
        if (a < 0x80) continue;
        if (a >= 0xc2 && a <= 0xdf) {
            if (i >= size || (text[i++] & 0xc0) != 0x80) return false;
            continue;
        }
        if (a >= 0xe0 && a <= 0xef) {
            if (i + 1 >= size) return false;
            unsigned char b = text[i++];
            unsigned char c = text[i++];
            if ((c & 0xc0) != 0x80 || (b & 0xc0) != 0x80) return false;
            if (a == 0xe0 && b < 0xa0) return false;
            if (a == 0xed && b >= 0xa0) return false;
            continue;
        }
        if (a >= 0xf0 && a <= 0xf4) {
            if (i + 2 >= size) return false;
            unsigned char b = text[i++];
            unsigned char c = text[i++];
            unsigned char d = text[i++];
            if ((b & 0xc0) != 0x80 || (c & 0xc0) != 0x80 ||
                (d & 0xc0) != 0x80) return false;
            if (a == 0xf0 && b < 0x90) return false;
            if (a == 0xf4 && b > 0x8f) return false;
            continue;
        }
        return false;
    }
    return true;
}

int xcm_usi_validate(const void *data, size_t size)
{
    if (data == NULL || size < sizeof(struct xcm_usi_stream_header) ||
        size > XCM_MAX_MESSAGE_SIZE)
        return -1;

    struct xcm_usi_stream_header stream;
    memcpy(&stream, data, sizeof(stream));
    if (stream.magic != XCM_USI_STREAM_MAGIC ||
        stream.version != XCM_USI_STREAM_VERSION ||
        stream.token_count == 0 || stream.token_count > XCM_USI_MAX_TOKENS ||
        stream.total_size != size || stream.reserved != 0)
        return -1;

    const unsigned char *bytes = data;
    size_t offset = sizeof(stream);
    uint16_t stack[XCM_USI_MAX_DEPTH];
    size_t depth = 0;

    for (uint16_t index = 0; index < stream.token_count; ++index) {
        if (size - offset < sizeof(struct xcm_usi_token_header)) return -1;
        struct xcm_usi_token_header token;
        memcpy(&token, bytes + offset, sizeof(token));
        offset += sizeof(token);
        if (token.flags != 0 || token.size > size - offset) return -1;
        const unsigned char *payload = bytes + offset;

        switch (token.type) {
        case XCM_USI_TEXT:
            if (token.size == 0 || token.size > XCM_USI_MAX_TEXT ||
                !valid_utf8(payload, token.size)) return -1;
            break;
        case XCM_USI_ROW_BEGIN:
        case XCM_USI_COLUMN_BEGIN:
            if (token.size != 0 || depth >= XCM_USI_MAX_DEPTH) return -1;
            stack[depth++] = token.type == XCM_USI_ROW_BEGIN
                ? XCM_USI_ROW_END : XCM_USI_COLUMN_END;
            break;
        case XCM_USI_ROW_END:
        case XCM_USI_COLUMN_END:
            if (token.size != 0 || depth == 0 || stack[depth - 1] != token.type)
                return -1;
            --depth;
            break;
        case XCM_USI_SPACER:
            if (token.size != 0) return -1;
            break;
        case XCM_USI_BUTTON: {
            if (token.size < sizeof(struct xcm_usi_button_prefix)) return -1;
            struct xcm_usi_button_prefix button;
            memcpy(&button, payload, sizeof(button));
            if (button.action_id == 0 || button.reserved != 0 ||
                button.reserved2 != 0 || button.label_size == 0 ||
                button.label_size > 128 ||
                token.size != sizeof(button) + button.label_size ||
                !valid_utf8(payload + sizeof(button), button.label_size))
                return -1;
            break;
        }
        case XCM_USI_IMAGE_RESOURCE: {
            uint64_t resource_id = 0;
            if (token.size != sizeof(resource_id)) return -1;
            memcpy(&resource_id, payload, sizeof(resource_id));
            if (resource_id == 0) return -1;
            break;
        }
        default:
            return -1;
        }
        offset += token.size;
    }

    return offset == size && depth == 0 ? 0 : -1;
}
