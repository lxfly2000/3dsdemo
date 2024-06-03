#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <3ds.h>
#include <map>
#include <set>
#include <vector>

#define DECLARESTR(MA,STRA) const char MA[]=STRA;

#define DLIST \
DECLARESTR(str_sample_window,"窗口")\
DECLARESTR(str_demo_text,"A:选择 B:取消 X:窗口 Y:编辑\nX+摇杆：移动窗口 X+十字键：调整大小")\
DECLARESTR(str_float_value,"浮点数")\
DECLARESTR(str_clear_color,"清空颜色")\
DECLARESTR(str_button,"按钮")\
DECLARESTR(str_press_count,"按下次数：%d")\
DECLARESTR(str_sound,"音频")\
DECLARESTR(str_fps_meter,"帧时间：%.3fms FPS: %.1f FPS")\
DECLARESTR(str_exit,"退出")\

DLIST
#undef DECLARESTR
#define DECLARESTR(MA,STRA) STRA,
const std::vector<const char*>msglist={DLIST};
std::vector<ImWchar> char_range = {};
void ImWcharRangeCalculate(std::vector<ImWchar>&out_range,const std::vector<const char*>&in_list)
{
	//基本ASCII：0x20-0xFF
	std::set<ImWchar>charset;
	for(ImWchar c=0x20;c<=0xFF;c++)
		charset.insert(c);
	for(size_t i=0;i<in_list.size();i++){
		size_t wcharlength=mbstowcs(nullptr,in_list[i],0);
		wchar_t *buf=(wchar_t*)malloc((wcharlength+1)*sizeof(wchar_t));
		mbstowcs(buf,in_list[i],strlen(in_list[i]));
		for(size_t j=0;j<wcharlength;j++)
			charset.insert(buf[j]);
		free(buf);
	}
	for(auto c:charset){
		if(out_range.empty()||out_range.back()+1!=c){
			out_range.push_back(c);
			out_range.push_back(c);
		}else{
			out_range[out_range.size()-1]=c;
		}
	}
	out_range.push_back(0);
}

int DemoInit();
int DemoUninit();
int DemoLoop();
int DemoBreakLoop();
bool _DemoIsLooping();


SDL_Window* window[2];
SDL_Renderer* renderer[2];
SDL_Rect displayBounds[2];
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
PrintConsole console[2];
SDL_Texture* demo_texture,*demo_text_texture;
SDL_Rect demo_texture_rect,demo_text_texture_rect;
Mix_Chunk* demo_mus;
TTF_Font* demo_font;

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

//加载文字，返回图像指针，失败时返回 NULL
//* 不支持多行文字
SDL_Texture *LoadTextToImage(SDL_Renderer *pRenderer, const char *text, TTF_Font *font, const SDL_Color color)
{
	SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, color);
	SDL_Texture *img = SDL_CreateTextureFromSurface(pRenderer, surface);
	SDL_FreeSurface(surface);
	return img;
}

void QueryTextureRect(SDL_Texture*tex,int*w,int *h)
{
	Uint32 fmt;
	int acc;
	SDL_QueryTexture(tex,&fmt,&acc,w,h);
}

void DrawTexture(SDL_Renderer *pRenderer,SDL_Texture *tex,int x,int y,int w=0,int h=0)
{
	SDL_Rect dst={x,y,w,h};
	if(dst.w==0||dst.h==0)
		QueryTextureRect(tex,&dst.w,&dst.h);
	SDL_RenderCopy(pRenderer,tex,NULL,&dst);
}

int DemoInit()
{
	//由于内存限制，绝对不能一口气把Unicode范围全部加载，只能是用到啥字符就加载啥字符
	ImWcharRangeCalculate(char_range,msglist);
    if(SDL_Init(SDL_INIT_EVERYTHING))
        return -1;
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
    
	//同一个屏幕不要同时使用Console和SDL，否则会花屏
    consoleInit(GFX_TOP,&console[0]);
	//consoleInit(GFX_BOTTOM,&console[1]);
	consoleSelect(&console[0]);

	if((IMG_Init(IMG_INIT_JPG)&IMG_INIT_JPG)==0)
		return -100;
	if((Mix_Init(MIX_INIT_OGG)&MIX_INIT_OGG)==0)
		return -101;
	if(TTF_Init())
		return -102;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
	io.IniFilename=NULL;//不要保存配置文件
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//默认字体是一种只有英文的点阵字体，不推荐中文使用
	/*if(io.Fonts->AddFontDefault())
		printf("Failed to load default font.\n");*/
	if(io.Fonts->AddFontFromFileTTF("romfs:/NotoSansSC-Medium.otf",14.0f*96/72.0f,NULL,char_range.data())==nullptr)
		printf("Failed to load custom font.\n");

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    for (int i=SDL_GetNumVideoDisplays();i>0;)
    {
        --i;
        SDL_GetDisplayBounds(i,&displayBounds[i]);
        window[i]=SDL_CreateWindow("Window",displayBounds[i].x,displayBounds[i].y,
									displayBounds[i].w,displayBounds[i].h,SDL_WINDOW_FULLSCREEN);
		if(window[i]==nullptr)
		{
			printf("Failed to create window[%d].\n",i);
			return -2;
		}
		renderer[i]=SDL_CreateRenderer(window[i],-1,SDL_RENDERER_PRESENTVSYNC);
		if(renderer[i]==nullptr)
		{
			printf("Failed to create renderer[%d].\n",i);
			return -3;
		}
    }

	// Setup Platform/Renderer backends
	//ImGui最好只用在一个Render上
	ImGui_ImplSDL2_InitForSDLRenderer(window[1], renderer[1]);
	ImGui_ImplSDLRenderer2_Init(renderer[1]);

	LoadRomFSFiles("romfs:/info.txt",textEN,sizeof(textEN));
	LoadRomFSFiles("romfs:/文本.txt",textCN,sizeof(textCN));

	printf(textEN);
	printf(textCN);

	//加载图像
	demo_texture=IMG_LoadTexture(renderer[1],"romfs:/chino.jpg");
	QueryTextureRect(demo_texture,&demo_texture_rect.w,&demo_texture_rect.h);
	//加载声音
	if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,MIX_DEFAULT_FORMAT,MIX_DEFAULT_CHANNELS,4096)){
		printf("Mix_OpenAudio error.\n");
		return -103;
	}
	demo_mus=Mix_LoadWAV("romfs:/newrecord.ogg");//几秒甚至不到1秒的音效用LoadWAV，较长的（如BGM）用LoadMUS
	//加载字体
	demo_font=TTF_OpenFont("romfs:/NotoSansSC-Medium.otf",18.0f);
	demo_text_texture=LoadTextToImage(renderer[1],"你好Hello",demo_font,{255,255,0,255});
	QueryTextureRect(demo_text_texture,&demo_text_texture_rect.w,&demo_text_texture_rect.h);

    return 0;
}

