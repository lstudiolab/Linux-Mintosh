#include "power_control.h"
#include <assert.h>
int main(void) {
    unsigned int cpu = 0;
    assert(iops_parse_cpu_id("0", &cpu) == 0 && cpu == 0);
    assert(iops_parse_cpu_id("128", &cpu) == 0 && cpu == 128);
    assert(iops_parse_cpu_id("-1", &cpu) < 0);
    assert(iops_parse_cpu_id("1x", &cpu) < 0);
    assert(iops_valid_token("powersave"));
    assert(iops_valid_token("schedutil"));
    assert(!iops_valid_token("--help"));
    assert(!iops_valid_token("power save"));
    assert(iops_choice_available("powersave", "performance powersave schedutil"));
    assert(!iops_choice_available("power", "performance powersave"));
    return 0;
}
