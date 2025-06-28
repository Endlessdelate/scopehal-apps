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

#include "ngscopeclient.h"
#include "FontManager.h"
#include "PreferenceManager.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction / destruction

FontManager::FontManager()
{
}

FontManager::~FontManager()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Font building

/**
	@brief Check for changes to our fonts and, if any are found, reload

	@return True if changes were made to the font atlas
 */
bool FontManager::UpdateFonts(PreferenceCategory& root, float contentScale)
{
	//Make a list of fonts we want to have
	set<FontDescription> fonts;
	AddFontDescriptions(root.AsCategory(), fonts);

	//Check if we have them all
	bool missingFonts = false;
	for(auto f : fonts)
	{
		if(m_fonts.find(f) == m_fonts.end())
		{
			missingFonts = true;
			break;
		}
	}

	//If no missing fonts, no need to touch anything
	if(!missingFonts)
		return false;

	//Clear existing fonts, if any
	ImGuiIO& io = ImGui::GetIO();
	auto& atlas = io.Fonts;
	atlas->Clear();
	m_fonts.clear();

	//Load the fonts
	ImFontConfig config;
	config.PixelSnapH = true;
	config.OversampleH = 5;
	config.OversampleV = 5;
	string defaultFontPath = FindDataFile("fonts/DejaVuSans.ttf");
	for(auto f : fonts)
	{
		float scaledsize = round(max(1.0f, f.second) * contentScale);

		//See if the file exists, if it doesn't exist use the default font
		//(note, things will go bad if you pass a file that's not a valid TTF)
		string fname = f.first;
		FILE* fp = fopen(fname.c_str(), "rb");
		if(fp)
			fclose(fp);
		else
		{
			LogWarning(
				"Could not find font file \"%s\" requested in preferences database. "
				"Using default font \"%s\" instead\n",
				fname.c_str(),
				defaultFontPath.c_str());
			fname = defaultFontPath;
		}

		m_fonts[f] = atlas->AddFontFromFileTTF(fname.c_str(), scaledsize, &config);
	}

	//Done loading fonts, build the texture
	atlas->Flags = ImFontAtlasFlags_NoMouseCursors;
	atlas->Build();

	return true;
}

/**
	@brief Get the font descriptions for each preference category
 */
void FontManager::AddFontDescriptions(PreferenceCategory& cat, set<FontDescription>& fonts)
{
	auto& children = cat.GetChildren();
	for(const auto& identifier: cat.GetOrdering())
	{
		auto& node = children[identifier];

		if(node->IsCategory())
			AddFontDescriptions(node->AsCategory(), fonts);

		if(node->IsPreference())
		{
			auto& pref = node->AsPreference();
			if(pref.GetType() != PreferenceType::Font)
				continue;

			fonts.emplace(pref.GetFont());
		}
	}
}
