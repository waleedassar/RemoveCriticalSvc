copy RemoveCriticalSvc.dll %SystemRoot%\System32\RemoveCriticalSvc.dll

sc create RemoveCriticalSvc binPath= ^%%SystemRoot^%%"\system32\svchost.exe -k mygroup" type= share start= demand
reg add HKLM\SYSTEM\CurrentControlSet\services\RemoveCriticalSvc\Parameters /v ServiceDll /t REG_EXPAND_SZ /d ^%%SystemRoot^%%\System32\RemoveCriticalSvc.dll /f
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Svchost" /v mygroup /t REG_MULTI_SZ /d RemoveCriticalSvc /f