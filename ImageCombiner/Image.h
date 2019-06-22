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
	Image(std::string width, std::string height, std::string max_val);
	~Image();

	std::string height;
	std::string width;
	std::string max_val;
	std::string image_format;
	std::vector<std::shared_ptr<char[]>> pixel_rows;

	std::string determineFormat(std::fstream& image_file);
	void determineDimensionsAndMaxValue(std::fstream& image_file);
	void extractImageRows(std::fstream& image_file);
	void fillWithBlack();

private:
	bool is_ppm_;
	bool is_pgm_;
	int m_data_points_per_pixel_;
	int m_bytes_per_data_point_;
};

