# Registration-free WinRT in C++ Desktop App Sample

This sample demonstrates how to use [Registration-free WinRT](https://aka.ms/regfreewinrtblog) a new feature in Windows 10 Version 1903 to reference a C++ Windows Runtime component in a non-packaged C++ desktop app. It contains a C++ desktop app (CppConsoleApp.vcxproj) and a WinRT Component (WinRTComponent.vcxproj).

You can learn more about Registration-free WinRT [here](https://aka.ms/regfreewinrtblog).

The C++ desktop app uses:
* [C++/WinRT](https://github.com/Microsoft/xlang/blob/f1309fe42d929d612aa9b66557d9a22769067b3f/src/package/cppwinrt/nuget/readme.md) to create projection header files of the component for use in the app code
* The [Microsoft.VCRTForwarders.140](https://aka.ms/vcrtfwdnuget) NuGet package to manage C++ Runtime dependencies for the component. The GitHub repo can be found [here](https://aka.ms/vcrtfwdgit).

## Requirements

The sample requires Windows 10 Version 1903.

## Instructions

### CppConsoleApp

A non-packaged C++ app referencing a C++ WinRT Component.

* The specific DLLs and classes (from the WinRT component) being referenced should be declared in the [Win32 Application manifest](https://docs.microsoft.com/en-us/windows/desktop/SbsCs/application-manifests) - CppConsoleApp.exe.manifest.

To add a new Win32 Application Manifest to a desktop app, right click on the project, then select Add, New Item, General, Application Manifest File, and name it yourappname.exe.manifest.

* For the System to locate the component DLLs specified in the application manifest at runtime, the DLLs must be in the same directory as the app's .exe. This project uses a propery sheet (PropertySheet.props) to copy the component DLL to the app's output directory.

* The property sheet is also used to add a reference to the component's WinMD to enable [C++/WinRT](https://github.com/Microsoft/xlang/tree/master/src/package/cppwinrt/nuget) to generate projection headers of the component which are used in the app code.

* To add your own property sheet to a C++ project:
    * right-click the project node
    * select Add, New Item, Visual C++, Property Sheets, Property Sheet (.props) 
    * edit the resulting property sheet file
    * add the necessary project customizations
    * select View, Other Windows, Property Manager
    * right-click the project node
    * select Add Existing Property Sheet...
    * select the newly created property sheet file

### WinRTComponent

A simple C++ WinRT Component with a class returning a string

* The component is written using [C++/WinRT](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/intro-to-using-cpp-with-winrt) instead of [C++/CX](https://docs.microsoft.com/en-us/cpp/cppcx/visual-c-language-reference-c-cx?view=vs-2017). Therefore, it requires an installed Nuget Package: [Microsoft.Windows.CppWinRT](https://www.nuget.org/packages/Microsoft.Windows.CppWinRT/).
* To manage the specific C++ Runtime Dependencies of the WinRT component, the app uses the [Microsoft.VCRTForwarders.140](https://aka.ms/vcrtfwdnuget) NuGet package.

### Building and running the sample

1. Load the Project solution.
2. Retarget the solution to the SDK version on your machine: Right click, Retarget solution
3. Build the WinRTComponent project.
4. Build the CppConsoleApp project
5. Run the app


