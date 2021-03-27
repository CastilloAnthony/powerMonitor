#include <iostream>
#include "monitor.h"

int main()
{
    monitor arbiter;
	do {  arbiter.primaryRoutine(); } while (true);
    return 0;
}
