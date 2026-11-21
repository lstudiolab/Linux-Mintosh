#include "Foundation.h"
#include <stdio.h>
#include <string.h>

void mf_error_clear(MFError *error) {
    if (error) *error = (MFError){0};
}

void mf_error_set(MFError *error, MFErrorCode code, int system_code,
                  const char *message) {
    if (!error) return;
    error->code = code;
    error->system_code = system_code;
    if (!message) message = "";
    snprintf(error->message, sizeof error->message, "%s", message);
}
