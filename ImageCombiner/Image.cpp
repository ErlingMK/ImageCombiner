#include "pch.h"
#include "Image.h"


Image::Image()
{
}

Image::Image(std::string width, std::string height, std::string maxVal)
{ 
	Image::height = height;
	Image::width = width;
	Image::colorVal = maxVal;
}


Image::~Image()
{
}

bool Image::IsPpm(std::fstream & imageFile)
{
	if (imageFile.is_open())
	{
		char * magicNumber = new char[3];
		magicNumber[2] = '\0';

		imageFile.read(magicNumber, 2);

		std::string no = magicNumber;
		if (no != "P6")
		{
			delete[] magicNumber;
			std::cout << "Not a .PPM file. Exiting..." << std::endl;
			exit(-1);
		}
		else
		{
			delete[] magicNumber;
			isPpm = true;
			return true;
		}
	}
	isPpm = false;
	return false;
}

void Image::GetDimensionsAndColorValue(std::fstream& imageFile)
{
	if (imageFile.is_open())
	{
		imageFile.seekg(3);

		char c = '0';
		while (!isspace(c))
		{
			imageFile.get(c);
			width += c;
		}
		width.pop_back();

		c = '0';
		while (!isspace(c))
		{
			imageFile.get(c);
			height += c;
		}
		height.pop_back();

		c = '0';
		while (!isspace(c))
		{
			imageFile.get(c);
			colorVal+= c;
		}
		colorVal.pop_back();
	}
}

void Image::ExtractImageRows(std::fstream & imageFile)
{
	if (!isPpm) 
	{
		if (!IsPpm(imageFile)) return;
		GetDimensionsAndColorValue(imageFile);
	}

	int i_width = std::stoi(width);
	int i_height = std::stoi(height);

	//char* row;
	for (int i = 0; i < i_height; ++i)
	{
		std::shared_ptr<char[]> row(new char[i_width * 3]);
		//row = new char[i_width * 3];
		if (imageFile.is_open())
		{
			imageFile.read(row.get(), (i_width * 3));
		}
		pixelRows.emplace_back(row);
	}
	//row = nullptr;
}

void Image::FillWithBlack()
{
	int i_width = std::stoi(width);
	int i_height = std::stoi(height);
	for (int i = 0; i < i_height; ++i)
	{
		std::shared_ptr<char[]> blackRow(new char[i_width * 3]);
		char * t = blackRow.get();
		for (int i = 0; i < i_width * 3; ++i)
		{
			t[i] = '0';
		}
		pixelRows.emplace_back(blackRow);
	}
}
