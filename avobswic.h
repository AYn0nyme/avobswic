/* TODO: Write a comment here like other professional projects and stuff */

// TODO: Refactor the code, it's not really good right now
// TODO: Prevent writing default config
// TODO: AvoCompile();

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
#ifndef AVO_OUT_DIR
	#define AVO_OUT_DIR "bin"
#endif

struct source {
	char* path  ; // Relative path
	char* libs[]; // Optional libraries, like -lm for math.h
};
struct impl {
	char* path  ; // Relative path
	char* libs[]; // Optional libraries, like -lm for math.h
};
typedef struct {
	char*  output               ; // Binary output
	struct source* sources      ; // Source files
	size_t sources_len          ; // Length of source files
	struct impl* implementations; // Implementations of header files (idk...)
	size_t implementations_len  ; // Length of implementations of header files
} AvoProject;
struct Config {
	char* compile_path; // Compiler path TODO: rename it to compiler_path
	char* cflags      ; // The CFLAGS for each build
	char* src_dir     ; // Src directory
	char* include_dir ; // Include directory
	char* out_dir     ; // Output directory
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
static void _set_out_dir()
{
	config.out_dir = (char*)malloc(strlen(AVO_OUT_DIR));
	strcpy(config.out_dir, AVO_OUT_DIR);
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
	unsigned int cursor = 0;
	for(size_t i = 0; i < config_file_bsize; i++)
	{
		char* key = NULL;
		char* value = NULL;
		if (buf[i]=='=')
		{
			key = strndup(buf+cursor, i-cursor);
			if(i+1<config_file_bsize)
				cursor = i+1;
			for(size_t j = i; j < config_file_bsize; j++)
			{
				if (buf[j] == '\n' || buf[j] == -99)
				{
					value = strndup(buf+cursor, j-cursor);

					if(!strcmp(key,"COMPILER_PATH"))
					{
						config.compile_path = strdup(value);
					}
					if(!strcmp(key,"CFLAGS"))
					{
						config.cflags = strdup(value);
					}
					if(!strcmp(key,"SRC_DIR"))
					{
						config.src_dir = strdup(value);
					}
					if(!strcmp(key,"OUT_DIR"))
					{
						config.out_dir = strdup(value);
					}
					if(!strcmp(key,"INCLUDE_DIR"))
					{
						config.include_dir = strdup(value);
					}
					if(j+1<config_file_bsize)
						i=j+1;
					if(j+1<config_file_bsize)
						cursor=j+1;
					break;
				}
			}
		}
	}
	if(config.compile_path == NULL)
	{
		fprintf(stderr, "Compiler not found.\n");
		exit(4);
	}
	if(config.cflags == NULL)
	{
		config.cflags = (char*)malloc(strlen(AVO_CFLAGS));
		strcpy(config.cflags, AVO_CFLAGS);
	}
	if(config.src_dir == NULL)
	{
		config.src_dir = (char*)malloc(strlen(AVO_SRC_DIR));
		strcpy(config.src_dir, AVO_SRC_DIR);
	}
	if(config.out_dir == NULL)
	{
		config.out_dir = (char*)malloc(strlen(AVO_OUT_DIR));
		strcpy(config.out_dir, AVO_OUT_DIR);
	}
	if(config.include_dir == NULL)
	{
		config.include_dir = (char*)malloc(strlen(AVO_INCLUDE_DIR));
		strcpy(config.include_dir, AVO_INCLUDE_DIR);
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
	}	if(strlen(config.src_dir)>0)
	{
		buf = (char*)realloc(buf, strlen(config.src_dir)+strlen("SRC_DIR=\n")+strlen(buf)+1);
		strcat(buf, "SRC_DIR=");
		strcat(buf, config.src_dir);
		strcat(buf,"\n");
	}
	if(strlen(config.include_dir)>0)
	{
		buf = (char*)realloc(buf, strlen(config.include_dir)+strlen("INCLUDE_DIR=\n")+strlen(buf)+1);
		strcat(buf, "INCLUDE_DIR=");
		strcat(buf, config.include_dir);
		strcat(buf,"\n");
	}
	if(strlen(config.out_dir)>0)
	{
		buf = (char*)realloc(buf, strlen(config.out_dir)+strlen("OUT_DIR=\n")+strlen(buf)+1);
		strcat(buf, "OUT_DIR=");
		strcat(buf, config.out_dir);
		strcat(buf,"\n");
	}

	if(write(config_fd, buf, strlen(buf)) == -1)
	{
		perror("write_config");
		exit(2);
	}
	close(config_fd);
}

static char* _basename(char*);
static char* _absolute_path(char*);
static char* _object_name_from_source(char*);


// Configure some options
void AvoConfigure()
{
	_set_cflags();
	_set_src_dir();
	_set_include_dir();
	_set_out_dir();
	_check_compiler();
	_write_config();
}

void AvoSetOutput(AvoProject* prjct, char* bin_name)
{
	prjct->output = (char*)malloc(strlen(bin_name));
	strcpy(prjct->output, bin_name);
}

// path is the relative path from the src dir.
void AvoAddSrc(AvoProject* prjct, char* path, ...)
{
	if(prjct->sources == NULL)
	{
		prjct->sources = (struct source*)malloc(sizeof(struct source*));
		prjct->sources_len = 0;
	}
	struct stat file_stat;
	char file_path[strlen(path)+strlen(AVO_SRC_DIR)+1];
	strcpy(file_path, AVO_SRC_DIR);
	strcat(file_path, "/");
	strcat(file_path, path);
	if(stat(file_path, &file_stat) == -1)
	{
		perror(file_path);
		exit(1);
	}
	prjct->sources = (struct source*)realloc(prjct->sources, sizeof(struct source*)*(prjct->sources_len+2));
	struct source source_file;
	source_file.path = (char*)malloc(strlen(file_path)+1);
	strcpy(source_file.path, file_path);
	prjct->sources[prjct->sources_len] = source_file;
	prjct->sources_len++;
}

int AvoCompile(AvoProject* prjct)
{
	_read_config();
	if(prjct->sources_len==0)
	{
		printf("Nothing to compile.\nExiting...\n");
		exit(0);
	}
	struct stat build_dir;
	char** compiled_sources = NULL;
	size_t compiled_sources_len = 0;
	if(stat(config.out_dir, &build_dir) == -1)
	{
		if(errno == ENOENT)
		{
			mkdir(config.out_dir, 0777);
		} else {
			perror("out_dir opening");
			exit(3);
		}
	}
	for(size_t i = 0; i < prjct->sources_len; i++)
	{
		pid_t compiler_pid;
		compiler_pid = fork();
		// Error
		if(compiler_pid < 0)
		{
			perror("compile");
			exit(1);
		}
		char output_file_path[strlen(_absolute_path(config.out_dir))+strlen(_absolute_path(prjct->sources[i].path)+1)];
		strcpy(output_file_path, _absolute_path(config.out_dir));
		strcat(output_file_path, "/");
		strcat(output_file_path, _object_name_from_source(_basename(prjct->sources[i].path)));
		// We're in the forked process
		if(compiler_pid == 0)
		{
			char** argv = (char**)malloc(sizeof(char*)*5);
			size_t argc = 0;
			/* = { */
			/*	config.compile_path, */
			/*	"-c", */
			/*	_absolute_path(prjct->sources[i].path), */
			/*	"-o", */
			/*	output_file_path, */
			/* }; */
			argv[0] = config.compile_path;
			argv[1] = "-c";
			argv[2] = _absolute_path(prjct->sources[i].path);
			argv[3] = "-o";
			argv[4] = output_file_path;
			argc=5;
			if(strlen(config.cflags)==0)
			{
				argv = (char**)realloc(argv, sizeof(char*)*(argc+1));
				argv[argc] = NULL;
			}
			else
			{
				unsigned int cursor = 0;
				for(size_t j = 0; j < strlen(config.cflags)+1; j++)
				{
					if(config.cflags[j] == ' ' || config.cflags[j] == 0)
					{
						argv = (char**)realloc(argv, sizeof(char*)*(argc+1));
						argv[argc] = strndup(config.cflags+cursor, j-cursor);
						if(j+1<strlen(config.cflags)+1)
							cursor=j+1;
						else
							cursor = j;
						argc++;
					}
				}
				argv = (char**)realloc(argv, sizeof(char*)*(argc+1));
				argv[argc] = NULL;
			}
			if(execv(config.compile_path, argv))
			{
				perror("compiling");
				exit(2);
			}
		} else {
			int ret = 0;
			waitpid(compiler_pid, &ret, 0);
			if(ret != 0)exit(33);
			compiled_sources = (char**)realloc(compiled_sources, sizeof(char*)*(compiled_sources_len+1));
			compiled_sources[compiled_sources_len] = (char*)malloc(strlen(output_file_path));
			strcpy(compiled_sources[compiled_sources_len], output_file_path);
			compiled_sources_len++;
			memset(output_file_path,0,strlen(output_file_path));
		}
	}

	// Add implementations?
	char** argv = (char**)malloc(sizeof(char*));
	argv[0] = config.compile_path;
	size_t argc = 1;
	for(size_t i = 0; i < compiled_sources_len; i++)
	{
		argv = (char**)realloc(argv, sizeof(char*)*(argc+1));
		argv[argc] = strdup(compiled_sources[i]);
		argc++;
	}
	argc+=3;
	argv = (char**)realloc(argv, sizeof(char*)*(argc));
	argv[argc-3] = strdup("-o");
	char project_output[strlen(prjct->output)+strlen(_absolute_path(config.out_dir))+1];
	strcpy(project_output, _absolute_path(config.out_dir));
	strcat(project_output, "/");
	strcat(project_output, strdup(prjct->output));
	argv[argc-2] = strdup(project_output);
	argv[argc-1] = NULL;

	pid_t compiler_pid = fork();
	if(compiler_pid<0)
	{
		perror("assembling objects");
		exit(3);
	}
	else if(compiler_pid == 0)
	{
		if(execv(config.compile_path, argv))
		{
			perror("assembling objects");
			exit(4);
		}
	}
	else
	{
		int ret = 0;
		waitpid(compiler_pid, &ret, 0);
		if(ret != 0)
		{
			exit(5);
		}
	}
	return 0;
}

static char* _basename(char*s)
{
	if(s==NULL)return 0;
	unsigned int i=0;
	unsigned int lastSlashIndex = 0;
	char* t = strdup(s);
	while(*t != 0)
	{
		if(*t == '/')lastSlashIndex=i;
		i++;
		t++;
	}
	if(lastSlashIndex!=0)return s+(lastSlashIndex+1);
	return s;
}

static char* _absolute_path(char*s)
{
	if(s==NULL)return 0;
	char* buf = (char*)malloc(4096*2);
	getcwd(buf, 4096*2);
	buf = (char*)realloc(buf, strlen(buf)+strlen(_basename(s))+1);
	strcat(buf, "/");
	strcat(buf, _basename(strdup(s)));
	return buf;
}

static char* _object_name_from_source(char*s)
{
	if(s==NULL)return 0;
	char* t = strndup(s, strlen(s)-1);
	strcat(t, "o");
	return t;
}
