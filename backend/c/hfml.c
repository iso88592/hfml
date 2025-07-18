#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "hfml.h"

#define BACKLOG 10
#define BUF_SIZE 4096

const char* BAD_REQUEST_TEMPLATE = 
  "HTTP/1.1 400 Bad request\r\n"
  "Content-Type: text/plain\r\n"
  "Access-Control-Allow-Origin: *\r\n"
  "Content-Length: 0\r\n\r\n";

const char* OK_TEMPLATE = 
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/hfml\r\n"
  "Access-Control-Allow-Origin: *\r\n"
  "Content-Length: ";

struct hfml_route;

struct hfml_linked {
  void (*func)(HFMLContext*);
  const char* name;
  HFMLContext* context;
  struct hfml_linked *next;
  char* response;
  struct hfml_route *routes;
};

typedef struct hfml_route {
  const char* name;
  void (*invoke)(struct HFMLContext *context);
  struct hfml_route *next;
} hfml_route;

struct hfml_linked *first = NULL;

void trigger_event(struct HFMLContext* context, const char* event, const char* payload) {
  free(context->link->response);
  char buffer[BUF_SIZE];
  snprintf(buffer, 4096, "<[event:effect=%s({%s})]>", event, payload);
  context->link->response = strdup(buffer);
}
void raise_error(struct HFMLContext* context, const char* message) {
  free(context->link->response);
  char buffer[BUF_SIZE];
  snprintf(buffer, 4096, "<[error]{%s}>", message);
  context->link->response = strdup(buffer);
}
void register_endpoint(struct HFMLContext* context, const char* route, void (*cb)(struct HFMLContext *context)) {
  printf("Registering route: `%s`\n", route);
  hfml_route* r = malloc(sizeof(hfml_route));
  r->name = route;
  r->next = context->link->routes;
  r->invoke = cb;
  context->link->routes = r;
}
volatile sig_atomic_t hfml_running = 1;

void handle_signal(int signum) {
    (void)signum;
    hfml_running = 0;
}

int hfml_server_fd;

int make_socket_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int create_server_socket(int port) {
    int sockfd;
    struct sockaddr_in addr;
    int opt = 1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    if (listen(sockfd, BACKLOG) < 0) {
        perror("listen");
        close(sockfd);
        exit(1);
    }

    if (make_socket_nonblocking(sockfd) < 0) {
        perror("fcntl");
        close(sockfd);
        exit(1);
    }

    return sockfd;
}

void hfml_setup() {
  printf(" *** Initializing HFML backend ***\n");
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);
  struct hfml_linked *current = first;
  while (current != NULL) {
    HFMLContext* context = (HFMLContext*)malloc(sizeof(HFMLContext));
    current->context = context;
    current->routes = NULL;
    context->link = current;
    context->name = current->name;
    context->trigger_event = &trigger_event;
    context->raise_error = &raise_error;
    context->register_endpoint = &register_endpoint;
    current->func(context);
    current = current -> next;
  }
  hfml_server_fd = create_server_socket(3273);
  printf("HFML server started on port %d\n", 3273);
  printf(" *** HFML backend initialized  ***\n");

}

char* hfml_process_request(const char* request) {
  if (strncmp(request, "GET ", 4) != 0) {
    return strdup(BAD_REQUEST_TEMPLATE); 
  }
  int pos = 4;
  while (request[pos++] != ' ');
  int size = pos - 4;
  char* path = malloc(size + 1);
  memcpy(path, request + 4, pos - 4);
  path[size] = '\0';
  struct hfml_linked *current = first;
  
  while (current != NULL) {
    hfml_route *c = current->routes;
    while (c != NULL) {
      if (strncmp(c->name, path, strlen(c->name)) == 0) {
        current->response = strdup("<[error]{The controller did not provide an answer.}>");
        c->invoke(current->context);
        char buffer[BUF_SIZE];
        snprintf(buffer, 4096, "%s %d\r\n\r\n%s",
          OK_TEMPLATE,
          strlen(current->response),
          current->response);
        free(current->response);
        free(path);
        return strdup(buffer);
      }
      c = c->next;
    }
    current = current->next;
  }
  free(path);
  return strdup(BAD_REQUEST_TEMPLATE);
}


void hfml_main() {
  fd_set read_fds;
  char buffer[BUF_SIZE];
  printf(" *** HFML main running         ***\n");
  while (hfml_running) {
    FD_ZERO(&read_fds);
    FD_SET(hfml_server_fd, &read_fds);
    int max_fd = hfml_server_fd;

    struct timeval timeout = {1, 0};
    int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
    if (activity < 0 && errno != EINTR) {
      perror("select");
      break;
    }
    if (FD_ISSET(hfml_server_fd, &read_fds)) {
      struct sockaddr_in client_addr;
      socklen_t addrlen = sizeof(client_addr);
      int client_fd = accept(hfml_server_fd, (struct sockaddr*)&client_addr, &addrlen);
      if (client_fd < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
          perror("accept");
        }
      } else {
        int n = read(client_fd, buffer, BUF_SIZE - 1);
        if (n > 0) {
          buffer[n] = '\0';
          char *response = hfml_process_request(buffer);
          write(client_fd, response, strlen(response));
          free(response);
          close(client_fd);
        }
      }
    }
  }
  printf("\n *** HFML main stopped         ***\n");
}

void hfml_teardown() {
  printf(" *** HFML backend teardown     ***\n");
  close(hfml_server_fd);
  struct hfml_linked *current = first;
  while (first != NULL) {
    printf("Tearing down `%s`\n", current->name);
    while (current->routes != NULL) {
      struct hfml_route *r = current->routes;
      current->routes = current->routes->next;
      free(r);
    }
    first = current->next;
    free(current->context);
    free(current);
  }
  printf(" *** Bye!                      ***\n");
}

int main(void) {
  hfml_setup();
  hfml_main();
  hfml_teardown();
}

void __hfml__register(const char* name, void (*func)(HFMLContext* context)) {
  struct hfml_linked* current = (struct hfml_linked*)malloc(sizeof(struct hfml_linked));
  current->func = func;
  current->name = name;
  struct hfml_linked* next = first;
  current->next = next;
  first = current;
}