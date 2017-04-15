#include <windows.h>
#include <Shlobj.h>
#include <stdio.h>

#include "resource.h"
#include "archive.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

void ProgressCallback(int percent, int parameter) {
	SendDlgItemMessage((HWND)parameter, IDC_PRG, PBM_SETPOS, percent, NULL);
}

DWORD WINAPI ThreadProc(
	_In_ LPVOID lpParameter
)
{
	char arcpath[256], outpath[256];
	char str[9];
	unsigned int magickey;
	bool writelog;
	HWND hwndDlg = (HWND)lpParameter;
	GetDlgItemText(hwndDlg, IDC_EDITARCPATH, arcpath, 256);
	GetDlgItemText(hwndDlg, IDC_EDITOUTPATH, outpath, 256);
	GetDlgItemText(hwndDlg, IDC_EDITMK, str, 9);
	sscanf_s(str, "%X", &magickey);
	writelog = SendDlgItemMessage(hwndDlg, IDC_CHKLOG, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
	switch (extract_v1(arcpath, outpath, magickey, writelog, ProgressCallback, (int)hwndDlg)) {
	case RESULT_OK:
		MessageBox(hwndDlg, "OK!", NULL, NULL);
		break;
	case RESULT_IOERR:
		MessageBox(hwndDlg, "An I/O error has occurred. Check to make sure the file and path are accessible.", NULL, NULL);
		break;
	case RESULT_INVALID:
		MessageBox(hwndDlg, "Invalid archive format", NULL, NULL);
		break;
	}
	ShowWindow(GetDlgItem(hwndDlg, IDC_PRG), 0);
	EnableWindow(GetDlgItem(hwndDlg, IDC_EDITARCPATH), 1);
	EnableWindow(GetDlgItem(hwndDlg, IDC_EDITOUTPATH), 1);
	EnableWindow(GetDlgItem(hwndDlg, IDC_EDITMK), 1);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHKLOG), 1);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BTNARCPATH), 1);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BTNOUTPATH), 1);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BTNUNPACK), 1);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BTNEXIT), 1);
	return 0;
}

INT_PTR CALLBACK DialogProc(
	_In_ HWND   hwndDlg,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		SetDlgItemText(hwndDlg, IDC_EDITMK, "DEADCAFE");
		SendDlgItemMessage(hwndDlg, IDC_CHKLOG, BM_SETCHECK, BST_CHECKED, NULL);
		ShowWindow(GetDlgItem(hwndDlg, IDC_PRG), 0);
		return TRUE;
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return TRUE;
	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			switch (LOWORD(wParam)) {
			case IDC_BTNARCPATH:
				char szPathName[MAX_PATH];
				OPENFILENAME ofn;
				memset(szPathName, 0, sizeof(szPathName));
				memset(&ofn, 0, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hwndDlg;
				ofn.lpstrFilter = "RGSS Archive Format v1(*.rgssad;*.rgss2a)\0*.rgssad;*.rgss2a\0\0";
				ofn.lpstrFile = szPathName;
				ofn.nMaxFile = sizeof(szPathName);
				ofn.lpstrTitle = "Select the archive file:";
				ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
				if (GetOpenFileName(&ofn)) {
					int i;
					SetDlgItemText(hwndDlg, IDC_EDITARCPATH, szPathName);
					for (i = sizeof(szPathName); szPathName[i] != '\\'; i--);
					szPathName[i] = '\0';
					SetDlgItemText(hwndDlg, IDC_EDITOUTPATH, szPathName);
				}
				break;
			case IDC_BTNOUTPATH:
				BROWSEINFO bi;
				char FileName[MAX_PATH];
				LPITEMIDLIST idl;
				memset(&bi, 0, sizeof(bi));
				bi.hwndOwner = hwndDlg;
				bi.pszDisplayName = FileName;
				bi.lpszTitle = "Select the output path:";
				bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
				idl = SHBrowseForFolder(&bi);
				if (idl != NULL) {
					SHGetPathFromIDList(idl, FileName);
					SetDlgItemText(hwndDlg, IDC_EDITOUTPATH, FileName);
				}
				break;
			case IDC_BTNUNPACK:
				ShowWindow(GetDlgItem(hwndDlg, IDC_PRG), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDITARCPATH), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDITOUTPATH), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDITMK), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHKLOG), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTNARCPATH), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTNOUTPATH), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTNUNPACK), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTNEXIT), 0);
				CloseHandle(CreateThread(NULL, 0, ThreadProc, (LPVOID)hwndDlg, 0, NULL));
				break;
			case IDC_BTNEXIT:
				EndDialog(hwndDlg, 0);
				break;
			}
			break;
		case EN_CHANGE:
			if (LOWORD(wParam) == IDC_EDITMK) {
				char str[9];
				unsigned int mk = 0;
				GetDlgItemText(hwndDlg, IDC_EDITMK, str, 9);
				sscanf_s(str, "%X", &mk);
				sprintf_s(str, "%X", mk);
				SetDlgItemText(hwndDlg, IDC_EDITMK, str);
				SendDlgItemMessage(hwndDlg, IDC_EDITMK, EM_SETSEL, 8, 8);
			}
			break;
		}
		return TRUE;
	default:
		return FALSE;
	}
}

int _stdcall WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc);
}