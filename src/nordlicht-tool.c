#include <pthread.h>
#include <unistd.h>
#include <popt.h>
#include "nordlicht.h"
#include "common.h"

const char *gnu_basename(const char *path) {
    char *base = strrchr(path, '/');
    return base ? base+1 : path;
}

void print_help(poptContext popt, int ret) {
    poptPrintHelp(popt, stderr, 0);
    exit(ret);
}

int main(int argc, const char **argv) {
    int width = 1000;
    int height = 150;
    char *output_file = NULL;
    char *style_string = NULL;
    nordlicht_style style;
    int exact = 0;
    int free_output_file = 0;

    int help = 0;

    struct poptOption optionsTable[] = {
        {"help", '\0', 0, &help, 0, NULL, NULL},
        {"width", 'w', POPT_ARG_INT, &width, 0, "Override default width of 1000 pixels.", NULL},
        {"height", 'h', POPT_ARG_INT, &height, 0, "Override default height of 150 pixels.", NULL},
        {"output", 'o', POPT_ARG_STRING, &output_file, 0, "Set filename of output PNG. Default: $(basename VIDEOFILE).png", "FILENAME"},
        {"style", 's', POPT_ARG_STRING, &style_string, 0, "Default is 'horizontal'. Can also be 'vertical', which compresses the frames \"down\" to rows, rotates them counterclockwise by 90 degrees and then appends them.", "STYLE"},
        {"exact", 'e', POPT_ARG_NONE, &exact, 0, "Do exact seeking. Will produce nicer barcodes for videos with few keyframes.", NULL},
        POPT_TABLEEND
    };

    poptContext popt = poptGetContext(NULL, argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(popt, "VIDEOFILE");

    char c;

    // The next line leaks 3 bytes, blame popt!
    while ((c = poptGetNextOpt(popt)) >= 0) { }

    if (c < -1) {
        fprintf(stderr, "nordlicht: %s: %s\n", poptBadOption(popt, POPT_BADOPTION_NOALIAS), poptStrerror(c));
        return 1;
    }

    if (help) {
        fprintf(stderr, "nordlicht creates colorful barcodes from video files.\n\n");
        print_help(popt, 0);
    }

    char *filename = (char*)poptGetArg(popt);

    if (filename == NULL) {
        error("Please specify an input file.");
        print_help(popt, 1);
    }

    if (poptGetArg(popt) != NULL) {
        error("Please specify only one input file.");
        print_help(popt, 1);
    }

    if (output_file == NULL) {
        output_file = malloc(snprintf(NULL, 0, "%s.png", gnu_basename(filename)) + 1);
        sprintf(output_file, "%s.png", gnu_basename(filename));
        free_output_file = 1;
    }

    if (style_string == NULL) {
        style = NORDLICHT_STYLE_HORIZONTAL;
    } else {
        if (strcmp(style_string, "horizontal") == 0) {
            style = NORDLICHT_STYLE_HORIZONTAL;
        } else if (strcmp(style_string, "vertical") == 0) {
            style = NORDLICHT_STYLE_VERTICAL;
        } else {
            error("Unknown style '%s'.", style_string);
            print_help(popt, 1);
        }
    }

    // MAIN PART

    nordlicht *code = nordlicht_init_exact(filename, width, height, exact);

    if (code == NULL) {
        return 1;
    }

    nordlicht_set_style(code, style);

    // Try to write the empty code to fail early if this does not work
    if (nordlicht_write(code, output_file) != 0) {
        return 1;
    }

    pthread_t thread;
    pthread_create(&thread, NULL, (void*(*)(void*))nordlicht_generate, code);

    float progress = 0;
    while (progress < 1) {
        progress = nordlicht_progress(code);
        printf("\rnordlicht: %02.0f%%", progress*100);
        fflush(stdout);
        usleep(100000);
    }
    pthread_join(thread, NULL);

    nordlicht_write(code, output_file);
    nordlicht_free(code);
    printf(" -> '%s'\n", output_file);

    if (free_output_file) {
        free(output_file);
    }

    poptFreeContext(popt);
    return 0;
}
