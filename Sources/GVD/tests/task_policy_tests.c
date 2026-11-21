#include "task_policy.h"
#include <assert.h>
#include <sched.h>
int main(void) {
    pid_t pid = 0;
    int value = 0, policy = -1;
    assert(gvd_parse_pid("42", &pid) == 0 && pid == 42);
    assert(gvd_parse_pid("1", &pid) < 0);
    assert(gvd_parse_pid("-5", &pid) < 0);
    assert(gvd_parse_pid("42x", &pid) < 0);
    assert(gvd_parse_nice("-20", &value) == 0 && value == -20);
    assert(gvd_parse_nice("19", &value) == 0 && value == 19);
    assert(gvd_parse_nice("20", &value) < 0);
    assert(gvd_parse_nice("abc", &value) < 0);
    assert(gvd_parse_policy("normal", &policy) == 0 && policy == SCHED_OTHER);
    assert(gvd_parse_policy("batch", &policy) == 0 && policy == SCHED_BATCH);
    assert(gvd_parse_policy("idle", &policy) == 0 && policy == SCHED_IDLE);
    assert(gvd_parse_policy("fifo", &policy) < 0);
    return 0;
}
