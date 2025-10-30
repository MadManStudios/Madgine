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
template <typename T>
struct Constant;
struct Comment;
struct ForEach;

struct Struct;

using Statement = std::variant<Assignment, Return, VariableAccess, CustomCodeBlock, Namespace, MemberAccess, VariableDefinition, ArithOperation, Constructor, Comment, ForEach, Constant<int>, Constant<std::string>>;
struct Type;

struct ShaderFile;
struct CppFile;
struct File;

}