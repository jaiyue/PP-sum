#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "func.h"


void read_headerdata(FILE *input, struct Video video) {
        // Read header
    fread(&video.frames, sizeof(long), 1, input);
    fread(&video.channels, sizeof(unsigned char), 1, input);
    fread(&video.height, sizeof(unsigned char), 1, input);
    fread(&video.width, sizeof(unsigned char), 1, input);
    //maximal size of video
    if (video.channels > MAX_CH || video.height > MAX_H || video.width > MAX_W) {
        printf("Error: Video size exceeds maximum limit\n");
        fclose(input);
    }
}

void write_file(const char *output_file, struct Video video, size_t total_size) {
    FILE *output = fopen(output_file, "wb");
    if (!output) {
        printf("Error opening output file.\n");
        free(video.data);
        return;
    }

    // Header
    fwrite(&video.frames, sizeof(long), 1, output);
    fwrite(&video.channels, sizeof(unsigned char), 1, output);
    fwrite(&video.height, sizeof(unsigned char), 1, output);
    fwrite(&video.width, sizeof(unsigned char), 1, output);

    // Frames
    fwrite(video.data, 1, total_size, output);
    fclose(output);
}


//-S --Save all Frames in memory and reverse them
void reverse_video_performance(const char *input_file, const char *output_file) {
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }
    struct Video video;
    read_headerdata(input, video);

    // Calculate frame and video sizes
    size_t frame_size = video.channels * video.height * video.width;
    size_t total_size = video.frames * frame_size;
    video.data = (unsigned char *)malloc(total_size);
    if (!video.data) {
        printf("Memory allocation failed!\n");
        fclose(input);
        return;
    }

    // Read frames
    fread(video.data, 1, total_size, input);
    fclose(input);

    // Reverse frames in place
    for (long i = 0; i < video.frames / 2; i++) {
        unsigned char *frame_data_start = &video.data[i * frame_size];
        unsigned char *frame_data_end = &video.data[(video.frames - 1 - i) * frame_size];
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


// -M --Read each frame, reverse it and write it to output file directly(one frame at a time)
void reverse_video_memory(const char *input_file, const char *output_file) {
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }

    // 读取视频文件的头部信息
    struct Video video;
    read_headerdata(input, video);
    // 计算每帧大小
    size_t frame_size = video.channels * video.height * video.width;

    // 打开输出文件并准备写入
    FILE *output = fopen(output_file, "wb");
    if (!output) {
        printf("Error opening output file.\n");
        fclose(input);
        return;
    }

    // 写入视频头部信息
    fwrite(&video.frames, sizeof(long), 1, output);
    fwrite(&video.channels, sizeof(unsigned char), 1, output);
    fwrite(&video.height, sizeof(unsigned char), 1, output);
    fwrite(&video.width, sizeof(unsigned char), 1, output);

    // 逐帧读取、反转并写入输出文件
    unsigned char *frame_data = (unsigned char *)malloc(frame_size);
    if (!frame_data) {
        printf("Memory allocation failed!\n");
        fclose(input);
        fclose(output);
        return;
    }

    // 反转视频帧
    for (long i = video.frames - 1; i >= 0; i--) {
        fseek(input, sizeof(long) + sizeof(unsigned char) * 3 + i * frame_size, SEEK_SET);
        fread(frame_data, 1, frame_size, input);
        fwrite(frame_data, 1, frame_size, output);
    }

    fclose(input);
    fclose(output);
    free(frame_data);

    printf("Video frames reversed and saved to %s\n", output_file);
}

// -S --Save video in memory, clip each channel to specified range, and write it to output file
void swap_channel_performance(const char *input_file, const char *output_file, unsigned char ch1, unsigned char ch2) {
    // 打开输入文件
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }

    // 初始化 Video 结构并读取头部数据
    struct Video video;
    read_headerdata(input, video);

    // 检查通道索引是否合法
    if (ch1 >= video.channels || ch2 >= video.channels) {
        printf("Error: Invalid channel indices.\n");
        fclose(input);
        return;
    }

    // 计算帧和通道大小
    size_t frame_size = video.channels * video.height * video.width;
    size_t channel_size = video.height * video.width;
    size_t total_size = video.frames * frame_size;

    // 分配数据缓冲区
    video.data = (unsigned char *)malloc(total_size);
    if (!video.data) {
        printf("Memory allocation failed!\n");
        fclose(input);
        return;
    }

    // 读取所有帧数据到缓冲区
    fread(video.data, 1, total_size, input);
    fclose(input);

    // 分配临时缓冲区，用于交换通道
    unsigned char *temp_channel = (unsigned char *)malloc(channel_size);
    if (!temp_channel) {
        printf("Memory allocation for temp channel failed!\n");
        free(video.data);
        return;
    }

    // 交换指定通道的数据
    for (long f = 0; f < video.frames; ++f) {
        unsigned char *frame_start = video.data + f * frame_size;
        unsigned char *channel1_data = frame_start + ch1 * channel_size;
        unsigned char *channel2_data = frame_start + ch2 * channel_size;

        // 一次性交换整个通道数据
        memcpy(temp_channel, channel1_data, channel_size);
        memcpy(channel1_data, channel2_data, channel_size);
        memcpy(channel2_data, temp_channel, channel_size);
    }

    // 写入修改后的数据到输出文件
    write_file(output_file, video, total_size);

    // 释放内存
    free(temp_channel);
    free(video.data);

    printf("Channels swapped and saved to %s\n", output_file);
}

