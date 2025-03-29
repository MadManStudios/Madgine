#pragma once

namespace Engine {
namespace NodeGraph {

    struct CodeGeneratorData {
        virtual ~CodeGeneratorData() = default;
    };

    struct MADGINE_NODEGRAPH_EXPORT CodeGenerator {

        CodeGenerator(const NodeGraph &graph);
        CodeGenerator(const CodeGenerator &) = delete;
        virtual ~CodeGenerator() = default;

        CodeGenerator &operator=(const CodeGenerator &) = delete;

        void generate(uint32_t flowIn, const NodeBase *startNode = nullptr);
        void generate(Pin pin);

        virtual CodeGen::Statement read(Pin pin);
        virtual CodeGen::Statement write(Pin pin, CodeGen::Statement statement);

        virtual CodeGen::File &file() = 0;

    protected:
        const NodeGraph &mGraph;    
        std::vector<std::unique_ptr<CodeGeneratorData>> mData;        
    };

}
}