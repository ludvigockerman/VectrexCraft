#include <vectrex.h>
#include "LUT.h"
#include "controller.h"

#define bool int
#define true 1
#define false 0

typedef struct {
    int x;
    int y;
} vec2;

typedef struct {
    int x;
    int y;
    int z;
} vec3;

const int edges[12][2] = {
    {0, 1},
    {1, 2},
    {2, 3},
    {3, 0},
    {4, 5},
    {5, 6},
    {6, 7},
    {7, 4},
    {0, 4},
    {1, 5},
    {2, 6},
    {3, 7}
};

bool tf[8][3] = {
    {false, false, false},
    {true,  false, false},
    {true,  true,  false},
    {false, true,  false},
    {false, false, true },
    {true,  false, true },
    {true,  true,  true },
    {false, true,  true }  
};

vec3 playerposition;
vec2 playerrotation;
long decimalx = 0;
long decimalz = 0;
int speed = 2;
int sensitivity = 10;

// ---------------------------------------------------------
// Help functions
// ---------------------------------------------------------

int SearchThroughArray(vec2* list, int length, vec2 line) {
    for(int i = 0; i < length; i++) {
        if (list[i].x == line.x && list[i].y == line.y) {
            return 1;
        }
    }
    return 0;
}

void project_point(vec3 p, vec2* out) {
    long angle = (long)((playerrotation.x + 128) & 0xff);
    int sinv = sin_table[angle];
    int cosv = sin_table[(angle + 64) & 0xff];
    
    long dz = (long)(p.z - playerposition.z);
    long dy = (long)(p.y - playerposition.y);
    long dx = (long)(p.x - playerposition.x);
    
    long rz = (long)(dx * sinv + dz * cosv) / 127;
    long rx = (long)(dx * cosv - dz * sinv) / 127;
    
    if(rz <= 0) {
        out->x = (int)-128;
        out->y = (int)-128;
        return;
    }
    
    long fx = (rx * 50) / rz;
    long fy = (dy * 50) / rz;
    
    if (fx > 127 || fx < -127 || fy > 127 || fy < -127) {
        out->x = -128;
        out->y = -128;
        return;
    }
    
    out->x = (int)fx;
    out->y = (int)fy;
}

// ---------------------------------------------------------
// Drawing functions
// ---------------------------------------------------------

void drawcube(vec3* cube) {
    vec2 pts[8];
    for(int i = 0; i < 8; i++) {
        if (cube[i].x == -128) {
            pts[i] = (vec2){ -128, -128 };
        } else {
            project_point(cube[i], &pts[i]);
        }
    }

    int currentposx;
    int currentposy;
    int newposx;
    int newposy;
    int drawnlines = 0;

    Reset0Ref();

    for(int i = 0; i < 12; i++) {
        vec2 p1 = pts[edges[i][0]];
        vec2 p2 = pts[edges[i][1]];
        
        if (p1.x == -128 || p2.x == -128) {
            continue;
        }
        
        if (drawnlines == 0 || drawnlines == 6) {
            currentposx = p1.x;
            currentposy = p1.y;
            Reset0Ref();
            Moveto_d(currentposy, currentposx);
        } else {
            newposx = p1.x;
            newposy = p1.y;
            Moveto_d(newposy - currentposy, newposx - currentposx);
            currentposx = newposx;
            currentposy = newposy;
        }
        
        int deltax = p2.x - p1.x;
        int deltay = p2.y - p1.y;
        Draw_Line_d(deltay, deltax);
        drawnlines++;
        
        currentposx += deltax;
        currentposy += deltay;
    }
}

void createcubeat(vec3 cubepos) {
    vec3 out[8];
    out[0] = (vec3){ cubepos.x - 10, cubepos.y - 10, cubepos.z - 10 };
    out[1] = (vec3){ cubepos.x,      cubepos.y - 10, cubepos.z - 10 };
    out[2] = (vec3){ cubepos.x,      cubepos.y,      cubepos.z - 10 };
    out[3] = (vec3){ cubepos.x - 10, cubepos.y,      cubepos.z - 10 };
    out[4] = (vec3){ cubepos.x - 10, cubepos.y - 10, cubepos.z };
    out[5] = (vec3){ cubepos.x,      cubepos.y - 10, cubepos.z };
    out[6] = (vec3){ cubepos.x,      cubepos.y,      cubepos.z };
    out[7] = (vec3){ cubepos.x - 10, cubepos.y,      cubepos.z };

    for (int i = 0; i < 8; i++) {
        bool x = true;
        bool y = true;
        bool z = true;
        
        if (playerposition.x >= out[i].x) x = false;
        if (playerposition.y >= out[i].y) y = false;
        if (playerposition.z >= out[i].z) z = false;
        
        if (x == tf[i][0] && y == tf[i][1] && z == tf[i][2]) {
            out[i] = (vec3){ -128, -128, -128 };
        }
    }
    drawcube(&out[0]);
}

// ---------------------------------------------------------
// Main
// ---------------------------------------------------------

int main(void) {
    playerposition = (vec3){ 0, 0, 0 };
    playerrotation = (vec2){ 0, 0 };

    while(1) {
        Wait_Recal();
        Intensity_a(0x5f);

        createcubeat((vec3){-10, 0, 30});
        createcubeat((vec3){-10, -10, 30});
        createcubeat((vec3){-10, 10, 30});
        createcubeat((vec3){-10, 10, 40});
        createcubeat((vec3){-10, 10, 20});

        Read_Btns();
        check_joysticks();

        if (joystick_1_x() > 0) playerrotation.x += sensitivity;
        if (joystick_1_x() < 0) playerrotation.x -= sensitivity;
        if (joystick_1_y() > 0) playerrotation.y += sensitivity;
        if (joystick_1_y() < 0) playerrotation.y -= sensitivity;

        if (Vec_Btn_State & 0b00000001) {
            long move_angle = (playerrotation.x + 128) & 0xff;
            long s = (long)sin_table[move_angle];
            long c = (long)sin_table[(move_angle + 64) & 0xff];
            
            decimalx += (speed * s);
            decimalz += (speed * c);
            
            while (decimalx > 127)  { decimalx -= 127; playerposition.x += 1; }
            while (decimalz > 127)  { decimalz -= 127; playerposition.z += 1; }
            while (decimalx < -127) { decimalx += 127; playerposition.x -= 1; }
            while (decimalz < -127) { decimalz += 127; playerposition.z -= 1; }
        }

        if (Vec_Btn_State & 0b00000010) { /* Button 2 */ }
        if (Vec_Btn_State & 0b00000100) { /* Button 3 */ }
        if (Vec_Btn_State & 0b00001000) { /* Button 4 */ }
    }
    
    return 0;
}