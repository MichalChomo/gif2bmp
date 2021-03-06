/**
 * @mainpage gif2bmp
 *
 * @section intro Introduction
 * gif2bmp is an app for ... you guessed it, converting GIF images to BMP
 * images. It's a school assigment for Data Coding and Compression course.
 * Written in C language, standard C99.
 *
 * @section usage Usage
 * 1. make
 * 2. ./gif2bmp [-i ifile] [-o ofile] [-l logfile] | -h
 */

#include "gif2bmp.h"

void usage() {
    printf("This program converts image in GIF format to BMP format.\n"
            "Usage: ./gif2bmp [-i ifile] [-o ofile] [-l logfile] | -h\n");
}

void cleanup(char *ifile, char *ofile, char *logfile, FILE *ifp, FILE *ofp) {
    if (ifile != NULL) {
        free(ifile);
    }
    if (ofile != NULL) {
        free(ofile);
    }
    if (logfile != NULL) {
        free(logfile);
    }
    if (ifp != NULL && ifp != stdin) {
        fclose(ifp);
    }
    if (ofp != NULL && ofp != stdout) {
        fclose(ofp);
    }
}

int main(int argc, char *argv[]) {
    char *ifile = NULL;
    char *ofile = NULL;
    char *logfile = NULL;
    int c = 0;
    tGIF2BMP g2b;
    FILE *ifp = NULL;
    FILE *ofp = NULL;

    while (-1 != (c = getopt(argc, argv, "i:o:l:h"))) {
        switch (c) {
            case 'i':
                ifile = strdup(optarg);
                break;
            case 'o':
                ofile = strdup(optarg);
                break;
            case 'l':
                logfile = strdup(optarg);
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

    g2b.gifSize = 0;
    if (ifile != NULL) {
        ifp = fopen(ifile, "r");
        if (ifp == NULL) {
            fprintf(stderr, "Cannot open input file %s.\n", ifile);
            cleanup(ifile, ofile, logfile, ifp, ofp);
            return 1;
        }
        g2b.gifSize = getFileSize(ifile);
    } else {
        ifp = stdin;
    }

    if (ofile != NULL) {
        ofp = fopen(ofile, "w");
        if (ofp == NULL) {
            fprintf(stderr, "Cannot open output file %s.\n", ofile);
            cleanup(ifile, ofile, logfile, ifp, ofp);
            return 1;
        }
    } else {
        ofp = stdout;
    }

    if (0 != gif2bmp(&g2b, ifp, ofp)) {
        cleanup(ifile, ofile, logfile, ifp, ofp);
        return 1;
    }

    if (logfile != NULL) {
        g2b.bmpSize = getFileSize(ofile);
        if (0 != createLogfile(logfile, &g2b)) {
            fprintf(stderr, "Cannot create logfile.\n");
            cleanup(ifile, ofile, logfile, ifp, NULL);
            return 1;
        }
    }

    cleanup(ifile, ofile, logfile, ifp, NULL);

    return 0;
}
