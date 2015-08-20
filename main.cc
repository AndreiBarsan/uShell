// µShell (microShell, uShell, ush).  An experimental Unix shell implementation
// written in C++.
//
// Author: Andrei Barsan
// Project started in December 2014.
// License: TBD (will be free)

#include "util.h"
#include "shell.h"

// TODO(andre) This belongs in a specific shell component.
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

// This project uses Google's C++ Style Guide.
// See: http://google-styleguide.googlecode.com/svn/trunk/cppguide.html

/* Ideas and TODOS
 * ---------------
 *  - setup hotkeys for proper code navigation using YCM
 *  - auto-format code (vim + git hook)
 *  - fully qualified names for include guards (doesn't C++14 already have
 *  ``#pragma once''? plz.)
 *  - auto-header for every file (e.g. license, copyright, etc.)
 *  - everything should have a proper interface and a dedicated implementation
 *  - make sure that code handling shell debug traps, pipes, variable handling
 *  etc.  is as clean as possible
 *  - setup process groups correctly when e.g. forking
 *  - bison should work to generate the grammar like in the regular Bash
 *  - always bear in mind that plugins will have to be able to change anything
 *  - consider blogging about progress
 *  - document self about all variants of redirection in Bash and ensure that
 *  they are all supported
 *  - possible milestones:
 *    - manual (limited) redirection support (i.e. look for >foo >>foo tokens)
 *    - simple job control
 *      - async tasks (&)
 *      - jobs
 *      - bg
 *      - fg
 *      - wait
 *      - disown
 *      - suspend
 *      - kill
 *      - killall
 *      - etc. (http://tldp.org/LDP/abs/html/x9644.html)
 *    - **correct** job control (i.e. use proper group ids and signals)
 *      - ensure everything is wrapped in C++ (portability, modularity)
 *      - see: http://www.gnu.org/software/libc/manual/html_node/Implementing-a-Shell.html#Implementing-a-Shell
 *    - pipelines:
 *      - as above (`foo & bar`)
 *      - `foo; bar` (no pipes established)
 *      - `foo | bar`
 *      - mixed (`foo; bar | baz`)
 *    - figure out streamlined way to perform e2e testing
 */

void handler(int sig) {
  void *array[10];
  size_t size;

  size = backtrace(array, 10);

  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main(int argc, char **argv) {
  // Enable stack traces on crashes.
  signal(SIGSEGV, handler);

  microshell::core::Shell *shell = microshell::core::Shell::initialize(util::argv_to_strvec(argc, argv));
  return shell->interactive();
}
