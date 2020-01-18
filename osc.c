#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define N 72 // 2*N_osc
#define h 0.01

void init_s(double *s) {
	for (int i=0; i<N; i++) {
		s[i] = 0.;
	}
}

void force(double *in, double *out) {
	for (int i=0; i<N; i+=2) {
		out[i] = in[i+1];
		out[i+1] = -50*(2*in[i]-in[i?i-2:N-2]-in[i!=N-2?i+2:0]);
	}
}

void rk4(double *s, double *tmp, double* k1, double *k2, double *k3, double *k4) {
	force(s, k1);

	for (int i=0; i<N; i++)
		tmp[i] = s[i]+k1[i]*h*.5;
	force(tmp, k2);

	for (int i=0; i<N; i++)
		tmp[i] = s[i]+k2[i]*h*.5;
	force(tmp, k3);

	for (int i=0; i<N; i++)
		tmp[i] = s[i]+k3[i]*h;
	force(tmp, k4);

	for (int i=0; i<N; i++)
		s[i] += (k1[i]+k2[i]*2.+k3[i]*2.+k4[i])*(h/6.);
}

void msleep(long ms) {
	struct timespec req = {0, ms*1000000}, rem;
	nanosleep(&req, &rem);
}

void print_osc(int x) {
	if (x > 0) {
		printf("%*c", 40, ' ');
		for (int i=0; i<x; i++) printf("%s", "█");
		printf("\n");
	} else {
		printf("%*c█", 40+x, ' ');
		for (int i=0; i<-x; i++) printf("%s", "█");
		printf("\n");
	}
}

int main() {
	double *s = malloc(N*sizeof(double));
	double *tmp = malloc(N*sizeof(double));

	double *k1 = malloc(N*sizeof(double));
	double *k2 = malloc(N*sizeof(double));
	double *k3 = malloc(N*sizeof(double));
	double *k4 = malloc(N*sizeof(double));

	init_s(s);
	printf("\e[1;1H\e[2J"); // clear
	for (int i=0; i<N; i+=2) print_osc(0);
	fflush(stdout);
	sleep(1);

	s[N/2-1] += 100;
	s[N/2+1] += 100;
	s[N/2+3] += 100;
	rk4(s, tmp, k1, k2, k3, k4);
	s[N/2-1] -= 100;
	s[N/2+1] -= 100;
	s[N/2+3] -= 100;

	setvbuf(stdout, NULL, _IOFBF, 0); // fully buffered
	// TODO: provide a buffer ourselves (so that there's only 1 flush)
	for (;;) {
		printf("\e[1;1H\e[2J"); // clear
		for (int i=0; i<N; i+=2) {
			int x = s[i]/0.025;
			print_osc(x);
		}
		printf("Press Ctrl-C to exit ");
		fflush(stdout);
		rk4(s, tmp, k1, k2, k3, k4);
		msleep(16);
	}
}
