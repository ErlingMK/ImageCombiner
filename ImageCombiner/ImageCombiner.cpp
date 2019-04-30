
#include "pch.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>


static std::string recordingDir;
static int numberOfImageColumns;
static int numberOfImageRows;

static std::streampos widthPos = 3;
static int widthBytes = 4;
static std::streampos startOfImageData;

bool IsPpm(std::fstream& imageFile)
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
			return true;
		}
	}
	return false;
}

void GetDimensionsAndColorValue(std::fstream& imageFile, std::string& imageWidth, std::string& imageHeight, std::string& maxColorValue)
{
	if (imageFile.is_open())
	{
		imageFile.seekg(widthPos);
		
		char c = '0';
		while (!isspace(c))
		{
			imageFile.get(c);
			imageWidth += c;
		}
		imageWidth.pop_back();
		
		c = '0';
		while (!isspace(c))
		{
			imageFile.get(c);
			imageHeight += c;
		}
		imageHeight.pop_back();

		c = '0';
		while (!isspace(c))
		{
			imageFile.get(c);
			maxColorValue += c;
		}
		maxColorValue.pop_back();
	}
}

void ExtractImageRows(std::fstream& imageFile, std::vector<std::shared_ptr<char[]>>& imageRows)
{
	std::string width;
	std::string heigth;
	std::string maxColorValue;

	if (!IsPpm(imageFile)) return;
	GetDimensionsAndColorValue(imageFile, width, heigth, maxColorValue);

	int i_width = std::stoi(width);
	int i_heigth = std::stoi(heigth);

	//char* row;
	for (int i = 0; i < i_heigth; ++i)
	{
		std::shared_ptr<char[]> row(new char[i_width * 3]);
		//row = new char[i_width * 3];
		if (imageFile.is_open())
		{
			imageFile.read(row.get(),(i_width * 3));
		}
		imageRows.emplace_back(row);
	}
	//row = nullptr;
}

void CreatePpm(std::vector<std::vector<std::shared_ptr<char[]>>> images, int imageNumber)
{
	int imageWidth = 1280;
	int imageHeigth = 1024;
	std::filesystem::create_directory(recordingDir + "/combined");

	std::fstream newFile;
	newFile.open(recordingDir + "/combined/" + std::to_string(imageNumber) + ".ppm", std::ios::binary | std::ios::out);
	newFile << "P6\n" << "2560 2048\n" << "255\n";

	// må gjøres like mange ganger som det er rader med bilder

	for (int k = 0; k < numberOfImageRows; ++k)
	{
		for (int i = 0; i < imageHeigth; ++i)
		{
			// må gjøres like mange ganger som det er søyler med bilder
			for (int j = 0; j < numberOfImageColumns; ++j) {
				std::vector<std::shared_ptr<char[]>>& pixelRows = images[j + numberOfImageColumns * k];
				newFile.write(pixelRows[i].get(), imageWidth * 3);
			}

			/*
			for (std::vector<std::shared_ptr<char[]>>& image : images)
			{
			}
			*/
		}
	}
	newFile.close();
}

void CombineImagesFromOneTimestamp(const int imageNumber, const int numberOfCameras)
{
	std::fstream imageFile;
	std::vector<std::vector<std::shared_ptr<char[]>>> images;

	for (int cameraNumber = 0; cameraNumber < numberOfCameras; ++cameraNumber)
	{
		imageFile.open(recordingDir + "/" + std::to_string(cameraNumber) + "/" + std::to_string(imageNumber) + ".ppm", std::ios::binary | std::ios::in);
		std::vector<std::shared_ptr<char[]>> image;
		ExtractImageRows(imageFile, image);
		imageFile.close();
		images.push_back(image);
	}
	CreatePpm(images, imageNumber);
}

void CombineImages(const int numberOfImagesPerCamera, const int numberOfCameras)
{
	for (int imageNumber = 0; imageNumber < numberOfImagesPerCamera; ++imageNumber)
	{
		CombineImagesFromOneTimestamp(imageNumber, numberOfCameras);
	}
}

int main(int arg, char const* argv[])
{
	// Path to folder with subfolders for each cam, number of images per cam, number of cams, row number, col number

	numberOfImageRows = std::stoi(argv[4]);
	numberOfImageColumns = std::stoi(argv[5]);

	recordingDir = argv[1];
	CombineImages(std::stoi(argv[2]), std::stoi(argv[3]));
}

