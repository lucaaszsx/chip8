#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include "buffer.h"
#include "pipe.h"

/* initial quantity to be allocated for rom buffer */
#define ROM_BUFFER_ICAP 1024

static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file)
        return NULL;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *source = malloc((size_t)size + 1);
    if (!source) {
        fclose(file);
        return NULL;
    }

    fread(source, 1, (size_t)size, file);
    source[size] = '\0';

    fclose(file);
    return source;
}

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [options] source\n"
        "       %s [-h | --help]\n"
        "Options:\n"
        "  -o, --output The file where the generated ROM is to be written\n",
        prog,
        prog
    );
}

int main(int argc, char **argv) {
    const char *src_path = NULL;
    const char *out_path = NULL;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];

        if (strcmp(arg, "-o") == 0 || strcmp(arg, "--output") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "missing output filename\n");
                return EXIT_FAILURE;
            }

            out_path = argv[i];
            continue;
        } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }

        if (arg[0] == '-') {
            fprintf(stderr, "unknown option: %s\n", arg);
            return EXIT_FAILURE;
        }

        if (src_path != NULL) {
            fprintf(stderr, "multiple source files specified\n");
            return EXIT_FAILURE;
        }

        src_path = arg;
    }

    if (!src_path || !out_path) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    char *source = read_file(src_path);
    if (!source) {
        fprintf(stderr, "failed to open '%s'\n", src_path);
        return EXIT_FAILURE;
    }

    FILE *out = fopen(out_path, "wb");
    if (!out) {
        fprintf(stderr, "failed to open '%s' for writing\n", out_path);
        free(source);
        return EXIT_FAILURE;
    }

    // runs the pipeline
    Pipe pipe;
    ByteBuffer buffer;
    buf_init(&buffer, ROM_BUFFER_ICAP);
    pipe_run(&pipe, source, &buffer);

    int status = EXIT_SUCCESS;

    // writes the output on specified file
    size_t written = fwrite(buffer.data, sizeof(uint8_t), buffer.size, out);

    if (written != buffer.size) {
        fprintf(stderr, "failed to write to '%s': ", out_path);
        if (ferror(out))
            fprintf(stderr, "%s\n", strerror(errno));
        else
            fprintf(stderr, "incomplete write (%zu/%zu bytes)\n", written, buffer.size);

        status = EXIT_FAILURE;
        goto cleanup;
    } else printf("%zu bytes written to '%s'\n", buffer.size, out_path);

    // clear memory
    cleanup:
    pipe_free(&pipe);
    buf_free(&buffer);
    fclose(out);
    free(source);

    return status;
}
