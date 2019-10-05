#include "pch.h"
#include "Image.h"
#include <utility>


Image::Image(): m_data_points_per_pixel(0), m_bytes_per_data_point(0), is_ppm_(false), is_pgm_(false)
{
}

Image::Image(const long width, const long height, const long max_val): height(height), width(width), max_val(max_val), m_data_points_per_pixel(0),
                                                  m_bytes_per_data_point(0),
                                                  is_ppm_(false),
                                                  is_pgm_(false)
{
}


Image::~Image()
= default;

void Image::determineFormat(std::fstream & image_file)
{
	if (image_file.is_open())
	{
		const auto m_n = new char[3];
		m_n[2] = '\0';

		image_file.read(m_n, 2);

		const std::string no = m_n;
		if (no != "P6" && no != "P5")
		{
			delete[] m_n;
			std::cout << "Not a .PPM or .PGM file. Exiting..." << std::endl;
			return;
		}
		if (no == "P6")
		{
			is_ppm_ = true;
			image_format = ".ppm";
			m_data_points_per_pixel = 3;
			delete[] m_n;
			magic_number = "P6";
		}
		else
		{			
			is_pgm_ = true;
			image_format = ".pgm";
			m_data_points_per_pixel = 1;
			delete[] m_n;
			magic_number = "P5";
		}
	}
}

// determineFormat() must be called before this.
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
			width_ += c;
		}
		width_.pop_back();
		width = std::stoi(width_);

		c = '0';
		while (!isspace(c))
		{
			image_file.get(c);
			height_ += c;
		}
		height_.pop_back();
		height = std::stoi(height_);

		c = '0';
		while (!isspace(c))
		{
			image_file.get(c);
			max_val_ += c;
		}
		max_val_.pop_back();
		max_val = std::stoi(max_val_);

		m_bytes_per_data_point = max_val >= 256 ? 2 : 1;
	}
}

void Image::extractImageRows(std::fstream & image_file)
{
	determineFormat(image_file);
	if (!is_ppm_ && !is_pgm_) return;		
	determineDimensionsAndMaxValue(image_file);

	const auto s = m_data_points_per_pixel * width * m_bytes_per_data_point;

	for (auto i = 0; i < height; ++i)
	{
		std::shared_ptr<char[]> row(new char[s]);
		if (image_file.is_open())
		{
			image_file.read(row.get(), (s));
		}
		pixel_rows.emplace_back(row);
	}
}


// Helper method to make a gray/black image. Used to fill in empty spots in an image matrix.
void Image::fillWithBlack()
{
	const auto s = m_data_points_per_pixel * width * m_bytes_per_data_point;

	for (auto i = 0; i < height; ++i)
	{
		std::shared_ptr<char[]> black_row(new char[s]);
		const auto t = black_row.get();
		for (size_t z = 0; z < strlen(t); ++z)
		{
			t[z] = NULL;
		}
		pixel_rows.emplace_back(black_row);
	}
}
