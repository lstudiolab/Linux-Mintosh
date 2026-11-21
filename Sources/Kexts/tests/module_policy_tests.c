#include "module_policy.h"
#include <assert.h>
int main(void) {
    assert(kexts_signature_value_enforced("Y\n") == 1);
    assert(kexts_signature_value_enforced("1") == 1);
    assert(kexts_signature_value_enforced(" n ") == 0);
    assert(kexts_signature_value_enforced("0") == 0);
    assert(kexts_signature_value_enforced("unknown") == -1);
    assert(kexts_signature_value_enforced(NULL) == -1);
    return 0;
}
