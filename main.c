#include "gif2bmp.h"

void usage() {
    printf("This program converts image in GIF format to BMP format.\n"
            "Usage: ./gif2bmp [-i ifile] [-o ofile] [-l logfile] | -h\n");
}

int main(int argc, char *argv[]) {
    char *ifile = NULL;
    char *ofile = NULL;
    char *logfile = NULL;
    int c = 0;
    tGIF2BMP g2b;
    FILE *ifp;
    FILE *ofp;

    while (-1 != (c = getopt(argc, argv, "i:o:l:h"))) {
        switch (c) {
            case 'i':
                ifile = optarg;
                break;
            case 'o':
                ofile = optarg;
                break;
            case 'l':
                logfile = optarg;
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

    ifp = fopen(ifile, "r");
    if (ifp == NULL) {
        ifp = stdin;
    }

    ofp = fopen(ofile, "w");
    if (ofp == NULL) {
        ofp = stdout;
    }

    if (0 != gif2bmp(&g2b, ifp, ofp)) {
        return 1;
    }

    if (logfile != NULL) {
        if (0 != createLogfile(logfile, &g2b)) {
            fprintf(stderr, "Cannot create logfile.\n");
        }
    }

    return 0;
}
