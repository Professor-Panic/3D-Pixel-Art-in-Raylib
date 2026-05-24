#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "Extras/Extras.h"
#include "Editor.h"
struct RenderCamera{
    Camera3D cam;
    double near;
    double far;
};
int main(){
    InitWindow(1200,800,"3D Pixel Art");
    float pixel_scale=4.0;
    Vector2 raster_size=Vector2{GetScreenWidth()/pixel_scale,GetScreenHeight()/pixel_scale};
    Vector2 window_size=Vector2{float(GetScreenWidth()),float(GetScreenHeight())};
   
    //First Load the gBufferShader
    Shader gbufferShader=LoadShader("shaders/gBuffer.vs","shaders/gBuffer.fs");
    //Initialize Gbuffer
    Gbuffer gBuffer={ 0 };
    gBuffer.frameBufferId=rlLoadFramebuffer();
    if(!gBuffer.frameBufferId==0){
        TraceLog(LOG_INFO,"Framebuffer Loaded Successfully");
    }
    //Enable Guffer and create the textures for the data it will store
    rlEnableFramebuffer(gBuffer.frameBufferId);
    gBuffer.positionTextureId=rlLoadTexture(NULL,raster_size.x,raster_size.y,PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,1);
    gBuffer.normalTextureId=rlLoadTexture(NULL,raster_size.x,raster_size.y,PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,1);
    gBuffer.albedoTextureId = rlLoadTexture(NULL,raster_size.x,raster_size.y, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
    gBuffer.depthTextureId= rlLoadTextureDepth(raster_size.x,raster_size.y,false);
    rlActiveDrawBuffers(3);
    rlFramebufferAttach(gBuffer.frameBufferId,gBuffer.positionTextureId,RL_ATTACHMENT_COLOR_CHANNEL0,RL_ATTACHMENT_TEXTURE2D,0);
    rlFramebufferAttach(gBuffer.frameBufferId,gBuffer.normalTextureId,RL_ATTACHMENT_COLOR_CHANNEL1,RL_ATTACHMENT_TEXTURE2D,0);
    rlFramebufferAttach(gBuffer.frameBufferId,gBuffer.albedoTextureId,RL_ATTACHMENT_COLOR_CHANNEL2,RL_ATTACHMENT_TEXTURE2D,0);
    rlFramebufferAttach(gBuffer.frameBufferId,gBuffer.depthTextureId,RL_ATTACHMENT_DEPTH,RL_ATTACHMENT_TEXTURE2D,0);
    if (!rlFramebufferComplete(gBuffer.frameBufferId)) TraceLog(LOG_WARNING, "Framebuffer is not complete");
    int texUnitPosition=0;
    int texUnitNormal=1;
    int texUnitAlbedo=2;
    int texUnitDepth=3;
    Model floorModel=LoadModelFromMesh(GenMeshPlane(25,25,1,1));
    Model Cube=LoadModelFromMesh(GenMeshCube(1.0,1.0,1.0));
    floorModel.materials[0].shader=gbufferShader;
    Cube.materials[0].shader=gbufferShader;
    RenderTexture rd=LoadRenderTexture(raster_size.x,raster_size.y);
    RenderCamera camera;
    camera.cam.position={30.0,30.0,30.0};
    camera.cam.target={0.0,0.0,0.0};
    camera.cam.up={0.0,1.0,0.0};
    camera.cam.fovy=20.0f;
    camera.cam.projection=CAMERA_ORTHOGRAPHIC;
    camera.far=100.0;
    camera.near=0.01;
    float rot=0.0f;
    float rot_speed=100.0f;
    Vector3 cubePos={0.0,1.0,0.0};
    SetTextureFilter(rd.texture,RL_TEXTURE_FILTER_NEAREST);
    rlEnableDepthTest();
    rlImGuiSetup(true);
    while (!WindowShouldClose())
    {
        float dt=GetFrameTime();
        rot+=dt*rot_speed;
        //Ensure the clip Planes for the camera is okay
        rlSetClipPlanes(camera.near,camera.far);
        BeginDrawing();
        //First draw the gBuffer
        rlEnableFramebuffer(gBuffer.frameBufferId);
        //The viewport is usually the window size reset it to be your raster size 
        //to ensure everything is rendered with corect sizes
        rlViewport(0,0,raster_size.x,raster_size.y);
        rlClearColor(0,0,0,0);//Essentially ClearBackground
        rlClearScreenBuffers();  // Clear color and depth buffer
        rlDisableColorBlend();//We dont want it to blend values we need exact

        BeginMode3D(camera.cam);

        rlEnableShader(gbufferShader.id);
        DrawModel(floorModel,{0.0,0.0,0.0},1.0,WHITE);
        DrawModelEx(Cube,cubePos,{0.0,1.0,0.0},rot,{1.0,1.0,1.0},RED);
        rlDisableShader();
        EndMode3D();
        rlEnableColorBlend();
        //Copy depth buffer to main buffer 0
        rlBindFramebuffer(RL_READ_FRAMEBUFFER, gBuffer.frameBufferId);
        rlBindFramebuffer(RL_DRAW_FRAMEBUFFER, 0);
        rlBlitFramebuffer(0, 0, raster_size.x,raster_size.y, 0, 0,window_size.x,window_size.y, 0x00000100); // GL_DEPTH_BUFFER_BIT
        rlDisableFramebuffer();
        // Go back to the default framebufferId (0) and draw our deferred shading
        rlClearScreenBuffers(); // Clear color & depth buffer
        //Reset Viewport for upscaling
        rlViewport(0,0,window_size.x,window_size.y);
        Texture2D t;
        t.id = gBuffer.positionTextureId;
        t.width = window_size.x;
        t.height = window_size.y;
        DrawTextureRec(t,{0.0,0.0,window_size.x,-window_size.y},{0.0,0.0},WHITE);
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