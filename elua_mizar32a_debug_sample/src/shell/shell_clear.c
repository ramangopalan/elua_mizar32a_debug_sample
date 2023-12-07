// Shell: 'clear' implementation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "shell.h"
#include "platform_conf.h"
#include "type.h"
#include <sys/stat.h>
#include <sys/types.h>
#include "term.h"

const char shell_help_clear[] = "\n"
  "Clears the working screen.\n";
const char shell_help_summary_clear[] = "clear screen";

void shell_clear( int argc, char **argv )
{
  term_clrscr();
}

