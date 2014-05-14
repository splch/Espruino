/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Contains built-in functions for SD card access
 * ----------------------------------------------------------------------------
 */
#ifndef LINUX
#include "ff.h" // filesystem stuff
typedef FIL File_Handle;
#define JS_DIR_BUF_SIZE 64
#else
#include <stdio.h>
#include <dirent.h> // for readdir
typedef FILE* File_Handle;
typedef int FRESULT;
#define FR_OK (0)
#define FR_DISK_ERR (1)
#define FR_INVALID_NAME (6)
#define JS_DIR_BUF_SIZE 256
#endif

#include "jsutils.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jsinteractive.h"

typedef enum {
  FT_NONE=0,
  FT_FILE,
  FT_STREAM
} FileType;

typedef enum {
  FM_NONE=0,
  FM_READ,
  FM_WRITE,
  FM_READ_WRITE
} FileMode;

typedef enum {
  FS_NONE=0,
  FS_OPEN,
  FS_CLOSED
} FileState;

typedef struct {
  File_Handle handle;
  FileType type;
  FileMode mode;
  FileState state;
} PACKED_FLAGS JsFileData;

typedef struct JsFile {
  JsVar* fileVar; // this won't be locked again - we just know that it is already locked by something else
  JsFileData data;
  unsigned char _blank; //< this is needed as jsvGetString for 'data' wants to add a trailing zero
  size_t (*read)(struct JsFile* file, JsVar* buffer, int length, int position, FRESULT* res);
  size_t (*write)(struct JsFile* file, JsVar* buffer, int length, int position, FRESULT* res);
  void (*close)(struct JsFile* file);
} PACKED_FLAGS JsFile;

// Called when stopping, to make sure all files are closed
void jswrap_file_kill();

JsVar* jswrap_file_constructor(JsVar* path, JsVar* mode);
size_t jswrap_file_read(JsVar* parent, JsVar* buffer, int length, int position, JsVar* callback);
size_t jswrap_file_write(JsVar* parent, JsVar* buffer, int length, int position, JsVar* callback);
JsVar* jswrap_file_pipe(JsVar* parent, JsVar* destfd, JsVar* chunkSize, JsVar* callback);
void jswrap_file_close(JsVar* parent);

//var r = fs.createReadStream('file.txt');
JsVar* jswrap_fs_createReadStream(JsVar* path);
//var w = fs.createWriteStream('file.txt');
JsVar* jswrap_fs_createWriteStream(JsVar* path);
