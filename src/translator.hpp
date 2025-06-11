#ifndef __TRANSLATOR_HPP__
#define __TRANSLATOR_HPP__

#include <iostream>
#include <list>
#include <mutex>

class Tag {
  public:
    Tag();
    virtual ~Tag();
    void addAttribute(std::string str);
    void addLiteral(std::string str);
    void addChild(Tag* child);
    bool isEvent(std::string str);
    void addEvent(std::string str);
    Tag* getParent();
    virtual std::string getContent();
  private:
    Tag* parent;
    std::list<Tag*> children;
    std::string attr;
};

class TextTag: public Tag {
  public:
    TextTag(std::string str);
    virtual ~TextTag();
    virtual std::string getContent() override;
  private:
    std::string content;
};


class Translator {
  public:
    Translator();
    virtual ~Translator();
    std::string translate(std::string input);
    void createTag();
    void appendAttribute(const char* str);
    void appendLiteral(const char* str);
    void reportError(const char* str);
    void popTag();
  private:
    Tag* currentTag;
    Tag* root;
    std::mutex translating;
    std::string error;
};

#endif