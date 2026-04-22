#include "Shader.h"
namespace MiniCAD
{
    ShaderProgram CreateShader(ID3D11Device* device, const wchar_t* file)
    {
        ShaderProgram sp;
        ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;
        auto compileShader = [file](const char* entry, const char* target, ID3DBlob** shaderBlob)
            {
                ComPtr<ID3DBlob> errorBlob;
                HRESULT hr = D3DCompileFromFile(file, nullptr, nullptr, entry, target, 0, 0, shaderBlob, errorBlob.GetAddressOf());
                if (FAILED(hr))
                {
                    std::string details = errorBlob
                        ? static_cast<const char*>(errorBlob->GetBufferPointer())
                        : std::format("HRESULT=0x{:08X}", static_cast<unsigned int>(hr));

                    ReportError(std::format("Shader compile failed: {} [{} -> {}]", details, entry, target));
                }
            };

        compileShader("VSMain", "vs_5_0", vsBlob.GetAddressOf());
        compileShader("PSMain", "ps_5_0", psBlob.GetAddressOf());

        device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, sp.vs.GetAddressOf());
        device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, sp.ps.GetAddressOf());

        sp.vsBlob = vsBlob;  // 注意保存 vsBlob，因为 InputLayout 需要用到它

        return sp;
    }

    ComPtr<ID3D11InputLayout> CreateLayout(ID3D11Device* device, D3D11_INPUT_ELEMENT_DESC* desc, UINT count, ID3DBlob* vsBlob)
    {
        ComPtr<ID3D11InputLayout> layout;

        device->CreateInputLayout(desc, count, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), layout.GetAddressOf());

        return layout;
    }

}

