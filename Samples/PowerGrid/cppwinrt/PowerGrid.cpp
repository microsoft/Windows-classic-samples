#include <winrt/Windows.Devices.Power.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Globalization.DateTimeFormatting.h>
#include <winrt/Windows.Globalization.NumberFormatting.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

namespace winrt
{
    using namespace winrt::Windows::Devices::Power;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Globalization::DateTimeFormatting;
    using namespace winrt::Windows::Globalization::NumberFormatting;
}

std::wstring FormatDateTime(winrt::DateTime dateTime)
{
    return std::wstring{ winrt::DateTimeFormatter(L"shortdate shorttime").Format(dateTime) };
}

std::wstring FormatSeverity(double severity)
{
    winrt::DecimalFormatter severityFormatter;
    severityFormatter.FractionDigits(2);
    severityFormatter.IntegerDigits(1);
    severityFormatter.IsDecimalPointAlwaysDisplayed(true);
    winrt::IncrementNumberRounder rounder;
    rounder.Increment(0.01);
    severityFormatter.NumberRounder(rounder);
    return std::wstring{ severityFormatter.Format(severity) };
}

void ShowForecast()
{
    winrt::PowerGridForecast gridForecast = winrt::PowerGridForecast::GetForecast();

    // If the API cannot obtain a forecast, the forecast is empty
    winrt::IVectorView<winrt::PowerGridData> forecast = gridForecast.Forecast();
    if (forecast.Size() > 0)
    {
        // Print some forecast general information.
        winrt::DateTime blockStartTime = gridForecast.StartTime();
        winrt::TimeSpan blockDuration = gridForecast.BlockDuration();

        std::wcout << L"Forecast start time:" << FormatDateTime(blockStartTime) << L"\n";
        std::wcout << L"Forecast block duration (minutes):" << std::chrono::round<std::chrono::minutes>(blockDuration).count() << L"\n";
        std::wcout << L"\n";

        // Print each entry in the forecast.
        for (winrt::PowerGridData const& data : forecast)
        {
            std::wcout << L"Date/Time: " << FormatDateTime(blockStartTime) <<
                L", Severity: " << FormatSeverity(data.Severity()) <<
                L", Is low user impact: " << (data.IsLowUserExperienceImpact() ? "Yes" : "No") << "\n";
            blockStartTime += blockDuration;
        }
    }
    else
    {
        std::wcout << L"No forecast available. Try again later.\n";
    }
    std::wcout << L"\n";
}


// Calculate the index of the forecast entry that contains the requested time.
// If the time is before the start of the forecast, then returns 0.
// If the time is past the end of the forecast, then returns the number of forecasts.
int GetForecastIndexContainingTime(winrt::PowerGridForecast const& gridForecast, winrt::DateTime time)
{
    winrt::TimeSpan blockDuration = gridForecast.BlockDuration();

    // Avoid division by zero.
    if (blockDuration.count() == 0)
    {
        return 0;
    }

    auto startBlock = static_cast<int>((time - gridForecast.StartTime()) / blockDuration);
    return std::clamp(startBlock, 0, static_cast<int>(gridForecast.Forecast().Size()));
}

void FindBest(winrt::TimeSpan lookAhead, bool restrictToLowUXImpact)
{
    winrt::PowerGridForecast gridForecast = winrt::PowerGridForecast::GetForecast();

    // Find the first and last blocks that include the time range we are
    // interested in.
    winrt::DateTime startTime = winrt::clock::now();
    winrt::DateTime endTime = startTime + lookAhead;

    int startBlock = GetForecastIndexContainingTime(gridForecast, startTime);
    int endBlock = GetForecastIndexContainingTime(gridForecast, endTime + gridForecast.BlockDuration());

    double lowestSeverity = (std::numeric_limits<double>::max)();
    winrt::DateTime timeWithLowestSeverity = (winrt::DateTime::max)();

    for (int index = startBlock; index < endBlock; ++index)
    {
        winrt::PowerGridData data = gridForecast.Forecast().GetAt(index);

        // If we are restricting to low impact, then use only low impact time periods.
        if (restrictToLowUXImpact && !data.IsLowUserExperienceImpact())
        {
            continue;
        }

        // If the severity is not an improvement, then don't use this one.
        double severity = data.Severity();
        if (severity >= lowestSeverity)
        {
            continue;
        }

        lowestSeverity = severity;
        timeWithLowestSeverity = gridForecast.StartTime() + index * gridForecast.BlockDuration();
    }

    // Print the results.
    if (lowestSeverity <= 1.0)
    {
        std::wcout <<
            FormatDateTime(timeWithLowestSeverity) <<
            L" to " <<
            FormatDateTime(timeWithLowestSeverity + gridForecast.BlockDuration()) <<
            L" (severity = " <<
            FormatSeverity(lowestSeverity) <<
            L")\n";
    }
    else
    {
        std::wcout << L"Unable to find a good time to do work\n";
    }
    std::wcout << L"\n";
}

void PerformForecastCalculations()
{
    // Show the entire forecast.
    ShowForecast();

    // Arbitrarily look ahead 10 hours with low user impact.
    std::wcout << L"Best time to do work in the next 10 hours with low user experience impact:\n";
    FindBest(std::chrono::hours(10), true);

    // Arbitrarily look ahead 10 hours with no regard for low user impact.
    std::wcout << L"Best time to do work in the next 10 hours without regard for user experience impact:\n";
    FindBest(std::chrono::hours(10), false);
}

int wmain(int /* argc */, wchar_t** /* argv */)
{
    winrt::init_apartment();

    // Do calculations immediately.
    PerformForecastCalculations();

    // If the forecast changes, then do the calculations again.
    auto token = winrt::PowerGridForecast::ForecastUpdated([](auto&&, auto&&)
        {
            PerformForecastCalculations();
        });


    // Wait until the user presses a key to exit.
    std::wcout << L"Waiting for the forecast to change...\n";
    std::wcout << L"Press Enter to exit the program.\n";
    std::cin.get();

    // Clean up.
    winrt::PowerGridForecast::ForecastUpdated(token); // unsubscribe from event
    winrt::uninit_apartment();


    return 0;
}
