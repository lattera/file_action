#ifndef _FILE_ACTION_H
#define _FILE_ACTION_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>

typedef enum _action_return { CONTINUE=0, TERMINATE=1 } action_return;

typedef action_return (*action_callback)(char *, void *);

typedef struct _file_action {
    char **paths;
    size_t path_count;

    FTS *fts;
    int options;
} FileAction;

FileAction *InitFileAction(void);
char *AddPath(FileAction *, char *);
void RunAction(FileAction *, action_callback, void *);
int IsOptionSet(FileAction *, int);
void ToggleOption(FileAction *, int);
void FreeFileAction(FileAction *);

#endif
