#include "swap.h"

void Swap(char *left, char *right)
{
	// ваш код здесь
    if (!left || !right) return;
    char tmp = *left;
    *left = *right;
    *right = tmp;
}
