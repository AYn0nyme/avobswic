#include "avobswic.h"

int main()
{
	AvoProject cmd = {};
	AvoAddSrc(&cmd, "main.c");
	AvoCompile(&cmd);
}
