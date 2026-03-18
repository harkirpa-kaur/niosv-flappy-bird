#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// constants
#define SCREEN_WIDTH 319
#define SCREEN_HEIGHT 239

#define PIPE_MIN_HEIGHT 20
#define PIPE_MAX_HEIGHT 120
#define PIPE_WIDTH 20
#define PIPE_COLOUR 0x0ff0

#define MAX_PIPES 10

#define PIPE_SPAWN_X SCREEN_WIDTH - PIPE_WIDTH

#define PIPE_SPAWN_INTERVAL_HIGH 0b0000101111101011
#define PIPE_SPAWN_INTERVAL_LOW 0b1100001000000000

#define BIRD_HEIGHT 14
#define BIRD_WIDTH 20

// array size is 560
static const uint16_t BIRD[]  = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x62e2, 0x9424, 0x9444, 0x7b63, 0x20e3, 0x9492, 0x9492, 0x2945, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x5242, 0x5a82, 0xb503, 0xfec4, 0xe624, 0x9465, 0x736d, 0xffff, 0xffdf, 0x7bcf, 0x2945, 0x0020, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0840, 0x62e3, 0xdde4, 0xfec4, 0xfe42, 0xfe00, 0xac00, 0x2124, 0xc638, 0xffff, 0xffdf, 0xa534, 0x73ae, 0x4228, 0x0020, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0020, 0x3140, 0xd5e4, 0xfe42, 0xfe01, 0xfe00, 0xfe00, 0xac00, 0x18e3, 0xa514, 0xffff, 0xf7be, 0x31a6, 0xc618, 0x6b6d, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0xdd40, 0xfe00, 0xfe00, 0xfe00, 0xfe00, 0xfe00, 0xc4a0, 0x5222, 0x7bef, 0xe71c, 0xf7be, 0x6b6d, 0xc638, 0x5acb, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0xd4e0, 0xed80, 0xed80, 0xed80, 0xf5c0, 0xfe00, 0xfde0, 0xcce0, 0x3983, 0x94b2, 0xe71c, 0xe73c, 0xbdf7, 0x0820, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x18c1, 0x2943, 0x2965, 0x2965, 0x2965, 0x2964, 0x8b61, 0xe560, 0xfe00, 0xf5c0, 0xbc60, 0x2861, 0x2861, 0x2861, 0x2861, 0x2841, 0x2841, 0x2041, 0x0800, 
  0x0000, 0x2943, 0xe6f2, 0xe73b, 0xe73c, 0xe73c, 0xe6d6, 0x7347, 0x72a0, 0xf5c0, 0xd4e0, 0x4921, 0xd1c5, 0xd1c5, 0xd1c5, 0xd1c5, 0xd1c5, 0xd1c5, 0x8923, 0x0000, 
  0x0000, 0x2945, 0xffff, 0xffff, 0xffff, 0xf7be, 0x738d, 0x92c0, 0xfca0, 0xb360, 0x38a1, 0xb184, 0x1020, 0x1820, 0x1820, 0x1820, 0x1820, 0x1020, 0x0800, 0x0000, 
  0x0000, 0x2945, 0xf7be, 0xffde, 0xf797, 0x5ac9, 0x8ac2, 0xec60, 0xfcc0, 0xf460, 0xb340, 0x50e1, 0xc1a5, 0xc1a5, 0xc1a5, 0xc1a5, 0xc1a5, 0x50a2, 0x0000, 0x0000, 
  0x0000, 0x18c3, 0x2104, 0x2104, 0x2102, 0x0840, 0x92a0, 0xfca0, 0xfca0, 0xfca0, 0xf480, 0xc380, 0x2040, 0x2040, 0x2040, 0x2040, 0x1840, 0x1840, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4120, 0x4960, 0x5180, 0x5180, 0x5160, 0x4960, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

// global variables
volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
volatile int *timer_status   = (int *)0xFF202000;
volatile int *timer_control  = (int *)0xFF202004;
volatile int *timer_periodl  = (int *)0xFF202008;
volatile int *timer_periodh  = (int *)0xFF20200C;

volatile int pixel_buffer_start; 
short int Buffer1[240][512];
short int Buffer2[240][512];
int pipe_pos[MAX_PIPES][3];
int next_pipe_index = 0;
int num_pipes_spawned = 0;

