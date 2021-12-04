#include "stdafx.h"
#include "Header.h"
#include "stdio.h"



ulonglong EnumThreads(PROCESSCALLBACK pProcessCallback,
					  THREADCALLBACK pThreadCallback)
{
	ulong Size = 0x1000;
	void* p = VirtualAlloc(0,Size,MEM_COMMIT,PAGE_READWRITE);
	ulong ReturnedLength = 0;

	if(p)
	{
		int ret = ZwQuerySystemInformation(SystemProcessInformation,p,Size,&ReturnedLength);
		//printf("ZwQuerySystemInformation, ret: %X, ReturnedLength: %X\r\n",ret,ReturnedLength);


		if(ret == STATUS_INFO_LENGTH_MISMATCH)
		{
			VirtualFree(p,0,MEM_RELEASE);
			p = 0;
			Size  = ReturnedLength;
			p = VirtualAlloc(0,Size,MEM_COMMIT,PAGE_READWRITE);
			if(p)
			{
				ret = ZwQuerySystemInformation(SystemProcessInformation,p,Size,&ReturnedLength);
				//printf("ZwQuerySystemInformation, ret: %X, ReturnedLength: %X\r\n",ret,ReturnedLength);
			}
			else
			{
				printf("EnumThreads: Error allocating memory\r\n");
				return -1;
			}
		}



		_SYSTEM_PROCESS_INFORMATION* pSysInfo = (_SYSTEM_PROCESS_INFORMATION*)p;



		unsigned long Offset = 0;
		while(Offset < ReturnedLength)
		{
			//printf("Offset: %X\r\n",Offset);
			//printf("ReturnedLength: %X\r\n",ReturnedLength);

			if(pSysInfo->ImageName.Buffer)
			{
				wprintf(L"=> %s, pid: %I64X\r\n",pSysInfo->ImageName.Buffer,pSysInfo->UniqueProcessId);
			}

			if( (pSysInfo->UniqueProcessId))
			{
				if(pProcessCallback)
				{
					pProcessCallback(pSysInfo->UniqueProcessId);
				}

				ulonglong NumThreads = pSysInfo->NumberOfThreads;
				ulonglong iii = 0;
				while(iii < NumThreads)
				{
					_SYSTEM_THREAD_INFORMATION* pRunnerThread = (_SYSTEM_THREAD_INFORMATION*)(&pSysInfo->Threads[iii]);

					if(pRunnerThread->CID.UniqueThread)
					{
						if(pThreadCallback)
						{
							pThreadCallback(pRunnerThread->CID.UniqueThread);
						}
					}
					iii++;
				}
			}

			Offset = pSysInfo->NextEntryOffset;

			if(Offset >= ReturnedLength)
			{
				printf("Corrupt Offset within _SYSTEM_PROCESS_INFORMATION structure\r\n");
				return -2;
			}

			pSysInfo = (_SYSTEM_PROCESS_INFORMATION*) (   ((char*)pSysInfo) + Offset);
		
			//printf("Press any key to continue\r\n");
			//getchar();

			if(Offset == 0)
			{
				printf("Done\r\n");
				break;
			}
		}




		//printf("%X\r\n",SizeRead);
		//printf("%X\r\n",NumModules);
		//printf("%X\r\n",ReturnedLength);
		if(p)
		{
			VirtualFree(p,0,MEM_RELEASE);
			p = 0;
		}
		return 0;
	}
	return -1;
}