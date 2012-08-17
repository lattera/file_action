/*
Copyright (c) 2012, Shawn Webb
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _FILE_ACTION_H
#define _FILE_ACTION_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>

typedef enum _action_return { CONTINUE=0, TERMINATE=1 } action_return;

typedef action_return (*action_callback)(FTSENT *, void *, void *);

typedef struct _file_action {
    char **paths;
    size_t path_count;

    action_callback *callbacks;
    size_t callback_count;

    FTS *fts;
    int options;
} FileAction;

FileAction *InitFileAction(void);
char *AddPath(FileAction *, char *);
void AddCallback(FileAction *, action_callback);
void RunAction(FileAction *, void *, void *);
void RunDefaultAction(char **, action_callback, void *, void *);
int IsOptionSet(FileAction *, int);
void ToggleOption(FileAction *, int);
void FreeFileAction(FileAction *);

#endif
