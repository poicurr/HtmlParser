#pragma once

#include <Tokenizer.hpp>
#include <iostream>
#include <vector>

enum class NodeType { None, Tag, Text, Comment };

struct TagNode;

struct Node {
  TagNode *parent;
  virtual NodeType type() = 0;
  virtual void print(const std::string &prefix) = 0;
};

struct TagNode : Node {
  struct Attribute {
    std::string name;
    std::string value;
    Attribute(const std::string &name, const std::string &value) : name{name}, value{value} {}
  };
  std::string name;
  std::vector<Attribute> attributes;
  bool closed;
  std::vector<Node *> children;

  TagNode(const std::string &name) : name{name}, attributes{}, closed{false}, children{} {}

  virtual NodeType type() override { return NodeType::Tag; }

  void addAttribute(const std::string &name, const std::string &value) {
    attributes.emplace_back(name, value);
  }

  void addChild(Node *child) {
    child->parent = this;
    children.push_back(child);
  }

  virtual void print(const std::string &prefix) override {
    std::cout << prefix << "<" << name;
    for (auto &&attr : attributes) {
      std::cout << " " << attr.name;
      if (!attr.value.empty()) {
        std::cout << "=\"" << attr.value << "\"";
      }
    }
    std::cout << ">" << std::endl;

    for (Node *child : children) {
      child->print(prefix + "  ");
    }

    std::cout << prefix << "</" << name << ">" << std::endl;
  }
};

struct TextNode : Node {
  std::string value;

  TextNode(const std::string &value) : value{value} {}

  virtual NodeType type() override { return NodeType::Text; }
  virtual void print(const std::string &prefix) override {
    std::cout << prefix << "\"" << value << "\"" << std::endl;
  }
};

struct CommentNode : Node {
  std::string value;

  CommentNode(const std::string &value) : value{value} {}

  virtual NodeType type() override { return NodeType::Comment; }
  virtual void print(const std::string &prefix) override {
    std::cout << prefix << "<!--\n";
    std::cout << prefix << "  " << value << "\n";
    std::cout << prefix << "-->" << std::endl;
  }
};

struct Context {
  TagNode *current;
  Context() : current{nullptr} {}
};

Context ctx;

void consume(Token *&token) { token = token->next; }

bool parseDoctype(Token *token) {
  if (token->kind != TokenKind::Doctype) return false;
  TagNode *self = new TagNode("!doctype");
  self->addAttribute(token->value, "");
  ctx.current->addChild(self);
  return true;
}

bool parseTagOpen(Token *token) {
  if (token->kind != TokenKind::TagBegin) return false;
  consume(token);

  if (token->kind != TokenKind::TagName) return false;
  auto tagName = token->value;
  TagNode *self = new TagNode(tagName);
  ctx.current->addChild(self);
  ctx.current = self;
  consume(token);

  while (token->kind == TokenKind::AttributeName) {
    auto attrName = token->value;
    consume(token);

    auto attrValue = std::string{};
    if (token->kind == TokenKind::AttributeValue) {
      attrValue = token->value;
      consume(token);
    }
    self->addAttribute(attrName, attrValue);
  }

  if (token->kind == TokenKind::TagEnd) {
    consume(token);
    std::cout << "[info] tag: " << tagName << " not closed yet" << std::endl;
    return true;
  }

  if (token->kind == TokenKind::TagSelfClose) {
    consume(token);
    self->closed = true;
    ctx.current = ctx.current->parent;
    std::cout << "[info] tag: " << tagName << " selfClosing" << std::endl;
    return true;
  }

  return true;
}

bool parseTagClose(Token *token) {
  if (token->kind != TokenKind::TagClose) return false;
  consume(token);

  if (token->kind != TokenKind::TagName) return false;
  auto tagName = token->value;

  std::cout << "[info] tag: " << tagName << " is now resolving parent" << std::endl;
  TagNode *node = ctx.current;
  while (node) {
    std::cout << "  [info] node tag: " << node->name << std::endl;
    if (node->name == tagName) {
      ctx.current = ctx.current->parent;
      node->closed = true;
      std::cout << "  [info] node tag: " << node->name << " is closed successfully" << std::endl;
      break;
    }
    if (!node->closed) {
      std::cerr << "[error] tag: " << node->name << " not closed" << std::endl;
      return false;
    }
    node = node->parent;
  }
  return true;
}

bool parseTextNode(Token *token) {
  if (token->kind != TokenKind::Text) return false;
  TextNode *textNode = new TextNode(token->value);
  ctx.current->addChild(textNode);
  return true;
}

bool parseCommentNode(Token *token) {
  if (token->kind != TokenKind::Comment) return false;
  CommentNode *node = new CommentNode(token->value);
  ctx.current->addChild(node);
  return true;
}

TagNode *parse(TokenList &tokenList) {
  TagNode *head = new TagNode("document");
  ctx.current = head;

  Token *token = tokenList.begin(), *last = tokenList.end();
  while (token != last) {
    parseDoctype(token);
    parseTagOpen(token);
    parseTagClose(token);
    parseTextNode(token);
    parseCommentNode(token);
    token = token->next;
  }

  return head;
}
