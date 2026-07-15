#include <vectrex/bios.h>
#include "LUT.h"
#include "terminal.h"

#define bool char
#define true 1
#define false 0

#define SHOW_POSTITION 1
#define SHOW_ROTATION 1
#define MAX_VERTICES 64
#define MAX_QUADS 32
#define BLOCK_SIZE 8

typedef struct {
    int x;
    int y;
} vec2;

typedef struct {
    char x;
    char y;
} c_vec2;

typedef struct {
    int x;
    int y;
    int z;
} vec3;

typedef struct {
    unsigned char v0;
    unsigned char v1;
    unsigned char v2;
    unsigned char v3;
} quad;

char world[3][3][3] = { // x, y, z
    { {0, 0, 0}, {0, 0, 0}, {0, 0, 0} },
    { {0, 0, 0}, {0, 0, 0}, {0, 0, 0} },
    { {0, 0, 0}, {0, 0, 0}, {0, 0, 0} }
};

//quad quads[MAX_QUADS];
//vec2 projected[MAX_VERTICES];

vec3 playerposition;
vec2 playerrotation;
long decimalx = 0;
long decimalz = 0;
//int sensitivity = 10;

signed int sinv, cosv, sinu, cosu;

// ---------------------------------------------------------
// Help functions
// ---------------------------------------------------------

int mul8(char a, char b);

long mul16x8(unsigned int a, unsigned char b)
{
    unsigned char low = (unsigned char)a;
    unsigned char high = (unsigned char)(a >> 8);

    unsigned int low_result = mul8(low, b);
    unsigned int high_result = mul8(high, b);

    long result;

    result = (long)low_result;
    result += (long)high_result << 8;

    return result;
}

long mul16x16(unsigned int a, unsigned int b){
    unsigned char low = (unsigned char)a;
    unsigned char high = (unsigned char)(a >> 8);

    unsigned long low_result = mul16x8((unsigned int)b, (unsigned char)low);
    unsigned long high_result = mul16x8((unsigned int)b, (unsigned char)high);

    long result = (long)low_result + ((long)high_result << 8);

    return result;
}

long mul8_u(long a, long b) {
    return (long)mul8((unsigned char)a, (unsigned char)b);
}

long mul16x8_u(long a, long b){
    return (long)mul16x8((unsigned int)a, (unsigned char)b);
}

long mul16x16_u(long a, long b){
    return (long)mul16x16((unsigned int)a, (unsigned int)b);
}

unsigned long mul_unsigned_neg(long a, long b, long (*func)(long, long), char *neg){
    if (a < 0) {
        a = -a;
        *neg ^= 1;
    }

    if (b < 0) {
        b = -b;
        *neg ^= 1;
    }

    long result = func(a, b);

    return result;
}

long mul_signed(long a, long b, long (*func)(long, long)){
    char neg = 0;

    if (a < 0) {
        a = -a;
        neg ^= 1;
    }

    if (b < 0) {
        b = -b;
        neg ^= 1;
    }

    long result = func(a, b);

    if (neg) {
        result = -result;
    }

    return result;
}

int division(char a, int bfour){
    if (bfour < 16) return 0;

    int out = (a * (int)(recip_table[bfour] >> 10));
    if ((a < 0 && bfour > 0) || (a > 0 && bfour < 0)) {
        out = -out;
    }
    return out;
}

void int_to_string(int n, char *str)
{
    int i = 0;
    int neg = 0;

    if (n < 0)
    {
        neg = 1;
        n = -n;
    }

    if (n == 0)
    {
        str[0] = '0';
        str[1] = 0;
        return;
    }

    while (n > 0)
    {
        str[i++] = (char)((n % 10) + '0');
        n /= 10;
    }

    if (neg)
        str[i++] = '-';

    str[i] = 0;

    int a = 0;
    int b = i - 1;

    while (a < b)
    {
        char t = str[a];
        str[a] = str[b];
        str[b] = t;
        a++;
        b--;
    }
}

bool SearchThroughArray(vec2* list, char length, vec2 line) {
    for(char i = 0; i < length; i++) {
        if (list[i].x == line.x && list[i].y == line.y) {
            return 1;
        }
    }
    return 0;
}

