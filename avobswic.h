/* TODO: Write a comment here like other professional projects and stuff */

// TODO: Refactor the code, it's not really good right now

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
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
	#define AVO_CFLAGS ""
#endif

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
struct Config {
	char* compile_path;
	char* cflags;
	char* src_dir;
	char* include_dir;
};
static const char* compilers_paths[] = {"tcc", "gcc", "cc"}; //TODO: Do better than this, not very good/optimized I think
static const unsigned int compilers_paths_len = 3; // TODO: Do better than this, not good to maintain I think
static struct Config config = {};
static void _set_cflags()
{
	config.cflags = (char*)malloc(strlen(AVO_CFLAGS));
	strcpy(config.cflags, AVO_CFLAGS);
	printf("CFLAGS: %s\n", AVO_CFLAGS);
}
static void _set_src_dir()
{
	config.src_dir = (char*)malloc(strlen(AVO_SRC_DIR));
	strcpy(config.src_dir, AVO_SRC_DIR);
}
static void _set_include_dir()
{
	config.include_dir = (char*)malloc(strlen(AVO_INCLUDE_DIR));
	strcpy(config.include_dir, AVO_INCLUDE_DIR);
}
static void _check_compiler()
{
	if(config.compile_path == NULL)
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
						config.compile_path = (char*)malloc(strlen(tmp_path));
						strcpy(config.compile_path, tmp_path);
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
	}
}

static void _read_config()
{
	int config_fd = open("avobswic.ini", O_RDONLY);
	if(config_fd < 0)
	{
		config_fd = creat("avobswic.ini", S_IROTH | S_IWGRP | S_IWUSR | S_IRGRP | S_IRUSR);
		if(config_fd < 0)
		{
		perror("read_config");
		exit(1);
		}
	}
	struct stat config_stat;
	if(stat("avobswic.ini", &config_stat))
	{
		perror("read_config");
		exit(2);
	}
	size_t config_file_bsize = config_stat.st_size;
	char buf[config_file_bsize];
	if(read(config_fd, buf, config_file_bsize)==-1)
	{
		perror("read_config");
		exit(3);
	}
	for(size_t i = 0; i < config_file_bsize; i++)
	{
		if(buf[i]=='=')
		{
			char *key = strndup(buf, i);
			char *value = buf+i+1;
			if(!strcmp(key, "COMPILER_PATH"))
			{
				config.compile_path = strdup(value);
			}
		}
	}
	close(config_fd);
}
static void _write_config() {
	int config_fd = open("avobswic.ini", O_WRONLY);
	if(config_fd < 0)
	{
		config_fd = creat("avobswic.ini", S_IROTH | S_IWGRP | S_IWUSR | S_IRGRP | S_IRUSR);
		if(config_fd < 0)
		{
			perror("write_config");
			exit(1);
		}
	}
	char* buf = (char*)malloc(1);
	if(config.compile_path != NULL)
	{
		buf = (char*)realloc(buf, strlen(config.compile_path)+strlen("COMPILER_PATH=\n")+strlen(buf)+1);
		strcpy(buf, "COMPILER_PATH=");
		strcat(buf, config.compile_path);
		strcat(buf,"\n");
	}
	if(strlen(config.cflags)>0)
	{
		buf = (char*)realloc(buf, strlen(config.cflags)+strlen("CFLAGS=\n")+strlen(buf)+1);
		strcat(buf, "CFLAGS=");
		strcat(buf, config.cflags);
		strcat(buf,"\n");
	}
	if(strlen(config.src_dir)>0)
	{
		buf = (char*)realloc(buf, strlen(config.src_dir)+strlen("SRC_DIR=\n")+strlen(buf)+1);
		strcat(buf, "SRC_DIR=");
		strcat(buf, config.src_dir);
		strcat(buf,"\n");
	}

	if(write(config_fd, buf, strlen(buf)) == -1)
	{
		perror("write_config");
		exit(2);
	}
	close(config_fd);
}


// Configure some options
void AvoConfigure()
{
	_set_cflags();
	_set_src_dir();
	_set_include_dir();
	_check_compiler();
	_write_config();
}

// path is the relative path from the src dir.
void AvoAddSrc(AvoProject* prjct, char* path, ...)
{
	// Nothing
}
