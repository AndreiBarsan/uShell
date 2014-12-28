// µShell (microShell, uShell, ush).  An experimental Unix shell implementation
// written in C++.
//
// Author: Andrei Barsan
// Project started in December 2014.
// License: TBD (will be free)

#include "util.h"
#include "shell.h"

// This project uses Google's C++ Style Guide.
// See: http://google-styleguide.googlecode.com/svn/trunk/cppguide.html

/* Ideas and TODOS
 * ---------------
 *  - setup hotkeys for proper code navigation using YCM
 *  - vim print margin
 *  - auto-format code
 *  - auto-header for every file
 *  - everything should have a proper interface and a dedicated implementation
 *  - make sure that code handling shell debug traps, pipes, variable handling etc.
 *  is as clean as possible
 *  - yacc should work to generate the grammar like in the regular Bash
 *  - always bear in mind that plugins will have to be able to change anything
 *  - consider blogging about progress
 *  - document self about all variants of redirection in Bash and ensure that they
 *    are all supported
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

int main(int argc, char **argv) {
    microshell::core::Shell shell(util::argv_to_strvec(argc, argv));
    int result = shell.interactive();
    return result;
}
