#include "Foundation.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static char *copy_range(const char *start, size_t length) {
    char *copy = malloc(length + 1);
    if (!copy) return NULL;
    memcpy(copy, start, length);
    copy[length] = '\0';
    return copy;
}

static int valid_scheme_character(unsigned char value, int first) {
    if ((value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z'))
        return 1;
    if (first) return 0;
    return (value >= '0' && value <= '9') || value == '+' ||
           value == '-' || value == '.';
}

static int valid_percent_encoding(const char *text) {
    for (const unsigned char *cursor = (const unsigned char *)text; *cursor; ++cursor) {
        if (*cursor != '%') continue;
        if (!isxdigit(cursor[1]) || !isxdigit(cursor[2])) return 0;
        cursor += 2;
    }
    return 1;
}


static int valid_hostname(const char *host) {
    if (!host || !host[0]) return 0;
    size_t label_length = 0;
    for (const unsigned char *cursor = (const unsigned char *)host; ; ++cursor) {
        unsigned char value = *cursor;
        if (value == '.' || value == '\\0') {
            if (!label_length || host[cursor - (const unsigned char *)host - 1] == '-')
                return 0;
            label_length = 0;
            if (!value) return 1;
            continue;
        }
        if (!((value >= 'a' && value <= 'z') ||
              (value >= '0' && value <= '9') || value == '-')) return 0;
        if (!label_length && value == '-') return 0;
        ++label_length;
        if (label_length > 63) return 0;
    }
}

static int parse_port(const char *text, size_t length, unsigned int *port) {
    if (!text || !length) return -1;
    unsigned int value = 0;
    for (size_t index = 0; index < length; ++index) {
        unsigned char character = (unsigned char)text[index];
        if (character < '0' || character > '9') return -1;
        unsigned int digit = (unsigned int)(character - '0');
        if (value > (65535u - digit) / 10u) return -1;
        value = value * 10u + digit;
    }
    if (value == 0) return -1;
    *port = value;
    return 0;
}

void mf_url_destroy(MFURL *url) {
    if (!url) return;
    free(url->scheme);
    free(url->authority);
    free(url->host);
    free(url->path);
    free(url->query);
    free(url->fragment);
    *url = (MFURL){0};
}

int mf_url_parse(const char *text, MFURL *url, MFError *error) {
    mf_error_clear(error);
    if (!text || !url || !text[0]) {
        mf_error_set(error, MF_ERROR_INVALID_ARGUMENT, 0, "URL and output are required");
        return -1;
    }
    size_t length = strlen(text);
    if (length > 8192) {
        mf_error_set(error, MF_ERROR_LIMIT, 0, "URL exceeds 8192 bytes");
        return -1;
    }
    for (const unsigned char *cursor = (const unsigned char *)text; *cursor; ++cursor) {
        if (*cursor <= 0x20 || *cursor == 0x7f) {
            mf_error_set(error, MF_ERROR_INVALID_ARGUMENT, 0, "URL contains whitespace or control characters");
            return -1;
        }
    }
    if (!valid_percent_encoding(text)) {
        mf_error_set(error, MF_ERROR_INVALID_ARGUMENT, 0, "URL contains invalid percent encoding");
        return -1;
    }

    MFURL parsed = {0};
    const char *colon = strchr(text, ':');
    if (!colon || colon == text) {
        mf_error_set(error, MF_ERROR_INVALID_ARGUMENT, 0, "absolute URL scheme is required");
        return -1;
    }
    for (const unsigned char *cursor = (const unsigned char *)text;
         cursor < (const unsigned char *)colon; ++cursor) {
        if (!valid_scheme_character(*cursor,
                cursor == (const unsigned char *)text)) {
            mf_error_set(error, MF_ERROR_INVALID_ARGUMENT, 0, "invalid URL scheme");
            return -1;
        }
    }
    parsed.scheme = copy_range(text, (size_t)(colon - text));
    if (!parsed.scheme) goto memory_error;
    for (char *cursor = parsed.scheme; *cursor; ++cursor)
        *cursor = (char)tolower((unsigned char)*cursor);

    const char *cursor = colon + 1;
    const char *end = text + length;
    const char *fragment = memchr(cursor, '#', (size_t)(end - cursor));
    const char *query = memchr(cursor, '?',
        (size_t)((fragment ? fragment : end) - cursor));
    const char *path_end = query ? query : (fragment ? fragment : end);

    if (fragment) {
        parsed.fragment = copy_range(fragment + 1, (size_t)(end - fragment - 1));
        if (!parsed.fragment) goto memory_error;
    }
    if (query) {
        parsed.query = copy_range(query + 1,
            (size_t)((fragment ? fragment : end) - query - 1));
        if (!parsed.query) goto memory_error;
    }

    if (path_end - cursor >= 2 && cursor[0] == '/' && cursor[1] == '/') {
        parsed.has_authority = 1;
        const char *authority_start = cursor + 2;
        const char *authority_end = authority_start;
        while (authority_end < path_end && *authority_end != '/') ++authority_end;
        parsed.authority = copy_range(authority_start,
                                      (size_t)(authority_end - authority_start));
        if (!parsed.authority) goto memory_error;
        const char *host_start = authority_start;
        for (const char *scan = authority_start; scan < authority_end; ++scan)
            if (*scan == '@') host_start = scan + 1;
        const char *host_end = authority_end;
        const char *port_start = NULL;

        if (host_start < authority_end && *host_start == '[') {
            const char *closing = memchr(host_start + 1, ']',
                                          (size_t)(authority_end - host_start - 1));
            if (!closing) goto invalid_authority;
            parsed.host = copy_range(host_start + 1,
                                     (size_t)(closing - host_start - 1));
            if (!parsed.host) goto memory_error;
            struct in6_addr ipv6_address;
            if (inet_pton(AF_INET6, parsed.host, &ipv6_address) != 1)
                goto invalid_authority;
            if (closing + 1 < authority_end) {
                if (closing[1] != ':') goto invalid_authority;
                port_start = closing + 2;
            }
        } else {
            const char *first_colon = memchr(host_start, ':',
                                             (size_t)(authority_end - host_start));
            if (first_colon) {
                if (memchr(first_colon + 1, ':',
                           (size_t)(authority_end - first_colon - 1)))
                    goto invalid_authority;
                host_end = first_colon;
                port_start = first_colon + 1;
            }
            if (host_end == host_start && strcmp(parsed.scheme, "file") != 0)
                goto invalid_authority;
            parsed.host = copy_range(host_start, (size_t)(host_end - host_start));
            if (!parsed.host) goto memory_error;
            if (parsed.host[0] && !valid_hostname(parsed.host))
                goto invalid_authority;
        }
        if (port_start) {
            if (parse_port(port_start, (size_t)(authority_end - port_start),
                           &parsed.port) < 0) goto invalid_authority;
            parsed.has_port = 1;
        }
        for (char *host = parsed.host; host && *host; ++host)
            *host = (char)tolower((unsigned char)*host);
        if (!parsed.host || !parsed.host[0]) {
            if (strcmp(parsed.scheme, "file") != 0) goto invalid_authority;
        }
        cursor = authority_end;
    }

    if ((strcmp(parsed.scheme, "http") == 0 ||
         strcmp(parsed.scheme, "https") == 0) &&
        (!parsed.has_authority || !parsed.host || !parsed.host[0]))
        goto invalid_authority;

    parsed.path = copy_range(cursor, (size_t)(path_end - cursor));
    if (!parsed.path) goto memory_error;
    *url = parsed;
    return 0;

invalid_authority:
    mf_url_destroy(&parsed);
    mf_error_set(error, MF_ERROR_INVALID_ARGUMENT, 0, "invalid URL authority or port");
    return -1;
memory_error:
    mf_url_destroy(&parsed);
    mf_error_set(error, MF_ERROR_OUT_OF_MEMORY, 0, "unable to allocate URL components");
    return -1;
}

int mf_url_is_secure(const MFURL *url) {
    return url && url->scheme && strcmp(url->scheme, "https") == 0;
}
