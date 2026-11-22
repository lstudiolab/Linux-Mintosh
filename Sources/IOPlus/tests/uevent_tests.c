#include "uevent.h"
#include <assert.h>
#include <string.h>

int main(void) {
    const char valid[] =
        "add@/devices/pci0000:00/0000:00:14.0\0"
        "ACTION=add\0DEVPATH=/devices/pci0000:00/0000:00:14.0\0"
        "SUBSYSTEM=pci\0DEVTYPE=pci_device\0DRIVER=xhci_hcd\0SEQNUM=42\0";
    IOPLUSUEvent event;
    assert(ioplus_parse_uevent(valid, sizeof valid - 1, &event) == 0);
    assert(strcmp(event.action, "add") == 0);
    assert(strcmp(event.subsystem, "pci") == 0);
    assert(strcmp(event.driver, "xhci_hcd") == 0);
    assert(strcmp(event.sequence, "42") == 0);

    const char bad_action[] = "explode@/devices/x\0ACTION=explode\0DEVPATH=/devices/x\0";
    assert(ioplus_parse_uevent(bad_action, sizeof bad_action - 1, &event) < 0);
    const char missing_path[] = "add@\0ACTION=add\0";
    assert(ioplus_parse_uevent(missing_path, sizeof missing_path - 1, &event) < 0);
    assert(ioplus_parse_uevent(NULL, 0, &event) < 0);
    char oversized[1024];
    memset(oversized, 'x', sizeof oversized);
    oversized[0] = 'a'; oversized[1] = 'd'; oversized[2] = 'd'; oversized[3] = '@';
    assert(ioplus_parse_uevent(oversized, sizeof oversized, &event) < 0);
    return 0;
}
