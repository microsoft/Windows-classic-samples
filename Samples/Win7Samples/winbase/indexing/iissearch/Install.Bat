@echo off

set dst=%systemdrive%\inetpub\iissamples\issamples

echo Copying Indexing Service query sample files to %dst%

md %dst% 2>nul 1>nul

copy adovbs.inc %dst% 2>nul 1>nul
copy advquery.asp %dst% 2>nul 1>nul
copy advsqlq.asp %dst% 2>nul 1>nul
copy default.htm %dst% 2>nul 1>nul
copy deferror.htx %dst% 2>nul 1>nul
copy fastq.htm %dst% 2>nul 1>nul
copy fastq.htx %dst% 2>nul 1>nul
copy fastq.idq %dst% 2>nul 1>nul
copy hilight.gif %dst% 2>nul 1>nul
copy htxerror.htx %dst% 2>nul 1>nul
copy idqerror.htx %dst% 2>nul 1>nul
copy ie.gif %dst% 2>nul 1>nul
copy is2bkgnd.gif %dst% 2>nul 1>nul
copy is2foot.inc %dst% 2>nul 1>nul
copy is2logo.gif %dst% 2>nul 1>nul
copy is2style.css %dst% 2>nul 1>nul
copy ixqlang.htm %dst% 2>nul 1>nul
copy ixtiphlp.htm %dst% 2>nul 1>nul
copy ixtipsql.htm %dst% 2>nul 1>nul
copy navbar.htm %dst% 2>nul 1>nul
copy qfullhit.htw %dst% 2>nul 1>nul
copy qsumrhit.htw %dst% 2>nul 1>nul
copy query.asp %dst% 2>nul 1>nul
copy query.htm %dst% 2>nul 1>nul
copy query.htx %dst% 2>nul 1>nul
copy query.idq %dst% 2>nul 1>nul
copy rankbtn1.gif %dst% 2>nul 1>nul
copy rankbtn2.gif %dst% 2>nul 1>nul
copy rankbtn3.gif %dst% 2>nul 1>nul
copy rankbtn4.gif %dst% 2>nul 1>nul
copy rankbtn5.gif %dst% 2>nul 1>nul
copy reserror.htx %dst% 2>nul 1>nul
copy win2k.gif %dst% 2>nul 1>nul

