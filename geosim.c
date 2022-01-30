#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#ifdef _WIN32
	#include <conio.h>
#else
	#include <stdio.h>
	#define clrscr() printf("\e[1;1H\e[2J")
#endif


static char* normal = "Looks fine to me.";
static char* shifty = "Things are a bit shifty!";
static char* earthquake = "EARTHQUAKE!";

static int SEED = 0;

static int hash[] = {208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
                     185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
                     9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
                     70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
                     203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
                     164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
                     228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
                     232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
                     193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
                     101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
                     135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
                     114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219};

int noise2(int x, int y)
{
    int tmp = hash[(y + SEED) % 256];
    return hash[(tmp + x) % 256];
}

float lin_inter(float x, float y, float s)
{
    return x + s * (y-x);
}

float smooth_inter(float x, float y, float s)
{
    return lin_inter(x, y, s * s * (3-2*s));
}

float noise2d(float x, float y)
{
    int x_int = x;
    int y_int = y;
    float x_frac = x - x_int;
    float y_frac = y - y_int;
    int s = noise2(x_int, y_int);
    int t = noise2(x_int+1, y_int);
    int u = noise2(x_int, y_int+1);
    int v = noise2(x_int+1, y_int+1);
    float low = smooth_inter(s, t, x_frac);
    float high = smooth_inter(u, v, x_frac);
    return smooth_inter(low, high, y_frac);
}

float perlin2d(float x, float y, float freq, int depth)
{
    float xa = x*freq;
    float ya = y*freq;
    float amp = 1.0;
    float fin = 0;
    float div = 0.0;

    int i;
    for(i=0; i<depth; i++)
    {
        div += 256 * amp;
        fin += noise2d(xa, ya) * amp;
        amp /= 2;
        xa *= 2;
        ya *= 2;
    }

    return fin/div;
}

typedef struct Earth {
	int age; // age in years
	int heightMap[40]; // height of the crust at each column
	int mantleLevel; // Y level of the mantle
	int plates[3]; // column dividers of the plates
	char plateDirs[3]; // ~ = stationary, < = left, > = right
	int oceanLevel; // Y level of the ocean
	char* status; // the earth's status line
} Earth;

Earth createEarth(float roughness, int mantleLevel, int oceanLevel, int plate1, int plate2, int plate3) {
	Earth e;
	e.age = 0;
	e.mantleLevel = mantleLevel;
	e.oceanLevel = oceanLevel;
	e.plates[0] = plate1;
	e.plates[1] = plate2;
	e.plates[2] = plate3;

	e.plateDirs[0] = '~';
	e.plateDirs[1] = '~';
	e.plateDirs[2] = '~';

	e.status = normal;

	// generate surface
	for(int i = 0; i < 40; i++){
//		printf("%d", perlin2d(i, 0, 1, roughness / 2));
		float peak = perlin2d(i, 0, roughness / 10.0f, roughness / 2.0f);
		e.heightMap[i] = floor(peak * 40);
	}

	return e;
}

void printEarth(Earth e){
	printf("Age (in millions): %d\n", e.age);
//	printf("Mantle is at level: %d\n", e.mantleLevel);
	printf("Plates are at: %d%c %d%c %d%c\n", e.plates[0], e.plateDirs[0],
						  e.plates[1], e.plateDirs[1],
						  e.plates[2], e.plateDirs[2]);
	printf("%s\n", e.status);
//	printf("Ocean is at: %d\n", e.oceanLevel);
	for(int y = 0; y < 20; y++){
		for(int x = 0; x < 40; x++){
			char* td = " ";
			if(e.heightMap[x] == y){
				td = "▁";
			}
			if(e.heightMap[x] < y){
				if(20 - e.mantleLevel >= y){
        				td = "█";
				} else {
					td = "▓";
				}
			}
			else if(20 - e.oceanLevel >= y){
				td = " ";
			}
			else if(20 - e.mantleLevel >= y){
                        	td = "░";
	                } else {
        	                td = "▓";
			}
			for(int i = 0; i < 3; i++){
				if(e.plates[i] == x){
					td = "┃";
				}
			}
			printf(td);
		}
		printf("%d", y);
		printf("\n");
	}
}

Earth stepEarth(Earth e, int times){
	for(int t = 0; t < times; t++) {
	e.age += 10;
	e.status = normal;
	for(int i = 0; i < 3; i++) {
		switch(e.plateDirs[i]){
			case '~':
				break;
			case '>':
				if(e.plates[i] + 1 < 39) {
					e.heightMap[e.plates[i]] = e.heightMap[e.plates[i] + 1];
					e.status = shifty;
					e.plates[i] ++;
				}
				e.plateDirs[i] = rand() % 3 == 0 ? '~' : e.plateDirs[i];
				break;
			case '<':
				if(e.plates[i] - 1 > 1){
					e.heightMap[e.plates[i]] = e.heightMap[e.plates[i] - 1];
					e.status = shifty;
					e.plates[i] --;
				}
				e.plateDirs[i] = rand() % 3 == 0 ? '~' : e.plateDirs[i];
				break;
			default:
				printf("Hmm...");
		}
		if(e.plateDirs[i] == '~'){
			e.plateDirs[i] = rand() % 3 == 0 ? '~' : rand() % 2 == 1 ? '>' : '<';
		}
	}
	if(e.plates[0] == e.plates[1]){
		e.status = earthquake;
		if(e.heightMap[e.plates[0]] > 3) {
                        e.heightMap[e.plates[1] - 1] --;
                        e.heightMap[e.plates[1]] -= 2;
                        e.heightMap[e.plates[1] + 1] --;
                }
		e.plateDirs[0] = e.plateDirs[0] == '>' ? '<' : e.plateDirs[0] == '~' ? '~' : '>';
		e.plateDirs[1] = e.plateDirs[1] == '>' ? '<' : e.plateDirs[1] == '~' ? '~' : '>';
	} else if(e.plates[1] == e.plates[2]) {
		e.status = earthquake;
		if(e.heightMap[e.plates[1]] > 3) {
			e.heightMap[e.plates[1] - 1] --;
	        	e.heightMap[e.plates[1]] -= 2;
                	e.heightMap[e.plates[1] + 1] --;
                }
		e.plateDirs[1] = e.plateDirs[1] == '>' ? '<' : e.plateDirs[1] == '~' ? '~' : '>';
                e.plateDirs[2] = e.plateDirs[2] == '>' ? '<' : e.plateDirs[2] == '~' ? '~' : '>';
	}
	}
	return e;
}

int main(){
	Earth e = createEarth(2.0f, 3, 9, 5, 10, 15);
	while(1){
		clrscr();
		e = stepEarth(e, 10);
		printEarth(e);
		getchar();
	}
}
