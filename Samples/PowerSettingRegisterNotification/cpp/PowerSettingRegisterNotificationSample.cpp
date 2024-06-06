#include "initguid.h"
#include "Windows.h"
#include "winnt.h"
#include "powrprof.h"
#include "powersetting.h"
#include "stdio.h"
#include "iostream"

#pragma comment(lib, "powrprof.lib")

ULONG CALLBACK EnergySaverPowerSettingCallback(
    _In_opt_ PVOID Context,
    _In_ ULONG Type,
    _In_ PVOID Setting
)

/*++

Routine Description:

    This is the callback function for when GUID_ENERGY_SAVER_STATUS power setting
    notification is triggered. It shows an example of how an App can
    adjust behavior depending on the energy saver status. 

Arguments:

    Context - The context provided when registering for the power notification.

    Type    - The type of power event that caused this notification (e.g.
              GUID_ENERGY_SAVER_STATUS)

    Setting - The data associated with the power event. For
              GUID_ENERGY_SAVER_STATUS, this is a value of type ENERGY_SAVER_STATUS.

Return Value:

    Returns error status.

--*/

{

    UNREFERENCED_PARAMETER(Type);
    UNREFERENCED_PARAMETER(Context);

    auto powerSetting = reinterpret_cast<PPOWERBROADCAST_SETTING>(Setting);
    GUID const& settingId = powerSetting->PowerSetting;

    //
    // Check the data size is expected
    //
    if (settingId != GUID_ENERGY_SAVER_STATUS  || powerSetting->DataLength != sizeof(ENERGY_SAVER_STATUS))
    {
        return ERROR_INVALID_PARAMETER;
    }

    auto status = *reinterpret_cast<PENERGY_SAVER_STATUS>(powerSetting->Data);

    //
    // Change app behavior depending on energy saver status.
    // For example, an app that does data synchronization might reduce its
    // synchronization when under standard energy saver mode and pause it
    // entirely when under extreme energy saver mode.
    //

    switch (status) {
        case ENERGY_SAVER_STANDARD:
            printf("Standard energy saver mode: Reduce activities.\n");
            break;

        case ENERGY_SAVER_HIGH_SAVINGS:
            printf("Extreme energy saver mode: Pause all non-essential activities.\n");
            break;

        default:
            printf("Energy saver not active: Run normally.\n");
            break; 
    }

    return ERROR_SUCCESS;
}


int main()

/*++

Routine Description:

    Main method which runs when exe is launched. It registers for the GUID_ENERGY_SAVER_STATUS notification
    and waits for notifications until the user terminates the program by entering any input and hitting "enter".
    While the program is running, the callback method will print out to the console the notification is triggered
    (according to the callback method). 

Return Value:

    Returns error status.

--*/

{

    DEVICE_NOTIFY_SUBSCRIBE_PARAMETERS params{};
    params.Callback = EnergySaverPowerSettingCallback;

    HPOWERNOTIFY registrationHandle{ nullptr };

    //
    // Register for GUID_ENERGY_SAVER_STATUS to receive notifications whenever setting updates
    //

    DWORD error = PowerSettingRegisterNotification(
        &GUID_ENERGY_SAVER_STATUS,
        DEVICE_NOTIFY_CALLBACK,
        &params,
        &registrationHandle);

    if (error != ERROR_SUCCESS) {
        printf("Error registering for GUID_ENERGY_SAVER_STATUS: %d. Terminating program...\n\n", error);

        return (int)error;
    }

    printf("Registered for GUID_ENERGY_SAVER_STATUS notifications\n\n"); 

    //
    // Wait for user input before unregistering... 
    //

    printf("Waiting for GUID_ENERGY_SAVER_STATUS notifications... \n\n");
    printf("You can toggle Energy Saver in Quick Settings or Settings > Power & Battery to trigger the notification.\n\n");
    printf("Press any key to end the program...\n\n"); 
    std::cin.get(); 
    
    //
    // Unregister from GUID_ENERGY_SAVER_STATUS notifications for cleanup
    //

    PowerSettingUnregisterNotification(registrationHandle);
    printf("Unregistered for GUID_ENERGY_SAVER_STATUS notifications\n\n");

    return ERROR_SUCCESS;
}

