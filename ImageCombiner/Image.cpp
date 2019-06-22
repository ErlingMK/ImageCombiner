#include "pch.h"
#include "Image.h"
#include <utility>


Image::Image(): is_ppm_(false), is_pgm_(false), m_data_points_per_pixel(0), m_bytes_per_data_point_(0)
{
}

Image::Image(std::string width, std::string height, std::string max_val): is_ppm_(false), is_pgm_(false),
                                                                          m_data_points_per_pixel(0),
                                                                          m_bytes_per_data_point_(0)
{
	Image::height = std::move(height);
	Image::width = std::move(width);
	Image::max_val = std::move(max_val);
}


Image::~Image()
= default;

std::string Image::determineFormat(std::fstream & image_file)
{
	if (image_file.is_open())
	{
		const auto magic_number = new char[3];
		magic_number[2] = '\0';

		image_file.read(magic_number, 2);

		const std::string no = magic_number;
		if (no != "P6" && no != "P5")
		{
			delete[] magic_number;
			std::cout << "Not a .PPM or .PGM file. Exiting..." << std::endl;
			return "Unknown format.";
		}
		if (no == "P6")
		{
			is_ppm_ = true;
			image_format = ".ppm";
			m_data_points_per_pixel = 3;
			delete[] magic_number;
			return "P6";
		}
		is_pgm_ = true;
		image_format = ".pgm";
		m_data_points_per_pixel = 1;
		delete[] magic_number;
		return "P5";
	}
	return "File is not open.";
}

void Image::determineDimensionsAndMaxValue(std::fstream& image_file)
{
	if (!is_ppm_ && !is_pgm_) return;

	if (image_file.is_open())
	{
		image_file.seekg(3);

		auto c = '0';
		while (!isspace(c))
		{
			image_file.get(c);
			width += c;
		}
		width.pop_back();

		c = '0';
		while (!isspace(c))
		{
			image_file.get(c);
			height += c;
		}
		height.pop_back();

		c = '0';
		while (!isspace(c))
		{
			image_file.get(c);
			max_val+= c;
		}
		max_val.pop_back();

		m_bytes_per_data_point_ = std::stoi(max_val) >= 256 ? 2 : 1;
	}
}

void Image::extractImageRows(std::fstream & image_file)
{
	determineFormat(image_file);
	if (!is_ppm_ && !is_pgm_) return;		
	determineDimensionsAndMaxValue(image_file);

	const auto i_width = std::stoi(width);
	const auto i_height = std::stoi(height);

	for (auto i = 0; i < i_height; ++i)
	{
		std::shared_ptr<char[]> row(new char[i_width * m_data_points_per_pixel * m_bytes_per_data_point_]);
		if (image_file.is_open())
		{
			image_file.read(row.get(), (i_width * m_data_points_per_pixel * m_bytes_per_data_point_));
		}
		pixel_rows.emplace_back(row);
	}
}

void Image::fillWithBlack()
{
	const auto i_width = std::stoi(width);
	const auto i_height = std::stoi(height);

	for (auto i = 0; i < i_height; ++i)
	{
		std::shared_ptr<char[]> black_row(new char[i_width * m_data_points_per_pixel * m_bytes_per_data_point_]);
		const auto t = black_row.get();
		for (size_t z = 0; z < strlen(t); ++z)
		{
			t[z] = NULL;
		}
		pixel_rows.emplace_back(black_row);
	}
}
