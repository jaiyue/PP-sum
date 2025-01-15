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
void write_header(FILE *output, const struct Video *video);
void reverse_video(const char *input_file, const char *output_file, int memory_free);
void swap_channels(const char *input_file, const char *output_file, unsigned char ch1, unsigned char ch2, int memory_free);
void clip_channel(const char *input_file, const char *output_file, unsigned char channel, unsigned char min_val, unsigned char max_val, int memory_free);
void scale_channel(const char *input_file, const char *output_file, unsigned char channel, float scale_factor, int memory_free);
#endif