// -M --Read each frame, clip specified channel to specified range, and write it to output file directly(one frame at a time)
void swap_channel_memory(const char *input_file, const char *output_file, unsigned char ch1, unsigned char ch2) {
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }

    struct Video video;
    read_headerdata(input, video);

    size_t channel_size = video.height * video.width;

    size_t frame_size = video.channels * video.height * video.width;
    unsigned char *frame_data = (unsigned char *)malloc(frame_size);
    if (!frame_data) {
        printf("Memory allocation failed!\n");
        fclose(input);
        return;
    }

    FILE *output = fopen(output_file, "wb");
    if (!output) {
        printf("Error opening output file.\n");
        fclose(input);
        free(frame_data);
        return;
    }

    fwrite(&video.frames, sizeof(long), 1, output);
    fwrite(&video.channels, sizeof(unsigned char), 1, output);
    fwrite(&video.height, sizeof(unsigned char), 1, output);
    fwrite(&video.width, sizeof(unsigned char), 1, output);

    unsigned char *temp_channel = (unsigned char *)malloc(channel_size);
    if (!temp_channel) {
        printf("Memory allocation for temp channel failed!\n");
        free(video.data);
        return;
    }
    for (long f = 0; f < video.frames; f++) {
        fread(frame_data, 1, frame_size, input);
        
        unsigned char *frame_start = video.data + f * frame_size;
        unsigned char *channel1_data = frame_start + ch1 * channel_size;
        unsigned char *channel2_data = frame_start + ch2 * channel_size;

        // 一次性交换整个通道数据
        memcpy(temp_channel, channel1_data, channel_size);
        memcpy(channel1_data, channel2_data, channel_size);
        memcpy(channel2_data, temp_channel, channel_size);
    }
    fclose(input);
    fclose(output);
    free(frame_data);
    printf("Channels swapped and saved to %s\n", output_file);
    free(temp_channel);}

// -S --Save video in memory, clip specified channel to specified range, and write it to output file
void clip_channel_performance(const char *input_file, const char *output_file, unsigned char channel, unsigned char min_val, unsigned char max_val) {
    // 打开输入文件
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }

    // 初始化 Video 结构并读取头部数据
    struct Video video;
    read_headerdata(input, video);

    // 检查通道索引是否合法
    if (channel > video.channels) {
        printf("Error: Invalid channel index.\n");
        fclose(input);
        return;
    }

    size_t frame_size = video.channels * video.height * video.width;
    size_t channel_size = video.height * video.width;
    size_t total_size = video.frames * frame_size;

    // 分配内存以存储视频数据
    video.data = (unsigned char *)malloc(total_size);
    if (!video.data) {
        printf("Memory allocation failed!\n");
        fclose(input);
        return;
    }

    // 读取所有视频帧数据
    fread(video.data, 1, total_size, input);
    fclose(input);

    // 处理裁剪
    for (long f = 0; f < video.frames; ++f) {
        unsigned char *frame_start = video.data + f * frame_size;
        unsigned char *channel_data = frame_start + channel * channel_size;

        // 对每个像素值进行裁剪
        for (long h = 0; h < video.height; ++h) {
            for (long w = 0; w < video.width; ++w) {
                size_t pixel_idx = h * video.width + w;
                if (channel_data[pixel_idx] < min_val) {
                    channel_data[pixel_idx] = min_val;  // 裁剪到最小值
                } else if (channel_data[pixel_idx] > max_val) {
                    channel_data[pixel_idx] = max_val;  // 裁剪到最大值
                }
            }
        }
    }

    write_file(output_file, video, total_size);
    free(video.data);

    printf("Channel %d clipped and saved to %s\n", channel, output_file);
}

