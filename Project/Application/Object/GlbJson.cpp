#include "GlbJson.h"
#include <cstdint>
#include <cstring>
#include <fstream>

namespace App::GlbJson
{
	//12バイトの頭のあとに4バイトのJSONの長さと4バイトの種類があり、そのあとがJSON
	nlohmann::json Read(const std::string& path)
	{
		std::ifstream file(path, std::ios::binary);
		char header[20] = {};
		if (!file.read(header, sizeof(header)))
		{
			return nlohmann::json();
		}
		uint32_t length = 0;
		std::memcpy(&length, header + 12, sizeof(length));
		std::string text(length, '\0');
		if (!file.read(text.data(), static_cast<std::streamsize>(length)))
		{
			return nlohmann::json();
		}
		const nlohmann::json json = nlohmann::json::parse(text, nullptr, false);
		if (json.is_discarded())
		{
			return nlohmann::json();
		}
		return json;
	}
}
