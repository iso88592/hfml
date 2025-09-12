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
#include <regex.h>
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

const char* OPTIONS_TEMPLATE = 
  "HTTP/1.1 204 No Content\r\n" 
  "Access-Control-Allow-Origin: *\r\n"
  "Access-Control-Allow-Methods: GET, POST, PUT, DELETE\r\n"
  "Access-Control-Allow-Headers: Content-Type\r\n"
  "Content-Length: ";

struct hfml_route;

struct hfml_linked {
  void (*func)(HFMLContext*);
  const char* name;
  HFMLContext* context;
  struct hfml_linked *next;
  char* response;
  struct hfml_route *routes;
  struct hfml_route *currentRoute;
};

typedef struct hfml_route {
  const char* name;
  const char* method;
  char** regex_parts;
  int regex_parts_count;
  void (*invoke)(struct HFMLContext *context);
  void (*session_created)(struct HFMLContext *context, struct HFMLSession *session);
  void (*session_deleted)(struct HFMLContext *context);
  struct hfml_route *next;
  regex_t regex;
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

void process_route_regex(hfml_route* r) {
    const char* src = r->name;
    size_t len = strlen(src);

    // worst-case allocate enough space for regex
    char* regex_buf = malloc(len * 2 + 10); 
    strcpy(regex_buf, "^");

    char** parts = NULL;
    int parts_count = 0;

    for (size_t i = 0; i < len; i++) {
        if (src[i] == '{') {
            size_t j = i + 1;
            while (j < len && src[j] != '}') j++;
            if (j >= len) break; // malformed

            // extract param name
            size_t plen = j - (i + 1);
            char* pname = malloc(plen + 1);
            strncpy(pname, src + i + 1, plen);
            pname[plen] = '\0';

            // append to parts list
            parts = realloc(parts, sizeof(char*) * (parts_count + 1));
            parts[parts_count++] = pname;

            // add regex capture
            strcat(regex_buf, "([^/]+)");

            i = j; // skip past '}'
        } else {
            // copy literal, escape regex specials
            if (strchr(".^$|()[]*+?\\", src[i])) {
                size_t l = strlen(regex_buf);
                regex_buf[l] = '\\';
                regex_buf[l+1] = src[i];
                regex_buf[l+2] = '\0';
            } else {
                size_t l = strlen(regex_buf);
                regex_buf[l] = src[i];
                regex_buf[l+1] = '\0';
            }
        }
    }

    strcat(regex_buf, "$");

    // replace name with regex string
    r->name = regex_buf;
    if (regcomp(&r->regex, r->name, REG_EXTENDED) != 0) {
      printf("Could not make regex from %s\n", r->name);
    }
    r->regex_parts = parts;
    r->regex_parts_count = parts_count;  
}

void register_endpoint(struct HFMLContext* context, const char* route, void (*cb)(struct HFMLContext *context)) {
  printf("Registering GET route: `%s`\n", route);
  hfml_route* r = malloc(sizeof(hfml_route));
  r->name = route;
  r->next = context->link->routes;
  r->invoke = cb;
  r->method = "GET";
  process_route_regex(r);
  context->link->routes = r;
}

void register_put_endpoint(struct HFMLContext* context, const char* route, void (*cb)(struct HFMLContext *context)) {
  printf("Registering PUT route: `%s`\n", route);
  hfml_route* r = malloc(sizeof(hfml_route));
  r->name = route;
  r->next = context->link->routes;
  r->invoke = cb;
  r->method = "PUT";
  process_route_regex(r);
  context->link->routes = r;
}

void register_delete_endpoint(struct HFMLContext* context, const char* route, void (*cb)(struct HFMLContext *context)) {
  printf("Registering DELETE route: `%s`\n", route);
  hfml_route* r = malloc(sizeof(hfml_route));
  r->name = route;
  r->next = context->link->routes;
  r->invoke = cb;
  r->method = "DELETE";
  process_route_regex(r);
  context->link->routes = r;
}


void create_session(struct HFMLContext * context) {
  HFMLSession* session = context->createSession(context, context->getSessionName(context));
  hfml_route* route = context->link->currentRoute;
  if (route->session_created != NULL) route->session_created(context, session);
  context->trigger_event(context, "session", "Session deleted successfully.");
}
void delete_session(struct HFMLContext * context) {
  context->deleteSession(context, context->getSessionName(context));
  hfml_route* route = context->link->currentRoute;
  if (route->session_deleted != NULL) route->session_deleted(context);
  context->trigger_event(context, "session", "Session deleted successfully.");
}

void register_session_endpoint(struct HFMLContext* context, const char* route, void (*created)(struct HFMLContext *context, struct HFMLSession *session), void (*deleted)(struct HFMLContext *context)) {
  register_put_endpoint(context, route, create_session);
  context->link->routes->session_created = created;
  register_delete_endpoint(context, route, delete_session);
  context->link->routes->session_deleted = deleted;
}

void createValue(HFMLSession* session, const char* name, size_t size) {
  session->blob = malloc(size);
  session->blobSize = size;
}

void getValue(HFMLSession* session, const char* name, void* ptr) {
  memcpy(ptr, session->blob, session->blobSize);
}

void setValue(HFMLSession* session, const char* name, const void* ptr) {
  memcpy(session->blob, ptr, session->blobSize);
}

HFMLSession* createSession(struct HFMLContext* context, const char* sessionName) {
  HFMLSession* session = malloc(sizeof(HFMLSession));
  context->session = session;
  session->context = context;
  session->createValue = &createValue;
  session->getValue = &getValue;
  session->setValue = &setValue;
  return session;
}
void deleteSession(struct HFMLContext* context, const char* sessionName) {
  free(context->session);
  context->session = NULL;
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

void addParam(HFMLContext* context, const char* key, const char* value) {
  context->params[context->paramCount*2] = strdup(key);
  context->params[context->paramCount*2+1] = strdup(value);
}

const char* pathParam(HFMLContext* context, const char* key) {
  for (int i = 0; i < context->paramCount; i++) {
    if (strncmp(key, context->params[i*2], strlen(key)) == 0) {
      return context->params[i*2+1];
    }
  }
  return "";
}

const char* getSessionName(HFMLContext* context) {
  return "dummy";
}

HFMLSession* getSession(HFMLContext* context, const char* name) {
  return context->session;
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
    context->register_put_endpoint = &register_put_endpoint;
    context->register_delete_endpoint = &register_delete_endpoint;
    context->register_session_endpoint = &register_session_endpoint;
    context->createSession = &createSession;
    context->deleteSession = &deleteSession;
    context->getSession = &getSession;
    context->getSessionName = &getSessionName;
    context->pathParam = &pathParam;
    context->addParam = &addParam;
    current->func(context);
    current = current -> next;
  }
  hfml_server_fd = create_server_socket(3273);
  printf("HFML server started on port %d\n", 3273);
  printf(" *** HFML backend initialized  ***\n");

}

char* hfml_process_request(const char* request) {
  const char* method = "";
  if (strncmp(request, "GET ", 4) == 0) {
    method = "GET";
  }
  if (strncmp(request, "POST ", 4) == 0) {
    method = "POST";
  }
  if (strncmp(request, "PUT ", 4) == 0) {
    method = "PUT";
  }
  if (strncmp(request, "PATCH ", 4) == 0) {
    method = "PATCH";
  }
  if (strncmp(request, "DELETE ", 4) == 0) {
    method = "DELETE";
  }
  if (strncmp(request, "OPTIONS", 4) == 0) {
    method = "OPTIONS";
    return strdup(OPTIONS_TEMPLATE);
  }
  if (method == "") {
    printf("Method not supported: %s\n", request);
    return strdup(BAD_REQUEST_TEMPLATE); 
  }
  int pos = strlen(method)+1;
  while (request[pos++] != ' ');
  int size = pos - 4;
  char* path = malloc(size + 1);
  memcpy(path, request + 4, pos - 4);
  path[size-1] = '\0';
  struct hfml_linked *current = first;
  
  while (current != NULL) {
    hfml_route *c = current->routes;
    while (c != NULL) {
      if (strncmp(c->method, method, strlen(method)) == 0) {
        regmatch_t matches[1 + c->regex_parts_count];
        int re = regexec(&c->regex, path, 1 + c->regex_parts_count, matches, 0);
        if (re == 0) {
          current->context->link->currentRoute = c;
          current->context->paramCount = 0;
          int paramLength = sizeof(char*)*2*c->regex_parts_count;
          current->context->params = malloc(paramLength);
          for (int i = 0; i < c->regex_parts_count; i++) {
            ssize_t len = matches[i+1].rm_eo - matches[i+1].rm_so;
            char* v = malloc(len+1);
            v[0] = '\0';
            strncat(v, path + matches[i+1].rm_so, len);
            current->context->addParam(current->context, c->regex_parts[i], v);
            current->context->paramCount++;
          }
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