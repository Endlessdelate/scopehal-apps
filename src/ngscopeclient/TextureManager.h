/***********************************************************************************************************************
*                                                                                                                      *
* ngscopeclient                                                                                                        *
*                                                                                                                      *
* Copyright (c) 2012-2025 Andrew D. Zonenberg                                                                          *
* All rights reserved.                                                                                                 *
*                                                                                                                      *
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the     *
* following conditions are met:                                                                                        *
*                                                                                                                      *
*    * Redistributions of source code must retain the above copyright notice, this list of conditions, and the         *
*      following disclaimer.                                                                                           *
*                                                                                                                      *
*    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the       *
*      following disclaimer in the documentation and/or other materials provided with the distribution.                *
*                                                                                                                      *
*    * Neither the name of the author nor the names of any contributors may be used to endorse or promote products     *
*      derived from this software without specific prior written permission.                                           *
*                                                                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL *
* THE AUTHORS BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES        *
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR       *
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
* POSSIBILITY OF SUCH DAMAGE.                                                                                          *
*                                                                                                                      *
***********************************************************************************************************************/

/**
	@file
	@author Andrew D. Zonenberg
	@brief Declaration of TextureManager
 */
#ifndef TextureManager_h
#define TextureManager_h

class TextureManager;

#include <png.h>

/**
	@brief Encapsulates the various Vulkan objects we need to represent texture image memory

	We don't do multitexturing at the moment, so each texture has its own single-binding descriptor set
 */
class Texture
{
public:
	Texture(
		const vk::raii::Device& device,
		const vk::ImageCreateInfo& imageInfo,
		const vk::raii::Buffer& srcBuf,
		int width,
		int height,
		TextureManager* mgr,
		const std::string& name = "",
		bool upsampleLinear = true		//false for nearest neighbor upsampling instead
		);

	Texture(
		const vk::raii::Device& device,
		const vk::ImageCreateInfo& imageInfo,
		TextureManager* mgr,
		const std::string& name = ""
		);

	~Texture();

	ImTextureID GetTexture()
	{ return m_texture; }

	vk::ImageView GetView()
	{ return **m_view; }

	vk::Image GetImage()
	{ return *m_image; }

	void SetName(const std::string& name);

protected:
	void LayoutTransition(
		vk::raii::CommandBuffer& cmdBuf,
		vk::AccessFlags src,
		vk::AccessFlags dst,
		vk::ImageLayout from,
		vk::ImageLayout to);

	///@brief Image object for our texture
	vk::raii::Image m_image;

	///@brief View of the image
	std::unique_ptr<vk::raii::ImageView> m_view;

	ImTextureID m_texture;

	///@brief Device memory backing the image
	std::unique_ptr<vk::raii::DeviceMemory> m_deviceMemory;
};

/**
	@brief Manages loading and saving texture resources to files
 */
class TextureManager
{
public:
	TextureManager(std::shared_ptr<QueueHandle> queue);
	virtual ~TextureManager();

	void LoadTexture(
		const std::string& name,
		const std::string& path);

	GLFWimage LoadPNGToGLFWImage(const std::string& path);

	ImTextureID GetTexture(const std::string& name)
	{
		auto it = m_textures.find(name);
		if(it == m_textures.end())
		{
			LogFatal(
				"Texture \"%s\" not found. This is probably the result of a developer mistyping a texture ID.\n",
				name.c_str());
		}
		else
			return it->second->GetTexture();
	}

	std::unique_ptr<vk::raii::Sampler>& GetSampler()
	{ return m_sampler; }

	std::unique_ptr<vk::raii::Sampler>& GetNearestSampler()
	{ return m_nearestSampler; }

	void clear()
	{ m_textures.clear(); }

	vk::raii::CommandBuffer& GetCmdBuffer()
	{ return *m_cmdBuf; }

	std::shared_ptr<QueueHandle> GetQueue()
	{ return m_queue; }

	vk::ImageView GetView(const std::string& name)
	{ return m_textures[name]->GetView(); }

protected:

	png_structp LoadPNG(
		const std::string& path,
		size_t& width,
		size_t& height,
		FILE*& fp,
		png_infop& info,
		png_infop& end);

	std::map<std::string, std::shared_ptr<Texture> > m_textures;

	///@brief Sampler for textures
	std::unique_ptr<vk::raii::Sampler> m_sampler;

	std::unique_ptr<vk::raii::Sampler> m_nearestSampler;

	std::shared_ptr<QueueHandle> m_queue;
	std::unique_ptr<vk::raii::CommandPool> m_cmdPool;
	std::unique_ptr<vk::raii::CommandBuffer> m_cmdBuf;
};

#endif
