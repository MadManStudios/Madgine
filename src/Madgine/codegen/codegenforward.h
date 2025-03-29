#pragma once

namespace CodeGen {

struct Assignment;
struct Return;
struct VariableAccess;
struct VariableDefinition;
struct MemberAccess;
struct CustomCodeBlock;
struct Namespace;
struct Function;
struct ArithOperation;
struct Constructor;
struct Constant;
struct Comment;
struct ForEach;

struct Struct;

using Statement = std::variant<Assignment, Return, VariableAccess, CustomCodeBlock, Namespace, MemberAccess, VariableDefinition, ArithOperation, Constructor, Constant, Comment, ForEach>;
struct Type;

struct ShaderFile;
struct CppFile;
struct File;

}