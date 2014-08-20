/************************************************************************

		Copyright (c) 2003-2014 Brad Martin.

This file is part of the OpenSPC example program set.

OpenSPC is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

OpenSPC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with OpenSPC; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



ospcplay.c: Example, rudimentary SPC player.  Sound output goes to stdout,
which means a pipe is required in order to play the output.  Output format
is 16-bit stereo 32kHz raw PCM.  Can handle multiple files on the command
line: they will be played in shuffled order.

 ************************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <openspc.h>

#define BUFSIZE_1S (32000 * sizeof(short) * 2)  /* one second of audio */


static int GetIntArg(int argc, char** argv, int* index, const char* op) {
  char* endptr = NULL;
  int result;
  ++(*index);
  if (*index >= argc) {
    fprintf(stderr, "%s requires an argument!", op);
    exit(1);
  }
  result = strtol(argv[*index], &endptr, 0);
  if (endptr == argv[*index]) {
    fprintf(stderr, "Unable to parse %s argument: %s", op, argv[*index]);
    exit(1);
  }
  return result;
}


int main(int argc, char **argv) {
  int fd;
  int f;
  int* list;
  int first_file;
  int n_files;
  int randomize = 0;
  int limit_seconds = 0;
  off_t size;
  void *ptr;
  void *buf;
  char c;

  /* When this loop ends, first_file will point to the first non-option
     argument. */
  for (first_file = 1; first_file < argc; ++first_file) {
    if (!strcmp(argv[first_file], "-h") ||
        !strcmp(argv[first_file], "--help")) {
      fprintf(stderr, "Usage: %s [options] <file1> [file2...]\n",
              argv[0]);
      fprintf(stderr, " where [options] are any of the following:\n");
      fprintf(stderr, "  -r       Randomize song order\n");
      fprintf(stderr, "  -s SECS  End playback after this many seconds\n");
      return 0;
    } else if (!strcmp(argv[first_file], "-r")) {
      randomize = 1;
    } else if (!strcmp(argv[first_file], "-s")) {
      limit_seconds = GetIntArg(argc, argv, &first_file, "-s");
    } else {
      // First file.
      break;
    }
  }

  n_files = argc - first_file;
  if (n_files < 1) {
    fprintf(stderr, "Please specify at least one filename to play!\n");
    exit(1);
  }
  list = malloc(n_files * sizeof(int));
  for (f = 0; f < n_files; ++f) {
    list[f] = first_file + f;
  }
  if (randomize) {
    srand(time(NULL));
    for (f = 0; f < n_files; ++f) {
      int t = list[f];
      fd = rand() % n_files;
      list[f] = list[fd];
      list[fd] = t;
    }
  }
  buf = malloc(BUFSIZE_1S);
  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
  fprintf(stderr, "Press RETURN to change songs\n");
  for (f = 0; f < n_files; f++) {
    fprintf(stderr, "Now playing '%s'...\n", argv[list[f]]);
    fd = open(argv[list[f]], O_RDONLY);
    if (fd < 0) {
      fprintf(stderr, "\nUnable to open file '%s'!\n", argv[1]);
      continue;
    }
    size = lseek(fd, 0, SEEK_END);

    lseek(fd, 0, SEEK_SET);
    ptr = malloc(size);
    if (read(fd, ptr, size) < size) {
      fprintf(stderr, "\nError reading file %s!\n", argv[1]);
      continue;
    }

    close(fd);
    if ((fd = OSPC_Init(ptr, size))) {
      fprintf(stderr, "\nOSPC_Init returned %d!\n", fd);
      continue;
    }
    free(ptr);

    for (int elapsed_seconds = 0;
         (limit_seconds <= 0) || (elapsed_seconds < limit_seconds);
         ++elapsed_seconds) {
      if ((read(STDIN_FILENO, &c, 1) > 0) && (c == '\n')) { break; }
      size = OSPC_Run(-1, buf, BUFSIZE_1S);
      if (write(STDOUT_FILENO, buf, size) < size) {
        fprintf(stderr, "\nLost output data!\n");
      }
    }
  }

  return 0;
}
