#include "config.h"
#include "git2.h"
#include <stdio.h>
#include <readline/readline.h>
#include <stdlib.h>
#include <string.h>

/* function prototypes */

static char **commit_type_completion(const char *text, int start, int end);
static char *match_gen(const char *text, int state);
static int confirm(void);
static int get_commit_title(char *title, int n);
static void display_matches(char **matches, int num_matches, int max_length);
static void check_lg2(int error);
static void get_commit_summary(char *summary, int n);
static void gen_commit_msg(char *commit, int n, const char *title,
                           const char *summary);
static void create_commit(const char *msg);
static void trim_trailing_whitespace(char *s, int end);

/* variables */

static unsigned int staged_mask =
    GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED |
    GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED |
    GIT_STATUS_INDEX_TYPECHANGE;

int
main(void)
{
        char title[72];
        char summary[500];
        char commit[sizeof(title) + sizeof(summary)];

        rl_attempted_completion_function = commit_type_completion;

        get_commit_title(title, sizeof(title));
        get_commit_summary(summary, sizeof(summary));

        if (!confirm()) {
                puts(RED "aborted" RESET);
                return EXIT_FAILURE;
        }

        gen_commit_msg(commit, sizeof(commit), title, summary);
        create_commit(commit);

        return EXIT_SUCCESS;
}

/* function implementations */

static void
create_commit(const char *msg)
{
        git_libgit2_init();

        int error, changes = 0;
        unsigned int i;

        git_oid commit_oid, tree_oid;
        git_tree *tree;
        git_index *index;
        git_signature *sig;
        git_status_list *status = NULL;
        git_object *parent = NULL;
        git_reference *ref = NULL;
        git_buf *out = NULL;
        git_repository *repo = NULL;

        check_lg2(git_repository_open_ext(
            &repo, ".", GIT_REPOSITORY_OPEN_NO_SEARCH, NULL));

        check_lg2(git_status_list_new(&status, repo, NULL));

        for (i = 0; i < git_status_list_entrycount(status); i++) {
                const git_status_entry *e = git_status_byindex(status, i);
                if (e->status & staged_mask) {
                        changes++;
                        break;
                }
        }

        if (!changes) {
                puts(YELLOW "Nothing to commit!" RESET);
                exit(EXIT_FAILURE);
        }

        if ((error = git_revparse_ext(&parent, &ref, repo, "HEAD")) < 0) {
                if (error == GIT_ENOTFOUND) {
                        puts("HEAD not found. Creating first commit");
                        error = 0;
                } else if (error < 0) {
                        check_lg2(error);
                }
        }

        check_lg2(git_repository_index(&index, repo));
        check_lg2(git_index_write_tree(&tree_oid, index));
        check_lg2(git_index_write(index));
        check_lg2(git_tree_lookup(&tree, repo, &tree_oid));
        check_lg2(git_signature_default(&sig, repo));

        git_message_prettify(out, msg, 0, 0);
        check_lg2(git_commit_create_v(&commit_oid, repo, "HEAD", sig, sig,
                                      NULL, msg, tree, parent ? 1 : 0,
                                      parent));

        printf(GREEN "Changes committed: %s\n" RESET, commit_oid.id);

        git_index_free(index);
        git_signature_free(sig);
        git_tree_free(tree);
        git_object_free(parent);
        git_reference_free(ref);
        git_buf_dispose(out);

        git_libgit2_shutdown();
}

static char **
commit_type_completion(const char *text, int __attribute__((unused)) start,
                       __attribute__((unused)) int end)
{
        rl_attempted_completion_over = 1;
        rl_completion_display_matches_hook = display_matches;
        return rl_completion_matches(text, match_gen);
}

static char *
match_gen(const char *text, int state)
{
        static int list_index, len;
        char *name;

        if (!state) {
                list_index = 0;
                len = strlen(text);
        }

        while ((name = commands[list_index++].name))
                if (strncmp(name, text, len) == 0) return strdup(name);

        return NULL;
}

static void
display_matches(char **matches, int num_matches,
                int __attribute__((unused)) max_length)
{
        int i;
        command *cmd;

        putchar('\n');

        for (i = 1; i <= num_matches; i++) {
                const char *m = matches[i];

                int j = 0;
                while ((cmd = &commands[j++])) {
                        if (strcmp(cmd->name, m) == 0) {
                                printf(match_fmt, m, cmd->description);
                                break;
                        }
                }
        }

        rl_on_new_line();
        rl_forced_update_display();
}

static int
confirm(void)
{
        char confirm;

        puts(GRAY "Proceed? (" GREEN "y" GRAY "/" RED "N" GRAY ")" RESET);
        confirm = getchar();

        switch (confirm) {
        case 'n': /* FALLTHROUGH */
        case 'N':
                return 0;
        case 'y': /* FALLTHROUGH */
        case 'Y': /* FALLTHROUGH */
        default:
                return 1;
        }
}

static int
get_commit_title(char *title, int n)
{
        int len = strlen(title);
        char buf[n - len];
        char *commit_type;
        int i = 0;

        commit_type = readline("Type of change: ");
        if (!commit_type) {
                puts("could not read type");
                return -1;
        }

        while (commit_type[i] != '\0') {
                title[i] = commit_type[i];
                i++;
        }
        free(commit_type);

        trim_trailing_whitespace(title, i);

        puts(GRAY "The title of your commit:" RESET);
        fputs(title, stdout);
        fgets(buf, (n - len), stdin);
        strncat(title, buf, (n - len - 1));

        return 1;
}

static void
trim_trailing_whitespace(char *s, int len)
{

        while (s[len - 1] == ' ') {
                len--;
        }
        s[len] = ':';
        s[++len] = ' ';
        s[++len] = '\0';
}

static void
get_commit_summary(char *summary, int n)
{
        puts(GRAY "Describe your changes:" RESET);
        fgets(summary, n, stdin);
}

static void
gen_commit_msg(char *commit, int n, const char *title, const char *summary)
{
        snprintf(commit, n, "%s\n%s\n", title, summary);
}

static void
check_lg2(int error)
{
        if (error < 0) {
                const git_error *e = git_error_last();
                printf(RED "Error: %d/%d: %s\n" RESET, error, e->klass,
                       e->message);
                exit(EXIT_FAILURE);
        }
}
