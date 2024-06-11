#include <stdlib.h>

#define GRAY   "\033[90m"
#define GREEN  "\033[32m"
#define PURPLE "\033[35m"
#define RED    "\033[31m"
#define RESET  "\033[0m"
#define YELLOW "\033[93m"

typedef struct {
        char *name;
        char *description;
} command;

/*
 * taken from
 * https://gist.github.com/joshbuchea/6f47e86d2510bce28f8e7f42ae84c716
 */
static command commands[] = {
    {"feat", "new feature for the user, not a new feature for build script"},
    {"fix", "bug fix for the user, not a fix to a build script"},
    {"docs", "changes to the documentation"},
    {"style",
     "formatting, missing semi colons, etc; no production code change"},
    {"refactor", "refactoring production code, eg. renaming a variable"},
    {"test",
     "adding missing tests, refactoring tests; no production code change"},
    {"chore", "updating grunt tasks etc; no production code change"},
    {NULL, NULL}};

/*
 * Format used to print completion suggestions when tabbing
 */
static char *match_fmt = "\t" PURPLE "%s" RESET " - %s\n";
