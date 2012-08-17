/*
Copyright (c) 2012, Shawn Webb
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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

void AddCallback(FileAction *fa, action_callback callback) {
    action_callback *newcallbacks;

    if (!(fa))
        return;

    newcallbacks = realloc(fa->callbacks, (fa->callback_count+1) * sizeof(action_callback *));
    if (!(newcallbacks))
        return;

    fa->callbacks = newcallbacks;
    fa->callbacks[fa->callback_count] = callback;
    fa->callback_count++;
}

void RunAction(FileAction *fa, void *callback_arguments, void *output) {
    FTSENT *p;
    char **newpaths;
    size_t i;

    if (!(fa) || fa->callback_count == 0 || fa->path_count == 0)
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

        for (i=0; i < fa->callback_count; i++) {
            if (!(fa->callbacks[i]))
                continue;

            switch (fa->callbacks[i](p, callback_arguments, output)) {
                case TERMINATE:
                    fts_close(fa->fts);
                    return;
            }
        }
    }

    fts_close(fa->fts);
}

void RunDefaultAction(char **paths, action_callback callback, void *arguments, void *output) {
    FileAction *fa;
    unsigned long i;
    
    fa = InitFileAction();
    ToggleOption(fa, FTS_PHYSICAL);

    for (i=0; paths[i]; i++)
        AddPath(fa, paths[i]);

    AddCallback(fa, callback);
    RunAction(fa, arguments, output);

    FreeFileAction(fa);
}

int IsOptionSet(FileAction *fa, int option) {
    if (!(fa))
        return 0;

    return (fa->options & option) == option;
}

void ToggleOption(FileAction *fa, int option) {
    /* Only one of these must be set at a time, not both (FTS_PHYSICAL, FTS_LOGICAL) */
    if (option & FTS_LOGICAL && IsOptionSet(fa, FTS_PHYSICAL))
        fa->options ^= FTS_PHYSICAL;
    else if (option & FTS_PHYSICAL && IsOptionSet(fa, FTS_LOGICAL))
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

    if ((fa->callbacks))
        free(fa->callbacks);

    free(fa);
}

#if defined(TESTLIB)

action_return test_callback(FTSENT *ent, void *args, void *output) {
    printf("callback: %s\n", ent->fts_path);

    return CONTINUE;
}

int main(int argc, char **argv) {
    RunDefaultAction(&argv[1], test_callback, NULL, NULL);

    return 0;
}

#endif
