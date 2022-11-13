#include "miru_core_common.h"
#if defined(MIRU_D3D12)
#include "D3D12Shader.h"

#include "base/DescriptorPoolSet.h"

#include <d3d12shader.h>
#include <dxcapi.h>

using namespace miru;
using namespace d3d12;

Shader::Shader(CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	m_ShaderModelData.HighestShaderModel = D3D_SHADER_MODEL_6_6;
	MIRU_WARN(m_Device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &m_ShaderModelData, sizeof(m_ShaderModelData)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_SHADER_MODEL.");

	Reconstruct();
	GetShaderResources();
}

Shader::~Shader()
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_ShaderByteCode.pShaderBytecode = nullptr;
	m_ShaderByteCode.BytecodeLength = 0;
}

void Shader::Reconstruct()
{
	MIRU_CPU_PROFILE_FUNCTION();

	GetShaderByteCode();
	m_ShaderByteCode.pShaderBytecode = m_ShaderBinary.data();
	m_ShaderByteCode.BytecodeLength = m_ShaderBinary.size();
}

void Shader::GetShaderResources()
{
	MIRU_CPU_PROFILE_FUNCTION();

	D3D12ShaderReflection(m_ShaderBinary, m_CI.stageAndEntryPoints, m_VSIADs, m_PSOADs, m_GroupCountXYZ, m_RBDs);
}

