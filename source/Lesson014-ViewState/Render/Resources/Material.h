#pragma once
#include <string>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <d3d11.h>
namespace MiniCAD
{ 
    class Material
    {
    public:
        Material(const std::string& name);

        void SetDiffuseColor(const DirectX::XMFLOAT4& color) { m_diffuseColor = color; }
        void SetTexture(ID3D11ShaderResourceView* srv)       { m_texture = srv; }

        const std::string&        GetName()         const { return m_name; }
        const DirectX::XMFLOAT4&  GetDiffuseColor() const { return m_diffuseColor; }
        ID3D11ShaderResourceView* GetTexture()      const { return m_texture.Get(); }

    private: 

        std::string       m_name;
        DirectX::XMFLOAT4 m_diffuseColor{ 1.f,1.f,1.f,1.f };

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
    };
}