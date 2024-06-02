#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_image.h>
#include <3ds.h>

int DemoInit();
int DemoUninit();
int DemoLoop();
int DemoBreakLoop();
int _DemoIsLooping();


SDL_Window* window[2];
SDL_Renderer* renderer[2];
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
PrintConsole console[2];

char textCN[128],textEN[128];
void LoadRomFSFiles(const char *path,char *buf,int buflen)
{
	FILE*f=fopen(path,"r");
	if(f!=NULL)
	{
		int offset=0;
		while(fgets(buf+offset,buflen-offset,f))
			offset+=strlen(buf+offset);
	}
}

int DemoInit()
{
    if(SDL_Init(SDL_INIT_EVERYTHING))
        return -1;
    
	//同一个屏幕不要同时使用Console和SDL，否则会花屏
    //consoleInit(GFX_TOP,&console[0]);
	consoleInit(GFX_BOTTOM,&console[1]);
	consoleSelect(&console[1]);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.Fonts->AddFontDefault();
	io.Fonts->AddFontFromFileTTF("romfs:/NotoSansSC-Medium.otf",18.0f);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    for (int i=SDL_GetNumVideoDisplays();i>0;)
    {
        --i;
        SDL_Rect r;
        SDL_GetDisplayBounds(i,&r);
        window[i]=SDL_CreateWindow("Window",r.x,r.y,r.w,r.h,SDL_WINDOW_FULLSCREEN);
		if(window[i]==nullptr)
		{
			printf("Fail to create window[%d].\n",i);
			return -2;
		}
		renderer[i]=SDL_CreateRenderer(window[i],-1,SDL_RENDERER_PRESENTVSYNC);
		if(renderer[i]==nullptr)
		{
			printf("Fail to create renderer[%d].\n",i);
			return -3;
		}
    }

	// Setup Platform/Renderer backends
	//ImGui最好只用在一个Render上
	ImGui_ImplSDL2_InitForSDLRenderer(window[0], renderer[0]);
	ImGui_ImplSDLRenderer2_Init(renderer[0]);
	ImGui::SetNextWindowPos({0.0f,0.0f});

	LoadRomFSFiles("romfs:/info.txt",textEN,sizeof(textEN));
	LoadRomFSFiles("romfs:/文本.txt",textCN,sizeof(textCN));

    return 0;
}

int DemoUninit()
{
    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

	for (int i=SDL_GetNumVideoDisplays();i>0;)
    {
        --i;
		SDL_DestroyRenderer(renderer[i]);
		SDL_DestroyWindow(window[i]);
	}
    SDL_Quit();
    return 0;
}

int DemoLoop()
{
    SDL_Event event;
    ImGuiIO& io = ImGui::GetIO();
	//事件处理
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            DemoBreakLoop();
    }

	//上屏幕
    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

	static float f = 0.0f;
	static int counter = 0;

	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	if(ImGui::Button("Exit"))
		DemoBreakLoop();
	ImGui::End();

    // Rendering
    ImGui::Render();
    SDL_RenderSetScale(renderer[0], io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(renderer[0], (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    SDL_RenderClear(renderer[0]);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());//将ImGui预渲染的内容显示出来
    SDL_RenderPresent(renderer[0]);//显示上屏幕

	//TODO:下屏幕

    return 0;
}

static int looping=1;

int DemoBreakLoop()
{
    looping=0;
    return 0;
}

int _DemoIsLooping()
{
    return looping;
}


int main(int argc, char* argv[])
{
	if(DemoInit()==0)
	{
		while (_DemoIsLooping())
		{
			DemoLoop();
		}
		DemoUninit();
	}
	return 0;
}