void build_position_string(char *buf, int x, int y, int z)
{
    int pos = 0;
    char cstr[12];
    int i;

    buf[pos++] = 'X';
    buf[pos++] = ':';
    int_to_string(x, cstr);

    i = 0;
    while (cstr[i] != 0){
        buf[pos++] = cstr[i];
        i++;
    }

    buf[pos++] = ' ';
    buf[pos++] = 'Y';
    buf[pos++] = ':';
    int_to_string(y, cstr);

    i = 0;
    while (cstr[i] != 0){
        buf[pos++] = cstr[i];
        i++;
    }

    buf[pos++] = ' ';
    buf[pos++] = 'Z';
    buf[pos++] = ':';
    int_to_string(z, cstr);

    i = 0;
    while (cstr[i] != 0){
        buf[pos++] = cstr[i];
        i++;
    }

    buf[pos] = 0;
}

void build_rotation_string(char *buf, int x, int y)
{
    int pos = 0;
    char cstr[12];
    int i;

    buf[pos++] = 'R';
    buf[pos++] = ' ';
    buf[pos++] = 'X';
    buf[pos++] = ':';
    int_to_string(x, (char*)cstr);

    i = 0;
    while (cstr[i] != 0){
        buf[pos++] = cstr[i];
        i++;
    }

    buf[pos++] = ' ';
    buf[pos++] = 'Y';
    buf[pos++] = ':';
    int_to_string(y, cstr);

    i = 0;
    while (cstr[i] != 0){
        buf[pos++] = cstr[i];
        i++;
    }

    buf[pos] = 0;
}

void GetSin(int* out, unsigned char angle)
{
    if(angle < 64){
        *out = sin_table[angle];
    }
    else if (angle < 128){
        *out = sin_table[127-angle];
    }
    else if (angle < 192){
        *out = (int)0 - sin_table[angle-128];
    }
    else{
        *out = (int)0 - sin_table[255-angle];
    }
}

void UpdateDirections()
{
    // Rotation in x is called v and rotation in y is called u
    unsigned char anglev = (unsigned char)(playerrotation.x + 128);
    unsigned char angleu = (unsigned char)(playerrotation.y);
    GetSin(&sinv, anglev);
    GetSin(&cosv, anglev+64);

    GetSin(&sinu, angleu);
    GetSin(&cosu, angleu+64);
}

void MovePlayer(signed char dist, unsigned char move_angle){
    decimalz += ((long)dist * cosv);
    decimalx += ((long)dist * sinv);
    
    while (decimalz > 255)  { decimalz -= 255; playerposition.z += 1; }
    while (decimalx > 255)  { decimalx -= 255; playerposition.x += 1; }
    while (decimalx < -255) { decimalx += 255; playerposition.x -= 1; }
    while (decimalz < -255) { decimalz += 255; playerposition.z -= 1; }
}

void project_point(vec3 p, vec2* out) {
    //terminal_print((char*)"PROJECTING POINT");
    
    int dz = (int)(p.z - playerposition.z);
    int dy = (int)(p.y - playerposition.y);
    int dx = (int)(p.x - playerposition.x);
    
    if (dx >= 90 || dy >= 90 || dz >= 90 || dx <= -90 || dy <= -90 || dz <= -90){
        out->x = -128;
        out->y = -128;
        return;
    }

    char c_dz = (char)dz;
    char c_dy = (char)dy;
    char c_dx = (char)dx;
    
    //long r1z = (long)(dx * sinv + dz * cosv); // Bit shifting >> 8 (same as dividing by 256)
    //long r1x = (long)(dx * cosv - dz * sinv);

    long r1z = ((long)mul_signed(c_dx, sinv, mul8_u)) +
            ((long)mul_signed(c_dz, cosv, mul8_u)); //17 bit signed

    long r1x = ((long)mul_signed(c_dx, cosv, mul8_u)) -
            ((long)mul_signed(c_dz, sinv, mul8_u)); //17 bit signed

    //long r2y = (long)((long)(dy << 8) * cosu - r1z * sinu);
    //long r2z = (long)((long)(dy << 8) * sinu + r1z * cosu);

    long r2y = (long)((mul_signed(dy, cosu, mul8_u) << 8) - mul_signed(r1z, sinu, mul16x8_u)); //25 bit signed
    long r2z = (long)((mul_signed(dy, sinu, mul8_u) << 8) + mul_signed(r1z, cosu, mul16x8_u)); //25 bit signed

    //int r1xShift = (int)(r1x >> 8);
    //int r2yShift = (int)(r2y >> 16);
    int r2zShift = (int)(r2z >> 16);
    
    if(r2zShift <= 0) {
        out->x = (signed char)-128;
        out->y = (signed char)-128;
        return;
    }

    //if (r1xShift >= 127 || r2yShift >= 127 || r2zShift >= 127 || r1xShift <= -127 || r2yShift <= -127 || r2zShift <= -127){
    //    out->x = -128;
    //    out->y = -128;
    //    return;
    //}

    char neg_fx = 0;
    unsigned long fx_o = mul_unsigned_neg(r1x, (long)recip_table[(int)(r2z >> 12)], mul16x16_u, &neg_fx);
    signed int fx = (signed int)(fx_o >> 18);

    char neg_fy = 0;
    unsigned long fy_o = mul_unsigned_neg((r2y >> 8), (long)recip_table[(int)(r2z >> 12)], mul16x16_u, &neg_fy);
    signed int fy = (signed int)(fy_o >> 18);

    if (fx > 127 || fy > 127) {
        out->x = -128;
        out->y = -128;
        return;
    }

    if (neg_fx) {
        fx = -fx;
    }
    if (neg_fy) {
        fy = -fy;
    }
    
    //long fx = ((long)r1x * (long)recip_table[(int)(r2z >> 12)]) >> 18;
    //long fy = ((long)(r2y >> 8) * ((long)recip_table[(int)(r2z >> 12)])) >> 18;
    
    out->x = (signed char)fx;
    out->y = (signed char)fy;
}

