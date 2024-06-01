#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <citro2d.h>

void printfile(const char* path)
{
	FILE* f = fopen(path, "r");//该函数可使用romfs:前缀
	if (f)
	{
		char mystring[100];
		while (fgets(mystring, sizeof(mystring), f))
		{
			int a = strlen(mystring);
			if (mystring[a-1] == '\n')
			{
				mystring[a-1] = 0;
				if (mystring[a-2] == '\r')
					mystring[a-2] = 0;
			}
			puts(mystring);
		}
		printf(">>EOF<<\n");
		fclose(f);
	}
}

int main(int argc, char* argv[])
{
	gfxInitDefault();

	PrintConsole /*topScreen,*/bottomScreen;
	//consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM,&bottomScreen);
	consoleSelect(&bottomScreen);//选择绘制屏幕

	Result rc = romfsInit();
	if(rc==0)
	{
		printfile("romfs:/info.txt");
	}

	//consoleSelect(&bottomScreen);

	printf("\x1b[33mHello, World!\x1b[0m\n");
	printf("This is ...\n\x1b[31mLF!\x1b[0m\n");
	printf("This is ...\r\n\x1b[7mCRLF!\x1b[0m\r\n");
	for(int i=0;i<=7;i++)
	{
		printf("\x1b[3%imColor : 3%i\x1b[0m\n",i,i);
	}
	printf("Press START to exit.\n");

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	// Create screen
	C3D_RenderTarget* top3d = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

	C2D_TextBuf textBuf=C2D_TextBufNew(4096);
	C2D_Text text;

	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(top3d, C2D_Color32(0x68, 0xB0, 0xD8, 0xFF));
	C2D_SceneBegin(top3d);

	C2D_TextParse(&text,textBuf,"ベクター文字");
	C2D_TextOptimize(&text);
	C2D_DrawText(&text,C2D_AlignCenter,200.0f, 120.0f, 0.5f, 1.0f, 1.0f);

	C3D_FrameEnd(0);

	// Main loop
	while (aptMainLoop())
	{
		//gspWaitForVBlank();
		//gfxSwapBuffers();
		hidScanInput();

		// Your code goes here
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
		{
			printf("Breaking...\n");
			gspWaitForVBlank();
			break; // break in order to return to hbmenu
		}
	}

	C2D_TextBufDelete(textBuf);
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	return 0;
}
