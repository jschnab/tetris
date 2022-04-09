#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    char name[10];
    uint32_t score;
} Score;


int main(int argc, char *argv[]) {
    /*
    Score *scores = (Score *) malloc(sizeof(Score) * 10);
    memcpy(scores[0].name, "alice", 10); scores[0].score = 1234;
    memcpy(scores[1].name, "bob", 10); scores[1].score = 2345;
    memcpy(scores[2].name, "charlie", 10); scores[2].score = 3456;

    FILE *fp = fopen("scores.txt", "w");

    for (int i = 0; i < 3; i++) {
        fprintf(fp, "%s;%d\n", scores[i].name, scores[i].score);
    }

    free(scores);
    fclose(fp);
    */
    
    FILE *fp = fopen("scores.txt", "r");
    char *string = NULL;
    size_t size = 20;
    size_t read;
    Score *scores = (Score *) malloc(sizeof(Score) * 10);
    char *token;
    char delim[2] = ";";

    for (int i = 0; (read = getline(&string, &size, fp)) != -1; i++) {
        string[read-1] = '\0';
        token = strtok(string, delim);
        memcpy(scores[i].name, token, 10);
        token = strtok(NULL, delim);
        scores[i].score = atoi(token);
    }

    Score key;
    int j;
    for (int i = 1; i < 10; i++) {
        key = scores[i];
        for (j = i - 1; j >= 0 && scores[j].score < key.score; j--) {
            scores[j + 1] = scores[j];
        }
        scores[j + 1] = key;
    }

    for (int i = 0; i < 10; i++) {
        printf("%s scored %d points\n", scores[i].name, scores[i].score);
    }

    free(string);

    return 0;
}
