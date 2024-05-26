#include <stdio.h>

void untabify_str(char *str, FILE *out)
{
    /* like fputs, but convert tabs to spaces */

    int col = 0;
    char *s;

    s = str;

    while (s[0]) {
        switch(s[0]) {
        case '\t':
            fputc(' ', out);
            col++;

            while (col%8) {
                fputc(' ', out);
                col++;
            }
            break;
        default:
            fputc(s[0], out);
            col++;
        }

        s++;
    }

    /* fputc('\n', out); /* newline at end */
}

void untabify_file(FILE *in, FILE *out)
{
    char str[80];

    /* very simple function to read strings with fgets and print
        them with untabify_str */

    while (fgets(str, 80, in)) {
        untabify_str(str, out);
    }
}

int main(int argc, char **argv)
{
    int i;
    FILE *pfile;

    /* untabify each file on the command line */

    for (i = 1; i < argc; i++) {
        pfile = fopen(argv[i], "r");

        if (pfile == NULL) {
            fputs("cannot open file: ", stderr);
            fputs(argv[i], stderr);
            fputc('\n', stderr);
        }
        else {
            untabify_file(pfile, stdout);
            fclose(pfile);
        }
    }

    /* if no files, read from stdin */

    if (argc == 1) {
        untabify_file(stdin, stdout);
    }

    return 0;
}