// ---------------------------------------------------------
// Drawing functions
// ---------------------------------------------------------

void drawcube(vec3* cube, signed char edges[12][2]) {
    //terminal_print((char*)"DRAWING CUBE");

    vec2 pts[8];
    for(unsigned char i = 0; i < 8; i++) {
        if (cube[i].x == -128) {
            pts[i].x = -128;
            pts[i].y = -128;
        } else {
            project_point(cube[i], &pts[i]);
        }
    }

    signed char currentposx;
    signed char currentposy;
    signed char newposx;
    signed char newposy;
    currentposx = 0;
    currentposy = 0;


    reset_beam();

    //Moveto_d(currentposy, currentposx);

    for(unsigned char i = 0; i < 12; i++) {
        if (edges[i][0] == -1) continue;
        
        vec2 p1 = pts[edges[i][0]];
        vec2 p2 = pts[edges[i][1]];
        
        if (p1.x == -128 || p2.x == -128) continue;
            
        newposx = p1.x;
        newposy = p1.y;
        
        signed char move_dy = newposy - currentposy;
        signed char move_dx = newposx - currentposx;
        
        if(move_dx != 0 || move_dy != 0){
            move(move_dy, move_dx);
            //move(newposx, newposy);
            currentposx = newposx;
            currentposy = newposy;
        }
        
        signed char deltax = p2.x - p1.x;
        signed char deltay = p2.y - p1.y;
        //terminal_print((char*)"DRAWING LINE");
        line(deltay, deltax);
        
        currentposx += deltax;
        currentposy += deltay;
    }
}

