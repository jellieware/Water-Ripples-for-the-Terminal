#include <ncurses.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MAX_DROPS 45
#define FPS 20
#define DELAY (1000000 / FPS)

typedef struct {
    double x, y;
    int start_frame;
    int active;
} Drop;

// 256-color blue gradient (from dark to light)
int blue_shades[] = {17, 18, 19, 20, 21, 27, 33, 39, 45, 51};
#define NUM_SHADES 10

void init_colors() {
    start_color();

    for (int i = 0; i < NUM_SHADES; i++) {
    init_pair(1, 18, 18);
    wbkgd(stdscr, COLOR_PAIR(1));
        // Initialize pairs with background/foreground the same for solid blocks
        init_pair(i + 1, blue_shades[i], blue_shades[i]);
    }
}

int main() {
    initscr();
   
    noecho();
    curs_set(0);
    timeout(0);
    init_colors();

    Drop drops[MAX_DROPS];
    for (int i = 0; i < MAX_DROPS; i++) drops[i].active = 0;

    int frame = 0;
    int rows, cols;

    while (getch() == ERR) {
        getmaxyx(stdscr, rows, cols);
        erase();

        // Randomly spawn new drops
        if (rand() % 5 == 0) {
            for (int i = 0; i < MAX_DROPS; i++) {
                if (!drops[i].active) {
                    drops[i].active = 1;
                    drops[i].x = rand() % cols;
                    drops[i].y = rand() % rows;
                    drops[i].start_frame = frame;
                    break;
                }
            }
        }

        // Calculate ripple intensity for each cell
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < cols; x += 2) {
                double total_intensity = 0;

                for (int i = 0; i < MAX_DROPS; i++) {
                    if (!drops[i].active) continue;

                    int age = frame - drops[i].start_frame;
                    // Distance calculation (x is halved because we use 2 blocks per cell)
                    double dx = (x - drops[i].x) * 0.5; 
                    double dy = (y - drops[i].y);
                    double dist = sqrt(dx*dx + dy*dy);

                    // Physics: Wave speed and wavelength
                    double speed = 0.8;
                    double wavelength = 2.5;
                    double phase = dist - (age * speed);

                    if (phase < 0 && phase > -12) {
                        // Sine wave for multiple ridges
                        double wave = (sin(phase * (2 * M_PI / wavelength)) + 1) / 2;
                        // Fade over distance and time
                        double decay = (1.0 - (dist / 25.0)) * (1.0 - (age / 60.0));
                        if (decay > 0) total_intensity += wave * decay;
                    }

                    // Deactivate old drops
                    if (age > 60) drops[i].active = 0;
                }

                if (total_intensity > 0.05) {
                    int color_idx = (int)(total_intensity * (NUM_SHADES - 1));
                    if (color_idx >= NUM_SHADES) color_idx = NUM_SHADES - 1;
                    
                    attron(COLOR_PAIR(color_idx + 1));
                    mvaddstr(y, x, "  "); // Two solid full blocks via bg color
                    attroff(COLOR_PAIR(color_idx + 1));
                }
            }
        }

        refresh();
        usleep(DELAY);
        frame++;
    }

    endwin();
    return 0;
}
