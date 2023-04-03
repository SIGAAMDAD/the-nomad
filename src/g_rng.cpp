#include "n_shared.h"
#include "g_game.h"

int	rndindex = 0;

int G_GetRandom(int difficulty)
{
	int rnd = rand() % 256;
	int num = 0;
	switch (difficulty) {
	case 0: // DIF_NOOB
		num = rndtable[rnd & 50];
		break;
	case 1: // DIF_RECRUIT
		num = rndtable[rnd & 100];
		break;
	case 2: // DIF_MERCENART
		num = rndtable[rnd & 150];
		break;
	case 3: // DIF_NOMAD
		num = rndtable[rnd & 200];
		break;
	case 4: // DIF_BLACKDEATH
		num = rndtable[rnd & 250];
		break;
	case 5: // DIF_MINORINCONVENIENCE
		num = rndtable[rnd & 300];
		break;
	default: break;
	};
	return num;
}

int P_Random(void)
{
	if (rndindex > 255) {
		G_ClearRandom();
	}
    rndindex = (rndindex+1) & 0xff;
    return rndtable[rndindex];
}

void G_ClearRandom(void)
{
	rndindex = 0;
}