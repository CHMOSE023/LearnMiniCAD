#pragma once
#include "Document.h" 
namespace MiniCAD
{
    class DocumentManager
    {  
    public:
        DocumentManager() = default;

        Document& Create(Renderer& r, float w, float h);

        void Close(Document* doc); 

        Document* GetActive()const { return m_active; } 
         
        void SetActive(Document* doc)
        {
            m_active = doc;
        } 

        std::vector<std::unique_ptr<Document>>& GetAll() { return m_docs; }

		// 实现菜单这些实现 我来弄  头文件先占位
    public:
        void New()    { printf("New\n"); }
        void Open()   { printf("Open\n"); }
        void Save()   { printf("Save\n"); }
		void SaveAs() { printf("Save As\n"); }
		void Undo()   { printf("Undo\n");    }
		void Redo()   { printf("Redo\n"); }
		void Paste()  { printf("Paste\n"); }

		void CopySelected()   { printf("Copy Selected\n"); }
    private:
        std::string GenerateUniqueName();

    private:
        std::vector<std::unique_ptr<Document>> m_docs; 

        Document* m_active = nullptr;
          
    private:
        int m_untitledCounter = 0;
    };
}