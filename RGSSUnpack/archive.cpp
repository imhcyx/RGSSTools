#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "archive.h"
#include "util.h"

struct RGSSAD_SUB_INFO {
	RGSSAD_SUB_INFO* next;
	unsigned int FileNameSize;
	unsigned int Offset;
	unsigned int FileSize;
	unsigned int MagicKey;
	char FileName[256];
};

EXTRACT_RESULT extract_v1(char* arcfile, char* outpath, unsigned int magickey, bool writelog, PROGRESS_CALLBACK callback, int parameter) {
	FILE *farc, *flog, *fout;
	unsigned int i, leftsize;
	unsigned int filecount = 1, c = 0;
	char magic[8];
	char buf[1024];
	if (fopen_s(&farc, arcfile, "rb")) {
		return RESULT_IOERR;
	}
	fread(magic, 8, 1, farc);
	if (memcmp(magic, RGSSAD_V1_MAGIC, 8)) {
		fclose(farc);
		return RESULT_INVALID;
	}
	if (!SetCurrentDirectory(outpath)) {
		fclose(farc);
		return RESULT_IOERR;
	}
	if (fopen_s(&flog, (writelog ? "rgss_unpacker.log" : "nul"), "w")) {
		fclose(farc);
		return RESULT_IOERR;
	}
	fprintf(flog, "=== RGSS Unpacker Log ===\n");
	RGSSAD_SUB_INFO *sihead, *si;
	sihead = new RGSSAD_SUB_INFO;
	sihead->next = NULL;
	si = sihead;
	while (!feof(farc)) {
		if (fread(&si->FileNameSize, 4, 1, farc) != 1) break;
		si->FileNameSize ^= magickey;
		magickey = magickey * 7 + 3;
		if (fread(&si->FileName, si->FileNameSize, 1, farc) != 1) break;
		for (i = 0; i < si->FileNameSize; i++) {
			si->FileName[i] ^= magickey & 0xff;
			magickey = magickey * 7 + 3;
		}
		si->FileName[i] = 0;
		utf8_to_ansi(si->FileName, 256);
		if (fread(&si->FileSize, 4, 1, farc) != 1) break;
		si->FileSize ^= magickey;
		magickey = magickey * 7 + 3;
		si->MagicKey = magickey;
		si->Offset = ftell(farc);
		fseek(farc, si->FileSize, SEEK_CUR);
		si->next = new RGSSAD_SUB_INFO;
		si = si->next;
		si->next = NULL;
		filecount++;
	}
	fprintf(flog, "file count: %d\n", filecount);
	si = sihead;
	while (si->next != NULL) {
		callback(c * 100 / filecount, parameter);
		fprintf(flog, "%s\n", si->FileName);
		sprintf_s(buf, "%s\\%s", outpath, si->FileName);
		for (i = strlen(buf); buf[i] != '\\'; i--);
		buf[i] = '\0';
		mkdir_p(buf);
		if (fopen_s(&fout, si->FileName, "wb")) {
			fclose(flog);
			fclose(farc);
			return RESULT_IOERR;
		}
		leftsize = si->FileSize;
		magickey = si->MagicKey;
		fseek(farc, si->Offset, SEEK_SET);
		while (leftsize >= 1024) {
			fread(buf, 1024, 1, farc);
			leftsize -= 1024;
			for (i = 0; i < 1024; i += 4) {
				*(unsigned int*)(buf + i) ^= magickey;
				magickey = magickey * 7 + 3;
			}
			fwrite(buf, 1024, 1, fout);
		}
		if (leftsize > 0) {
			fread(buf, leftsize, 1, farc);
			for (i = 0; i < leftsize; i += 4) {
				*(unsigned int*)(buf + i) ^= magickey;
				magickey = magickey * 7 + 3;
			}
			fwrite(buf, leftsize, 1, fout);
		}
		fclose(fout);
		sihead = si->next;
		delete si;
		si = sihead;
		c++;
	}
	delete si;
	fprintf(flog, "done.\n");
	callback(100, parameter);
	fclose(flog);
	fclose(farc);
	return RESULT_OK;
}