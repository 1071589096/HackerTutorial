// CreateRemoteThread_Test.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>


void ShowError(PCWSTR pszText)
{
	WCHAR szErr[MAX_PATH] = { 0 };
	::wsprintf(szErr, L"%s Error[%d]\n", pszText, ::GetLastError());
	::MessageBox(NULL, szErr, L"ERROR", MB_OK);
}


// ʹ�� CreateRemoteThread ʵ��Զ�߳�ע��
BOOL CreateRemoteThreadInjectDll(DWORD dwProcessId, PCWSTR pszDllFileName)
{
	if (dwProcessId == 0 || lstrlen(pszDllFileName) == 0)
	{
		ShowError(L"Invalid pid or dll file name\n");
		return FALSE;
	}
	HANDLE hProcess = NULL;
	SIZE_T dwSize = 0;
	LPVOID pDllAddr = NULL;
	FARPROC pFuncProcAddr = NULL;

	// ��ע����̣���ȡ���̾��
	hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	if (NULL == hProcess)
	{
		ShowError(L"OpenProcess");
		return FALSE;
	}
	// ��ע������������ڴ�
	dwSize = lstrlen(pszDllFileName) * 2;
	pDllAddr = ::VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	if (NULL == pDllAddr)
	{
		ShowError(L"VirtualAllocEx");
		return FALSE;
	}
	// ��������ڴ���д������
	if (FALSE == ::WriteProcessMemory(hProcess, pDllAddr, pszDllFileName, dwSize, NULL))
	{
		ShowError(L"WriteProcessMemory");
		return FALSE;
	}
	// ��ȡLoadLibraryW������ַ
	pFuncProcAddr = ::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
	if (NULL == pFuncProcAddr)
	{
		ShowError(L"GetProcAddress_LoadLibraryW");
		return FALSE;
	}
	// ʹ�� CreateRemoteThread ����Զ�߳�, ʵ�� DLL ע��
	HANDLE hRemoteThread = ::CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFuncProcAddr, pDllAddr, 0, NULL);
	if (NULL == hRemoteThread)
	{
		ShowError(L"CreateRemoteThread for LoadLibraryW");
		return FALSE;
	}
	
	WaitForSingleObject(hRemoteThread, INFINITE);
	// �رվ��
	::CloseHandle(hRemoteThread);
	::CloseHandle(hProcess);
	// ��ʾ�ɹ�
	printf("Զ���߳�ע��DLL�ɹ�,���޹�ע����ʦ���~\n");
	

	return TRUE;
}
BOOL CreateRemoteThreadUnInjectDll(DWORD dwProcessId, PCWSTR pszDllFileName) 
{
	if (dwProcessId == 0 || lstrlen(pszDllFileName) == 0)
	{
		ShowError(L"Invalid pid or dll file name\n");
		return FALSE;
	}
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
	MODULEENTRY32 me32;
	me32.dwSize = sizeof(me32);
	BOOL bRet = Module32First(hSnap, &me32);
	while (bRet) 
	{
		if (lstrcmp(me32.szExePath,pszDllFileName) == 0)
		{
			break;
		}
		bRet = Module32Next(hSnap, &me32);
	}
	CloseHandle(hSnap);
	HANDLE hProcess = NULL;
	FARPROC pFuncProcAddr = NULL;

	// ��ע����̣���ȡ���̾��
	hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	if (NULL == hProcess)
	{
		ShowError(L"OpenProcess");
		return FALSE;
	}
	// ��ȡFreeLibrary������ַ
	pFuncProcAddr = ::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "FreeLibrary");
	if (NULL == pFuncProcAddr)
	{
		ShowError(L"GetProcAddress_FreeLibrary");
		return FALSE;
	}
	// ʹ�� CreateRemoteThread ����Զ�߳�, ʵ�� DLL ж��
	HANDLE hRemoteThread = ::CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFuncProcAddr, me32.hModule, 0, NULL);
	if (NULL == hRemoteThread)
	{
		ShowError(L"CreateRemoteThread for FreeLibrary");
		return FALSE;
	}
	WaitForSingleObject(hRemoteThread, INFINITE);
	// �رվ��
	::CloseHandle(hRemoteThread);
	::CloseHandle(hProcess);
	printf("Զ���߳�ж��DLL�ɹ�,���޹�ע����ʦ���~\n");
}

DWORD GetPidByName(PCWSTR name)
{
	BOOL bRet;
	PROCESSENTRY32W pe;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe.dwSize = sizeof(pe);
	//WCHAR nameArr[MAX_PATH] = { 0 };
	//lstrcpy(nameArr, name);
	bRet = Process32FirstW(hSnapshot, &pe);
	while (bRet)
	{
		if (lstrcmp(pe.szExeFile,name)== 0)
		{
			printf("taskmgr pid:%d\n", pe.th32ProcessID);
			return pe.th32ProcessID;
		}

		bRet = Process32NextW(hSnapshot, &pe);
	}
	return 0;
}


int main(int argc, WCHAR* argv[])
{
	printf("��ʱ�㿴�ļ���\n");
	system("pause");
	//����Զ���߳̽��������ɵ���������DLLע�뵽�������������
	BOOL bRet = CreateRemoteThreadInjectDll(GetPidByName(L"Taskmgr.exe"), L"E:\\Users\\Ron\\source\\repos\\UseMinHook\\x64\\Release\\UseMinHook.dll");
	printf("��ʱ�㿴������\n");
	system("pause");
	if (bRet) 
	{
		//����Զ���߳̽����ǵ��������Ӵ������������ж��
		bRet = CreateRemoteThreadUnInjectDll(GetPidByName(L"Taskmgr.exe"), L"E:\\Users\\Ron\\source\\repos\\UseMinHook\\x64\\Release\\UseMinHook.dll");
		printf("��ʱ���ֿ��ļ���\n");
		system("pause");
	}
	
	return 0;
}

