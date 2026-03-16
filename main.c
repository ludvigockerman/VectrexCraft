#include <vectrex.h>

typedef struct { int x,y,z; } Vec3;
typedef struct { int x,y; } Vec2;

int edges[12][2] = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7}
};

Vec3 playerPosition;

void project_point(Vec3 p, Vec2 *out)
{
    if(p.z - playerPosition.z <= 1) 
    {
        out->x = (int)-128;
        out->y = (int)-128;
        return;
    }

    
    // gör multiplikation i long för att undvika varningsfel
    long fx = ((long)(p.x - playerPosition.x) * 50) / (p.z - playerPosition.z);
    long fy = ((long)(p.y - playerPosition.y) * 50) / (p.z - playerPosition.z);
    
    if (fx > 127 || fx < -127 || fy > 127 || fy < -127){
        out->x = -128;
        out->y = -128;
        return;
    }
    
    out->x = (int)fx;
    out->y = (int)fy;
}

// Flytta till punkt
void MoveToScreenPosition(Vec2 p)
{
    //Wait_Recal();
    Reset0Ref();
    Moveto_d(p.y, p.x);
}

void DrawCube(Vec3 *cube)
{
    Vec2 pts[8];
    for(int w=0;w<8;w++)
    {
        project_point(cube[w], &pts[w]);
    }
    
    int currentPosx;
    int currentPosy;
    int newPosx;
    int newPosy;
    int drawnLines = 0;
    
    // Rita kuben
    for(int i=0;i<12;i++)
    {
        if (pts[edges[i][0]].x == -128){
            continue;
        }
        if (pts[edges[i][1]].x == -128){
            continue;
        }
        if (drawnLines==0 || drawnLines==6){
            currentPosx = pts[edges[i][0]].x;
            currentPosy = pts[edges[i][0]].y;
            MoveToScreenPosition((Vec2){currentPosx, currentPosy});
        }
        newPosx = pts[edges[i][0]].x;
        newPosy = pts[edges[i][0]].y;
        Moveto_d(newPosy - currentPosy, newPosx - currentPosx);
        currentPosx = newPosx;
        currentPosy = newPosy;
        
        int deltax = pts[edges[i][1]].x - pts[edges[i][0]].x;
        int deltay = pts[edges[i][1]].y - pts[edges[i][0]].y;
        Draw_Line_d(deltay, deltax);
        drawnLines++;
        currentPosx += deltax;
        currentPosy += deltay;
    }
}

void CreateCubeAt(Vec3 cubePos, Vec3 *out)
{
    out[0] = (Vec3){cubePos.x-10,cubePos.y-10,cubePos.z-10};
    out[1] = (Vec3){cubePos.x,   cubePos.y-10,cubePos.z-10};
    out[2] = (Vec3){cubePos.x,   cubePos.y,   cubePos.z-10};
    out[3] = (Vec3){cubePos.x-10,cubePos.y,   cubePos.z-10};
    
    out[4] = (Vec3){cubePos.x-10,cubePos.y-10,cubePos.z};
    out[5] = (Vec3){cubePos.x,   cubePos.y-10,cubePos.z};
    out[6] = (Vec3){cubePos.x,   cubePos.y,   cubePos.z};
    out[7] = (Vec3){cubePos.x-10,cubePos.y,   cubePos.z};
}

int main(void)
{
    Vec3 cube[8]; // 8 points per cube
    
    playerPosition = (Vec3){ 0, 0, 0 };
    
    while(1)
    {

        CreateCubeAt((Vec3){-10, 0, 30}, &cube[0]);
        DrawCube(&cube[0]);
        CreateCubeAt((Vec3){-10, -10, 30}, &cube[0]);
        DrawCube(&cube[0]);
        CreateCubeAt((Vec3){-10, 10, 30}, &cube[0]);
        DrawCube(&cube[0]);
        CreateCubeAt((Vec3){-10, 10, 40}, &cube[0]);
        DrawCube(&cube[0]);
        CreateCubeAt((Vec3){-10, 10, 20}, &cube[0]);
        DrawCube(&cube[0]);
        
        Wait_Recal();
        Intensity_a(0x5F);
        //Joy_Digital();
        Read_Btns();
        if (Vec_Btn_State & 0b00000001){
            //Print_Str_d( 50, -50, "BUTTON1");
            playerPosition.z ++;
        }
        if (Vec_Btn_State & 0b00000010){
            playerPosition.z --;
        }
        if (Vec_Btn_State & 0b00000100){
            playerPosition.x ++;
        }
        if (Vec_Btn_State & 0b00001000){
            playerPosition.x --;
        }
    }
}