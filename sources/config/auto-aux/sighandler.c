#include <signal.h>

int main(void)
{
#if defined(_GNU_SOURCE)
  sighandler_t old;
#elif defined(_BSD_SOURCE)
  sig_t old:
#else
  void *old;
#endif

  old = signal(SIGQUIT, SIG_DFL);
  return 0;
}
