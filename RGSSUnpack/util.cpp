#include "util.h"
#include <windows.h>

void utf8_to_ansi(char* buf, int size) {
	wchar_t* wbuf = new wchar_t[size];
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, 256);
	WideCharToMultiByte(CP_ACP, 0, wbuf, -1, buf, 256, NULL, NULL);
	delete[] wbuf;
}

bool mkdir_p(char* path) {
	int i = 0;
	while (true) {
		for (; path[i] != '\\' && path[i] != 0; i++);
		if (path[i] == '\\') {
			path[i] = 0;
			if (!CreateDirectory(path, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) return false;
			path[i] = '\\';
			i++;
		}
		else {
			if (!CreateDirectory(path, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) return false;
			break;
		}
	}
	return true;
}