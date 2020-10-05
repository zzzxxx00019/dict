#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bench.h"
#include "bloom.h"

#define DICT_FILE "cities.txt"
#define WORDMAX 256
#define PREFIX_LEN 3

double tvgetf()
{
    struct timespec ts;
    double sec;

    clock_gettime(CLOCK_REALTIME, &ts);
    sec = ts.tv_nsec;
    sec /= 1e9;
    sec += ts.tv_sec;

    return sec;
}

int bench_test(const tst_node *root, char *out_file, const int max)
{
    char prefix[PREFIX_LEN + 1] = "";
    char word[WORDMAX] = "";
    char **sgl;
    FILE *fp = fopen(out_file, "w");
    FILE *dict = fopen(DICT_FILE, "r");
    int idx = 0, sidx = 0;
    double t1, t2;

    if (!fp || !dict) {
        if (fp) {
            fprintf(stderr, "error: file open failed in '%s'.\n", DICT_FILE);
            fclose(fp);
        }
        if (dict) {
            fprintf(stderr, "error: file open failed in '%s'.\n", out_file);
            fclose(dict);
        }
        return 1;
    }

    sgl = (char **) malloc(sizeof(char *) * max);
    while (fscanf(dict, "%s", word) != EOF) {
        if (strlen(word) < sizeof(prefix) - 1)
            continue;
        strncpy(prefix, word, sizeof(prefix) - 1);
        t1 = tvgetf();
        tst_search_prefix(root, prefix, sgl, &sidx, max);
        t2 = tvgetf();
        fprintf(fp, "%d %f\n", idx, (t2 - t1) * 1000);
        idx++;
    }

    free(sgl);
    fclose(fp);
    fclose(dict);
    return 0;
}
int benchbloom_test(const tst_node *root,
                    char *out_file,
                    const int max,
                    bloom_t *filter)
{
    char prefix[PREFIX_LEN + 1] = "";
    char word[WORDMAX] = "";
    char buf[WORDMAX];
    char **sgl;
    FILE *fp = fopen(out_file, "w");
    FILE *dict = fopen(DICT_FILE, "r");
    int idx = 0, sidx = 0;
    double t1, t2;

    printf("bench bloom address = %p\n", &(*filter));

    if (!fp || !dict) {
        if (fp) {
            fprintf(stderr, "error: file open failed in '%s'.\n", DICT_FILE);
            fclose(fp);
        }
        if (dict) {
            fprintf(stderr, "error: file open failed in '%s'.\n", out_file);
            fclose(dict);
        }
        return 1;
    }

    sgl = (char **) malloc(sizeof(char *) * max);
    while (fgets(buf, WORDMAX, dict)) {
        for (int i = 0, j = 0; buf[i]; i++) {
            word[i] =
                (buf[i + j] == ',' || buf[i + j] == '\n') ? '\0' : buf[i + j];
            j += (buf[i + j] == ',');
        }

        if (strlen(word) < sizeof(prefix) - 1)
            continue;

        t1 = tvgetf();
        if (bloom_test(*filter, word)) {
            printf("%d found !!!\n", idx);
            tst_search_prefix(root, word, sgl, &sidx, max);
        }
        t2 = tvgetf();
        fprintf(fp, "%d %f\n", idx, (t2 - t1) * 1000);
        idx++;
        if (idx == 1000)
            break;
    }
    free(sgl);
    fclose(fp);
    fclose(dict);

    return 0;
}
