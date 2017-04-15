#pragma once

int const RESULT_OK = 0;
int const RESULT_IOERR = 1;
int const RESULT_INVALID = 2;

typedef void (*PROGRESS_CALLBACK)(int percent, int parameter);

// RGSSAD v1

const char RGSSAD_V1_MAGIC[8] = { 'R','G','S','S','A','D',0,1 };

int extract_v1(char* arcfile, char* outpath, unsigned int magickey, bool writelog, PROGRESS_CALLBACK callback, int parameter);