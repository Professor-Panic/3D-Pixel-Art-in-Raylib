#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "Editor.h"
struct RenderCamera{
    Camera3D cam;
    double near;
    double far;
};
int main(){
    InitWindow(1200,800,"3D Pixel Art");
    rlImGuiSetup(true);
    float pixel_scale=4;
    Mesh floor=GenMeshPlane(25,25,1,1);
    Model floorModel=LoadModelFromMesh(floor);
    Model Cube=LoadModelFromMesh(GenMeshCube(1.0,1.0,1.0));
    Vector2 raster_size=Vector2{GetScreenWidth()/pixel_scale,GetScreenHeight()/pixel_scale};
    Vector2 window_size=Vector2{GetScreenWidth(),GetScreenHeight()};
   
    RenderTexture rd=LoadRenderTexture(raster_size.x,raster_size.y);
    RenderCamera camera;
    camera.cam.position={-2.0,1.0,-2.0};
    camera.cam.target={0.0,0.0,0.0};
    camera.cam.up={0.0,1.0,0.0};
    camera.cam.fovy=45.0f;
    camera.cam.projection=CAMERA_ORTHOGRAPHIC;
    camera.far=100.0;
    camera.near=0.01;
    float rot=0.0f;
    float rot_speed=20.0f;
    Vector3 cubePos={0.0,1.0,0.0};
    SetTextureFilter(rd.texture,RL_TEXTURE_FILTER_NEAREST);
    while (!WindowShouldClose())
    {
        float dt=GetFrameTime();
        rot+=dt*rot_speed;
        rlSetClipPlanes(camera.near,camera.far);
        BeginDrawing();
        BeginTextureMode(rd);
        ClearBackground(BLACK);
        BeginMode3D(camera.cam);
        DrawModel(floorModel,{0.0,0.0,0.0},1.0,WHITE);
        DrawModelEx(Cube,cubePos,{0.0,1.0,0.0},rot,{1.0,1.0,1.0},RED);
        EndMode3D();
        EndTextureMode();
        ClearBackground(BLACK);
        DrawTexturePro(rd.texture,{0.0,0.0,raster_size.x,raster_size.y},{0.0,0.0,window_size.x,window_size.y},Vector2Zero(),0.0,WHITE);
        rlImGuiBegin();
        ImGui::Begin("Camera Controls");
        ImGui::InputFloat("FOVY",&camera.cam.fovy);
        ImGui::SliderFloat3("Camera Position",&camera.cam.position.x,-50.0f,50.0f);
        ImGui::SliderFloat3("Cube Position",&cubePos.x,-50.0f,50.0f);
        ImGui::InputDouble("Far Plane",&camera.far);
        ImGui::InputDouble("Near Plane",&camera.near);
        ImGui::End();
        rlImGuiEnd();
        EndDrawing();
    }
    rlImGuiShutdown();
    CloseWindow();
}