#include "translator.hpp"
#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include <set>

std::set<std::tuple<uint64_t,uint64_t>> mems;
std::set<std::tuple<uint64_t,uint64_t>> static_mems;

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

void restore_phase() {
  mems.insert(static_mems.begin(), static_mems.end());
}

void reset_phase() {
  static_mems.insert(mems.begin(), mems.end());
  fprintf(stderr, "Initialization done. Removing %ld items from memory pool\n", mems.size());
  mems.clear();
}

void * yyalloc (size_t bytes, void * context) {
  void* p = malloc(bytes);
  D("alloc: %p\n", p);
  mems.insert({reinterpret_cast<uint64_t>(p),reinterpret_cast<uint64_t>(context)});
  return p;
}
void * yyrealloc (void * ptr, size_t bytes, void * context) {
  void* p = realloc(ptr, bytes);
  D("realloc: %p -> %p\n", ptr, p);
  if (ptr != p) {
    if (!mems.contains({reinterpret_cast<uint64_t>(ptr), reinterpret_cast<uint64_t>(context)})) {
      W("WARN: MEMORY ERROR AT %p\n", ptr);
    }
    mems.erase({reinterpret_cast<uint64_t>(ptr),reinterpret_cast<uint64_t>(context)});
    mems.insert({reinterpret_cast<uint64_t>(p),reinterpret_cast<uint64_t>(context)});

  }
  return p;
}
void   yyfree (void * ptr, void * context) {
  if (ptr == 0) {
    D("%s", "free: NULL\n");
    return;
  }
  D("free: %p\n", ptr);
  if (!mems.contains({reinterpret_cast<uint64_t>(ptr),reinterpret_cast<uint64_t>(context)})) {
    W("WARN: MEMORY ERROR AT %p\n", ptr);
  } else {
    mems.erase({reinterpret_cast<uint64_t>(ptr),reinterpret_cast<uint64_t>(context)});
  }
  free(ptr);
}

#pragma GCC diagnostic pop

#else

void restore_phase() {}

void reset_phase() {}

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
void pop_tag(void* p) {
  reinterpret_cast<Translator*>(p)->popTag();
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
    if (reinterpret_cast<uint64_t>(context) != std::get<1>(p)) continue;
    W("LEAK: %p\n", reinterpret_cast<void*>(std::get<0>(p)));
    yyfree(reinterpret_cast<void*>(std::get<0>(p)), context);
  }
}

std::string Translator::translate(std::string input) {
  #ifdef USE_TRANSLATE_MUTEX
    std::lock_guard<std::mutex> lock(translating);
  #endif
  error = "";
  root = new Tag();
  currentTag = root;
  root->addAttribute("body");
  D("translate: %p\n", this);
  parse_str(input.c_str(), reinterpret_cast<void*>(this));
  std::string result;
  result = root->getContent();
  removeLeaks(this);
      if (error != "") {
      result += "<div class=\"error-list\">"+error+"</div>";
    }
  return result;
}

void Translator::createTag() {
  Tag* newTag = new Tag();
  currentTag->addChild(newTag);
  currentTag = newTag;
}

void Translator::popTag() {
  currentTag = currentTag->getParent();
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

Tag* Tag::getParent() {
  return parent;
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
  addChild(new TextTag(str));
}

Tag::Tag() {
    attr = "div";
}

TextTag::TextTag(std::string str):Tag() {
  content = str;
}

TextTag::~TextTag() {
}

std::string TextTag::getContent() {
  return content;
}


std::string Tag::getContent() {
    std::string result = "<"+attr+">";
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