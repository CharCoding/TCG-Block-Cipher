#include <fstream>
#include <pthread.h>
#include <ctime>
#include <csignal>
#include <iostream>
using namespace std;
static const unsigned mask = 0xffffff, l = mask - 1;
volatile sig_atomic_t p = 0;
pthread_t BF1, BF2, BF3, BF4;
int p1, p2, p3, p4;
fstream out("out", out.in | out.out | out.app);
fstream in("in", in.in | in.out);
void stopping(int sig){
  if(p) exit(0);
  p = 1;
  nanosleep((const struct timespec[]){{0, 32767}}, NULL); // hopefully by this point all the values updated
  in.seekg(0);
  in << p1 << ' ' << p2 << ' ' << p3 << ' ' << p4;
  in.seekg(0);
  out.seekg(0);
  cout << '\n' << out.rdbuf() << in.rdbuf() << endl;
  nanosleep((const struct timespec[]){{7200, 0}}, NULL);
  raise(SIGINT);
}
void *bf(void *v) {
  unsigned char a = *(int*)v >> 24; // it will take months to overflow a uint8
  unsigned int b = *(int*)v & mask;
  const unsigned char k = 2 - ((a & 2) >> 1);
  for(;;) { // removed check because this will take years to finish
    for(; b < mask; b += 2) {
      for(unsigned int x = b, i = 0; i < mask; i++) {
        if(!x) break;
        if(i == l) {
          out << +a << ' ' << b << '\n';
          break;
        }
        x = a * (x * (x + 1) >> 1) + b & mask;
        if(p) {
          *(int*)v = a << 24 | b;
          nanosleep((const struct timespec[]){{900, 0}}, NULL);
          p = 0;
        }
      }
    }
    b = k;
    a += 8;
  }
  return NULL;
}
int main(){
  signal(SIGINT, stopping);
  in >> p1 >> p2 >> p3 >> p4;
  pthread_create(&BF1, NULL, bf, &p1);
  pthread_create(&BF2, NULL, bf, &p2);
  pthread_create(&BF3, NULL, bf, &p3);
  pthread_create(&BF4, NULL, bf, &p4);
  nanosleep((const struct timespec[]){{7200, 0}}, NULL);
  raise(SIGINT);
  pthread_join(BF1, NULL);
  pthread_join(BF2, NULL);
  pthread_join(BF3, NULL);
  pthread_join(BF4, NULL);
  in.close();
  out.close();
}
