#include <stdio.h>
#include <unistd.h>
#include "hfml.h"

void say_hello(HFMLContext *context) {
    context->trigger_event(context, "alert", "Hello, world!");
}

void say_error(HFMLContext *context) {
    context->raise_error(context, "I won't do what you tell me!");
}

void say_long(HFMLContext *context) {
    sleep(10);
    context->trigger_event(context, "alert", "I am sleepy endpoint.");
}

HFML_REGISTER_CONTROLLER(example_backend) {
    printf("Hello from backend registration for `%s`\n", context->name);
    context->register_endpoint(context, "/hello", &say_hello);
    context->register_endpoint(context, "/error", &say_error);
    context->register_endpoint(context, "/long", &say_long);
}
