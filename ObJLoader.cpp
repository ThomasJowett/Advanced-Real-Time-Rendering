#include "ObJLoader.h"
#include <fstream>

IndexedModel OBJLoader::Load(const char * filename, bool invertCoordinates)
{
	OBJModel model;

	model.hasUVs = false;
	model.hasNormals = false;

	std::ifstream file;
	file.open(filename);

	if (file.is_open())
	{
		std::string line;

		while (file.good())
		{
			getline(file, line);

			unsigned int lineLength = line.length();

			if (lineLength < 2)
				continue;

			const char* lineCstr = line.c_str();

			switch (lineCstr[0])
			{
			case 'V':

			}
		}
	}

	return ToIndexedModel(model);
}
