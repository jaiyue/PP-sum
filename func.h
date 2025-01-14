#ifndef FUNC_H
#define FUNC_H

#include <stdlib.h>
//maximum values for channels, height and width
#define MAX_CH 3
#define MAX_H 128
#define MAX_W 128

struct Video{   //Header and Frames of Video
    long frames;
    unsigned char channels;
    unsigned char height;
    unsigned char width;
    unsigned char *data;
};

void read_headerdata(FILE *input, struct Video *video);
void write_file(const char *output_file, struct Video video,size_t total_size);
void reverse_video_performance(const char *input_file, const char *output_file);
void reverse_video_memory(const char *input_file, const char *output_file);
void swap_channel_performance(const char *input_file, const char *output_file, unsigned char ch1, unsigned char ch2);
void swap_channel_memory(const char *input_file, const char *output_file, unsigned char ch1, unsigned char ch2);
void clip_channel_performance(const char *input_file, const char *output_file, unsigned char channel, unsigned char min_val, unsigned char max_val);
void clip_channel_memory(const char *input_file, const char *output_file, unsigned char channel, unsigned char min_val, unsigned char max_val);
void scale_channel_performance(const char *input_file, const char *output_file, unsigned char channel, float scale_factor);
void scale_channel_memory(const char *input_file, const char *output_file, unsigned char channel, float scale_factor);

#endif