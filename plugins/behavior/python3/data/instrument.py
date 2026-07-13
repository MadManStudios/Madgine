import inspect
import ast

class Visitor(ast.NodeTransformer):

    def visit_AsyncFunctionDef(self, node):
        node.decorator_list = []

        # Tie up loose ends in the AST.        
        node = self.generic_visit(node)

        for arg in node.args.args:
            if (isinstance(arg.annotation, ast.Subscript)):
                node.body.insert(0, ast.If(ast.UnaryOp(ast.Not(), ast.Name(arg.arg, ast.Load())), [ast.Assign([ast.Name(arg.arg, ast.Store())], ast.Call(ast.Attribute(ast.Attribute(ast.Call(ast.Name(id="__import__", ctx=ast.Load()), [ast.Constant("Engine")], []), "Named", ast.Load()), "resolve", ast.Load()), [ast.Constant(arg.arg)], []))], []))            

        return node;

    def visit_Expr(self, node):		
        newNode = ast.Expr(ast.Await(ast.Call(ast.Attribute(ast.Call(ast.Name(id="__import__", ctx=ast.Load()), [ast.Constant("Engine")], []), "DebugLine", ast.Load()), [ast.Constant(node.lineno)], [])));
        newNode.lineno = node.lineno;
        return [newNode, self.generic_visit(node)]

    def visit_IfExp(self, node):		
        newNode = ast.Expr(ast.Await(ast.Call(ast.Attribute(ast.Call(ast.Name(id="__import__", ctx=ast.Load()), [ast.Constant("Engine")], []), "DebugLine", ast.Load()), [ast.Constant(node.lineno)], [])));
        newNode.lineno = node.lineno;
        return [newNode, self.generic_visit(node)]

    def visit_Assign(self, node):		
        newNode = ast.Expr(ast.Await(ast.Call(ast.Attribute(ast.Call(ast.Name(id="__import__", ctx=ast.Load()), [ast.Constant("Engine")], []), "DebugLine", ast.Load()), [ast.Constant(node.lineno)], [])));
        newNode.lineno = node.lineno;
        return [newNode, self.generic_visit(node)]

    def visit_Return(self, node):		
        newNode = ast.Expr(ast.Await(ast.Call(ast.Attribute(ast.Call(ast.Name(id="__import__", ctx=ast.Load()), [ast.Constant("Engine")], []), "DebugLine", ast.Load()), [ast.Constant(node.lineno)], [])));
        newNode.lineno = node.lineno;
        return [newNode, self.generic_visit(node)]

def patchFunction(fn):
    source = inspect.getsource(fn)	
    tree = ast.parse(source)	
    ast.increment_lineno(tree, inspect.getsourcelines(fn)[1] - 1)
    newTree = ast.fix_missing_locations(Visitor().visit(tree));
    print(ast.unparse(newTree))
    newCode = compile(newTree, filename=inspect.getfile(fn), mode="exec")		
    module = inspect.getmodule(fn)
    output = {};
    exec(newCode, module.__dict__, output)
    return output[fn.__name__]