#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
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

void calculator_session_created(HFMLContext * context, HFMLSession* session) {
    session->createValue(session, "result", sizeof(int));
    int value = 0;
    session->setValue(session, "result", &value);
    context->trigger_event(context, "show", "#calculator_content");
    context->trigger_event(context, "hide", "#calculator_loading");
}
void calculator_session_deleted(HFMLContext * context) {
}

void calculator(HFMLContext * context) {
    const char* action = context->pathParam(context, "action");
    HFMLSession* session = context->getSession(context, context->getSessionName(context));
    if (strncmp(action, "0", 1) == 0) {
        int result;
        session->getValue(session, "result", &result);
        result *= 10;
        session->setValue(session, "result", &result);
    }
    if (strncmp(action, "1", 1) == 0) {
        int result;
        session->getValue(session, "result", &result);
        result *= 10;
        result += 1;
        session->setValue(session, "result", &result);
    }
    if (strncmp(action, "2", 1) == 0) {
        int result;
        session->getValue(session, "result", &result);
        result *= 10;
        result += 2;
        session->setValue(session, "result", &result);
    }
    if (strncmp(action, "3", 1) == 0) {
        int result;
        session->getValue(session, "result", &result);
        result *= 10;
        result += 3;
        session->setValue(session, "result", &result);
    }
    if (strncmp(action, "4", 1) == 0) {
        int result;
        session->getValue(session, "result", &result);
        result *= 10;
        result += 4;
        session->setValue(session, "result", &result);
    }
    if (strncmp(action, "5", 1) == 0) {
        int result;
        session->getValue(session, "result", &result);
        result *= 10;
        result += 5;
        session->setValue(session, "result", &result);
    }
    if (strncmp(action, "6", 1) == 0) {
        int result;
        session->getValue(session, "result", &result);
        result *= 10;
        result += 6;
        session->setValue(session, "result", &result);
    }
    if (strncmp(action, "7", 1) == 0) {
        int result;
        session->getValue(session, "result", &result);
        result *= 10;
        result += 7;
        session->setValue(session, "result", &result);
    }
    if (strncmp(action, "8", 1) == 0) {
        int result;
        session->getValue(session, "result", &result);
        result *= 10;
        result += 8;
        session->setValue(session, "result", &result);
    }
    if (strncmp(action, "9", 1) == 0) {
        int result;
        session->getValue(session, "result", &result);
        result *= 10;
        result += 9;
        session->setValue(session, "result", &result);
    }
    if (strncmp(action, "del", 1) == 0) {
        int result;
        session->getValue(session, "result", &result);
        result /= 10;
        session->setValue(session, "result", &result);
    }
    if (strncmp(action, "ac", 1) == 0) {
        int result;
        result = 0;
        session->setValue(session, "result", &result);
    }

    int result;
    char rstring[40];
    session->getValue(session, "result", &result);
    snprintf(rstring, 39, "#calculator_display},{%d",result);

    context->trigger_event(context, "setText", rstring);
}

HFML_REGISTER_CONTROLLER(example_backend) {
    printf("Hello from backend registration for `%s`\n", context->name);
    context->register_endpoint(context, "/hello", &say_hello);
    context->register_endpoint(context, "/error", &say_error);
    context->register_endpoint(context, "/long", &say_long);
    context->register_session_endpoint(context, "/calc", &calculator_session_created, &calculator_session_deleted);
    context->register_endpoint(context, "/calc/{action}", &calculator);
}
