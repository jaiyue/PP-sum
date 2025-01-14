#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "func.h"

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
    
    // 默认选择 -M（内存优化），如果参数中没有 -S/-M
    int is_memory = 0; // 标志变量：0 表示内存优化，1 表示性能优化

    // 判断是否指定了 -S 或 -M
    int operation_start_index = 3; // 默认操作从第 3 个参数开始
    if (argc > 4 && (strcmp(argv[3], "-S") == 0 || strcmp(argv[3], "-M") == 0)) {
        is_memory = (strcmp(argv[3], "-S") == 0); // -S 设置为性能优化
        operation_start_index = 4; // 如果指定了 -S/-M，操作参数从第 4 个参数开始
    }

    const char *operation = argv[operation_start_index];

    unsigned char ch1, ch2, channel;
    unsigned char min_val, max_val;
    float scale_factor;

    if (strcmp(operation, "reverse") == 0) {
        if (is_memory) {
            reverse_video_performance(input_file, output_file);
        } else {
            reverse_video_memory(input_file, output_file);
        }
    } else if (strcmp(operation, "swap_channel") == 0) {
        if (argc < operation_start_index + 2) {
            printf("Error: Two channels (ch1, ch2) are required for swap operation.\n");
            return 1;
        }
        ch1 = (unsigned char)atoi(argv[operation_start_index + 1]) - 1; // 1-based to 0-based
        ch2 = (unsigned char)atoi(argv[operation_start_index + 2]) - 1; // 1-based to 0-based
        if (is_memory) {
            swap_channel_performance(input_file, output_file, ch1, ch2);
        } else {
            swap_channel_memory(input_file, output_file, ch1, ch2);
        }
    } else if (strcmp(operation, "clip_channel") == 0) {
        if (argc < operation_start_index + 3) {
            printf("Error: Channel and range (min, max) are required for clipping operation.\n");
            return 1;
        }
        channel = (unsigned char)atoi(argv[operation_start_index + 1]) - 1; // 1-based to 0-based
        sscanf(argv[operation_start_index + 2], "%hhu,%hhu", &min_val, &max_val);
        if (is_memory) {
            clip_channel_performance(input_file, output_file, channel, min_val, max_val);
        } else {
            clip_channel_memory(input_file, output_file, channel, min_val, max_val);
        }
    } else if (strcmp(operation, "scale_channel") == 0) {
        if (argc < operation_start_index + 2) {
            printf("Error: Channel and scale factor are required for scaling operation.\n");
            return 1;
        }
        channel = (unsigned char)atoi(argv[operation_start_index + 1]) - 1; // 1-based to 0-based
        scale_factor = atof(argv[operation_start_index + 2]);
        if (is_memory) {
            scale_channel_performance(input_file, output_file, channel, scale_factor);
        } else {
            scale_channel_memory(input_file, output_file, channel, scale_factor);
        }
    } else {
        print_usage();
        return 1;
    }

    return 0;
}