// -M --Read each frame, clip specified channel to specified range, and write it to output file directly(one frame at a time)
void clip_channel_memory(const char *input_file, const char *output_file, unsigned char channel, unsigned char min_val, unsigned char max_val) {
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }

    struct Video video;
    read_headerdata(input, video);

    size_t frame_size = video.channels * video.height * video.width;
    size_t channel_size = video.height * video.width;

    FILE *output = fopen(output_file, "wb");
    if (!output) {
        printf("Error opening output file.\n");
        fclose(input);
        return;
    }

    // Write header to output file
    fwrite(&video.frames, sizeof(long), 1, output);
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

    // 逐帧处理
    for (long f = 0; f < video.frames; ++f) {
        fread(frame_data, 1, frame_size, input);  // 读取一个帧

        // 如果是指定的通道，则进行裁剪
        if (channel < video.channels) {
            unsigned char *channel_data = frame_data + channel * channel_size;

            // 裁剪指定通道的数据
            for (size_t i = 0; i < channel_size; ++i) {
                if (channel_data[i] < min_val) {
                    channel_data[i] = min_val;
                } else if (channel_data[i] > max_val) {
                    channel_data[i] = max_val;
                }
            }
        }

        fwrite(frame_data, 1, frame_size, output);  // 将处理后的帧写入输出文件
    }

    fclose(input);
    fclose(output);
    free(frame_data);

    printf("Video processed and saved to %s\n", output_file);
}

// -S --Save video in memory, scale specified channel by a factor, and write it to output file
void scale_channel_performance(const char *input_file, const char *output_file, unsigned char channel, float scale_factor) {
    // 打开输入文件
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }

    // 初始化 Video 结构并读取头部数据
    struct Video video;
    read_headerdata(input, video);

    // 检查通道索引是否合法
    if (channel > video.channels) {
        printf("Error: Invalid channel index.\n");
        fclose(input);
        return;
    }

    size_t frame_size = video.channels * video.height * video.width;
    size_t channel_size = video.height * video.width;
    size_t total_size = video.frames * frame_size;

    // 分配内存以存储视频数据
    video.data = (unsigned char *)malloc(total_size);
    if (!video.data) {
        printf("Memory allocation failed!\n");
        fclose(input);
        return;
    }

    // 读取所有视频帧数据
    fread(video.data, 1, total_size, input);
    fclose(input);

    // 处理裁剪
    for (long f = 0; f < video.frames; ++f) {
        unsigned char *frame_start = video.data + f * frame_size;
        unsigned char *channel_data = frame_start + channel * channel_size;

        // 对每个像素值进行裁剪
        for (long h = 0; h < video.height; ++h) {
            for (long w = 0; w < video.width; ++w) {
                size_t pixel_idx = h * video.width + w;
                channel_data[pixel_idx] = (unsigned char)(channel_data[pixel_idx] * scale_factor);
                if (channel_data[pixel_idx] > 255) {
                    printf("There are some pixel are over 255, already change to maximum 255.");
                    channel_data[pixel_idx] = 255; // 缩放像素值 
            }
        }
    }

    // 打开输出文件并准备写入
    write_file(output_file, video,total_size);
    free(video.data);

    printf("Channel %d clipped and saved to %s\n", channel, output_file);
}
}

// -M --Read each frame, scale specified channel by a factor, and write it to output file directly(one frame at a time)
void scale_channel_memory(const char *input_file, const char *output_file, unsigned char channel, float scale_factor) {
    FILE *input = fopen(input_file, "rb");
    if (!input) {
        printf("Error opening input file.\n");
        return;
    }

    struct Video video;
    read_headerdata(input, video);

    size_t frame_size = video.channels * video.height * video.width;
    size_t channel_size = video.height * video.width;

    FILE *output = fopen(output_file, "wb");
    if (!output) {
        printf("Error opening output file.\n");
        fclose(input);
        return;
    }

    // Write header to output file
    fwrite(&video.frames, sizeof(long), 1, output);
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

    // 逐帧处理
    for (long f = 0; f < video.frames; ++f) {
        fread(frame_data, 1, frame_size, input);  // 读取一个帧

        // 如果是指定的通道，则进行裁剪
        if (channel < video.channels) {
            unsigned char *channel_data = frame_data + channel * channel_size;

            // 裁剪指定通道的数据
            for (size_t i = 0; i < channel_size; ++i) {
                channel_data[i] = (unsigned char)(channel_data[i] * scale_factor);
                if (channel_data[i] > 255) {
                    printf("There are some pixel are over 255, already change to maximum 255.");
                    channel_data[i] = 255; // 缩放像素值 
            }
        }

        fwrite(frame_data, 1, frame_size, output);  // 将处理后的帧写入输出文件
    }

    fclose(input);
    fclose(output);
    free(frame_data);

    printf("Video processed and saved to %s\n", output_file);
}
}