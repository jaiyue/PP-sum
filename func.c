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
    if (video->channels > MAX_CH ||
    video->height > MAX_H || video->width > MAX_W) {
        fprintf(stderr, "Error: Video size "
        "exceeds maximum limit\n");
        exit(EXIT_FAILURE);
    }
}

void write_header(FILE *output, const struct Video *video) {
    // Write header data
    fwrite(&video->frames, sizeof(int64_t), 1, output);
    fwrite(&video->channels, sizeof(unsigned char), 1, output);
    fwrite(&video->height, sizeof(unsigned char), 1, output);
    fwrite(&video->width, sizeof(unsigned char), 1, output);
}

void reverse_video(const char *input_file, const char *output_file,
                   int memory_free) {
    FILE *input = fopen(input_file, "rb");
    if (!input)     {
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

    if (memory_free == 0) {
        // Reduce memory usage by processing frames one at
        // a time in reverse order
        fseek(input, 0, SEEK_END);
        int64_t file_size = ftell(input);
        int64_t header_size = file_size - (video.frames * frame_size);

        for (int64_t i = video.frames - 1; i >= 0; i--) {
            if (fseek(input, header_size + i * frame_size, SEEK_SET) != 0) {
                fprintf(stderr, "Error seeking to frame %ld\n", i);
                free(frame_data);
                fclose(input);
                fclose(output);
                exit(EXIT_FAILURE);
            }
            if (fread(frame_data, 1, frame_size, input) != frame_size) {
                fprintf(stderr, "Error reading frame %ld\n", i);
                free(frame_data);
                fclose(input);
                fclose(output);
                exit(EXIT_FAILURE);
            }
            if (fwrite(frame_data, 1, frame_size, output) != frame_size) {
                fprintf(stderr, "Error writing frame %ld\n", i);
                free(frame_data);
                fclose(input);
                fclose(output);
                exit(EXIT_FAILURE);
            }
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
        if (memory_free == 2) {
            for (int64_t i = 0; i < video.frames / 2; i++) {
                unsigned char *frame_data_start = &video.data[i * frame_size];
                unsigned char *frame_data_end = &video.data
                [(video.frames - 1 - i) * frame_size];
                for (size_t j = 0; j < frame_size; j++) {
                    unsigned char temp = frame_data_start[j];
                    frame_data_start[j] = frame_data_end[j];
                    frame_data_end[j] = temp;
                }
            }
        } else {
        // Parallelized frame reversal using OpenMP
        #pragma omp parallel for
        for (int64_t i = 0; i < video.frames / 2; i++) {
            unsigned char *frame_data_start = &video.data[i * frame_size];
            unsigned char *frame_data_end = &video.data
            [(video.frames - 1 - i) * frame_size];
            unsigned char *temp = (unsigned char *)malloc(frame_size);
        if (!temp) {
            fprintf(stderr, "Memory allocation failed for temp buffer!\n");
            exit(EXIT_FAILURE);
        }

        memcpy(temp, frame_data_start, frame_size);
        memcpy(frame_data_start, frame_data_end, frame_size);
        memcpy(frame_data_end, temp, frame_size);

        free(temp);
        }
    }

        if (fwrite(video.data, 1, total_size, output) != total_size) {
            fprintf(stderr, "Error writing video data\n");
            free(video.data);
            free(frame_data);
            fclose(output);
            exit(EXIT_FAILURE);
        }
        free(video.data);
    }

    free(frame_data);
    fclose(input);
    fclose(output);
    printf("Video frames reversed and saved to %s\n", output_file);
}

void swap_channels(const char *input_file, const char *output_file,
                   unsigned char ch1, unsigned char ch2, int memory_free) {
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

    unsigned char *temp_channel = (unsigned char *)malloc(channel_size);
    if (!temp_channel) {
        printf("Memory allocation for temp channel failed!\n");
        fclose(input);
        fclose(output);
        return;
    }

    if (memory_free == 0) {
        // Memory-saving mode: process one frame at a time
        unsigned char *frame_data = (unsigned char *)malloc(frame_size);
        if (!frame_data) {
            printf("Memory allocation for frame data failed!\n");
            free(temp_channel);
            fclose(input);
            fclose(output);
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
    } else {
    // Performance mode: load entire video into memory
    size_t total_size = video.frames * frame_size;
    video.data = (unsigned char *)malloc(total_size);
    if (!video.data) {
        printf("Memory allocation failed!\n");
        free(temp_channel);
        fclose(input);
        fclose(output);
        return;
    }

    fread(video.data, 1, total_size, input);

    if (memory_free == 2) {
        // Process frames sequentially
        for (int64_t f = 0; f < video.frames; ++f) {
            unsigned char *frame_start = video.data + f * frame_size;
            unsigned char *channel1_data = frame_start + ch1 * channel_size;
            unsigned char *channel2_data = frame_start + ch2 * channel_size;

            memcpy(temp_channel, channel1_data, channel_size);
            memcpy(channel1_data, channel2_data, channel_size);
            memcpy(channel2_data, temp_channel, channel_size);
        }
    } else {
        // Parallel processing using OpenMP
        #pragma omp parallel for
        for (int64_t f = 0; f < video.frames; ++f) {
            // 为每个线程分配一个临时通道缓冲区
            unsigned char *temp_channel = (unsigned char *)malloc(channel_size);
            if (!temp_channel) {
                printf("Memory allocation failed for temp_channel!\n");
                exit(EXIT_FAILURE);
            }

            unsigned char *frame_start = video.data + f * frame_size;
            unsigned char *channel1_data = frame_start + ch1 * channel_size;
            unsigned char *channel2_data = frame_start + ch2 * channel_size;

            // 交换通道数据
            memcpy(temp_channel, channel1_data, channel_size);
            memcpy(channel1_data, channel2_data, channel_size);
            memcpy(channel2_data, temp_channel, channel_size);

            // 释放线程局部的临时缓冲区
            free(temp_channel);
        }
    }

    // Move fwrite outside the parallel loop to avoid multiple concurrent writes
    fwrite(video.data, 1, total_size, output);

    free(video.data);
}


    free(temp_channel);
    fclose(input);
    fclose(output);

    printf("Channels swapped and saved to %s\n", output_file);
}

void clip_channel(const char *input_file, const char *output_file,
                  unsigned char channel, unsigned char min_val,
                  unsigned char max_val, int memory_free) {
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
        fclose(output);
        return;
    }

    size_t frame_size = video.channels * video.height * video.width;
    size_t channel_size = video.height * video.width;

    if (memory_free == 0) {
        // Memory-free mode: process one frame at a time
        unsigned char *channel_data = (unsigned char *)malloc(channel_size);
        if (!channel_data) {
            printf("Memory allocation failed!\n");
            fclose(input);
            fclose(output);
            return;
        }

        for (int64_t f = 0; f < video.frames; ++f) {
            for (int ch = 0; ch < video.channels; ++ch) {
                if (fread(channel_data, 1, channel_size, input)
                != channel_size) {
                    printf("Error reading channel data at frame %ld,"
                    "channel %d\n", f, ch);
                    free(channel_data);
                    fclose(input);
                    fclose(output);
                    return;
                }

                // Perform clipping only on the target channel
                if (ch == channel) {
                    for (size_t i = 0; i < channel_size; ++i) {
                        if (channel_data[i] < min_val) {
                            channel_data[i] = min_val;
                        } else if (channel_data[i] > max_val) {
                            channel_data[i] = max_val;
                        }
                    }
                }

                if (fwrite(channel_data, 1, channel_size, output)
                != channel_size) {
                    printf("Error writing channel data at frame %ld,"
                    "channel %d\n", f, ch);
                    free(channel_data);
                    fclose(input);
                    fclose(output);
                    return;
                }
            }
        }

        free(channel_data);
        fclose(input);
        fclose(output);
        printf("Video processed in memory-free mode"
        "and saved to %s\n", output_file);
    } else {
        size_t total_size = video.frames * frame_size;
        video.data = (unsigned char *)malloc(total_size);
        if (!video.data) {
            printf("Memory allocation failed!\n");
            fclose(input);
            fclose(output);
            return;
        }

        fread(video.data, 1, total_size, input);

        // Using ielse to choose between serial or parallel processing
        if (memory_free == 2) {
            // Original serial processing
            for (int64_t f = 0; f < video.frames; ++f) {
                unsigned char *frame_start = video.data + f * frame_size;
                unsigned char *channel_data = frame_start +
                channel * channel_size;

                for (int64_t h = 0; h < video.height; ++h) {
                    for (int64_t w = 0; w < video.width; ++w) {
                        size_t pixel_idx = h * video.width + w;
                        if (channel_data[pixel_idx] < min_val) {
                            channel_data[pixel_idx] = min_val;
                        } else if (channel_data[pixel_idx] > max_val) {
                            channel_data[pixel_idx] = max_val;
                        }
                    }
                }
            }
        } else {
            // Parallelized processing using OpenMP
            #pragma omp parallel for
            for (int64_t f = 0; f < video.frames; ++f) {
                unsigned char *frame_start = video.data + f * frame_size;
                unsigned char *channel_data = frame_start +
                channel * channel_size;

                for (int64_t h = 0; h < video.height; ++h) {
                    for (int64_t w = 0; w < video.width; ++w) {
                        size_t pixel_idx = h * video.width + w;
                        if (channel_data[pixel_idx] < min_val) {
                            channel_data[pixel_idx] = min_val;
                        } else if (channel_data[pixel_idx] > max_val) {
                            channel_data[pixel_idx] = max_val;
                        }
                    }
                }
            }
        }

        fwrite(video.data, 1, total_size, output);
        free(video.data);

        printf("Video processed and saved to %s\n", output_file);
        fclose(input);
        fclose(output);
    }
}

void scale_channel(const char *input_file, const char *output_file,
                   unsigned char channel, float scale_factor, int memory_free) {
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

    // Memory-free mode: Process one channel at a time
    if (memory_free == 0) {
        unsigned char *channel_data = (unsigned char *)malloc(channel_size);
        if (!channel_data) {
            printf("Memory allocation failed!\n");
            fclose(input);
            fclose(output);
            return;
        }

        for (int64_t f = 0; f < video.frames; ++f) {  // Iterate over each frame
            for (int ch = 0; ch < video.channels; ++ch) {
                // Iterate over each channel
                // Read data for the current channel (not the entire frame)
                if (fread(channel_data, 1, channel_size, input)
                != channel_size) {
                    printf("Error reading channel data at frame %ld,"
                    "channel %d\n", f, ch);
                    free(channel_data);
                    fclose(input);
                    fclose(output);
                    return;
                }

                // If the current channel is the target channel,
                // perform clipping
                if (ch == channel) {
                    for (size_t i = 0; i < channel_size; ++i) {
                        int scaled_value = (int)
                        (channel_data[i] * scale_factor);
                        channel_data[i] = (unsigned char)
                        (scaled_value < 0 ? 0 : (scaled_value
                        > 255 ? 255 : scaled_value));
                    }
                }

                // Write the channel data back to the output file
                if (fwrite(channel_data, 1, channel_size, output)
                != channel_size) {
                    printf("Error writing channel data at frame %ld,"
                    "channel %d\n", f, ch);
                    free(channel_data);
                    fclose(input);
                    fclose(output);
                    return;
                }
            }
        }

        free(channel_data);
        printf("Video processed in memory-free"
        "mode and saved to %s\n", output_file);
    } else {
        // Performance mode: Load all data into memory
        size_t total_size = video.frames * frame_size;
        video.data = (unsigned char *)malloc(total_size);
        if (!video.data) {
            printf("Memory allocation failed!\n");
            fclose(input);
            fclose(output);
            return;
        }

        if (fread(video.data, 1, total_size, input) != total_size) {
            fprintf(stderr, "Error: Failed to read video data.\n");
            free(video.data);
            fclose(input);
            fclose(output);
            return;
        }

        // Check if we should use parallelization or not
        if (memory_free == 2) {
            // Serial mode: process one frame at a time
            for (int64_t f = 0; f < video.frames; ++f) {
                unsigned char *frame_start = video.data + f * frame_size;
                unsigned char *channel_data = frame_start +
                channel * channel_size;

                for (size_t i = 0; i < channel_size; ++i) {
                    int scaled_value = (int)(channel_data[i] * scale_factor);
                    channel_data[i] = (unsigned char)(scaled_value > 255
                    ? 255 : (scaled_value < 0 ? 0 : scaled_value));
                }
            }
        } else {
            // Parallelized mode: process frames in parallel using OpenMP
            #pragma omp parallel for
            for (int64_t f = 0; f < video.frames; ++f) {
                unsigned char *frame_start = video.data + f * frame_size;
                unsigned char *channel_data = frame_start +
                channel * channel_size;

                for (size_t i = 0; i < channel_size; ++i) {
                    int scaled_value = (int)(channel_data[i] * scale_factor);
                    channel_data[i] = (unsigned char)(scaled_value > 255
                    ? 255 : (scaled_value < 0 ? 0 : scaled_value));
                }
            }
        }

        // Write processed data to output file
        if (fwrite(video.data, 1, total_size, output) != total_size) {
            fprintf(stderr, "Error: Failed to write video data.\n");
            free(video.data);
            fclose(input);
            fclose(output);
            return;
        }

        free(video.data);
        printf("Video processed and saved to %s\n", output_file);
    }
    fclose(input);
    fclose(output);
}
