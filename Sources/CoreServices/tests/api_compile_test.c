#include <coreservices/coreservices.h>

_Static_assert(sizeof(CoreServicesSystemStatus) == sizeof(CSFSystemStatus),
               "Core Services status ABI mismatch");

int main(void)
{
    int (*ping)(void) = core_system_ping;
    int (*valid_name)(const char *) = core_service_name_is_valid;
    return ping == 0 || valid_name == 0;
}
