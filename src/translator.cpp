#include "translator.hpp"
#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include <set>

std::set<uint64_t> mems;

#ifndef LOG_LEVEL
#define LOG_LEVEL 2
#endif

#define D(str, ...) { if (LOG_LEVEL >= 3) fprintf(stderr, str, __VA_ARGS__); }
#define I(str, ...) { if (LOG_LEVEL >= 2) fprintf(stderr, str, __VA_ARGS__); }
#define W(str, ...) { if (LOG_LEVEL >= 1) fprintf(stderr, str, __VA_ARGS__); }
#define E(str, ...) { if (LOG_LEVEL >= 0) fprintf(stderr, str, __VA_ARGS__); }

extern "C" {

#ifdef FULL_LEAK_CHECK

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuse-after-free"
void * yyalloc (size_t bytes, void * /*context*/) {
  void* p = malloc(bytes);
  D("alloc: %p\n", p);
  mems.insert(reinterpret_cast<uint64_t>(p));
  return p;
}
void * yyrealloc (void * ptr, size_t bytes, void * /*context*/) {
  void* p = realloc(ptr, bytes);
  D("realloc: %p -> %p\n", ptr, p);
  if (ptr != p) {
    if (!mems.contains(reinterpret_cast<uint64_t>(ptr))) {
      W("WARN: MEMORY ERROR AT %p\n", ptr);
    }
    mems.erase(reinterpret_cast<uint64_t>(ptr));
    mems.insert(reinterpret_cast<uint64_t>(p));

  }
  return p;
}
void   yyfree (void * ptr, void * /*context*/) {
  if (ptr == 0) {
    D("%s", "free: NULL\n");
    return;
  }
  D("free: %p\n", ptr);
  if (!mems.contains(reinterpret_cast<uint64_t>(ptr))) {
    W("WARN: MEMORY ERROR AT %p\n", ptr);
  } else {
    mems.erase(reinterpret_cast<uint64_t>(ptr));
  }
  free(ptr);
}

#pragma GCC diagnostic pop

#else
void * yyalloc (size_t bytes, void * /*context*/) {
  return malloc(bytes);
}

void * yyrealloc (void * ptr, size_t bytes, void * /*context*/) {
  return realloc(ptr, bytes);
}
void   yyfree (void * ptr, void * /*context*/) {
  free(ptr);
}
#endif

int parse_str(const char* str, void* p);
void create_tag(void* p) {
  reinterpret_cast<Translator*>(p)->createTag();
}
void append_attribute(void* p, const char* str) {
  reinterpret_cast<Translator*>(p)->appendAttribute(str);
}
void append_literal(void* p, const char* str) {
  reinterpret_cast<Translator*>(p)->appendLiteral(str);
}
void report_error(void* p, const char* str) {
  reinterpret_cast<Translator*>(p)->reportError(str);
}

char* myitoa(int i) {
  char c[33];
  snprintf(c, sizeof(c), "%d", i);
  return strdup(c);
}

}


Translator::Translator() {

}

Translator::~Translator() {
  delete root;
  delete currentTag;
}


void removeLeaks(void* context) {
  for (auto p : mems) {
    W("LEAK: %p\n", reinterpret_cast<void*>(p));
    yyfree(reinterpret_cast<void*>(p), context);
  }
}

std::string Translator::translate(std::string input) {
  #ifdef USE_TRANSLATE_MUTEX
    std::lock_guard<std::mutex> lock(translating);
  #endif
  error = "";
  currentTag = new Tag();
  root = new Tag();
  root->addAttribute("body");
  D("translate: %p\n", this);
  parse_str(input.c_str(), reinterpret_cast<void*>(this));
  std::string result;
  result = root->getContent();
  // TODO: get context pointer and only remove those
  removeLeaks(nullptr);
      if (error != "") {
      result += "<div class=\"error-list\">"+error+"</div>";
    }
  return result;
}

void Translator::createTag() {
  root->addChild(currentTag);
  currentTag = new Tag();
}

void Translator::appendAttribute(const char* str) {
  currentTag->addAttribute(std::string{str});
}

void Translator::appendLiteral(const char* str) {
  currentTag->addLiteral(std::string{str});
}

void Translator::reportError(const char* str) {
  error += str;
  error += "<br>";
}

void Tag::addAttribute(std::string str) {
  if (isEvent(str)) {
    addEvent(str);
    return;
  } 
  attr = str;
}

bool Tag::isEvent(std::string str) {
  if (str == "click") return true;
  return false;
}
void Tag::addEvent(std::string /*unused*/) {

}

void Tag::addLiteral(std::string str) {
  content = content + str;
}

Tag::Tag() {
    attr = "div";
}

std::string Tag::getContent() {
    std::string result = "<"+attr+">";
    result += content;
    for (auto v : children) {
        result += v->getContent();
    }
    result += "</"+attr+">";
    return result;
}

void Tag::addChild(Tag* child) {
    child->parent = this;
    children.push_back(child);
}

Tag::~Tag() {
    for (auto v : children) {
        delete v;
    }
    children.clear();
}