#include "config.h"
#include "git2.h"
#include <stdio.h>
#include <readline/readline.h>
#include <stdlib.h>
#include <string.h>

/* function prototypes */

/* readline */
char **commit_type_completion(const char *, int, int);
char *match_gen(const char *, int);
void display_matches(char **matches, int num_matches, int max_length);
/* lg2 */
void check_lg2(int error, const char *message);
/* cig */
void get_commit_summary(char *summary, int n);
void gen_commit_msg(char *commit, int n, const char *title,
                    const char *summary);
int confirm(void);
int create_commit(const char *msg);
int get_commit_title(char *title, int n);

int
main(void)
{
        char title[72];
        char summary[500];
        char commit[sizeof(title) + sizeof(summary)];
        int error;

        git_libgit2_init();
        rl_attempted_completion_function = commit_type_completion;

        get_commit_title(title, sizeof(title));
        get_commit_summary(summary, sizeof(summary));

        if (!confirm()) {
            puts(RED"aborted"RESET);
            return 0;
        }

        gen_commit_msg(commit, sizeof(commit), title, summary);
        if ((error = create_commit(commit)) < 0) {
                const git_error *e = git_error_last();
                printf("Error: %d/%d: %s\n", error, e->klass, e->message);
                return -1;
        };
        puts(GREEN "Changes committed!" RESET);

        git_libgit2_shutdown();

        return 0;
}

/* function implementations */

int
create_commit(const char *msg)
{
        int error;

        git_oid commit_oid, tree_oid;
        git_tree *tree;
        git_index *index;
        git_signature *sig;
        git_object *parent = NULL;
        git_reference *ref = NULL;
        git_buf *out = NULL;
        git_repository *repo = NULL;

        if ((error = git_repository_open_ext(
                 &repo, ".", GIT_REPOSITORY_OPEN_NO_SEARCH, NULL)) < 0)
                return error;

        if ((error = git_revparse_ext(&parent, &ref, repo, "HEAD")) < 0) {
                if (error == GIT_ENOTFOUND) {
                        puts("HEAD not found. Creating first commit");
                        error = 0;
                } else if (error < 0) {
                        return error;
                }
        }

        if ((error = git_repository_index(&index, repo)) < 0) return error;

        if ((error = git_index_write_tree(&tree_oid, index)) < 0) return error;

        if ((error = git_index_write(index)) < 0) return error;
        if ((error = git_tree_lookup(&tree, repo, &tree_oid)) < 0)
                return error;
        if ((error = git_signature_default(&sig, repo)) < 0) return error;

        git_message_prettify(out, msg, 0, 0);

        if ((error =
                 git_commit_create_v(&commit_oid, repo, "HEAD", sig, sig, NULL,
                                     msg, tree, parent ? 1 : 0, parent)) < 0)
                return error;

        git_index_free(index);
        git_signature_free(sig);
        git_tree_free(tree);
        git_object_free(parent);
        git_reference_free(ref);
        git_buf_dispose(out);

        return error;
}

char **
commit_type_completion(const char *text, int __attribute__((unused)) start,
                       __attribute__((unused)) int end)
{
        rl_attempted_completion_over = 1;
        rl_completion_display_matches_hook = display_matches;
        return rl_completion_matches(text, match_gen);
}

char *
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

void
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

int
confirm(void)
{
        char confirm;

        puts(GRAY "Proceed? (" GREEN "y" GRAY "/" RED "N" GRAY ")" RESET);
        confirm = getchar();

        switch (confirm) {
        case 'n': /* FALLTHROUGH */
        case 'N': /* FALLTHROUGH */
                return 0;
        case 'y': /* FALLTHROUGH */
        case 'Y': /* FALLTHROUGH */
        default:
                return 1;
        }
}

int
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
        title[i] = ' ';
        title[--i] = ':';

        puts(GRAY "The title of your commit:" RESET);
        fputs(title, stdout);
        fgets(buf, (n - len), stdin);
        strncat(title, buf, (n - len - 1));

        return 1;
}

void
get_commit_summary(char *summary, int n)
{
        puts(GRAY "Describe your changes:" RESET);
        fgets(summary, n, stdin);
}

void
gen_commit_msg(char *commit, int n, const char *title, const char *summary)
{
        snprintf(commit, n, "%s\n%s\n", title, summary);
}
