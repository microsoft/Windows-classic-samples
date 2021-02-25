---
page_type: sample
languages:
- csharp
products:
- windows-server
name: System Insights sample capability
urlFragment: system-insights
description: Demonstrates how to write a capability in System Insights.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# System Insights sample capability

This sample demonstrates how to write a capability in [System Insights](https://aka.ms/systeminsights). This sample demonstrates how to:

- Specify capability metadata, such as the version, publisher, and description.
- Register the data sources to collect and persist locally.
- Make a prediction by reading the data sources that System Insights has collected and persisted. 
- Return prediction results to System Insights.
- Cancel a prediction.

## Related topics

[System Insights overview](https://aka.ms/systeminsights)

[Adding and developing capabilities](https://aka.ms/systeminsights-addcapabilities)

## Operating system requirements
**Server:** Windows Server Version 1809 or later

## Build the sample
1. Start Microsoft Visual Studio and select **File** > **Open** > **Project/Solution**.
2. Open **SampleCapability.sln**, a Visual Studio Solution file.
3. Press F7 or use **Build** > **Build Solution** to build the sample. 

## Run the sample
After you've created the capability library, you need to add the capability to System Insights.
1. Confirm you're running the a version of Windows Server which has the System Insights feature installed.
    - This sample requires Windows Server Version 1809 or later.
2. Copy the capability library onto the server. 
3. Add the capability using the **Add-InsightsCapability** cmdlet:

```PowerShell
Add-InsightsCapability -Name "Sample capability" -Library "C:\SampleCapability.dll"
```
4. Invoke the capability using the **Invoke-InsightsCapability** cmdlet. Note that System Insights may not have collected any data yet if you invoke the capability immediately after adding it.


