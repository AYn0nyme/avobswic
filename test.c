#include "avobswic.h"

int main()
{
	AvoCmd_t cmd = {0};
	AvoAddSrc(&cmd, "test.c");
	return 0;
}
