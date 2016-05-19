//------------------------------------
//  SHARED.VBS
//
//  Shared VBScript Library. 2000-2001
//
//  Version 1.00.003
//------------------------------------

Function CurrentShortTime
  CurrentShortTime = FormatDateTime(Time, 3)
End Function

Function CurrentShortDate
  CurrentShortDate = FormatDateTime(Date)
End Function

Function CurrentLongWeekdayName
  CurrentLongWeekdayName = WeekdayName(Weekday(Date))
End Function

Function CurrentShortWeekdayName
  CurrentShortWeekdayName = WeekDayName(Weekday(Date), True)
End Function

Function LongMonthName(MonthNumber)
  LongMonthName = MonthName(MonthNumber)
End Function

Function FormatFixedDigits(Number, DigitsAfterDot)
  FormatFixedDigits = FormatNumber(Number, DigitsAfterDot, True, False, False)
End Function

Function ParseDouble(TextString)
  ParseDouble = CDbl(TextString)
End Function
