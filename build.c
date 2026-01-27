#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef unsigned int uint;

// Maximum of 1024 source/header files
#define MAX_SOURCES 1024
// TODO: define a uh header line length
// TODO: put "fixed" width to compile commands thing

char sources[MAX_SOURCES][256];
uint sources_length = 0;
char headers[MAX_SOURCES][256];
uint headers_length = 0;
char compile_commands[MAX_SOURCES][99999];
uint compile_commands_length = 0;

void needed_headers(const char*,char[257*10+10]);
char* basename(const char*);

int main()
{
	DIR *src_dir = opendir("src");
	if(src_dir == NULL)
	{
		printf("\"src\" directory not found\n");
		return 1;
	}
	DIR *header_dir = opendir("include");
	if(header_dir == NULL)
	{
		printf("\"include\" directory not found\n");
		return 1;
	}
	struct dirent *source;
	while((source = readdir(src_dir)) != NULL)
	{
		if(source->d_type == DT_REG)
		{
			strncpy(sources[sources_length], source->d_name, 256);
			sources_length++;
		}
	}
	struct dirent *header;
	while((header = readdir(header_dir)) != NULL)
	{
		if(header->d_type == DT_REG)
		{
			strncpy(headers[headers_length], header->d_name, 256);
			headers_length++;
		}
	}
	for(uint i=0;i<sources_length;i++)
	{
		char filepath[261];
		char headers_needed[257*10+10];
		strcpy(filepath, "src/");
		strcat(filepath, sources[i]);
		needed_headers(filepath, headers_needed);
		basename(filepath);
		char compile_command[99999];
		strcpy(compile_command, "cc ");
		strcat(compile_command, filepath);
		strcat(compile_command, " ");
		strcat(compile_command, headers_needed);
		strcat(compile_command, "-o bin/");
		strcat(compile_command, basename(sources[i]));

		strcpy(compile_commands[compile_commands_length], compile_command);
		compile_commands_length++;
	}

	DIR *build_dir = opendir("bin");
	if(build_dir == NULL)
	{
		if(!mkdir("bin", 0777)) perror("creating build folder");
	}

	for(uint i=0;i<compile_commands_length;i++)
	{
		system(compile_commands[i]);
	}


	closedir(src_dir);
	closedir(header_dir);
}

void needed_headers(const char* filepath, char output[257*10+10])
{
	FILE* file = fopen(filepath, "r");
	if(file==NULL)
	{
		perror("build");
	}
	int c;
	char headers_line[257*10+10];
	uint line_length = 0;

	while((c = fgetc(file)) != '\n' && c != EOF)
	{
		headers_line[line_length] = c;
		line_length++;
	}
	headers_line[line_length] = 0;
	char needed_headers_s[257*10+10];
	uint needed_headers_len = 0;
	if(line_length>=10 && !strncmp(headers_line,"//headers:",10))
	{
		char header[257];
		uint header_len = 0;
		for(uint c=10;c<line_length;c++)
		{
			if(headers_line[c]!=' ')
			{
				header[header_len] = headers_line[c];
				header_len++;
			}
			else
			{
				needed_headers_s[needed_headers_len] = '-';
				needed_headers_len++;
				needed_headers_s[needed_headers_len] = 'l';
				needed_headers_len++;
				for(uint j=0;j<header_len;j++)
				{
					needed_headers_s[needed_headers_len] = header[j];
					needed_headers_len++;
				}
				needed_headers_s[needed_headers_len] = ' ';
				needed_headers_len++;

				header[0] = 0;
				header_len=0;
			}
		}
	}
	needed_headers_s[needed_headers_len] = 0;

	strcpy(output, needed_headers_s);


	fclose(file);
}

char *basename(const char* filename)
{
	int x = 0;
	while(filename[x] && filename[x] != '.')x++;
	return strndup(filename, x);
}
