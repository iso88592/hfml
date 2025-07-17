#include "translator.hpp"
#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include <set>
#include <cassert>

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
void append_attribute_id(void* p, const char* str) {
  reinterpret_cast<Translator*>(p)->appendAttributeId(str);
}
void append_literal(void* p, const char* str) {
  reinterpret_cast<Translator*>(p)->appendLiteral(str);
}
void report_error(void* p, const char* str) {
  reinterpret_cast<Translator*>(p)->reportError(str);
}
void add_event(void* p) {
  reinterpret_cast<Translator*>(p)->addEvent(); 
  reinterpret_cast<Translator*>(p)->deleteList(); 
}
void create_list(void* p) {
  reinterpret_cast<Translator*>(p)->createList(); 
}
void append_list(void* p, const char* item) {
  reinterpret_cast<Translator*>(p)->appendList(item); 
}
void append_list_id(void* p, const char* item) {
  reinterpret_cast<Translator*>(p)->appendListId(item); 
}
void store_name(void* p, const char* n) {
  reinterpret_cast<Translator*>(p)->storeName(n); 
}
void store_event_handler(void* p, const char* str) {
  reinterpret_cast<Translator*>(p)->storeEventHandler(str); 
}
void append_modifier_number(void* p, int z) {
  reinterpret_cast<Translator*>(p)->appendModifierNumber(z); 
}

const char* get_error_context(void* p, int line, int col) {
  return reinterpret_cast<Translator*>(p)->getErrorContext(line, col);
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
  inputText = input;
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

const char* Translator::getErrorContext(int line, int /* unused */) {
  ssize_t from = 0;
  ssize_t pos = inputText.find('\n', 0);
  for (int i = 1; i < line; i++) {
    from = pos + 1;
    pos = inputText.find('\n', from);
  }
  return strdup(inputText.substr(from, pos-from).c_str());
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

void Translator::appendAttributeId(const char* str) {
  std::string s = "id=\"";
  s += str;
  s += "\"";
  currentTag->addEvent(s, "", {});
}


void Translator::appendLiteral(const char* str) {
  currentTag->addLiteral(std::string{str});
}

void Translator::addEvent() {
  if (storedEventHandler == "request") {
    currentList.push_front("event");
  }
  if (storedEventHandler == "show") {
    storedEventHandler = "hfml_show";
  }
  if (storedEventHandler == "hide") {
    storedEventHandler = "hfml_hide";
  }
  if (storedName == "nav") {
    storedName = "href";
  }  
  currentTag->addEvent(storedName, storedEventHandler, currentList);
  storedName = "";
}

void Translator::createList() {
  currentList.clear();
}
void Translator::deleteList() {
  currentList.clear();
}
void Translator::appendList(const char* str) {
  std::string s = "\"";
  s += str;
  s += "\"";
  currentList.push_back(s);
}
void Translator::appendListId(const char* str) {
  std::string s = "document.getElementById(\"";
  s+=str;
  s+="\")";
  currentList.push_back(s);
}

void Translator::appendModifierNumber(int z) {
  currentTag->addNumber(z);
}


void Translator::storeName(const char* str) {
  storedName = str;
}

void Translator::storeEventHandler(const char* str) {
  storedEventHandler = str;
}

void Translator::reportError(const char* str) {
  error += str;
  error += "<br>";
}

Tag* Tag::getParent() {
  return parent;
}

bool isNumeric(std::string str) {
  if (str.length() == 0) return false;
  for (size_t i = 0; i < str.length(); i++) {
    if (str[i] < '0') return false;
    if (str[i] > '9') return false;
  }
  return true;
}

void Tag::addNumber(int z) {
  modifiers.push_back(std::to_string(z));
}

void Tag::addAttribute(std::string str) {
  if (str == "hidden" || str == "window" || str == "title" || str == "split" || str == "primary" || str == "secondary" || str == "cancel" || str == "center") {
    modifiers.push_back("class=\""+str+"\"");
    return;
  }
  attr = str;
}

bool Tag::isEvent(std::string str) {
  if (str == "click") return true;
  return false;
}
void Tag::addEvent(std::string str, std::string eventName, std::list<std::string> list) {
  if (str == "pos") {
    modifiers.push_back("class=\"pos-"+eventName+"\"");
    list.clear();
  } else if (str == "drag") {
    modifiers.push_back("onmousedown='hfml_start_drag_" + eventName + "(event, " + list.front() + ")'");
  } else if (str == "click") {
    modifiers.push_back("onclick='"+eventName+"(");
    int count = 0;
    for (std::string item : list) {
      if (count++ >= 1) {
        modifiers.push_back(", ");
      }
      modifiers.push_back(item);
    }
    modifiers.push_back(")'");
    list.clear();
  } else {
    if (list.size() > 0) {
      for (std::string item : list) {
        str += "=" + item + "";
      }
    }
    modifiers.push_back(str);
  }
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
    if (attr == "h") {
      for (auto str : modifiers) {
        if (isNumeric(str)) {
          attr += str;
          break;
        }
      }
    }
    std::string result = "<"+attr;
    std::list<std::string> classes;
    for (auto str : modifiers) {
      if (isNumeric(str)) continue;
      if (str.starts_with("class=")) {
        classes.push_back(str.substr(7, str.length() - 8));
        if (str == "class=\"split\"") {
          for (auto s : modifiers) {
            if (!isNumeric(s)) continue;
            classes.push_back("split-" + s);
          }
        }
        continue;
      }
      result += " " + str;
    }
    if (classes.size() > 0) {
      result +=" class=\"";
      for (auto c : classes) {
        result += c;
        result += " ";
      }
      result +="\"";
    }
#ifdef DEBUG    
    result += " data-debug-child-count=\"";
    result += myitoa(children.size());
    result += "\"";
#endif    
    result += ">";
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