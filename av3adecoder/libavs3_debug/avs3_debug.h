#ifndef AVS3_DEBUG_H
#define AVS3_DEBUG_H

#define N_FILEPTR           250
#define N_DBGFLAG           100
#define N_DBGVAL            100
#define N_TYPES             6

int lookup(const char *const str, const char *const *const list, const int n_elem);

int dbgwrite(const void *const buffer, const int size, const int count, const int repeat, const char *const filename);
#endif