void createcubeat(vec3 cubepos, signed char edges[12][2], vec3 *out) { //edges is an array of 12 pairs of indices, out is an array of 8 vec3s
    //terminal_print((char*)"CREATECUBEAT");
    out[0].x = cubepos.x - BLOCK_SIZE;
    out[0].y = cubepos.y - BLOCK_SIZE;
    out[0].z = cubepos.z - BLOCK_SIZE;

    out[1].x = cubepos.x;
    out[1].y = cubepos.y - BLOCK_SIZE;
    out[1].z = cubepos.z - BLOCK_SIZE;

    out[2].x = cubepos.x;
    out[2].y = cubepos.y;
    out[2].z = cubepos.z - BLOCK_SIZE;

    out[3].x = cubepos.x - BLOCK_SIZE;
    out[3].y = cubepos.y;
    out[3].z = cubepos.z - BLOCK_SIZE;

    out[4].x = cubepos.x - BLOCK_SIZE;
    out[4].y = cubepos.y - BLOCK_SIZE;
    out[4].z = cubepos.z;

    out[5].x = cubepos.x;
    out[5].y = cubepos.y - BLOCK_SIZE;
    out[5].z = cubepos.z;

    out[6].x = cubepos.x;
    out[6].y = cubepos.y;
    out[6].z = cubepos.z;

    out[7].x = cubepos.x - BLOCK_SIZE;
    out[7].y = cubepos.y;
    out[7].z = cubepos.z;


    char x = cubepos.x >> 3;
    char y = cubepos.y >> 3;
    char z = cubepos.z >> 3;

    // True means a block
    bool upBlock   = ( (y < 2) && world[x][y+1][z] );
    bool downBlock = ( (y > 0) && world[x][y-1][z] );
    
    bool leftBlock  = ( (x > 0) && world[x-1][y][z] );
    bool rightBlock = ( (x < 2) && world[x+1][y][z] );
    
    bool frontBlock = ( (z < 2) && world[x][y][z+1] );
    bool backBlock  = ( (z > 0) && world[x][y][z-1] );

    // True means not visible
    bool upInvis   = (playerposition.y <= cubepos.y);
    bool downInvis = (playerposition.y >= cubepos.y - BLOCK_SIZE);
    
    bool leftInvis  = (playerposition.x >= cubepos.x - BLOCK_SIZE);
    bool rightInvis = (playerposition.x <= cubepos.x);
    
    bool frontInvis = (playerposition.z <= cubepos.z);
    bool backInvis  = (playerposition.z >= cubepos.z - BLOCK_SIZE);


    bool up   = (upBlock || upInvis);
    bool down = (downBlock || downInvis);
    
    bool left = (leftBlock || leftInvis);
    bool right = (rightBlock || rightInvis);
    
    bool front = (frontBlock || frontInvis);
    bool back = (backBlock || backInvis);
    
        
    if (back && down && left) {
        out[0].x = -128;
        out[0].y = -128;
        out[0].z = -128;
    }

    if (back && down && right) {
        out[1].x = -128;
        out[1].y = -128;
        out[1].z = -128;
    }

    if (back && up && right) {
        out[2].x = -128;
        out[2].y = -128;
        out[2].z = -128;
    }

    if (back && up && left) {
        out[3].x = -128;
        out[3].y = -128;
        out[3].z = -128;
    }

    if (front && down && left) {
        out[4].x = -128;
        out[4].y = -128;
        out[4].z = -128;
    }

    if (front && down && right) {
        out[5].x = -128;
        out[5].y = -128;
        out[5].z = -128;
    }

    if (front && up && right) {
        out[6].x = -128;
        out[6].y = -128;
        out[6].z = -128;
    }

    if (front && up && left) {
        out[7].x = -128;
        out[7].y = -128;
        out[7].z = -128;
    }

    edges[0][0] = 0; edges[0][1] = 1;
    edges[1][0] = 1; edges[1][1] = 2;
    edges[2][0] = 2; edges[2][1] = 3;
    edges[3][0] = 3; edges[3][1] = 0;
    edges[4][0] = 4; edges[4][1] = 5;
    edges[5][0] = 5; edges[5][1] = 6;
    edges[6][0] = 6; edges[6][1] = 7;
    edges[7][0] = 7; edges[7][1] = 4;
    edges[8][0] = 0; edges[8][1] = 4;
    edges[9][0] = 1; edges[9][1] = 5;
    edges[10][0] = 2; edges[10][1] = 6;
    edges[11][0] = 3; edges[11][1] = 7;    
   
    if(backBlock){
        edges[0][0] = -1; edges[0][1] = -1;
        edges[1][0] = -1; edges[1][1] = -1;
        edges[2][0] = -1; edges[2][1] = -1;
        edges[3][0] = -1; edges[3][1] = -1;
    }
    
    if(frontBlock){
        edges[4][0] = -1; edges[4][1] = -1;
        edges[5][0] = -1; edges[5][1] = -1;
        edges[6][0] = -1; edges[6][1] = -1;
        edges[7][0] = -1; edges[7][1] = -1;
    }

    if( downBlock ){
        edges[0][0] = -1; edges[0][1] = -1;
        edges[4][0] = -1; edges[4][1] = -1;
        edges[8][0] = -1; edges[8][1] = -1;
        edges[9][0] = -1; edges[9][1] = -1;
    }
}

void prepare_vertices() {
    //terminal_print((char*)"PREPARING VERTICES");
    c_vec2 pp[6];
    //vec3 pos1, pos2, pos3, pos4, pos5, pos6;
    //pos1 = {10, 0, 0};
    //project_point(pos, &pp[0]);
    //pos2 = (vec3){-10, 0, 0};
    //project_point(pos, &pp[1]);
//
    //pos3 = (vec3){0, 10, 0};
    //project_point(pos, &pp[2]);
    //pos4 = (vec3){0, -10, 0};
    //project_point(pos, &pp[3]);
//
    //pos5 = (vec3){0, 0, 10};
    //project_point(pos, &pp[4]);
    //pos6 = (vec3){0, 0, -10};
    //project_point(pos, &pp[5]);
}

