@ECHO OFF

REM Create the necessary folder structures
if "%1" == "" (
    ECHO Usage: DEPLOY destination [fully qualified path; no spaces or quoted]
    EXIT /b 1
)

    SET DestPath=%1
    md %DestPath%


REM Display the copy source / destination information
    ECHO.
    ECHO Copy files from:
    ECHO %~d0%~p0*.*
    ECHO To:
    ECHO %DestPath%
    ECHO.


REM Install primary components.
    copy "%~d0%~p0*.*" "%DestPath%"

    if ERRORLEVEL 1 goto ErrorExit
    GoTo NormalExit


:ErrorExit
    ECHO Setup incurred an error. Terminating!
:NormalExit
    ECHO. Subscription Sample set up successfully.
