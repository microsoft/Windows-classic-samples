#pragma once

#include "..\ProxyStub\ISparkleFinisher_h.h"

#include <wrl/module.h>
#include <wrl/implements.h>

using namespace Microsoft::WRL;

// Note that this GUID is also declared in ISparkleFinsher.idl to make the GUID form available for CoCreateInstance.
struct __declspec(uuid("EA27C73A-48C2-4714-9D20-A9D2C4F6AED3"))
    SparkleFinisher : public RuntimeClass<RuntimeClassFlags<ClassicCom>, ISparkleFinisher>
{
    STDMETHODIMP AddSparkleFinishToFile(HANDLE decorateThisFile,
        HANDLE whenThisEventFires,
        HANDLE* willNotifyWhenDone) noexcept override;
};

// This directs WRL to load metadata on this class into a section of the binary
// that ::Microsoft::WRL::Module can pick up to do server class registrations.
CoCreatableClass(SparkleFinisher);
