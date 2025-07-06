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
    void addNumber(int z);
    bool isEvent(std::string str);
    void addEvent(std::string str, std::string eventName, std::list<std::string> list);
    Tag* getParent();
    virtual std::string getContent();
  private:
    Tag* parent;
    std::list<Tag*> children;
    std::string attr;
    std::list<std::string> modifiers;
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
    void appendAttributeId(const char* str);
    void appendLiteral(const char* str);
    void appendModifierNumber(int z);
    void reportError(const char* str);
    void addEvent(/*stored name*/);
    void createList();
    void deleteList();
    void appendList(const char* str);
    void appendListId(const char* str);
    void popTag();
    void storeName(const char* str);
    void storeEventHandler(const char* str);
    const char* getErrorContext(int line, int col);
  private:
    Tag* currentTag;
    Tag* root;
    std::mutex translating;
    std::string error;
    std::list<std::string> currentList;
    std::string storedName;
    std::string inputText;
    std::string storedEventHandler;
};

#endif