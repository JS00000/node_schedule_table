#include <cstdio>
#include <cstdlib>
#include <ctime>

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

// generate more general jobs
int main(int argc, char const *argv[])
{
    srand(time(0));
    // grid of size * size
    for (int size = 2; size <= 4; size++) {
        int size2 = size * size;
        // mw: the max bandwidth of the jobs graph
        for (int mw = 1; mw <= size2; mw++) {
            char file_name[50];
            sprintf(file_name, "../input/rand_jobs/jobs_%dx%d_%d.txt", size2, size2, mw);
            FILE* f = freopen(file_name, "w", stdout);
            printf("%d %d\n", size2, size2);
            for (int i = 0; i < size2; i++) {
                int l = (mw + 1) / 2 + (rand() % (mw - (mw + 1) / 2 + 1));
                int sst = max(0, i - l / 2);
                int st = sst + (rand() % (i - sst + 1));
                // [st, st + l) is 1
                for (int j = 0; j < size2; j++) {
                    if ((st <= j) && (j < st + l))
                        printf("1 ");
                    else
                        printf("0 ");
                }
                printf("\n");
            }
        }
    }
    return 0;
}
