#ifndef MINTOSH_CORE_SERVICES_H
#define MINTOSH_CORE_SERVICES_H

#include "csf.h"

/* Stable public Core Services names over the first CSF implementation ABI. */
typedef CSFSystemStatus CoreServicesSystemStatus;

#define core_system_ping csf_system_ping
#define core_system_version csf_system_version
#define core_system_status csf_system_status
#define core_service_name_is_valid csf_service_name_is_valid
#define core_service_list csf_service_list
#define core_service_control csf_service_control

#endif
