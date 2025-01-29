#include "translator.hpp"
#include <cstring>
#include <cstdio>
#include <stdlib.h>

extern "C" int parse_str(const char* str);

Translator* translator;


extern "C" {
void create_tag() {
  translator->createTag();
}
void append_attribute(const char* str) {
  translator->appendAttribute(str);
}
void append_literal(const char* str) {
  translator->appendLiteral(str);
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


std::string Translator::translate(std::string input) {
  translator = this;
  currentTag = new Tag();
  root = new Tag();
  root->addAttribute("body");
  parse_str(input.c_str());
  std::string result;
  result = root->getContent();
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