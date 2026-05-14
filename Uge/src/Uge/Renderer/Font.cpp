#include <ugpch.h>
#include "Font.h"


// msdf
#undef INFINITE
#include <msdf-atlas-gen.h>
#include "GlyphGeometry.h"
#include <FontGeometry.h>

#include "Uge/Renderer/MSDFData.h"


namespace Uge
{
	

	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static Ref<Texture2D> createAndCacheAtlas(const std::string& fontName, float fontSize, 
		const std::vector<msdf_atlas::GlyphGeometry>& glyphs, const msdf_atlas::FontGeometry& fontGeometry, 
		uint32_t width, uint32_t height)
	{
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;

		msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
		generator.setAttributes(attributes);
		generator.setThreadCount(8);
		generator.generate(glyphs.data(), (int)glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

		TextureSpecification spec;
		spec.Width = bitmap.width;
		spec.Height = bitmap.height;
		spec.Format = ImageFormat::RGB8;
		spec.GenerateMips = false;

		Ref<Texture2D> texture = Texture2D::Create(spec);
		texture->SetData((void*)bitmap.pixels, bitmap.width * bitmap.height * 3);
		return texture;


	}

	Font::Font(const std::filesystem::path& filepath)
		: m_data(new MSDFData())
	{

		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		UG_CORE_ASSERT(ft, "FreeType not initialized!");

		
		std::string fileString = filepath.string();

		// TODO: msdfgen::loadFontData loads from memory buffer
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, fileString.c_str());

		if (!font)
		{
			UG_CORE_ERROR("Failed to load font: {0}", fileString);
			return;
		}

		struct CharsetRange
		{
			uint32_t Begin, End;
		};

		// From imgui_draw.cpp
		static const CharsetRange charsetRanges[] = {
			{ 0x0020, 0x00FF } // Basic Latin + Latin supplement
			
		};

		msdf_atlas::Charset charset;

		for (CharsetRange range : charsetRanges)
		{

			for (uint32_t c = charsetRanges->Begin; c <= charsetRanges->End; c++)
			{
				charset.add(c);
			}

		}
		
		double fontScale = 1.0;

		m_data->Fonts = msdf_atlas::FontGeometry(&m_data->Glyphs);
		int glyphsLoaded = m_data->Fonts.loadCharset(font, fontScale, charset);
		UG_CORE_INFO("{0} Glyphs Loaded out of {1} from font {2}", glyphsLoaded, charset.size(), fileString);

		double emSize = 40.0;

		msdf_atlas::TightAtlasPacker atlasPacker;
		atlasPacker.setPixelRange(2.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setPadding(0);
		atlasPacker.setScale(emSize);

		int remaining = atlasPacker.pack(m_data->Glyphs.data(), (int)m_data->Glyphs.size());
		UG_CORE_ASSERT(remaining == 0);

		int height, width;
		atlasPacker.getDimensions(width, height);
		emSize = atlasPacker.getScale();

#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define THREAD_COUNT 8
		// if MSDF || MTSDF

		uint64_t coloringSeed = 0;
		bool expensiveColoring = false;
		if (expensiveColoring)
		{
			msdf_atlas::Workload([&glyphs = m_data->Glyphs, &coloringSeed](int i, int threadNo) -> bool {
				uint64_t glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
				glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
				return true;
				}, m_data->Glyphs.size()).finish(THREAD_COUNT);
		}
		else {
			uint64_t glyphSeed = coloringSeed;
			for (msdf_atlas::GlyphGeometry& glyph : m_data->Glyphs)
			{
				glyphSeed *= LCG_MULTIPLIER;
				glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			}
		}



		m_atlasTexture = createAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>("Test", (float)emSize, 
			m_data->Glyphs, m_data->Fonts, width, height);


#if 0
		msdfgen::Shape shape;
		if (msdfgen::loadGlyph(shape, font, 'K'))
		{
			shape.normalize();
			//                      max. angle
			msdfgen::edgeColoringSimple(shape, 3.0);
			//           image width, height
			msdfgen::Bitmap<float, 3> msdf(32, 32);
			//                     range, scale, translation
			msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
			msdfgen::savePng(msdf, "output.png");
		}
#endif


		msdfgen::destroyFont(font);

		msdfgen::deinitializeFreetype(ft);


	}

	Font::~Font()
	{

		delete m_data;

	}

	Ref<Font> Font::GetDefault()
	{

		static Ref<Font> defaultFont;
		if (!defaultFont)
		{
			defaultFont = CreateRef<Font>("assets/fonts/Roboto-Regular/static/Roboto-Regular.ttf");
		}

		return defaultFont;
		
	}

}