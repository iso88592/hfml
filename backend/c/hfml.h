#define HFML_CONCAT2(x, y) x##y
#define HFML_CONCAT(x,y) HFML_CONCAT2(x, y)

#define HFML_REGISTER_CONTROLLER(x) \
  void x(HFMLContext* context);\
  __attribute__((constructor)) void HFML_CONCAT(__hfml__contruct__, __LINE__)(void) {\
    __hfml__register(#x, &x);\
  }\
  void x(HFMLContext* context)


struct HFMLContext;
struct HFMLSession;

struct hfml_linked;

typedef struct HFMLContext {
  const char *name;
  struct hfml_linked* link;
  struct HFMLSession* session;
  const char** params;
  int paramCount;
  void (*trigger_event)(struct HFMLContext*, const char* event, const char* payload);
  void (*raise_error)(struct HFMLContext*, const char* message);  
  void (*register_endpoint)(struct HFMLContext*, const char* route, void (*cb)(struct HFMLContext *context));
  void (*register_put_endpoint)(struct HFMLContext*, const char* route, void (*cb)(struct HFMLContext *context));
  void (*register_delete_endpoint)(struct HFMLContext*, const char* route, void (*cb)(struct HFMLContext *context));
  void (*register_session_endpoint)(struct HFMLContext*, const char* route, void (*created)(struct HFMLContext *context, struct HFMLSession *session), void (*deleted)(struct HFMLContext *context));
  struct HFMLSession* (*createSession)(struct HFMLContext*, const char*);
  void (*deleteSession)(struct HFMLContext*, const char*);
  const char* (*getSessionName)(struct HFMLContext*);
  struct HFMLSession* (*getSession)(struct HFMLContext*, const char*);
  const char* (*pathParam)(struct HFMLContext*, const char*);
  void (*addParam)(struct HFMLContext*, const char*, const char*);
} HFMLContext;

typedef struct HFMLSession {
  const char* name;
  HFMLContext* context;
  void (*createValue)(struct HFMLSession*, const char*, size_t);
  void (*setValue)(struct HFMLSession*, const char*, const void*);
  void (*getValue)(struct HFMLSession*, const char*, void*);
  char* blob;
  size_t blobSize;
} HFMLSession;

void __hfml__register(const char* name, void (*func)(HFMLContext*));
