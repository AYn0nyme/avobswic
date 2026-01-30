/* TODO: Write a comment here like other professional projects and stuff */

// TODO: Add config structure, and read_config, write_config to well write and read config duh
// 			 Refactor the code, it's not really good right now


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* Check for definitions of SRC and INCLUDE directories. */
#ifndef AVO_SRC_DIR
	#define AVO_SRC_DIR "."
#endif
#ifndef AVO_INCLUDE_DIR
	#define AVO_INCLUDE_DIR "."
#endif
#ifndef AVO_CFLAGS
	#define AVO_CFLAGS "-Wall"
#endif

struct compiler {
	char* path;
};

static struct compiler compiler = {""};
static const char* compilers_paths[] = {"tcc", "gcc", "cc"}; //TODO: Do better than this, not very good/optimized I think
static const unsigned int compilers_paths_len = 3; // TODO: Do better than this, not good to maintain I think
struct source {
	char* path; // Relative path
	char* output; // Output object
	char* libs[]; // Optional libraries, like -lm for math.h
};
struct impl {
	char* path; // Relative path
	char* output; // Output object
	char* libs[]; // Optional libraries, like -lm for math.h
};
typedef struct {
	char* output; // Binary output
	struct source* sources;
	size_t sources_len;
} AvoProject;
static void _check_compiler()
{
	if(compiler.path[0] == 0)
	{
		printf("Looking for compiler...\n");
		char* PATH = getenv("PATH");
		if(PATH == NULL)
		{
			fprintf(stderr, "Error while getting PATH var env\n");
			return;
		}
		char tmp[1024];
		unsigned int index = 0;
		char compiler_found = 0;
		size_t i = 0;
		// Open /dev/null to write the commands output there (maybe do something else to prevent that)
		int devnull = open("/dev/null", O_WRONLY);
		if(devnull==-1)
		{
			fprintf(stderr, "Could not open /dev/null");
			exit(1);
		}
		while(*PATH!=0&&!compiler_found)
		{
			// TODO: Add Winbloat support
			if(*PATH != ':')
			{
				if (index >= sizeof(tmp) - 1) {
					fprintf(stderr, "PATH entry too long\n");
					break;
				}
				tmp[index]=*PATH;
				index++;
			} else {
				size_t j = 0;
				while(j < compilers_paths_len && !compiler_found)
				{
					tmp[index]=0;
					char tmp_path[2048];
					strcpy(tmp_path, tmp);
					strcat(tmp_path, "/");
					strcat(tmp_path, compilers_paths[j]);

					if(!access(tmp_path, X_OK))
					{
						printf("Compiler found: %s\n", tmp_path);
						compiler_found = 1;
						int conf_file_d = open("avobswic.ini", O_WRONLY | O_CREAT | O_TRUNC,
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
						/* FILE* conf_f = fopen("avobswic.ini", "w"); */
						/* if(conf_f == NULL) */
						/* { */
						/* 	perror("config file"); */
						/* 	exit(1); */
						/* } */
						if(conf_file_d == -1)
						{
							perror("Error create \"avobswic.ini\"");
						};

						char compiler_entry[4096];
						strcpy(compiler_entry, "COMPILER_PATH=");
						strcat(compiler_entry, tmp_path);
						
						write(conf_file_d, compiler_entry, strlen(compiler_entry));

						close(conf_file_d);
					}else{
						memset(tmp_path,0,1024+128);
					}
					j++;
				}

				// Clear tmp buffer to check more compiler paths.
				memset(tmp,0,1024);
				index = 0;
			}
			PATH++;
			i++;
		}
		close(devnull);
	}
}

static const char* _get_compiler()
{
	int config_fd = open("avobswic.ini", O_RDONLY);
	if(config_fd==-1)
	{
		_check_compiler();
	}
	char buf[2];
	buf[1]=0;
	int ret;
	config_fd = open("avobswic.ini", O_RDONLY);
	if (config_fd == -1) {
    perror("open avobswic.ini");
    return NULL;
	}
	while((ret = read(config_fd, buf, 1)) > 0)
	{
		// Does that mean EOF?
		if(buf[0]==0)
		{
		}

	}
	close(config_fd);
	return NULL;
}

// Configure some options
void AvoConfigure()
{
	_check_compiler();
}

// path is the relative path from the src dir.
void AvoAddSrc(AvoProject* prjct, char* path, ...)
{
	_get_compiler();
}
