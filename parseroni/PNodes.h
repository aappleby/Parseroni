#pragma once

#include "parseroni/Combinators.h"

#include "metrolib/core/Log.h"

#include <stdint.h>
#include <vector>
//#include <span>
#include <assert.h>

//typedef std::span<const char> cspan;

void log_span(cspan s, uint32_t color = 0);

//------------------------------------------------------------------------------

struct PNode {
  PNode* parent;
  PNode* next;
  PNode* prev;
  cspan span;
  cspan gap;

  virtual void dump() {
    log_span(span, 0xFF00FF);
    LOG("\n");
  }
};

//------------------------------------------------------------------------------

struct PExpression;
struct PFieldExpression;
struct PType;
struct PToken;
struct PFieldExpression;
struct PStringLiteral;
struct PTemplateParameterList;
struct PTypeIdentifier;
struct PFieldDeclarationList;

//------------------------------------------------------------------------------

struct PDeclarator {};
struct PExpression {};
struct PStatement {};
struct PDeclaration {};

//------------------------------------------------------------------------------

struct PAccessSpecifier : public PNode {
};

struct PArgumentList : public PNode {
  PToken*      lit_lparen;
  PExpression* arg_head;
  PExpression* arg_tail;
  PToken*      lit_rparen;
};

struct PAssignmentExpression : public PNode, public PExpression {
  PExpression* lhs;
  PToken*      op;
  PExpression* rhs;
};

struct PBinaryExpression : public PNode, public PExpression {
  PExpression* lhs;
  PToken*      op;
  PExpression* rhs;
};

struct PCallExpression : public PNode {
  PFieldExpression* field;
  PArgumentList*    args;
};

struct PClassSpecifier : public PNode {
  PToken* lit_class;
  PTypeIdentifier* name;
  PFieldDeclarationList* body;
};

struct PComment : public PNode {};

struct PCompoundStatement : public PNode {
  PToken*     lit_lbrace;
  PStatement* statement_head;
  PStatement* statement_tail;
  PToken*     lit_rbrace;
};

struct PConditionClause : public PNode {};

struct PFieldExpression : public PNode {};

struct PFieldDeclarationList : public PNode {};

struct PFunctionDeclarator : public PNode {
  PType*       type;
  PDeclarator* declarator;
  PStatement*  body;
};

struct PIdentifier : public PNode {};

struct PIfStatement : public PNode {
  PToken*           lit_if;
  PConditionClause* condition;
  PStatement*       consequence;
  PToken*           lit_else;
  PStatement*       alternative;
};

struct PNamespaceIdentifier : public PNode {};

struct PParameterList : public PNode {
  PToken*      lit_lparen;
  PExpression* arg_head;
  PExpression* arg_tail;
  PToken*      lit_rparen;
};

//------------------------------------------------------------------------------

struct PPreproc : public PNode {
};

struct PPreprocIfdef : public PPreproc {
  //PToken*      lit_ifdef; // or ifndef, because treesitter...
  //PIdentifier* name;
  //std::vector<PNode*> children;
  //PToken*      lit_endif;
};

struct PPreprocDef : public PPreproc {
};

struct PPreprocInclude : public PPreproc {
  //PToken* lit_include;
  //PStringLiteral* path;
  cspan lit_include;
  cspan lit_ws;
  cspan lit_path;

  void dump() override {
    PNode::dump();
    LOG_INDENT_SCOPE();
    log_span(lit_include, 0x00FFFF);
    log_span(lit_ws,      0xFF00FF);
    log_span(lit_path,    0xFFFF00);
    LOG("\n");
  }
};

//------------------------------------------------------------------------------

struct PQualifiedIdentifier : public PNode {
  PNamespaceIdentifier* scope;
  PToken* lit_coloncolon;
  PIdentifier* name;
};

struct PReturnStatement : public PNode {
  PToken* lit_return;
  PExpression* expression;
  PToken* lit_semi;
};

struct PSpace : public PNode {};

struct PStringLiteral : public PNode {};

struct PToken : public PNode {};

struct PTemplateArgumentList : public PNode {};

struct PTemplateDeclaration : public PNode, public PDeclaration {
  PToken* lit_template;
  PTemplateParameterList* parameters;

};

struct PTemplateParameterList : public PNode {};

struct PTemplateType : public PNode {
  PIdentifier*           name;
  PTemplateArgumentList* args;
  PClassSpecifier*       pclass;
};

struct PTranslationUnit : public PNode {
  std::vector<PNode*> children;
};

struct PTypeIdentifier : public PNode {
};

struct PUsingDeclaration : public PNode {
  PToken*      lit_using;
  PToken*      lit_namespace;
  PIdentifier* identifier;
  PToken*      lit_semi;
};
