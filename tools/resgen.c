#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

char *file_name(char *path) {
    char *p = path;
    char *name = p;
    while (*p != '\0') {
        if (*p == '/')
            name = p + 1;
        p++;
    }
    return name;
}

char *do_file(FILE *namesf, FILE *blobsf, char *symbol_prefix, char *path)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        printf("fopen() failed for \"%s\"\n", path);
        return NULL;
    }
    char *p = path;
    char *name = p;
    while (*p != '\0') {
        if (*p == '/')
            name = p + 1;
        else if (*p == '.')
            *p = '\0';
        p++;
    }
    size_t len = 0;
    uint8_t b;
    fprintf(blobsf, "\nuint8_t %s_%s[] = {", symbol_prefix, name);
    while (fread(&b, sizeof(b), 1, f)) {
        fprintf(blobsf, "0x%x,", b);
        len++;
    }
    fprintf(blobsf, "};");
    fclose(f);

    fprintf(namesf, "extern uint8_t %s_%s[%zu];\n", symbol_prefix, name, len);
    return name;
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        printf("Usage: resgen NAMES_PATH BLOBS_PATH FILES...\n");
        exit(1);
    }
    char *names_path = argv[1];
    char *blobs_path = argv[2];
    char *symbol_prefix = argv[3];

    char **filesv = &argv[4];
    int filesc = argc -= 4;
    char *namesv[filesc];
    int namesc = filesc;

    FILE *namesf = fopen(names_path, "wb");
    FILE *blobsf = fopen(blobs_path, "wb");
    if (namesf == NULL || blobsf == NULL) {
        printf("Error in fopen()\n");
        exit(1);
    }

    // Top of names
    fprintf(namesf, "#ifndef %s_H\n", symbol_prefix);
    fprintf(namesf, "#define %s_H\n\n", symbol_prefix);
    fprintf(namesf, "#include <stdint.h>\n");
    fprintf(blobsf, "#include \"%s\"\n", file_name(names_path));
    // Main contents
    for (int i = 0; i < filesc; i++) {
        namesv[i] = do_file(namesf, blobsf, symbol_prefix, filesv[i]);
    }
    // End of names
    fprintf(namesf, "\n#define %s_for_each(m)", symbol_prefix);
    for (int i = 0; i < filesc; i++) {
        if (namesv[i] != NULL)
            fprintf(namesf, " \\\nm(%s)", namesv[i]);
    }
    fprintf(namesf, "\n\n#endif\n");
    fclose(namesf);
    fclose(blobsf);

    return 0;
}
