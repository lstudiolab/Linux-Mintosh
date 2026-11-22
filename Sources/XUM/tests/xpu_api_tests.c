#include <xpu/xpu.h>

#include <assert.h>
#include <string.h>

int main(void)
{
    xpu_request request;
    memset(&request, 0, sizeof(request));
    request.version = XPU_PROTOCOL_VERSION;
    request.operation = XPU_OP_READ_SYSTEM_INFO;
    request.target = XPU_TARGET_SYSTEM_INFO;
    request.payload_type = XPU_PAYLOAD_NONE;
    assert(sizeof(request) == 40);
    assert(XPU_DEFAULT_SOCKET[0] == '/');
    return 0;
}
