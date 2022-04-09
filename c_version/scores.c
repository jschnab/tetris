#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


typedef struct {
    char name[9];
    uint32_t score;
} ScoreItem;


typedef struct {
    char *buffer;
    size_t length;
} InputBuffer;


#define size_of_attr(Struct, Attr) sizeof(((Struct*)0)->Attr)
#define SCORE_ARRAY_LENGTH 10
#define SCORES_LENGTH_SIZE sizeof(uint8_t)
#define SCORES_LENGTH_OFFSET 0
#define SCORES_ARRAY_OFFSET SCORES_LENGTH_SIZE

#define NAME_SIZE size_of_attr(ScoreItem, name)
#define SCORE_SIZE size_of_attr(ScoreItem, score)
#define NAME_OFFSET 0
#define SCORE_OFFSET NAME_OFFSET + NAME_SIZE


InputBuffer *buffer_new();
void close_all();
void deserialize_score(void *, ScoreItem *);


ScoreItem score_array[SCORE_ARRAY_LENGTH];
int fd;


InputBuffer *buffer_new() {
    InputBuffer *buf = (InputBuffer*) malloc(sizeof(InputBuffer));
    buf->buffer = NULL;
    buf->length = 0;
    return buf;
}


void close_all() {
    close(fd);
}


void deserialize_score(void *source, ScoreItem *destination) {
    memcpy(&(destination->name), source + NAME_OFFSET, NAME_SIZE);
    memcpy(&(destination->score), source + SCORE_OFFSET, SCORE_SIZE);
}


int main(int argc, char *argv[]) {
    fd = open(
        "scores.db",
        O_CREAT | O_RDWR,  /* create file if not exists, for reading and writing */
        S_IRUSR | S_IWUSR  /* give file owner read and write permissions */
    );

    printf("NAME_SIZE = %ld\n", NAME_SIZE);
    printf("SCORE_SIZE = %ld\n", SCORE_SIZE);

    uint32_t n_scores = 0;
    ssize_t bytes_read = read(fd, &n_scores, SCORES_LENGTH_SIZE);
    printf("Read %ld bytes\n", bytes_read);
    if (bytes_read < 0) {
        printf("Error reading file: %d\n", errno);
        close_all();
        exit(EXIT_FAILURE);
    }

    printf("n_scores: %d\n", n_scores);

    printf("size of ScoreItem: %ld\n", sizeof(ScoreItem));
    printf("size of score_array: %ld\n", sizeof(score_array));

    /*
    printf("pointer to score_array[0]: %p\n", &score_array[0]);
    printf("pointer to score_array[1]: %p\n", &score_array[1]);

    printf("pointer to score_array[0].name: %p\n", &score_array[0].name);
    printf("pointer to score_array[1].name: %p\n", &score_array[1].name);
    */

    if (bytes_read > 0) {
        for (int i = 0; i < n_scores; i++) {
            bytes_read = read(fd, &score_array[i], sizeof(ScoreItem));
            if (bytes_read <= 0) {
                printf("Error reading file: %d\n", errno);
                close_all();
                exit(EXIT_FAILURE);
            }
        }
    }

    for (int i = 0; i < n_scores; i++) {
        printf("score_array[%d] = {%s, %d}\n", i, score_array[i].name, score_array[i].score);
    }

    /*
    char *name1 = "alice";
    uint32_t score1 = 12345;
    memcpy((score_array[0]).name, name1, NAME_SIZE);
    memcpy(&((score_array[0]).score), &score1, SCORE_SIZE);

    char *name2 = "bob";
    uint32_t score2 = 23456;
    memcpy((score_array[1]).name, name2, NAME_SIZE);
    memcpy(&((score_array[1]).score), &score2, SCORE_SIZE);

    char *name3 = "charlie";
    uint32_t score3 = 34567;
    memcpy((score_array[2]).name, name3, NAME_SIZE);
    memcpy(&((score_array[2]).score), &score3, SCORE_SIZE);

    n_scores = 3;
    lseek(fd, 0, SEEK_SET);
    ssize_t bytes_written = write(fd, &n_scores, SCORES_LENGTH_SIZE);

    printf("bytes written: %ld\n", bytes_written);
    if (bytes_written <= 0) {
        printf("Error writing file: %d\n", errno);
        close_all();
        exit(EXIT_FAILURE);
    }
    */
    for (int i = 0; i < 3; i++) {
        write(fd, &score_array[i], sizeof(ScoreItem));
    }

    close_all();

    return 0;
};
