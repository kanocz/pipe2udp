#define _BSD_SOURCE

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void dieWithError(char *s) { perror(s); exit(1); }

#define MAX_SOCKET_COUNT 8
#define BUFFER_SIZE 16384
// 1500 - 20 (ip header) - 8 (udp header) ... we work only with IPv4 now :)
#define MAX_PACKET_SIZE 1472

// comment this if you're using so_reuse_port balancing in the other side
#define USE_SAME_SOCKET

#define ALARM_COUNTER 50000
#define ALARM_TIMEOUT 60

int main(int argc, char *argv[])
{
  if (2 != argc && 3 != argc) {
    printf("usage:\n\t%1$s <config file>\n\t%1$s <IP> <PORT>\n", argv[0]);
    puts("\n  P.S.: anton@nsl.cz :)");
    exit(1);
  }

  int sockCount = 0;
#ifdef USE_SAME_SOCKET
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == -1) dieWithError("unable to init socket");
#else
  int sock[MAX_SOCKET_COUNT];
#endif

  struct sockaddr_in saddr[MAX_SOCKET_COUNT];

  if (argc == 3) {

    memset((char *) &saddr[sockCount], 0, sizeof(struct sockaddr_in));
    saddr[sockCount].sin_family = AF_INET;
    saddr[sockCount].sin_port = htons(atoi(argv[2]));
    if (inet_aton(argv[1], &saddr[sockCount].sin_addr)==0)
      dieWithError("inet_aton failed");

#ifndef USE_SAME_SOCKET
    sock[sockCount] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock[sockCount] == -1) dieWithError("unable to init socket");
#endif

    sockCount++;
  } else {

    int configLine = 0;

    FILE *config = fopen(argv[1], "r");
    if (!config) dieWithError("Unable to open config file");

    char line[256]; line[255] = 0;


    while ((sockCount < MAX_SOCKET_COUNT) && fgets(line, sizeof(line)-1, config))
    {
      configLine++;
      if (line[0] == '\n') continue;
      char *s_port = index(line, ':');
      if (!s_port)
      {
        fprintf(stderr, "Invalid ip:port at %s:%d: %s", argv[1], configLine, line);
        continue;
      }

      s_port++[0] = 0;

      int port = atoi(s_port);
      if (port < 1 || port > 65535)
      {
        fprintf(stderr, "Invalid port number %d at %s:%d\n", port, argv[1], configLine);
        continue;
      }

      memset((char *) &saddr[sockCount], 0, sizeof(struct sockaddr_in));
      saddr[sockCount].sin_family = AF_INET;
      saddr[sockCount].sin_port = htons(port);
      if (inet_aton(line, & saddr[sockCount].sin_addr)==0)
      {
        fprintf(stderr, "Invalid IP %s at %s:%d\n", line, argv[1],configLine);
        continue;
      }

#ifdef USE_SAME_SOCKET
      sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (sock == -1) dieWithError("unable to init socket");
#else
      sock[sockCount] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (sock[sockCount] == -1) dieWithError("unable to init socket");
#endif

      sockCount++;
    }

    fclose(config);
  }


// make buffer much lager, but send only first 1500 bytes
  char buf[BUFFER_SIZE]; buf[BUFFER_SIZE - 1] = 0;
  int maxLen = sizeof(buf) - 1;

  int c = 0;
#if ALARM_COUNTER > 0
  int alarmCounter = 0;
#endif // USE ALARM

  while (0 != fgets(buf, maxLen, stdin)) {
    int len = strlen(buf);

#ifdef USE_SAME_SOCKET
    if (sendto(sock, buf, (len > MAX_PACKET_SIZE) ? MAX_PACKET_SIZE : len, 0, (const struct sockaddr *) &saddr[c], sizeof(struct sockaddr_in))==-1)
#else
    if (sendto(sock[c], buf, (len > MAX_PACKET_SIZE) ? MAX_PACKET_SIZE : len, 0, (const struct sockaddr *) &saddr[c], sizeof(struct sockaddr_in))==-1)
#endif
      dieWithError("sending udp datagram");
    if (++c == sockCount) c = 0;
#if ALARM_COUNTER > 0
    if (alarmCounter++ % 50000 == 0)
    {
      alarm(60);
      alarmCounter = 0;
    }
#endif
  }

  // close all sockets before exit
#ifdef USE_SAME_SOCKET
  close(sock);
#else
  for (int i = 0; i < sockCount; i++)
    close(sock[i]);
#endif


  return 0;
}
