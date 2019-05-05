
#include "pch.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <exception>
#include <iomanip>
#include "Image.h"
#pragma warning(disable : 4996)

static std::filesystem::path recordingDir;
static int numberOfImageColumns;
static int numberOfImageRows;
static int numberOfCameras;
static int numberOfImagesPerCamera;
static std::string nameOfNewImageFolder = "combined";

static std::streampos widthPos = 3;
static int widthBytes = 4;
static std::streampos startOfImageData;


std::string CurrentDateTime()
{
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%d_%m_%y_%H_%M_%S", &tstruct);

	return buf;
}

int helper()
{
	return(numberOfImageColumns * numberOfImageRows) - numberOfCameras;
}

// All images must be of the same dimensions.
void CreatePpm(std::vector<std::vector<Image>>& imageMatrix, int imageFileName)
{
	std::fstream newFile;
	int imageWidth = std::stoi(imageMatrix[0][0].width);
	int imageHeight = std::stoi(imageMatrix[0][0].height);
	int maxVal = std::stoi(imageMatrix[0][0].colorVal);

	// Creates a folder in recordingDir for the new combined images, if it doesn't already exist.
	std::filesystem::path combinedImageFolder = recordingDir;
	combinedImageFolder /= nameOfNewImageFolder;
	std::filesystem::create_directory(combinedImageFolder);
	std::filesystem::path newFilePath = combinedImageFolder;

	newFile.open(newFilePath /= (std::to_string(imageFileName) + ".ppm"), std::ios::binary | std::ios::out);
	newFile << "P6\n" << imageWidth * numberOfImageColumns << " " << imageHeight * numberOfImageRows << "\n" << maxVal <<"\n";

	for (int k = 0; k < imageMatrix.size(); ++k)
	{
		for (int i = 0; i < imageHeight; ++i)
		{
			for (int j = 0; j < imageMatrix[k].size(); ++j) {
				std::vector<std::shared_ptr<char[]>>& pixelRows = imageMatrix[k][j].pixelRows;
				newFile.write(pixelRows[i].get(), imageWidth * 3);
			}
		}
	}
	newFile.close();
	imageMatrix.clear();
}

// TODO: Find a way to get imagefile name for each image to be combined
void CombineImagesFromOneTimestamp(const int imageFileName, std::vector<std::string>& foldersInOrder)
{
	std::fstream imageFile;
	std::vector<std::vector<Image>> imageMatrix;
	int camCounter = 0;

	// Outer loop goes through each row in combined image
	for (int rowNumber = 0; rowNumber < numberOfImageRows; ++rowNumber)
	{
		imageMatrix.push_back(std::vector<Image>());

		int imagesInLastCol = (numberOfImageColumns - helper());
		if (rowNumber == (numberOfImageRows - 1) && imagesInLastCol != numberOfImageColumns)
		{
			for (int posInLastRow = 0; posInLastRow < numberOfImageColumns; ++posInLastRow)
			{
				Image image;

				// Fills any remainding places in last image row with empty images
				if (posInLastRow >= imagesInLastCol)
				{
					Image emptyImage("1280", "1024", "255");
					emptyImage.FillWithBlack();
					imageMatrix[rowNumber].push_back(emptyImage);
				} 
				else
				{
					std::filesystem::path _recordingDir = recordingDir;
					std::filesystem::path imageFilePath(foldersInOrder[camCounter] + "\\" + std::to_string(imageFileName) + ".ppm");

					imageFile.open(_recordingDir /= imageFilePath, std::ios::binary | std::ios::in);
					image.ExtractImageRows(imageFile);
					imageFile.close();
					imageMatrix[rowNumber].push_back(image);
					camCounter++;
				}
			}
		}
		else
		{
			// This loop goes through each column in combined image
			for (int colNumber = 0; colNumber < numberOfImageColumns; ++colNumber)
			{
				Image image;

				std::filesystem::path _recordingDir = recordingDir;
				std::filesystem::path imageFilePath(foldersInOrder[camCounter] + "\\" + std::to_string(imageFileName) + ".ppm");

				imageFile.open(_recordingDir /= imageFilePath, std::ios::binary | std::ios::in);
				image.ExtractImageRows(imageFile);
				imageFile.close();
				imageMatrix[rowNumber].push_back(image);
				camCounter++;
			}
		}
	}
	CreatePpm(imageMatrix, imageFileName);
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

void WriteToLog(std::string message)
{
	std::string currentDateTime = CurrentDateTime();
	std::fstream logFile("error_log.txt", std::ios::out | std::ios::app);
	if (logFile.is_open())
	{
		logFile << currentDateTime << ' ' << message << '\n';
	}
	logFile.close();
}

int main(int arg, char const* argv[])
{
	// Arguments: [1]Path to folder with subfolders for each cam, [2]number of images per cam, [3]number of rows in combined image, [4]number of columns in combined image
	if (arg != 5)
	{
		WriteToLog("Incorrect number of arguments.");
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
	if (numberOfCameras > (numberOfImageRows - 1) * numberOfImageColumns && numberOfCameras <= (numberOfImageRows * numberOfImageColumns)) CombineImages(foldersInOrder);
	else
	{
		WriteToLog("Incorrect number of columns or rows.");
		exit(EXIT_FAILURE);
	}
}

