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
    if (video->channels > MAX_CH || video->height > MAX_H
    || video->width > MAX_W) {
        printf("Error: Video size exceeds maximum limit\n");
    }
}

void write_file(const char *output_file, struct Video video,
size_t total_size) {
    FILE *output = fopen(output_file, "wb");
    if (!output) {
        printf("Error opening output file.\n");
        free(video.data);
        return;
    }
    // Write header data
    fwrite(&video.frames, sizeof(int64_t), 1, output);
    fwrite(&video.channels, sizeof(unsigned char), 1, output);
    fwrite(&video.height, sizeof(unsigned char), 1, output);
    fwrite(&video.width, sizeof(unsigned char), 1, output);

    // Write video frame data
    fwrite(video.data, 1, total_size, output);
    fclose(output);
}

// -S -- save all frames to memory and reverse them
void reverse_video_performance(const char *input_file,
const char *output_file) {
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }
    read_headerdata(input, &video);

    // Calculate the size of each frame and the total video size
    size_t frame_size = video.channels * video.height * video.width;
    size_t total_size = video.frames * frame_size;
    video.data = (unsigned char *)malloc(total_size);
    if (!video.data) {
        printf("Memory allocation failed!\n");
        fclose(input);
        return;
    }

    fread(video.data, 1, total_size, input);
    fclose(input);

    // Swap all frames by double-pointering two pointed frames
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

    write_file(output_file, video, total_size);
    free(video.data);

    printf("Video frames reversed and saved to %s\n", output_file);
}
// -M -- reads, inverts and writes directly to the output file,
// frame by frame (processing one frame at a time)
void reverse_video_memory(const char *input_file, const char *output_file) {
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }

    read_headerdata(input, &video);

    size_t frame_size = video.channels * video.height * video.width;

    // Open output file and prepare to write
    FILE *output = fopen(output_file, "wb");
    if (!output) {
        printf("Error opening output file.\n");
        fclose(input);
        return;
    }

    fwrite(&video.frames, sizeof(int64_t), 1, output);
    fwrite(&video.channels, sizeof(unsigned char), 1, output);
    fwrite(&video.height, sizeof(unsigned char), 1, output);
    fwrite(&video.width, sizeof(unsigned char), 1, output);

    // Read, invert and write to output file frame by frame
    unsigned char *frame_data = (unsigned char *)malloc(frame_size);
    if (!frame_data) {
        printf("Memory allocation failed!\n");
        fclose(input);
        fclose(output);
        return;
    }

    for (int64_t i = video.frames - 1; i >= 0; i--) {
        fseek(input, sizeof(int64_t) + sizeof(unsigned char) * 3 +
        i * frame_size, SEEK_SET);
        fread(frame_data, 1, frame_size, input);
        fwrite(frame_data, 1, frame_size, output);
    }

    fclose(input);
    fclose(output);
    free(frame_data);

    printf("Video frames reversed and saved to %s\n", output_file);}
