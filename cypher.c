#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define OPTSTR "dk:p:"

unsigned long hash(unsigned char *str);
void          xor_bytes(char** dest, char* orig, size_t length, unsigned long key);
int           encrypt(char* path, char* pw);
int           write_file_bytes(char* orig, char* path);
int           get_file_bytes(char** dest, char* path);

int 
main(int argc, char** argv) {
    char* path = NULL, *pw = NULL;
    int (*operation)(char*, char*) = &encrypt;
    int opt;

    while ((opt = getopt(argc, argv, OPTSTR)) != -1) {
        switch (opt) {
        case 'd':
            //switch to decrypt here
            break;
        case 'k':
            pw = optarg;
            break;
        case 'p':
            path = optarg;
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }
    
    int status_code = (*operation)(path, pw);
    exit(status_code);
}

int 
encrypt(char* path, char* pw) {
    char* content_buffer = NULL, *cypher_buffer = NULL;
    int readStatus, writeStatus, buffer_len;
    unsigned long pw_hash;

    readStatus = get_file_bytes(&content_buffer, path);
    if (readStatus == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    pw_hash = hash(pw);
    buffer_len = strlen(content_buffer);
    cypher_buffer = malloc(sizeof(char) * buffer_len);

    if (cypher_buffer == NULL) {
        perror("Failed to allocate heap for cypher buffer");
        return EXIT_FAILURE;
    }

    xor_bytes(&cypher_buffer, content_buffer, buffer_len, pw_hash);

    writeStatus = write_file_bytes(cypher_buffer, path);
    if (writeStatus == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    free(cypher_buffer);
    free(content_buffer);
    return EXIT_SUCCESS;
}

int
get_file_bytes(char** dest, char* path) {
    FILE* file_handle = NULL;
    size_t file_length;

    file_handle = fopen(path, "r+b");
    if (file_handle == NULL) {
        perror("Cant read file from handle");
        return EXIT_FAILURE;
    }

    fseek(file_handle, 0, SEEK_END);
    file_length = ftell(file_handle);
    rewind(file_handle);

    *dest = malloc(sizeof(char) * file_length);
    if (*dest == NULL) {
        perror("Failed to allocate heap for buffer");
        return EXIT_FAILURE;
    }

    fgets(*dest, file_length + 1, file_handle);

    fclose(file_handle);
    return EXIT_SUCCESS;
}

int
write_file_bytes(char* orig, char* path) {
    FILE* file_handle = NULL;

    file_handle = fopen(path, "r+b");
    if (file_handle == NULL) {
        perror("Cant read file from handle");
        return EXIT_FAILURE;
    }

    fputs(orig, file_handle);

    fclose(file_handle);
    return EXIT_SUCCESS;
}

void
xor_bytes(char** dest, char* orig, size_t length, unsigned long key) {
    for (size_t i = 0; i < length; i++) {
        (*dest)[i] = orig[i] ^ key;
    }
}

unsigned long
hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
