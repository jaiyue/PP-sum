#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "func.h"
#include <stdint.h>

struct Video video;

void read_headerdata(FILE *input, struct Video *video) {
    // Read the header data
    fread(&video->frames, sizeof(int64_t), 1, input);
    fread(&video->channels, sizeof(unsigned char), 1, input);
    fread(&video->height, sizeof(unsigned char), 1, input);
    fread(&video->width, sizeof(unsigned char), 1, input);

    // Check the maximum size of the video
    if (video->channels > MAX_CH || video->height > MAX_H || video->width > MAX_W) {
        fprintf(stderr, "Error: Video size exceeds maximum limit\n");
        exit(EXIT_FAILURE);
    }
}

void write_header(FILE *output, const struct Video *video) {
    fwrite(&video->frames, sizeof(int64_t), 1, output);
    fwrite(&video->channels, sizeof(unsigned char), 1, output);
    fwrite(&video->height, sizeof(unsigned char), 1, output);
    fwrite(&video->width, sizeof(unsigned char), 1, output);
}

void reverse_video(const char *input_file, const char *output_file, int memory_free) {
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    read_headerdata(input, &video);
    size_t frame_size = video.channels * video.height * video.width;

    FILE *output = fopen(output_file, "wb");
    if (!output) {
        perror("Error opening output file");
        fclose(input);
        exit(EXIT_FAILURE);
    }

    write_header(output, &video);

    unsigned char *frame_data = (unsigned char *)malloc(frame_size);
    if (!frame_data) {
        fprintf(stderr, "Memory allocation failed!\n");
        fclose(input);
        fclose(output);
        exit(EXIT_FAILURE);
    }

    if (memory_free) {
        // Reduce memory usage by processing frames one at a time in reverse order
        fseek(input, 0, SEEK_END);
        long file_size = ftell(input);
        long header_size = file_size - (video.frames * frame_size);

        for (int64_t i = video.frames - 1; i >= 0; i--) {
            fseek(input, header_size + i * frame_size, SEEK_SET);
            fread(frame_data, 1, frame_size, input);
            fwrite(frame_data, 1, frame_size, output);
        }

    } else {
        // Load all frames into memory for faster processing
        size_t total_size = video.frames * frame_size;
        video.data = (unsigned char *)malloc(total_size);
        if (!video.data) {
            fprintf(stderr, "Memory allocation failed!\n");
            free(frame_data);
            fclose(input);
            fclose(output);
            exit(EXIT_FAILURE);
        }

        fread(video.data, 1, total_size, input);
        fclose(input);

        for (int64_t i = 0; i < video.frames / 2; i++) {
            unsigned char *frame_data_start = &video.data[i * frame_size];
            unsigned char *frame_data_end = &video.data[(video.frames - 1 - i) * frame_size];
            for (size_t j = 0; j < frame_size; j++) {
                unsigned char temp = frame_data_start[j];
                frame_data_start[j] = frame_data_end[j];
                frame_data_end[j] = temp;
            }
        }

        fwrite(video.data, 1, total_size, output);
        free(video.data);
    }

    free(frame_data);
    fclose(output);
    printf("Video frames reversed and saved to %s\n", output_file);
}

void swap_channels(const char *input_file, const char *output_file, unsigned char ch1, unsigned char ch2, int memory_free) {
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }

    read_headerdata(input, &video);

    if (ch1 >= video.channels || ch2 >= video.channels) {
        printf("Error: Invalid channel indices.\n");
        fclose(input);
        return;
    }

    size_t frame_size = video.channels * video.height * video.width;
    size_t channel_size = video.height * video.width;

    FILE *output = fopen(output_file, "wb");
    if (!output) {
        printf("Error opening output file.\n");
        fclose(input);
        return;
    }

    write_header(output, &video);

    if (memory_free) {
        // Memory-saving mode: process one frame at a time
        unsigned char *frame_data = (unsigned char *)malloc(frame_size);
        unsigned char *temp_channel = (unsigned char *)malloc(channel_size);
        if (!frame_data || !temp_channel) {
            printf("Memory allocation failed!\n");
            fclose(input);
            fclose(output);
            free(frame_data);
            free(temp_channel);
            return;
        }

        for (int64_t f = 0; f < video.frames; ++f) {
            fread(frame_data, 1, frame_size, input);

            unsigned char *channel1_data = frame_data + ch1 * channel_size;
            unsigned char *channel2_data = frame_data + ch2 * channel_size;

            memcpy(temp_channel, channel1_data, channel_size);
            memcpy(channel1_data, channel2_data, channel_size);
            memcpy(channel2_data, temp_channel, channel_size);

            fwrite(frame_data, 1, frame_size, output);
        }

        free(frame_data);
        free(temp_channel);
    } else {
        // Performance mode: load entire video into memory
        size_t total_size = video.frames * frame_size;
        video.data = (unsigned char *)malloc(total_size);
        if (!video.data) {
            printf("Memory allocation failed!\n");
            fclose(input);
            fclose(output);
            return;
        }

        fread(video.data, 1, total_size, input);

        unsigned char *temp_channel = (unsigned char *)malloc(channel_size);
        if (!temp_channel) {
            printf("Memory allocation for temp channel failed!\n");
            free(video.data);
            fclose(input);
            fclose(output);
            return;
        }

        for (int64_t f = 0; f < video.frames; ++f) {
            unsigned char *frame_start = video.data + f * frame_size;
            unsigned char *channel1_data = frame_start + ch1 * channel_size;
            unsigned char *channel2_data = frame_start + ch2 * channel_size;

            memcpy(temp_channel, channel1_data, channel_size);
            memcpy(channel1_data, channel2_data, channel_size);
            memcpy(channel2_data, temp_channel, channel_size);
        }

        fwrite(video.data, 1, total_size, output);

        free(temp_channel);
        free(video.data);
    }

    fclose(input);
    fclose(output);

    printf("Channels swapped and saved to %s\n", output_file);
}

