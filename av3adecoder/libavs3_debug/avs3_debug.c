#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "avs3_debug.h"

static FILE *in_fileptr[N_FILEPTR];
static FILE *out_fileptr[N_FILEPTR];

static char *in_filename[N_FILEPTR];
static char *out_filename[N_FILEPTR];

static int in_count = 0;
static int out_count = 0;

static int flag_count = 0;
static char *flag_name[N_DBGFLAG];
static int  val_count = 0;
static char *val_name[N_DBGVAL];
static char *val[N_DBGVAL];

static char *type_list[N_TYPES] = { "char", "short", "int", "long", "float", "double" };

static void setvalue(
    const char *value_name, /* i: Value name         */
    const char *value       /* i: Value as string    */
);

int lookup(const char *const str, const char *const *const list, const int n_elem)
{
    int i, result;

    result = -1;
    i = 0;
    while (i < n_elem && result == -1)
    {
        if (strcmp(str, list[i]) == 0)
        {
            result = i;
        }
        i++;
    }

    return result;
}

int dbgwrite(
    const void *const buffer,    /* i: Write buffer */
    const int size,              /* i: Element size */
    const int count,             /* i: Number of elements */
    const int repeat,            /* i: Number of times the elements are repeated */
    const char *const filename
)
{
    int index, i;
    void *tmp_buf;

    index = lookup(filename, (const char *const *)out_filename, out_count);

    if (index == -1)
    {
        index = out_count;
        out_fileptr[index] = fopen(filename, "wb");
        out_filename[index] = malloc(sizeof(char) * (strlen(filename) + 1));
        strcpy(out_filename[index], filename);
        out_count++;
    }

    if (out_fileptr[index] != NULL)
    {
        tmp_buf = calloc(count*repeat, size);
        if (buffer != 0)
        {
            for (i = 0; i < repeat; i++)
            {
                memcpy((char*)tmp_buf + i * size*count, buffer, size*count);
            }
        }
#ifdef DEBUG_MODE_INFO
#ifdef DEBUG_MODE_INFO_TWEAK
        if (1 == textmode && 2 == size) {  /* currently the textmode is only defined with "short" as input */
            fprintf(out_fileptr[index], "%d\n", x);
        }
        else {
#endif
#endif
            fwrite(tmp_buf, size*count*repeat, 1, out_fileptr[index]);
#ifdef DEBUG_MODE_INFO
#ifdef DEBUG_MODE_INFO_TWEAK
    }
#endif
#endif
        free(tmp_buf);
    }
    else
    {
        fprintf(stderr, "dbgwrite: Could not write to file: %s. Exiting..\n", filename);
        exit(-1);
    }

    return 0;
}

static void setvalue(const char *value_name, const char *value)
{
    int result;

    result = lookup(value_name, (const char *const *)val_name, val_count);

    if (result == -1)
    {
        val_name[val_count] = malloc(sizeof(char) * (strlen(value_name) + 1));
        strcpy(val_name[val_count], value_name);
        val[val_count] = malloc(sizeof(char) * (strlen(value) + 1));
        strcpy(val[val_count], value);
        val_count++;
        fprintf(stdout, "\nDebug value set: %s = %s\n", value_name, value);
    }
    else
    {
        fprintf(stdout, "\n*** Value %s already set: %s\n", value_name, val[result]);
    }
}