int DemoUninit()
{
    // Cleanup
	SDL_DestroyTexture(demo_texture);
	SDL_DestroyTexture(demo_text_texture);
	Mix_FreeChunk(demo_mus);
	TTF_CloseFont(demo_font);

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

	for (int i=SDL_GetNumVideoDisplays();i>0;)
    {
        --i;
		SDL_DestroyRenderer(renderer[i]);
		SDL_DestroyWindow(window[i]);
	}
	Mix_CloseAudio();
	IMG_Quit();
	Mix_Quit();
	TTF_Quit();
    SDL_Quit();
    return 0;
}

int DemoLoop()
{
	gspWaitForVBlank();//等待上一画面更新完毕
    SDL_Event event;
    ImGuiIO& io = ImGui::GetIO();
	//事件处理
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT){
            DemoBreakLoop();
		}else if (event.type == SDL_JOYBUTTONDOWN){
			printf("Joy Button:%#x\n",event.jbutton.button);
		}else if (event.type == SDL_JOYAXISMOTION){
			printf("Joy Axis:%d value:%d\n",event.jaxis.axis,event.jaxis.value);
		}else if (event.type == SDL_KEYDOWN){
			printf("Key:%#lx\n",event.key.keysym.sym);
		}else if (event.type == SDL_FINGERDOWN){
			printf("Finger Down x:%f y:%f\n",event.tfinger.x,event.tfinger.y);
		}else if (event.type == SDL_FINGERMOTION){
			printf("Finger Motion x:%f y:%f\n",event.tfinger.x,event.tfinger.y);
		}else if (event.type == SDL_CONTROLLERBUTTONDOWN){
			printf("Controller Button:%#x\n",event.cbutton.button);
		}else if (event.type == SDL_CONTROLLERAXISMOTION){
			printf("Controller Axis:%d value:%d\n",event.caxis.axis,event.caxis.value);
		}else if (event.type == SDL_MOUSEBUTTONDOWN){
			printf("Mouse Down x:%ld y:%ld\n",event.button.x,event.button.y);
		}else if (event.type == SDL_MOUSEMOTION){
			printf("Mouse Motion x:%ld y:%ld\n",event.motion.x,event.motion.y);
		}
    }

	//TODO:上屏幕
	//上屏幕使用Console模式时无需处理

	//下屏幕
    // Start ImGUI
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

	static float f = 0.0f;
	static int counter = 0;
	static bool needSetPos=true;
	if(needSetPos){
		ImGui::SetNextWindowPos({0.0f,0.0f});
		ImGui::SetNextWindowSize({displayBounds[1].w*0.7f,displayBounds[1].h*0.8f});
		needSetPos=false;
	}
	ImGui::Begin(str_sample_window);                          // Create a window called "Hello, world!" and append into it.

	ImGui::Text(str_demo_text);               // Display some text (you can use a format strings too)
	ImGui::SliderFloat(str_float_value, &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::ColorEdit3(str_clear_color, (float*)&clear_color); // Edit 3 floats representing a color

	if(ImGui::Button(str_exit))
		DemoBreakLoop();
	ImGui::SameLine();
	if(ImGui::Button(str_sound))
	{
		printf("Playing audio...\n");
		Mix_PlayChannel(-1,demo_mus,0);
	}
	ImGui::SameLine();
	if (ImGui::Button(str_button))                            // Buttons return true when clicked (most widgets return true when edited/activated)
	{
		counter++;
		printf("Counter: %d\n",counter);
	}
	ImGui::SameLine();
	ImGui::Text(str_press_count, counter);

	ImGui::Text(str_fps_meter, 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();
    ImGui::Render();//End ImGUI

	//Clear Renderer
    SDL_RenderSetScale(renderer[1], io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(renderer[1], (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    SDL_RenderClear(renderer[1]);

	//Draw Texture
	DrawTexture(renderer[1],demo_texture,displayBounds[1].w-demo_texture_rect.w,displayBounds[1].h-demo_texture_rect.h);
	DrawTexture(renderer[1],demo_text_texture,0,displayBounds[1].h-demo_text_texture_rect.h);

	//Draw ImGUI
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());//将ImGui预渲染的内容显示出来
    SDL_RenderPresent(renderer[1]);//显示上屏幕（隐含gfxSwapBuffers）

    return 0;
}

int looping=1;

int DemoBreakLoop()
{
    looping=0;
    return 0;
}

bool _DemoIsLooping()
{
    return looping==1&&aptMainLoop();//aptMainLoop用于检测系统是否通知结束运行
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
