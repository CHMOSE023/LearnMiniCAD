#pragma once
#include <string>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <d3d11.h>
#include "Material.h"
namespace MiniCAD
{
	MiniCAD::Material::Material(const std::string& name) : m_name(name)
	{
	}
}