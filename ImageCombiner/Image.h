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
	virtual ~Image();

	std::string heigth;
	std::string width;
	std::string colorVal;
	std::vector<std::shared_ptr<char[]>> pixelRows;

	bool IsPpm(std::fstream& imageFile);
	void GetDimensionsAndColorValue(std::fstream& imageFile);
	void ExtractImageRows(std::fstream& imageFile);

private:
	bool isPpm;
};