// function declarations
void wait_for_vsync();
bool timer_done();
void plot_pixel(int x, int y, short int color);
void clear_screen();
void draw_bird(int x, int y);

int main(void)
{
    srand(time(NULL));

    // clear timeout bit
    *timer_status = 0;

    // load start value
    *timer_periodl = PIPE_SPAWN_INTERVAL_LOW;
    *timer_periodh = PIPE_SPAWN_INTERVAL_HIGH;

    // START = bit 2, CONT = bit 1
    *timer_control = (1 << 2) | (1 << 1);

    // set front buffer
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1;
    wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();

    // set back buffer
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);

    spawn_pipe();

    while(1)
    {
        clear_screen();
        

        // update positions
        for(int i = 0; i < num_pipes_spawned; i++){
            pipe_pos[i][0]--;
        }

        if (timer_done()){
            spawn_pipe();
        }
        draw_pipe();
        draw_bird(50, 100);

        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }
}

bool timer_done() {
    if ((*timer_status) & 1) {
        *timer_status = 0;   // clear TO
        return true;
    }
    return false;
}

void spawn_pipe()
{   
    //randomize top and bottom pipes
    int pipe_spawn_y = (rand() % 2) ? 0 : SCREEN_HEIGHT;

    // pick a random pipe height between PIPE_MIN_HEIGHT and PIPE_MAX_HEIGHT
    int pipe_height = (rand() % (PIPE_MAX_HEIGHT - PIPE_MIN_HEIGHT + 1)) + PIPE_MIN_HEIGHT;

    //if pipe is at the top draw downward, otherwise draw updward
    int y_dir = pipe_spawn_y == 0 ? 1 : -1;

    //draw pipe
    for(int i = 0; i < pipe_height; i ++){
        for(int j = 0; j < PIPE_WIDTH; j++){
            plot_pixel(PIPE_SPAWN_X + j, pipe_spawn_y + i*y_dir, PIPE_COLOUR);
        }
    }
    pipe_pos[next_pipe_index][0] = PIPE_SPAWN_X;
    pipe_pos[next_pipe_index][1] = pipe_spawn_y;
    pipe_pos[next_pipe_index][2] = pipe_height;

    if (num_pipes_spawned < MAX_PIPES) {
        num_pipes_spawned++;
    }
    next_pipe_index = (next_pipe_index + 1) % MAX_PIPES;
}

void draw_pipe(){
    // iterate through all pipes and draw them
    for (int pipe = 0 ; pipe < num_pipes_spawned ; pipe++){
        int y_dir = pipe_pos[pipe][1] == 0 ? 1 : -1;

        for(int i = 0; i < pipe_pos[pipe][2]; i++){
            for(int j = 0; j < PIPE_WIDTH; j++){
                plot_pixel(pipe_pos[pipe][0] + j, pipe_pos[pipe][1] + i*y_dir, PIPE_COLOUR);
            }
        }
    }
}

void clear_screen()
{
    for (int pipe = 0 ; pipe < num_pipes_spawned ; pipe++){
        int y_dir = pipe_pos[pipe][1] == 0 ? 1 : -1;

        for(int i = 0; i < pipe_pos[pipe][2]; i++){
            for(int j = 0; j <= PIPE_WIDTH; j++){
                plot_pixel(pipe_pos[pipe][0] + j, pipe_pos[pipe][1] + i*y_dir, 0x0000);
            }
        }
    }
}

void plot_pixel(int x, int y, short int color)
{
    if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT)
        return;

    volatile short int *one_pixel_address;

    one_pixel_address = (short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *one_pixel_address = color;
}

void wait_for_vsync()
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    int status;

    *pixel_ctrl_ptr = 1;

    status = *(pixel_ctrl_ptr + 3);
    while((status & 0x01) != 0)
        status = *(pixel_ctrl_ptr + 3);
}

void draw_bird(int x, int y)
{
    for (int row = 0; row < BIRD_HEIGHT; row++) {
        for (int col = 0; col < BIRD_WIDTH; col++) {

            uint16_t color = BIRD[row * BIRD_WIDTH + col];

            // skip black pixels (transparent)
            if (color != 0x0000) {
                plot_pixel(x + col, y + row, color);
            }
        }
    }
}