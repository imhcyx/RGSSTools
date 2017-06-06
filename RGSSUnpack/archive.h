#pragma once

enum EXTRACT_RESULT {
	RESULT_OK = 0,
	RESULT_IOERR,
	RESULT_INVALID
};

typedef void (*PROGRESS_CALLBACK)(int percent, int parameter);

// RGSSAD v1

const char RGSSAD_V1_MAGIC[8] = { 'R','G','S','S','A','D',0,1 };

EXTRACT_RESULT extract_v1(char* arcfile, char* outpath, unsigned int magickey, bool writelog, PROGRESS_CALLBACK callback, int parameter);