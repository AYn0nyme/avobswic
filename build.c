#include "avobswic.h"

int main(int argc, char* argv[])
{
	AvoProject cmd = {0};
	AvoSetOutput(&cmd, "hello");
	AvoAddSrc(&cmd, "main.c", "x11", NULL);
	AvoCompile(&cmd);
}