// ---------------------------------------------------------
// misc
// ---------------------------------------------------------

void cast_ray_down_from_player(char dist, vec3* out) {

    //signed char dx = (long)(cosv * dist) >> 8;
    //signed char dz = (long)(sinv * dist) >> 8;
//
    //out->x = origin.x + dx;
    //out->y = origin.y;
    //out->z = origin.z + dz;
}

// idé jag sparar ett antal block framåt och ifall man tittar åt höger eller vänster sparas även den sidan. Sedan projiceras de i massor och sparas i ram. Varje möjlig punkt i en 8x8x8 lista (hälften av ram jag vet!)

// ---------------------------------------------------------
// Main
// ---------------------------------------------------------

int main(void) {
    playerposition.x = 20;
    playerposition.y = 0;
    playerposition.z = 11;

    playerrotation.x = 0;
    playerrotation.y = 0;
    UpdateDirections();

    terminal_print((char*)"TERMINAL INITIALIZED 0123456789");

    while(1) {
        //char message[64];
        //int_to_string(mul8((char)25, (char)25), message);
        //terminal_print(message);
        wait_retrace();
        intensity(0x5f);

        set_text_size(-6, 40);
        char stringy[32];

        if(SHOW_POSTITION){
            build_position_string(stringy, playerposition.x, playerposition.y, playerposition.z);
            print_str_c(120, -120, stringy);
        }
        if (SHOW_ROTATION){
            build_rotation_string(stringy, playerrotation.x, playerrotation.y);
            print_str_c(110, -120, stringy);
        }
        
        /*for (char x = 0; x < 3; x++) {
            for (char y = 0; y < 3; y++) {
                for (char z = 0; z < 3; z++) {
                    if (world[x][y][z] == 1) {
                        vec3 pos;
                        pos.x = x * 10;
                        pos.y = y * 10;
                        pos.z = (z * 10) + 30;
                        
                        createcubeat(pos, x, y, z);
                    }
                }
            }
        }*/

        //prepare_vertices();

        //c_vec2 blockpos;
        //blockpos.x = (signed char)(playerposition.x >> 3);
        //blockpos.y = (signed char)(playerposition.y >> 3);
        //blockpos.z = (signed char)(playerposition.z >> 3);


        char edges[12][2];
        vec3 out[8];

        vec3 p1 = {10, -10, 30};
        createcubeat(p1, edges, out);
        drawcube(out, edges);
        vec3 p2 = {10, -10, 50};
        createcubeat(p2, edges, out);
        drawcube(out, edges);

        uint8_t buttons = read_buttons();
        uint8_t joy = read_joystick(1);

        if (joy & JOY_RIGHT_MASK) {
            playerrotation.x -= 5;
            UpdateDirections();
            //terminal_print("JOYSTICK LEFT");
        }

        if (joy & JOY_LEFT_MASK) {
            playerrotation.x += 5;
            UpdateDirections();
            //terminal_print("JOYSTICK LEFT");
        }

        if ((joy & JOY_UP_MASK) && playerrotation.y < 64) {
            playerrotation.y += 5;
            UpdateDirections();
            //terminal_print("JOYSTICK UP");
        }

        if ((joy & JOY_DOWN_MASK) && playerrotation.y > -64) {
            playerrotation.y -= 5;
            UpdateDirections();
            //terminal_print("JOYSTICK DOWN");
        }

        if (buttons & JOY1_BTN1_MASK) {
            unsigned char move_angle = (unsigned char)(playerrotation.x + 128);
            MovePlayer(3, move_angle);
        }

        if (buttons & JOY1_BTN2_MASK)
        {
            unsigned char move_angle = (unsigned char)(playerrotation.x + 128);
            MovePlayer(-3, move_angle);
        }

        if (buttons & JOY1_BTN3_MASK) {
            unsigned char move_angle = (unsigned char)(playerrotation.x + 192);
            MovePlayer(3, move_angle);
        }

        if (buttons & JOY1_BTN4_MASK)
        {
            unsigned char move_angle = (unsigned char)(playerrotation.x + 192);
            MovePlayer(-3, move_angle);
        }

        terminal_render();
    }
    
    return 0;
}