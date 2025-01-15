#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "func.h"
#include <sys/resource.h>


void print_usage() {
    printf("Usage: ./runme [input] [output] [-S/-M] <operation> [params]\n");
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        print_usage();
        return 1;
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];
    // Default is -M (memory optimisation),
    // if there is no -S/-M in the parameters,
    // flag variable: 0 for memory optimisation, 1 for performance optimisation.
    int is_memory = 0;

    // Determine if -S or -M is specified.
    // Default operation starts with the 3rd argument
    int operation_start_index = 3;
    if (argc > 4 && (strcmp(argv[3], "-S") == 0 ||
    strcmp(argv[3], "-M") == 0)) {
        is_memory = (strcmp(argv[3], "-M") == 0);
        // If -S/-M is specified, the operation parameters
        // start with the 4th parameter.
        operation_start_index = 4;
    }

    const char *operation = argv[operation_start_index];

    unsigned char ch1, ch2, channel;
    unsigned char min_val, max_val;
    float scale_factor;

    if (strcmp(operation, "reverse") == 0) {
        reverse_video(input_file, output_file, is_memory);
    } else if (strcmp(operation, "swap_channel") == 0) {
        if (argc < operation_start_index + 2) {
            printf("Error: Two channels (ch1, ch2) are "
            "required for the swap operation.\n");
            return 1;
        }
        // Parse ch1 and ch2, expecting the format ‘1,2’.
        if (sscanf(argv[operation_start_index + 1],
        "%hhu,%hhu", &ch1, &ch2) != 2) {
            printf("Error: Invalid format for channels."
            "Use ch1,ch2 (e.g., 1,2).\n");
            return 1;
        }
        // Output parsed ch1 and ch2 for debugging.
        printf("Parsed Channels: ch1 = %hhu, ch2 = %hhu\n", ch1, ch2);

        // Ensure that the parameters are within the valid range
        printf("Channel 1: %d, Channel 2: %d\n", ch1, ch2);
        swap_channels(input_file, output_file, ch1, ch2, is_memory);
    } else if (strcmp(operation, "clip_channel") == 0) {
        if (argc < operation_start_index + 3) {
            printf("Error: Channel and range (min, max) are"
            "required for clipping operation.\n");
            return 1;
        }
        channel = (unsigned char)
        atoi(argv[operation_start_index + 1]);
        if (sscanf(argv[operation_start_index + 2],
        "[%hhu,%hhu]", &min_val, &max_val) != 2) {
            printf("Error: Invalid range format."
            "Use [min,max] (e.g., [10,200]).\n");
            return 1;
    }
        printf("Channel: %d, min: %d, max:%d\n", channel, min_val, max_val);
        clip_channel(input_file, output_file, channel,
        min_val, max_val, is_memory);

    } else if (strcmp(operation, "scale_channel") == 0) {
        if (argc < operation_start_index + 2) {
            printf("Error: Channel and scale factor are"
            "required for scaling operation.\n");
            return 1;
        }
        channel = (unsigned char)
        atoi(argv[operation_start_index + 1]);
        scale_factor = atof(argv[operation_start_index + 2]);
        printf("Channel: %d\n", channel);
        scale_channel(input_file, output_file, channel,
        scale_factor, is_memory);
    } else {
        print_usage();
        return 1;
    }
    return 0;
}
