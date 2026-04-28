#include "Document.h"
namespace MiniCAD
{
    class DocumentManager
    {
    public:
        Document& Create(Renderer& r, float w, float h);

        void Close(Document* doc); 

        Document* GetActive()const { return m_active; } 
         
        void SetActive(Document* doc)
        {
            m_active = doc;
        } 

        std::vector<std::unique_ptr<Document>>& GetAll() { return m_docs; }

    private:
        std::vector<std::unique_ptr<Document>> m_docs; 

        Document* m_active = nullptr;
    };
}