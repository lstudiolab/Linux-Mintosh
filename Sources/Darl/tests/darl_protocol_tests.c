#include "darl_protocol.h"
#include <assert.h>
#include <string.h>

int main(void) {
    char response[128];
    darl_format_response("PING", response, sizeof response);
    assert(strcmp(response, "OK Darl 0.2.0\n") == 0);
    darl_format_response("VERSION", response, sizeof response);
    assert(strcmp(response, "Mintosh 0.1.0; Darl 0.2.0\n") == 0);
    darl_format_response("STATUS", response, sizeof response);
    assert(strcmp(response, "Darl=running\n") == 0);
    darl_format_response("EXEC /bin/sh", response, sizeof response);
    assert(strcmp(response, "ERROR unsupported command\n") == 0);
    darl_format_response(NULL, response, sizeof response);
    assert(strcmp(response, "ERROR invalid request\n") == 0);
    char small[4];
    darl_format_response("PING", small, sizeof small);
    assert(small[sizeof small - 1] == '\0');
    return 0;
}
