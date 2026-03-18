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

#define MAX_PIPES 7

#define PIPE_SPAWN_X SCREEN_WIDTH - PIPE_WIDTH

#define PIPE_SPAWN_INTERVAL_HIGH 0b000010001111000110
#define PIPE_SPAWN_INTERVAL_LOW 0b1000110000000000

// global variables
volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
volatile int *timer_status   = (int *)0xFF202000;
volatile int *timer_control  = (int *)0xFF202004;
volatile int *timer_periodl  = (int *)0xFF202008;
volatile int *timer_periodh  = (int *)0xFF20200C;

volatile int pixel_buffer_start; 
short int Buffer1[240][512];
short int Buffer2[240][512];
int pipe_pos[4][3];
int next_pipe_index = 0;
int num_pipes_spawned = 0;

// function declarations
void wait_for_vsync();
bool timer_done();
void plot_pixel(int x, int y, short int color);
void clear_screen();

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
    for(int y = 0; y < SCREEN_HEIGHT; y++){
        for(int x = 0; x < SCREEN_WIDTH; x++){
            plot_pixel(x, y, 0);
        }
    }
}

void plot_pixel(int x, int y, short int color)
{
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