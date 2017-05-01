#include "gif2bmp.h"

void usage() {
    printf("This program converts image in GIF format to BMP format.\n"
            "Usage: ./gif2bmp [-i ifile] [-o ofile] [-l logfile] | -h\n");
}

int createLogfile(char *fileName, int uncodedSize, int codedSize) {
    FILE *lfp;
    char login[] = "xchomo01";

    lfp = fopen(fileName, "w");
    if (lfp == NULL) {
        fprintf(stderr, "Cannot open logfile %s\n.", fileName);
        return 1;
    }
    fprintf(lfp, "login = %s\nuncodedSize = %d\ncodedSize = %d\n", login,
            uncodedSize, codedSize);
    fclose(lfp);

    return 0;
}

int main(int argc, char *argv[]) {
    char *ifile;
    char *ofile;
    char *logfile;
    int c = 0;
    struct stat fileStats;
    tGIF2BMP g2b;

    while (-1 != (c = getopt(argc, argv, "i:o:h"))) {
        switch (c) {
            case 'i':
                ifile = optarg;
                break;
            case 'o':
                ofile = optarg;
                break;
            case 'h':
                usage();
                return 0;
            case '?':
                usage();
                return 1;
            default: break;
        }
    }
    printf("infile: %s\noutfile: %s\n", ifile, ofile);
    if (0 != stat(ifile, &fileStats)) {
        fprintf(stderr, "Cannot get input file stats.\n");
    }
    g2b.gifSize = (int64_t) fileStats.st_size;
    printf("infile size: %ld\n", g2b.gifSize);

    return 0;
}
