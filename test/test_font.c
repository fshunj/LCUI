﻿// 测试 LCUI 的字体处理功能
// 由于LCUI的版本变更，该程序的功能暂时无法完全实现
#include <LCUI_Build.h>
#include LC_LCUI_H
#include LC_WIDGET_H
#include LC_WINDOW_H
#include LC_LABEL_H

#include <dirent.h>

#ifdef LCUI_BUILD_IN_WIN32
#include <io.h>
#include <fcntl.h>

/* 在运行程序时会打开控制台，以查看打印的调试信息 */
static void InitConsoleWindow(void)
{
	int hCrt;
	FILE *hf;
	AllocConsole();
	hCrt=_open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE),_O_TEXT );
	hf=_fdopen( hCrt, "w" );
	*stdout=*hf;
	setvbuf (stdout, NULL, _IONBF, 0);
	// test code
	printf ("InitConsoleWindow OK!\n");
}
#endif

static LCUI_Widget *window, *text;
static char **filename = NULL;
static int total_files = 0, current = 0;

void get_filepath(char *filepath, char *out_path)
/* 功能：用于获取程序所在的文件目录 */
{
	int num;
	strcpy(out_path, filepath);
	for(num = strlen(filepath) - 1; num >= 0; --num)
	{
		if(filepath[num] == '/')
		{
			out_path[num+1] = 0;
			return;
		}
	}
	memset(out_path, 0, sizeof(char)*strlen(out_path));
}

int get_format(char *format, char *filename)
/* 功能：获取文件格式 */
{
	int i, j, len;
	len = strlen(filename);
	for(i=len-1; i>=0; --i)
	{
		if(filename[i] == '.')
		{
			for(j=0, ++i; i<len; ++i, ++j)
			{
				format[j] = filename[i];
			}
			format[j] = 0;
			return 0;
		}
	}
	strcpy(format, "");
	return -1;
}

/* 功能：扫描字体文件，并获得文件列表 */
char **scan_fontfile(char *dir, int *file_num)
{
	int i, len, n;
	char **filelist, format[256], path[1024];
	struct dirent **namelist;
	if(strlen(dir) == 0) {
		n = scandir(".", &namelist, 0, alphasort);
	} else {
		n = scandir(dir, &namelist, 0, alphasort);
	}

	if (n < 0) {
		return 0;
	}
	filelist = (char **)malloc(sizeof(char *)*n);
	for(i=0, *file_num=0; i<n; i++) {
		if(namelist[i]->d_type != 8) {
			continue;
		}
		get_format(format, namelist[i]->d_name);
		if(strlen(format) > 2
		&& (strcasecmp(format, "ttf") == 0
		|| strcasecmp(format, "ttc") == 0
		|| strcasecmp(format, "otf") == 0)) {
			sprintf(path, "%s%s", dir, namelist[i]->d_name);
			len = strlen( path );
			filelist[*file_num] = (char *)malloc(sizeof(char)*(len+1));
			strcpy(filelist[*file_num], path);
			++*file_num;
		}
	}
	if(*file_num > 0) {
		filelist = (char**)realloc(filelist, *file_num*sizeof(char*));
	} else {
		free(filelist);
		filelist = NULL;
	}
	for(i=0;i<n;++i) {
		free(namelist[i]);
	}
	free(namelist);

	return filelist;
}

void change_font(void *arg)
{
	char info[256];
	int font_id;
	LCUI_TextStyle style;
	style = Label_GetTextStyle( text );
	while(1) {
		sprintf(info, "(%d/%d) %s", current+1, total_files, filename[current]);
		Window_SetTitleText(window, info);
		font_id = FontLIB_LoadFontFile(filename[current]);
		TextStyle_FontFamilyID( &style, font_id );
		Label_TextStyle( text, style );
		LCUI_Sleep(1);
		++current;
		if(current >= total_files) {
			current = 0;
		}
	}
	LCUIThread_Exit(NULL);
}

static void destroy( LCUI_Widget *widget, LCUI_WidgetEvent *unused )
{
	LCUI_MainLoop_Quit(NULL);
}

int main(int argc, char **argv)
{
	LCUI_Thread t;

#ifdef LCUI_BUILD_IN_WIN32
	InitConsoleWindow();
#endif
	LCUI_Init(0,0,0);
	filename = scan_fontfile("../../fonts/", &total_files);
	window = Widget_New("window");
	text = Widget_New("label");
	Widget_Resize(window, Size(320, 240));
	Window_SetTitleTextW( window, L"测试字体" );
	Label_TextW( text,
		L"<size=10px>8px: 中国文字，China, Chinese,English.</size>\n"
		L"<size=10px>10px: 中国文字，China, Chinese,English.</size>\n"
		L"<size=11px>11px: 中国文字，China, Chinese,English.</size>\n"
		L"<size=12px>12px: 中国文字，China, Chinese,English.</size>\n"
		L"<size=14px>14px: 中国文字，China, Chinese,English.</size>\n"
		L"<size=16px>16px: 中国文字，1234567890.:,;(*!?)</size>\n"
		L"<size=18px>18px: 中国创造，慧及全球！</size>\n"
		L"<size=20px>20px: 中国创造，慧及全球！</size>");
	Widget_SetAlign(text, ALIGN_MIDDLE_CENTER, Pos(0,0));
	Window_ClientArea_Add(window, text);
	Widget_Event_Connect( Window_GetCloseButton(window), EVENT_CLICKED, destroy);
	Widget_Show(text);
	Widget_Show(window);
	LCUIThread_Create( &t, change_font, NULL );
	return LCUI_Main();
}