void Shader::D3D12ShaderReflection(
	const std::vector<char>& shaderBinary,
	const std::vector<std::pair<Shader::StageBit, std::string>>& stageAndEntryPoints,
	std::vector<Shader::VertexShaderInputAttributeDescription>& VSIADs,
	std::vector<Shader::PixelShaderOutputAttributeDescription>& PSOADs,
	std::array<uint32_t, 3>& GroupCountXYZ,
	std::map<uint32_t, std::map<uint32_t, Shader::ResourceBindingDescription>>& RBDs)
{
	std::filesystem::path s_DxcompilerFullpath = GetLibraryFullpath_dxcompiler();
	arc::DynamicLibrary::LibraryHandle s_HModeuleDxcompiler = LoadLibrary_dxcompiler();
	if (!s_HModeuleDxcompiler)
	{
		std::string error_str = "WARN: SHADER_CORE: Unable to load '" + s_DxcompilerFullpath.generic_string() + "'.";
		MIRU_WARN(GetLastError(), error_str.c_str());
	}
	DxcCreateInstanceProc DxcCreateInstance = (DxcCreateInstanceProc)arc::DynamicLibrary::LoadFunction(s_HModeuleDxcompiler, "DxcCreateInstance");
	if (!DxcCreateInstance)
		return;

	IDxcLibrary* dxc_library = nullptr;
	MIRU_WARN(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&dxc_library)), "WARN: SHADER_CORE: DxcCreateInstance failed to create IDxcLibrary.");
	if (dxc_library)
	{
		IDxcBlobEncoding* dxc_shader_bin = nullptr;
		MIRU_WARN(dxc_library->CreateBlobWithEncodingFromPinned(shaderBinary.data(), static_cast<UINT32>(shaderBinary.size()), 0, &dxc_shader_bin), "WARN: SHADER_CORE: IDxcLibrary::CreateBlobWithEncodingFromPinned failed to create IDxcBlobEncoding."); //binary, no code page
		if (dxc_shader_bin)
		{
			IDxcContainerReflection* dxc_container_reflection = nullptr;
			MIRU_WARN(DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&dxc_container_reflection)), "WARN: SHADER_CORE: DxcCreateInstance failed to create IDxcContainerReflection.");
			if (dxc_container_reflection)
			{
				HRESULT res = dxc_container_reflection->Load(dxc_shader_bin);
				MIRU_WARN(res, "WARN: SHADER_CORE: IDxcContainerReflection::Load failed.");
				if (res == S_OK)
				{
					uint32_t partIndex;
					uint32_t dxil_kind = DXC_PART_DXIL;
					MIRU_WARN(dxc_container_reflection->FindFirstPartKind(dxil_kind, &partIndex), "WARN: SHADER_CORE: IDxcContainerReflection::FindFirstPartKind failed.");
					if (partIndex != 0) //No reflection available for non-DXIL shaders.
					{
						//General parsing functions:
						auto D3D_SHADER_INPUT_TYPE_to_miru_crossplatform_DescriptorType = [](D3D_SHADER_INPUT_TYPE type, D3D_SRV_DIMENSION dimension) -> base::DescriptorType
						{
							switch (type)
							{
							case D3D_SIT_CBUFFER:
								return base::DescriptorType::UNIFORM_BUFFER;
							case D3D_SIT_TBUFFER:
								return base::DescriptorType::UNIFORM_TEXEL_BUFFER;
							case D3D_SIT_TEXTURE:
								return base::DescriptorType::SAMPLED_IMAGE;
							case D3D_SIT_SAMPLER:
								return base::DescriptorType::SAMPLER;
							case D3D_SIT_UAV_RWTYPED:
							case D3D_SIT_STRUCTURED:
							case D3D_SIT_UAV_RWSTRUCTURED:
							case D3D_SIT_BYTEADDRESS:
							case D3D_SIT_UAV_RWBYTEADDRESS:
							case D3D_SIT_UAV_APPEND_STRUCTURED:
							case D3D_SIT_UAV_CONSUME_STRUCTURED:
							case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
							case D3D_SIT_UAV_FEEDBACKTEXTURE:
								return dimension > D3D_SRV_DIMENSION_BUFFER ? base::DescriptorType::STORAGE_IMAGE : base::DescriptorType::STORAGE_BUFFER;
							case D3D_SIT_RTACCELERATIONSTRUCTURE:
								return base::DescriptorType::ACCELERATION_STRUCTURE;
							default:
								break;
							}

							MIRU_ASSERT(true, "ERROR: SHADER_CORE: Unsupported D3D_SHADER_INPUT_TYPE and/or D3D_SRV_DIMENSION. Cannot convert to miru::crossplatform::DescriptorType.");
							return static_cast<base::DescriptorType>(0);
						};
						auto get_shader_stage = [](D3D12_SHADER_VERSION_TYPE type) -> Shader::StageBit
						{
							switch (type)
							{
							case D3D12_SHVER_PIXEL_SHADER:
								return Shader::StageBit::PIXEL_BIT;
							case D3D12_SHVER_VERTEX_SHADER:
								return Shader::StageBit::VERTEX_BIT;
							case D3D12_SHVER_GEOMETRY_SHADER:
								return Shader::StageBit::GEOMETRY_BIT;
							case D3D12_SHVER_HULL_SHADER:
								return Shader::StageBit::HULL_BIT;
							case D3D12_SHVER_DOMAIN_SHADER:
								return Shader::StageBit::DOMAIN_BIT;
							case D3D12_SHVER_COMPUTE_SHADER:
								return Shader::StageBit::COMPUTE_BIT;
							case D3D12_SHVER_RESERVED0:
								return Shader::StageBit(0);
							case 7:
								return Shader::StageBit::RAYGEN_BIT;
							case 8:
								return Shader::StageBit::INTERSECTION_BIT;
							case 9:
								return Shader::StageBit::ANY_HIT_BIT;
							case 10:
								return Shader::StageBit::CLOSEST_HIT_BIT;
							case 11:
								return Shader::StageBit::MISS_BIT;
							case 12:
								return Shader::StageBit::CALLABLE_BIT;
							default:
								return Shader::StageBit(0);
							}
						};

						if (stageAndEntryPoints.size() == 1)
						{
							//Shader Reflection
							ID3D12ShaderReflection* shader_reflection = nullptr;
							MIRU_WARN(dxc_container_reflection->GetPartReflection(partIndex, IID_PPV_ARGS(&shader_reflection)), "WARN: SHADER_CORE: IDxcContainerReflection::GetPartReflection failed.");
							if (shader_reflection)
							{
								D3D12_SHADER_DESC shaderDesc;
								res = shader_reflection->GetDesc(&shaderDesc);
								MIRU_WARN(res, "WARN: SHADER_CORE: ID3D12ShaderReflection::GetDesc failed.");
								if (res == S_OK)
								{
									D3D12_SHADER_VERSION_TYPE stage = (D3D12_SHADER_VERSION_TYPE)D3D12_SHVER_GET_TYPE(shaderDesc.Version);
									uint16_t major = D3D12_SHVER_GET_MAJOR(shaderDesc.Version);
									uint16_t minor = D3D12_SHVER_GET_MINOR(shaderDesc.Version);

									D3D_FEATURE_LEVEL featureLevel;
									shader_reflection->GetMinFeatureLevel(&featureLevel);

									auto D3D_REGISTER_COMPONENT_TYPE_to_miru_crossplatform_VertexType = [](D3D_REGISTER_COMPONENT_TYPE type, uint32_t vector_count) -> base::VertexType
									{
										switch (type)
										{
										case D3D_REGISTER_COMPONENT_UNKNOWN:
											break;
										case D3D_REGISTER_COMPONENT_UINT32:
											return static_cast<base::VertexType>(static_cast<uint32_t>(base::VertexType::UINT)
												+ static_cast<uint32_t>(base::VertexType(vector_count - 1)));
										case D3D_REGISTER_COMPONENT_SINT32:
											return static_cast<base::VertexType>(static_cast<uint32_t>(base::VertexType::INT)
												+ static_cast<uint32_t>(base::VertexType(vector_count - 1)));
										case D3D_REGISTER_COMPONENT_FLOAT32:
											return static_cast<base::VertexType>(static_cast<uint32_t>(base::VertexType::FLOAT)
												+ static_cast<uint32_t>(base::VertexType(vector_count - 1)));
										default:
											break;
										}
										MIRU_WARN(true, "ERROR: SHADER_CORE: Unsupported D3D_REGISTER_COMPONENT_TYPE. Cannot convert to miru::crossplatform::VertexType.");
										return static_cast<base::VertexType>(0);
									};

									if (stage == D3D12_SHADER_VERSION_TYPE::D3D12_SHVER_VERTEX_SHADER)
									{
										auto sizeof_miru_crossplatform_VertexType = [](base::VertexType type) -> uint32_t
										{
											switch (type)
											{
											case miru::base::VertexType::FLOAT:
											case miru::base::VertexType::INT:
											case miru::base::VertexType::UINT:
												return 4;
											case miru::base::VertexType::VEC2:
											case miru::base::VertexType::IVEC2:
											case miru::base::VertexType::UVEC2:
												return 8;
											case miru::base::VertexType::VEC3:
											case miru::base::VertexType::IVEC3:
											case miru::base::VertexType::UVEC3:
												return 12;
											case miru::base::VertexType::VEC4:
											case miru::base::VertexType::IVEC4:
											case miru::base::VertexType::UVEC4:
												return 16;
											case miru::base::VertexType::DOUBLE:
												return 8;
											case miru::base::VertexType::DVEC2:
												return 16;
											case miru::base::VertexType::DVEC3:
												return 24;
											case miru::base::VertexType::DVEC4:
												return 32;
											default:
												return 0;
											}
										};

										VSIADs.clear();
										for (UINT i = 0; i < shaderDesc.InputParameters; i++)
										{
											D3D12_SIGNATURE_PARAMETER_DESC input_parameter;
											shader_reflection->GetInputParameterDesc(i, &input_parameter);
											MIRU_WARN(res, "WARN: SHADER_CORE: ID3D12ShaderReflection::GetInputParameterDesc failed.");
											if (res == S_OK)
											{
												Shader::VertexShaderInputAttributeDescription vsiad;
												vsiad.location = input_parameter.SemanticIndex;
												vsiad.binding = 0;
												vsiad.vertexType = D3D_REGISTER_COMPONENT_TYPE_to_miru_crossplatform_VertexType(input_parameter.ComponentType, (uint32_t)log2((double)(input_parameter.Mask + 1)));
												vsiad.offset = VSIADs.empty() ? 0 : VSIADs.back().offset + sizeof_miru_crossplatform_VertexType(VSIADs.back().vertexType);
												vsiad.semanticName = input_parameter.SemanticName;
												VSIADs.push_back(vsiad);
											}
										}
									}

									if (stage == D3D12_SHADER_VERSION_TYPE::D3D12_SHVER_PIXEL_SHADER)
									{
										PSOADs.clear();
										for (UINT i = 0; i < shaderDesc.OutputParameters; i++)
										{
											D3D12_SIGNATURE_PARAMETER_DESC out_parameter;
											res = shader_reflection->GetOutputParameterDesc(i, &out_parameter);
											MIRU_WARN(res, "WARN: SHADER_CORE: ID3D12ShaderReflection::GetOutputParameterDesc failed.");
											if (res == S_OK)
											{
												Shader::PixelShaderOutputAttributeDescription psoad;
												psoad.location = out_parameter.SemanticIndex;
												psoad.outputType = D3D_REGISTER_COMPONENT_TYPE_to_miru_crossplatform_VertexType(out_parameter.ComponentType, (uint32_t)log2((double)(out_parameter.Mask + 1)));
												psoad.semanticName = out_parameter.SemanticName;
												PSOADs.push_back(psoad);
											}
										}
									}

									if (stage == D3D12_SHADER_VERSION_TYPE::D3D12_SHVER_COMPUTE_SHADER)
									{

										UINT totalThread = shader_reflection->GetThreadGroupSize(&GroupCountXYZ[0], &GroupCountXYZ[1], &GroupCountXYZ[2]);
										if (totalThread == 0)
										{
											MIRU_WARN(true, "WARN: SHADER_CORE: ID3D12ShaderReflection::GetThreadGroupSize returned zero total threads for the ThreadGroupSize.");
										}
									}

									for (auto& rbds : RBDs)
									{
										rbds.second.clear();
									}
									RBDs.clear();

									std::vector<std::string> cis_list;
									for (UINT i = 0; i < shaderDesc.BoundResources; i++)
									{
										D3D12_SHADER_INPUT_BIND_DESC bindingDesc;
										res = shader_reflection->GetResourceBindingDesc(i, &bindingDesc);
										MIRU_WARN(res, "WARN: SHADER_CORE: ID3D12ShaderReflection::GetResourceBindingDesc failed.");
										if (res == S_OK)
										{
											size_t structSize = 0;

											base::DescriptorType descType = D3D_SHADER_INPUT_TYPE_to_miru_crossplatform_DescriptorType(bindingDesc.Type, bindingDesc.Dimension);
											if (descType == base::DescriptorType::UNIFORM_BUFFER)
											{
												ID3D12ShaderReflectionConstantBuffer* shader_reflection_constant_buffer = shader_reflection->GetConstantBufferByName(bindingDesc.Name); //No need to release.
												if (shader_reflection_constant_buffer)
												{
													_D3D12_SHADER_BUFFER_DESC shaderBufferDesc;
													res = shader_reflection_constant_buffer->GetDesc(&shaderBufferDesc);
													MIRU_WARN(res, "WARN: SHADER_CORE: ID3D12ShaderReflectionConstantBuffer::GetDesc failed.");
													if (res == S_OK)
														structSize = static_cast<size_t>(shaderBufferDesc.Size);
												}
												else
												{
													MIRU_WARN(true, "WARN: SHADER_CORE: ID3D12ShaderReflection::GetConstantBufferByName failed.");
												}
											}

											std::string name = bindingDesc.Name;
											if (name.find("CIS") != std::string::npos)
											{
												bool found = false;
												for (auto& cis : cis_list)
												{
													if (cis.compare(name.substr(0, name.find_last_of('_'))) == 0)
													{
														found = true;
														break;
													}
												}
												if (found)
												{
													continue;
												}
												else
													cis_list.push_back(name.substr(0, name.find_last_of('_')));
												descType = base::DescriptorType::COMBINED_IMAGE_SAMPLER;
											}

											Shader::ResourceBindingDescription rbd;
											rbd.binding = bindingDesc.BindPoint;
											rbd.type = descType;
											rbd.descriptorCount = bindingDesc.BindCount;
											rbd.stage = get_shader_stage(stage);
											rbd.name = name;
											rbd.structSize = structSize;
											RBDs[bindingDesc.Space][bindingDesc.BindPoint] = rbd;
										}
									}
								}
							}
							MIRU_D3D12_SAFE_RELEASE(shader_reflection);
						}
						else
						{
							//Library Refelection
							ID3D12LibraryReflection* library_reflection = nullptr;
							MIRU_WARN(dxc_container_reflection->GetPartReflection(partIndex, IID_PPV_ARGS(&library_reflection)), "WARN: SHADER_CORE: IDxcContainerReflection::GetPartReflection failed.");
							if (library_reflection)
							{
								for (auto& rbds : RBDs)
								{
									rbds.second.clear();
								}
								RBDs.clear();

								D3D12_LIBRARY_DESC libraryDesc;
								res = library_reflection->GetDesc(&libraryDesc);
								MIRU_WARN(res, "WARN: SHADER_CORE: ID3D12LibraryReflection::GetDesc failed.");
								if (res == S_OK)
								{
									for (UINT i = 0; i < libraryDesc.FunctionCount; i++)
									{
										ID3D12FunctionReflection* function_reflection = library_reflection->GetFunctionByIndex(i);
										if (function_reflection)
										{
											D3D12_FUNCTION_DESC functionDesc;
											res = function_reflection->GetDesc(&functionDesc);
											MIRU_WARN(res, "WARN: SHADER_CORE: ID3D12FunctionReflection::GetDesc failed.");
											if (res == S_OK)
											{
												D3D12_SHADER_VERSION_TYPE stage = (D3D12_SHADER_VERSION_TYPE)D3D12_SHVER_GET_TYPE(functionDesc.Version);
												uint16_t major = D3D12_SHVER_GET_MAJOR(functionDesc.Version);
												uint16_t minor = D3D12_SHVER_GET_MINOR(functionDesc.Version);

												std::vector<std::string> cis_list;
												for (UINT j = 0; j < functionDesc.BoundResources; j++)
												{
													D3D12_SHADER_INPUT_BIND_DESC bindingDesc;
													function_reflection->GetResourceBindingDesc(j, &bindingDesc);
													MIRU_WARN(res, "WARN: SHADER_CORE: ID3D12FunctionReflection::GetResourceBindingDesc failed.");
													if (res == S_OK)
													{
														size_t structSize = 0;

														base::DescriptorType descType = D3D_SHADER_INPUT_TYPE_to_miru_crossplatform_DescriptorType(bindingDesc.Type, bindingDesc.Dimension);
														if (descType == base::DescriptorType::UNIFORM_BUFFER)
														{
															ID3D12ShaderReflectionConstantBuffer* shader_reflection_constant_buffer = function_reflection->GetConstantBufferByName(bindingDesc.Name); //No need to release.
															if (shader_reflection_constant_buffer)
															{
																_D3D12_SHADER_BUFFER_DESC shaderBufferDesc;
																res = shader_reflection_constant_buffer->GetDesc(&shaderBufferDesc);
																MIRU_WARN(res, "WARN: SHADER_CORE: ID3D12ShaderReflectionConstantBuffer::GetDesc failed.");
																if (res == S_OK)
																	structSize = static_cast<size_t>(shaderBufferDesc.Size);
															}
															else
															{
																MIRU_WARN(true, "WARN: SHADER_CORE: ID3D12FunctionReflection::GetConstantBufferByName failed.");
															}
														}

														std::string name = bindingDesc.Name;
														if (name.find("CIS") != std::string::npos)
														{
															bool found = false;
															for (auto& cis : cis_list)
															{
																if (cis.compare(name.substr(0, name.find_last_of('_'))) == 0)
																{
																	found = true;
																	break;
																}
															}
															if (found)
															{
																continue;
															}
															else
																cis_list.push_back(name.substr(0, name.find_last_of('_')));
															descType = base::DescriptorType::COMBINED_IMAGE_SAMPLER;
														}

														Shader::ResourceBindingDescription& rbd = RBDs[bindingDesc.Space][bindingDesc.BindPoint];
														rbd.binding = bindingDesc.BindPoint;
														rbd.type = descType;
														rbd.descriptorCount = bindingDesc.BindCount;
														rbd.stage |= get_shader_stage(stage);
														rbd.name = name;
														rbd.structSize = structSize;
													}
												}
											}
										}
										else
										{
											MIRU_WARN(true, "WARN: SHADER_CORE: ID3D12LibraryReflection::GetFunctionByIndex failed.");
										}
									}
								}
							}
							MIRU_D3D12_SAFE_RELEASE(library_reflection);
						}
					}
				}
			}
			MIRU_D3D12_SAFE_RELEASE(dxc_container_reflection);
		}
		MIRU_D3D12_SAFE_RELEASE(dxc_shader_bin);
	}
	MIRU_D3D12_SAFE_RELEASE(dxc_library);

	arc::DynamicLibrary::Unload(s_HModeuleDxcompiler);
}
#endif