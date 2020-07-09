#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <string>
#include <tlhelp32.h>
#include <comdef.h>


using namespace std;

HANDLE FindHandle(PWSTR procName) {

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (snapshot == INVALID_HANDLE_VALUE) {
		cout << "Snapshot Failed! Error Code: " << GetLastError() << endl;
		return NULL;
	}

	if (Process32First(snapshot, &entry) == TRUE) {
		while (Process32Next(snapshot, &entry) == TRUE) {

			if (_wcsicmp(entry.szExeFile, procName) == 0)
			{
				cout << "Bulundu!" << endl;
				HANDLE ProcHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

				if (ProcHandle == NULL)
				{
					cout << "ProcHandle Failed! Error Code: " << GetLastError() << endl;
					return NULL;
				}

				return ProcHandle;

			}
		}
	}

	return NULL;


}

// Hedef process ve enjekte edilecek DLL'in tam yolunu argüman olarak alacağız. 
int wmain(int argc, wchar_t* argv[]) {



	if (argc == 3) {

		HANDLE retHandle = NULL;
		PVOID buffer;
		PTHREAD_START_ROUTINE loadlibrary;
		_bstr_t b(argv[2]);
		const char* dllpath = b;
		bool result;

		while (retHandle == NULL) {
			retHandle = FindHandle(argv[1]);
			wcout << "Waiting open the " << argv[1] << " !" << endl;
			Sleep(2000);
		}


		if (retHandle == NULL) {
			cout << "Invalid handle! " << endl;
			return -1;
		}

		cout << "Allocating memory!" << endl;
		buffer = VirtualAllocEx(retHandle, NULL, strlen(dllpath), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (buffer == NULL) {
			cout << "Error allocate memory! " << GetLastError() << endl;
			return -1;
		}
		cout << "Writing memory!" << endl;
		result = WriteProcessMemory(retHandle, buffer, dllpath, strlen(dllpath), NULL);
		if (!result) {
			cout << "Error writing memory!" << GetLastError() << endl;
			return -1;
		}

		loadlibrary = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
		if (loadlibrary == NULL) {
			cout << "Error GetProcAddress!" << GetLastError() << endl;
		}

		if ((CreateRemoteThread(retHandle, NULL, 0, loadlibrary, buffer, 0, NULL)) == NULL) {
			cout << "Error CreateRemoteThread!" << GetLastError() << endl;
			return -1;
		}

		CloseHandle(retHandle);

		cout << "All operations success!" << endl;

		return 0;
	}

	else {
		wcout << "USAGE: " << argv[0] << " [target.exe] [source.dll PATH]";
		return -1;
	}



}
