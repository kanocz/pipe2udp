#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void dieWithError(char *s) { perror(s); exit(1); }

int main(int argc, char *argv[])
{
  if (3 != argc) {
    printf("usage: %s <IP> <PORT>\n", argv[0]);
    puts("  P.S.: anton@nsl.cz :)");
    exit(1);
  }

  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == -1) dieWithError("unable to init socket");

  struct sockaddr_in si_other;
  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;
  si_other.sin_port = htons(atoi(argv[2]));
  if (inet_aton(argv[1], &si_other.sin_addr)==0)
    dieWithError("inet_aton failed");

  int slen = sizeof(si_other);

  char buf[1501];

  int maxLen = sizeof(buf) - 1;

  while (0 != fgets(buf, maxLen, stdin)) {
    if (sendto(sock, buf, strlen(buf), 0, (const struct sockaddr *) &si_other, slen)==-1)
      dieWithError("sending udp datagram");
// uncomend to send copy to stdout (tee like)
//    fputs(buf, stdout);
    fflush(stdout);
  }

  close(sock);

  return 0;
}
