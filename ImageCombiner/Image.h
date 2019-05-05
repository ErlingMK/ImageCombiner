#pragma once
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <exception>
class Image
{
public:
	Image();
	Image(std::string width, std::string height, std::string maxVal);
	virtual ~Image();

	std::string height;
	std::string width;
	std::string colorVal;
	std::vector<std::shared_ptr<char[]>> pixelRows;

	bool IsPpm(std::fstream& imageFile);
	void GetDimensionsAndColorValue(std::fstream& imageFile);
	void ExtractImageRows(std::fstream& imageFile);
	void FillWithBlack();

private:
	bool isPpm;
};

