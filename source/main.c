#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

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
	PrintConsole topScreen,bottomScreen;
	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM,&bottomScreen);
	consoleSelect(&topScreen);//选择绘制屏幕

	Result rc = romfsInit();
	if(rc==0)
	{
		printfile("romfs:/info.txt");
	}

	consoleSelect(&bottomScreen);

	printf("\x1b[33mHello, World!\x1b[0m\n");
	printf("This is ...\n\x1b[31mLF!\x1b[0m\n");
	printf("This is ...\r\n\x1b[7mCRLF!\x1b[0m\r\n");

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
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

	gfxExit();
	return 0;
}
