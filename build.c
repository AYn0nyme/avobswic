#include "avobswic.h"

int main()
{
	AvoProject cmd = {};
	AvoSetOutput(&cmd, "hello");
	AvoAddSrc(&cmd, "main.c");
	AvoCompile(&cmd);
}
