
#include "pch.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <exception>
#include "Image.h"


static std::filesystem::path recordingDir;
static int numberOfImageColumns;
static int numberOfImageRows;
static int numberOfCameras;
static int numberOfImagesPerCamera;
static std::string nameOfNewImageFolder = "combined";

static std::streampos widthPos = 3;
static int widthBytes = 4;
static std::streampos startOfImageData;

void CreatePpm(std::vector<Image> images, int imageFileName)
{
	// Creates a folder in recordingDir for the new combined images, if it doesn't already exist.
	std::filesystem::path combinedImageFolder = recordingDir;
	combinedImageFolder /= nameOfNewImageFolder;
	std::filesystem::create_directory(combinedImageFolder);

	int imageWidth = 1280;
	int imageHeigth = 1024;

	std::fstream newFile;

	std::filesystem::path newFilePath = combinedImageFolder;
	newFile.open(newFilePath /= (std::to_string(imageFileName) + ".ppm"), std::ios::binary | std::ios::out);
	newFile << "P6\n" << imageWidth * numberOfImageColumns << " " << imageHeigth * numberOfImageRows << "\n" << "255\n";

	// må gjøres like mange ganger som det er rader med bilder

	for (int k = 0; k < numberOfImageRows; ++k)
	{
		for (int i = 0; i < imageHeigth; ++i)
		{
			// må gjøres like mange ganger som det er søyler med bilder
			for (int j = 0; j < numberOfImageColumns; ++j) {
				std::vector<std::shared_ptr<char[]>>& pixelRows = images[j + numberOfImageColumns * k].pixelRows;
				newFile.write(pixelRows[i].get(), imageWidth * 3);
			}
		}
	}
	newFile.close();
}

// TODO: Find a way to get imagefile name for each image to be combined
void CombineImagesFromOneTimestamp(const int imageFileName, std::vector<std::string>& foldersInOrder)
{
	std::fstream imageFile;
	//std::vector<std::vector<std::shared_ptr<char[]>>> images;

	std::vector<Image> images;

	for (int cameraNumber = 0; cameraNumber < numberOfCameras; ++cameraNumber)
	{
		Image image;

		// Vector will be filled with rows of pixels in an image
		//std::vector<std::shared_ptr<char[]>> image;

		// Name of imagefolder and name of image. Images to be combined must have the same name.
		std::filesystem::path _recordingDir = recordingDir;
		std::filesystem::path imageFilePath(foldersInOrder[cameraNumber] + "\\" + std::to_string(imageFileName) + ".ppm");

		imageFile.open(_recordingDir /= imageFilePath, std::ios::binary | std::ios::in);
		
		image.ExtractImageRows(imageFile);
		images.push_back(image);
		
		imageFile.close();
	}
	CreatePpm(images, imageFileName);
}

void CombineImages(std::vector<std::string>& foldersInOrder)
{
	for (int imageFileName = 0; imageFileName < numberOfImagesPerCamera; ++imageFileName)
	{
		CombineImagesFromOneTimestamp(imageFileName, foldersInOrder);
	}
}

// Extracts the order the images from the cameras shall be put in from a txt file containing the names of the image folders in that order. The txt file must be named the same as the folder containing the image folders. One image folder per camera.
void FindFolderOrder(std::filesystem::path fileName, std::vector<std::string>& foldersInOrder)
{
	std::fstream txtFile;
	std::string line;
	txtFile.open(fileName /= fileName.filename() += ".txt", std::ios::in);

	if (txtFile.is_open())
	{
		while (std::getline(txtFile, line))
		{
			foldersInOrder.push_back(line);
		}
	}
	txtFile.close();
}

int main(int arg, char const* argv[])
{
	// Path to folder with subfolders for each cam, number of images per cam, row number, col number
	if (arg != 5)
	{
		std::cout << "Incorrect arguments!\n";
		exit(EXIT_FAILURE);
	}
	recordingDir = argv[1];
	numberOfImagesPerCamera = std::stoi(argv[2]);
	numberOfImageRows = std::stoi(argv[3]);
	numberOfImageColumns = std::stoi(argv[4]);
	
	recordingDir.make_preferred();

	std::vector<std::string> foldersInOrder;
	FindFolderOrder(recordingDir, foldersInOrder);

	numberOfCameras = foldersInOrder.size();

	CombineImages(foldersInOrder);
}