// -S --Save video in memory, clip each channel to specified range,
// and write it to output file
void swap_channel_performance(const char *input_file, const char *output_file,
unsigned char ch1, unsigned char ch2) {
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

    // Calculate frame and channel sizes
    size_t frame_size = video.channels * video.height * video.width;
    size_t channel_size = video.height * video.width;
    size_t total_size = video.frames * frame_size;

    video.data = (unsigned char *)malloc(total_size);
    if (!video.data) {
        printf("Memory allocation failed!\n");
        fclose(input);
        return;
    }

    fread(video.data, 1, total_size, input);
    fclose(input);

    unsigned char *temp_channel = (unsigned char *)malloc(channel_size);
    if (!temp_channel) {
        printf("Memory allocation for temp channel failed!\n");
        free(video.data);
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

    write_file(output_file, video, total_size);

    free(temp_channel);
    free(video.data);

    printf("Channels swapped and saved to %s\n",
    output_file);
}
// -M -- reads each frame, swaps specified channels and writes directly to
// the output file (one frame processed at a time)
void swap_channel_memory(const char *input_file, const char *output_file,
unsigned char ch1, unsigned char ch2) {
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }
    read_headerdata(input, &video);

    size_t channel_size = video.height * video.width;
    size_t frame_size = video.channels * video.height * video.width;

    FILE *output = fopen(output_file, "wb");
    if (!output) {
        printf("Error opening output file.\n");
        fclose(input);
        return;
    }
    fwrite(&video.frames, sizeof(int64_t), 1, output);
    fwrite(&video.channels, sizeof(unsigned char), 1, output);
    fwrite(&video.height, sizeof(unsigned char), 1, output);
    fwrite(&video.width, sizeof(unsigned char), 1, output);
    unsigned char *temp_channel = (unsigned char *)malloc(channel_size);
    if (!temp_channel) {
        printf("Memory allocation for temp channel failed!\n");
        fclose(input);
        fclose(output);
        return;
    }
    // Iterate through each frame of data,
    // swapping the specified channels and writing to the output file
    unsigned char *frame_data = (unsigned char *)malloc(frame_size);
    if (!frame_data) {
        printf("Memory allocation failed!\n");
        fclose(input);
        fclose(output);
        free(temp_channel);
        return;
    }
    for (int64_t f = 0; f < video.frames; f++) {
        fread(frame_data, 1, frame_size, input);
        // Calculate the position of the channel data
        unsigned char *frame_start = frame_data;
        unsigned char *channel1_data = frame_start + ch1 * channel_size;
        unsigned char *channel2_data = frame_start + ch2 * channel_size;
        // Exchange of channel data
        memcpy(temp_channel, channel1_data, channel_size);
        memcpy(channel1_data, channel2_data, channel_size);
        memcpy(channel2_data, temp_channel, channel_size);
        // Write the modified frame to the output file
        fwrite(frame_data, 1, frame_size, output);
    }
    fclose(input);
    fclose(output);
    free(frame_data);
    free(temp_channel);

    printf("Channels swapped and saved to %s\n", output_file);
}
// -S --Save video in memory, clip specified channel to specified range,
// and write it to output file
void clip_channel_performance(const char *input_file, const char *output_file,
unsigned char channel, unsigned char min_val, unsigned char max_val) {
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
    size_t total_size = video.frames * frame_size;
    // Allocate memory to store video data
    video.data = (unsigned char *)malloc(total_size);
    if (!video.data) {
        printf("Memory allocation failed!\n");
        fclose(input);
        return;
    }

    fread(video.data, 1, total_size, input);
    fclose(input);

    // Get every frame
    for (int64_t f = 0; f < video.frames; ++f) {
        unsigned char *frame_start = video.data + f * frame_size;
        unsigned char *channel_data = frame_start + channel * channel_size;

     // Clip each pixel value for the specified channel
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

    write_file(output_file, video, total_size);
    free(video.data);

    printf("Channel %d clipped and saved to %s\n", channel, output_file);
}
// -M -- Read each frame, clip the specified channel to the given range,
// and write the result to output file directly (one frame at a time)
void clip_channel_memory(const char *input_file, const char *output_file,
unsigned char channel, unsigned char min_val, unsigned char max_val) {
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }

    read_headerdata(input, &video);

    size_t frame_size = video.channels * video.height * video.width;
    size_t channel_size = video.height * video.width;

    FILE *output = fopen(output_file, "wb");
    if (!output) {
        printf("Error opening output file.\n");
        fclose(input);
        return;
    }

    fwrite(&video.frames, sizeof(int64_t), 1, output);
    fwrite(&video.channels, sizeof(unsigned char), 1, output);
    fwrite(&video.height, sizeof(unsigned char), 1, output);
    fwrite(&video.width, sizeof(unsigned char), 1, output);

    unsigned char *frame_data = (unsigned char *)malloc(frame_size);
    if (!frame_data) {
        printf("Memory allocation failed!\n");
        fclose(input);
        fclose(output);
        return;
    }

    for (int64_t f = 0; f < video.frames; ++f) {
        // Read each frame
        fread(frame_data, 1, frame_size, input);

        // 如果是指定的通道，则进行裁剪
        if (channel < video.channels) {
            unsigned char *channel_data = frame_data + channel * channel_size;

            // Clip if it's a specified channel
            for (size_t i = 0; i < channel_size; ++i) {
                if (channel_data[i] < min_val) {
                    channel_data[i] = min_val;
                } else if (channel_data[i] > max_val) {
                    channel_data[i] = max_val;
                }
            }
        }
        // Write processed frames to output file
        fwrite(frame_data, 1, frame_size, output);
    }

    fclose(input);
    fclose(output);
    free(frame_data);

    printf("Video processed and saved to %s\n", output_file);
}
// -S -- Save video in memory, scale the specified channel by a factor,
// and write it to output file
void scale_channel_performance(const char *input_file, const char *output_file,
unsigned char channel, float scale_factor) {
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
    size_t total_size = video.frames * frame_size;

    // Allocate memory to store video data
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

        // Scale each pixel value
        for (int64_t h = 0; h < video.height; ++h) {
            for (int64_t w = 0; w < video.width; ++w) {
                size_t pixel_idx = h * video.width + w;
                int scaled_value = (int)(channel_data[pixel_idx] *
                scale_factor);
                if (scaled_value > 255) {
                    scaled_value = 255;
                }
                channel_data[pixel_idx] = (unsigned char)scaled_value;
            }
        }
    }

    write_file(output_file, video, total_size);
    free(video.data);

    printf("Channel %d scaled and saved to %s\n", channel, output_file);
}
// -M -- Read each frame, scale the specified channel by a factor,
// and write it to output file directly (one frame at a time)
void scale_channel_memory(const char *input_file, const char *output_file,
unsigned char channel, float scale_factor) {
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }

    read_headerdata(input, &video);

    size_t frame_size = video.channels * video.height * video.width;
    size_t channel_size = video.height * video.width;

    FILE *output = fopen(output_file, "wb");
    if (!output) {
        printf("Error opening output file.\n");
        fclose(input);
        return;
    }

    fwrite(&video.frames, sizeof(int64_t), 1, output);
    fwrite(&video.channels, sizeof(unsigned char), 1, output);
    fwrite(&video.height, sizeof(unsigned char), 1, output);
    fwrite(&video.width, sizeof(unsigned char), 1, output);

    // Allocate memory for each frame
    unsigned char *frame_data = (unsigned char *)malloc(frame_size);
    if (!frame_data) {
        printf("Memory allocation failed!\n");
        fclose(input);
        fclose(output);
        return;
    }

    for (int64_t f = 0; f < video.frames; ++f) {
        fread(frame_data, 1, frame_size, input);

        // Scaling if it's a specified channel
        if (channel < video.channels) {
            unsigned char *channel_data = frame_data + channel * channel_size;

            // Scale the data of the specified channel
            for (size_t i = 0; i < channel_size; ++i) {
                int scaled_value = (int)(channel_data[i] * scale_factor);
                if (scaled_value > 255) {
                    channel_data[i] = 255;
                } else if (scaled_value < 0) {
                    channel_data[i] = 0;
                } else {
                    channel_data[i] = (unsigned char)scaled_value;
                }
            }
        }

        fwrite(frame_data, 1, frame_size, output);
    }

    fclose(input);
    fclose(output);
    free(frame_data);

    printf("Video processed and saved to %s\n", output_file);
}