void clip_channel(const char *input_file, const char *output_file, unsigned char channel, unsigned char min_val, unsigned char max_val, int memory_free) {
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }
    read_headerdata(input, &video);

    FILE *output = fopen(output_file, "wb");
            if (!output) {
                printf("Error opening output file.\n");
                fclose(input);
                return;
            }
    write_header(output, &video);

    if (channel >= video.channels) {
        printf("Error: Invalid channel index.\n");
        fclose(input);
        return;
    }

    size_t frame_size = video.channels * video.height * video.width;
    size_t channel_size = video.height * video.width;

    if (memory_free) {
        unsigned char *frame_data = (unsigned char *)malloc(frame_size);
        if (!frame_data) {
            printf("Memory allocation failed!\n");
            fclose(input);
            fclose(output);
            return;
        }
        for (int64_t f = 0; f < video.frames; ++f) {
            fread(frame_data, 1, frame_size, input);
            if (channel < video.channels) {
                unsigned char *channel_data = frame_data + channel * channel_size;
                for (size_t i = 0; i < channel_size; ++i) {
                    if (channel_data[i] < min_val) {
                        channel_data[i] = min_val;
                    } else if (channel_data[i] > max_val) {
                        channel_data[i] = max_val;
                    }
                }
            }
            fwrite(frame_data, 1, frame_size, output);
        }

        free(frame_data);
        fclose(input);
        fclose(output);

        printf("Video processed in memory-free mode and saved to %s\n", output_file);
    } else {
        size_t total_size = video.frames * frame_size;
        video.data = (unsigned char *)malloc(total_size);
        if (!video.data) {
            printf("Memory allocation failed!\n");
            fclose(input);
            return;
        }

        fread(video.data, 1, total_size, input);
        fclose(input);

        for (int64_t f = 0; f < video.frames; ++f) {
            unsigned char *frame_start = video.data + f * frame_size;
            unsigned char *channel_data = frame_start + channel * channel_size;

            for (size_t i = 0; i < channel_size; ++i) {
                if (channel_data[i] < min_val) {
                    channel_data[i] = min_val;
                } else if (channel_data[i] > max_val) {
                    channel_data[i] = max_val;
                }
            }
        }

        FILE *output = fopen(output_file, "wb");
        if (!output) {
            printf("Error opening output file.\n");
            free(video.data);
            return;
        }

        write_header(output, &video);
        fwrite(video.data, 1, total_size, output);
        fclose(output);
        free(video.data);

        printf("Video processed in performance mode and saved to %s\n", output_file);
    }
}

void scale_channel(const char *input_file, const char *output_file, unsigned char channel, float scale_factor, int memory_free) {
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }

    read_headerdata(input, &video);

    if (channel >= video.channels) {
        printf("Error: Invalid channel index.\n");
        fclose(input);
        return;
    }

    size_t frame_size = video.channels * video.height * video.width;
    size_t channel_size = video.height * video.width;

    FILE *output = fopen(output_file, "wb");
    if (!output) {
        printf("Error opening output file.\n");
        fclose(input);
        return;
    }

    write_header(output, &video);

    if (memory_free) { // Memory-free mode: Process one frame at a time
        unsigned char *frame_data = (unsigned char *)malloc(frame_size);
        if (!frame_data) {
            printf("Memory allocation failed!\n");
            fclose(input);
            fclose(output);
            return;
        }

        for (int64_t f = 0; f < video.frames; ++f) {
            fread(frame_data, 1, frame_size, input);

            if (channel < video.channels) {
                unsigned char *channel_data = frame_data + channel * channel_size;

                for (size_t i = 0; i < channel_size; ++i) {
                    int scaled_value = (int)(channel_data[i] * scale_factor);
                    channel_data[i] = (unsigned char)(scaled_value > 255 ? 255 : (scaled_value < 0 ? 0 : scaled_value));
                }
            }

            fwrite(frame_data, 1, frame_size, output);
        }

        free(frame_data);
    } else { // Performance mode: Load all data into memory
        size_t total_size = video.frames * frame_size;
        video.data = (unsigned char *)malloc(total_size);
        if (!video.data) {
            printf("Memory allocation failed!\n");
            fclose(input);
            fclose(output);
            return;
        }

        fread(video.data, 1, total_size, input);

        for (int64_t f = 0; f < video.frames; ++f) {
            unsigned char *frame_start = video.data + f * frame_size;
            unsigned char *channel_data = frame_start + channel * channel_size;

            for (int64_t h = 0; h < video.height; ++h) {
                for (int64_t w = 0; w < video.width; ++w) {
                    size_t pixel_idx = h * video.width + w;
                    int scaled_value = (int)(channel_data[pixel_idx] * scale_factor);
                    channel_data[pixel_idx] = (unsigned char)(scaled_value > 255 ? 255 : scaled_value);
                }
            }
        }

        fwrite(video.data, 1, total_size, output);
        free(video.data);
    }

    fclose(input);
    fclose(output);

    printf("Video processed and saved to %s\n", output_file);
}