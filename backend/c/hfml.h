#define HFML_CONCAT2(x, y) x##y
#define HFML_CONCAT(x,y) HFML_CONCAT2(x, y)

#define HFML_REGISTER_CONTROLLER(x) \
  void x(HFMLContext* context);\
  __attribute__((constructor)) void HFML_CONCAT(__hfml__contruct__, __LINE__)(void) {\
    __hfml__register(#x, &x);\
  }\
  void x(HFMLContext* context)


struct HFMLContext;

struct hfml_linked;

typedef struct HFMLContext {
  const char *name;
  struct hfml_linked* link;
  void (*trigger_event)(struct HFMLContext*, const char* event, const char* payload);
  void (*raise_error)(struct HFMLContext*, const char* message);  
  void (*register_endpoint)(struct HFMLContext*, const char* route, void (*cb)(struct HFMLContext *context));  
} HFMLContext;

void __hfml__register(const char* name, void (*func)(HFMLContext*));
