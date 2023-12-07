
// Shell: 'GNU Ed' implementation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "shell.h"
#include "common.h"
#include "type.h"
#include "platform_conf.h"

const char shell_help_ed[] = "[options] [file]\n"
  "\nOptions:\n"
  "  -h, --help                 display this help and exit\n"
  "  -V, --version              output version information and exit\n"
  "  -G, --traditional          run in compatibility mode\n"
  "  -l, --loose-exit-status    exit with 0 status even if a command fails\n"
  "  -p, --prompt=STRING        use STRING as an interactive prompt\n"
  "  -r, --restricted           run in restricted mode\n"
  "  -s, --quiet, --silent      suppress diagnostics, byte counts and '!' prompt\n"
  "  -v, --verbose              be verbose; equivalent to the 'H' command\n"
  "\nStart edit by reading in 'file' if given.\n"
  "If 'file' begins with a '!', read output of shell command.\n"
  "\nExit status: 0 for a normal exit, 1 for environmental problems (file\n"
  "not found, invalid flags, I/O errors, etc), 2 to indicate a corrupt or\n"
  "invalid input file, 3 for an internal consistency error (eg, bug) which\n"
  "caused ed to panic.\n";

const char shell_help_summary_ed[] = "Edit files with GNU Ed";

void shell_ed( int argc, char **argv )
{
  ed_main( argc, argv );
  clearerr( stdin );
}
