@echo off

set dst=%windir%\system32\inetsrv\iisadmin\isadmin

echo Copying Indexing Service administration sample files to %dst%

md %dst% 2>nul 1>nul

copy admin.htm %dst% 2>nul 1>nul
copy is2admin.css %dst% 2>nul 1>nul
copy is2logo.gif %dst% 2>nul 1>nul
copy is2side.gif %dst% 2>nul 1>nul
copy merge.ida %dst% 2>nul 1>nul
copy navbara.htm %dst% 2>nul 1>nul
copy scan.htx %dst% 2>nul 1>nul
copy scan.ida %dst% 2>nul 1>nul
copy scan.idq %dst% 2>nul 1>nul
copy state.htx %dst% 2>nul 1>nul
copy state.ida %dst% 2>nul 1>nul
copy unfilt.htx %dst% 2>nul 1>nul
copy unfilt.idq %dst% 2>nul 1>nul


