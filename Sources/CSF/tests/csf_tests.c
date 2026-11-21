#include "csf.h"
#include <assert.h>

int main(void) {
    assert(csf_service_name_is_valid("darl"));
    assert(csf_service_name_is_valid("user-session_2"));
    assert(!csf_service_name_is_valid(""));
    assert(!csf_service_name_is_valid("../darl"));
    assert(!csf_service_name_is_valid("darl service"));
    assert(!csf_service_name_is_valid("Darl"));
    assert(!csf_service_name_is_valid("darl;id"));
    assert(!csf_service_name_is_valid("a-name-that-is-longer-than-sixty-three-characters-and-invalid"));
    return 0;
}
