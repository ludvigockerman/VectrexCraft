#include <vectrex.h>

typedef struct { int x,y,z; } Vec3;
typedef struct { int x,y; } Vec2;

Vec3 playerPosition;

void project_point(Vec3 p, Vec2 *out)
{
    if(p.z - playerPosition.z <= 0) p.z = 1 + playerPosition.z;

    // gör multiplikation i long för att undvika varningsfel
    long fx = ((long)p.x * 50) / (p.z - playerPosition.z);
    long fy = ((long)p.y * 50) / (p.z - playerPosition.z);

    //if (fx > 127 || fx < -127 || fy > 127 || fy < -127){
    

    out->x = (int)fx;
    out->y = (int)fy;
}

// Flytta till punkt
void MoveToScreenPosition(Vec2 p)
{
    //Wait_Recal();
    Reset0Ref();
    Moveto_d(p.x, p.y);
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
    Vec3 cube[31][8]; //31 is the max amount of cubes loaded at the same time.
    CreateCubeAt((Vec3){-10, 0, 30}, &cube[0][0]);
    CreateCubeAt((Vec3){-10, -10, 30}, &cube[1][0]);
    CreateCubeAt((Vec3){-10, 10, 30}, &cube[2][0]);


    int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    playerPosition = (Vec3){ 0, 0, 0 };

    Vec2 pts[8];

    while(1)
    {

        Wait_Recal();
        Intensity_a(0x5F);
        //Joy_Digital();
        Read_Btns();
        if (Vec_Btn_State & 0b00000001){
            //Print_Str_d( 50, -50, "BUTTON1");
            playerPosition.z ++;
        }
        if (Vec_Btn_State & 0b00000010){
            //Print_Str_d( 50, -50, "BUTTON1");
            playerPosition.z --;
        }
        
        // Projektera
        for(int u=0;u<3;u++)
        {
            for(int w=0;w<8;w++)
            {
                project_point(cube[u][w], &pts[w]);
            }
            
            int currentPosx;
            int currentPosy;
            int newPosx;
            int newPosy;

            // Rita kuben
            for(int i=0;i<12;i++)
            {
                if (i==0){
                    currentPosx = pts[edges[i][0]].x;
                    currentPosy = pts[edges[i][0]].y;
                    MoveToScreenPosition((Vec2){currentPosx, currentPosy});
                }
                if (i==6){
                    currentPosx = pts[edges[i][0]].x;
                    currentPosy = pts[edges[i][0]].y;
                    MoveToScreenPosition((Vec2){currentPosx, currentPosy});
                }
                newPosx = pts[edges[i][0]].x;
                newPosy = pts[edges[i][0]].y;
                Moveto_d(newPosx - currentPosx, newPosy - currentPosy);
                currentPosx = newPosx;
                currentPosy = newPosy;

                int deltax = pts[edges[i][1]].x - pts[edges[i][0]].x;
                int deltay = pts[edges[i][1]].y - pts[edges[i][0]].y;
                Draw_Line_d(deltax, deltay);
                currentPosx += deltax;
                currentPosy += deltay;
            }
        }
    }
}
