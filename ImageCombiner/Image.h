#pragma once
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <exception>

class Image final
{
public:
	Image();
	Image(long width, long height, long max_val);
	~Image();

	long height;
	long width;
	long max_val;
	std::string image_format;
	std::string magic_number;
	long m_data_points_per_pixel;
	long m_bytes_per_data_point;

	std::vector<std::shared_ptr<char[]>> pixel_rows;

	void determineFormat(std::fstream& image_file);
	void determineDimensionsAndMaxValue(std::fstream& image_file);
	void extractImageRows(std::fstream& image_file);

	void fillWithBlack();

private:
	std::string height_;
	std::string width_;
	std::string max_val_;
	bool is_ppm_;
	bool is_pgm_;
};

