#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "file_action.h"

FileAction *InitFileAction() {
    FileAction *fa;

    fa = calloc(1, sizeof(FileAction));
    if (!(fa))
        return NULL;

    fa->options = FTS_NOCHDIR;

    return fa;
}

char *AddPath(FileAction *fa, char *path) {
    char **newpaths;

    if (!(fa) || !(path))
        return NULL;

    newpaths = realloc(fa->paths, (fa->path_count+1) * sizeof(char **));
    if (!(newpaths))
        return NULL;

    fa->paths = newpaths;
    fa->paths[fa->path_count] = strdup(path);
    fa->path_count++;

    return fa->paths[fa->path_count-1];
}

void RunAction(FileAction *fa, action_callback callback, void *callback_arguments) {
    FTSENT *p;
    char **newpaths;

    if (!(fa) || !(callback))
        return;

    /* Either FTS_LOGICAL or FTS_PHYSICAL needs to be set, but not both */
    if (!(fa->options & (FTS_LOGICAL | FTS_PHYSICAL)))
        return;

    if (fa->options & (FTS_LOGICAL | FTS_PHYSICAL) == (FTS_LOGICAL | FTS_PHYSICAL))
        return;

    /* NULL-terminate list if not already NULL-terminated */
    if (fa->paths[fa->path_count-1]) {
        newpaths = realloc(fa->paths, (fa->path_count+1) * sizeof(char **));
        if (!(newpaths))
            return;

        fa->paths = newpaths;
        fa->paths[fa->path_count] = NULL;
    }

    fa->fts = fts_open(fa->paths, fa->options, NULL);
    if (!(fa->fts))
        return;

    while ((p = fts_read(fa->fts))) {
        if (fa->options & FTS_LOGICAL && p->fts_info & FTS_DP)
            continue;
        else if (!(fa->options & FTS_LOGICAL) && p->fts_info & FTS_D)
            continue;

        switch (callback(strdup(p->fts_path), callback_arguments)) {
            case TERMINATE:
                fts_close(fa->fts);
                return;
        }
    }

    fts_close(fa->fts);
}

void ToggleOption(FileAction *fa, int option) {
    if (option & FTS_LOGICAL && fa->options & FTS_PHYSICAL)
        fa->options ^= FTS_PHYSICAL;
    else if (option & FTS_PHYSICAL && fa->options & FTS_LOGICAL)
        fa->options ^= FTS_LOGICAL;

    fa->options ^= option;
}

void FreeFileAction(FileAction *fa) {
    size_t i;

    if (!(fa))
        return;

    if ((fa->paths)) {
        for (i=0; i<=fa->path_count; i++)
            if (fa->paths[i])
                free(fa->paths[i]); /* Handle non-NULL-terminated arrays */

        free(fa->paths);
    }

    free(fa);
}

#if defined(TESTLIB)

action_return test_callback(char *path, void *args) {
    printf("callback: %s\n", path);
    remove(path);

    free(path);
    return CONTINUE;
}

int main(int argc, char *argv[]) {
    FileAction *fa;
    unsigned long i;

    fa = InitFileAction();
    ToggleOption(fa, FTS_PHYSICAL);

    for (i=1; i<argc; i++)
        AddPath(fa, argv[i]);

    RunAction(fa, test_callback, NULL);
    FreeFileAction(fa);

    return 0;
}

#endif
