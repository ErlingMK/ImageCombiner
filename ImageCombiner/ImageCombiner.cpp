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
static int numberOfImageRows;
static int numberOfImageColumns;
static int numberOfCameras;
static int numberOfImagesPerCamera;
static std::string nameOfNewImageFolder = "combined";
static std::string imageFormat = ".pgm";

static std::streampos widthPos = 3;
static int widthBytes = 4;
static std::streampos startOfImageData;


std::string currentDateTime()
{
	auto now = time(nullptr);
	struct tm tstruct{};
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%d_%m_%y_%H_%M_%S", &tstruct);

	return buf;
}

int helper()
{
	return (numberOfImageColumns * numberOfImageRows) - numberOfCameras;
}

void readImageAndExtractPixels(const std::filesystem::path& image_file_path, std::vector<Image>& image_row)
{
	std::fstream image_file;
	Image image;

	image_file.open(image_file_path, std::ios::binary | std::ios::in);
	image.extractImageRows(image_file);
	image_file.close();

	image_row.push_back(image);
}


// All images must be of the same dimensions.
void createCombinedImage(std::vector<std::vector<Image>>& image_matrix, const int image_file_name)
{
	std::fstream new_image_file;

	const auto single_image_width = std::stoi(image_matrix[0][0].width);
	const auto single_image_height = std::stoi(image_matrix[0][0].height);
	const auto max_val = std::stoi(image_matrix[0][0].max_val);
	const auto m_image_format = image_matrix[0][0].image_format;
	const auto data_points = image_matrix[0][0].m_data_points_per_pixel;

	// Creates a folder in recordingDir for the new combined images, if it doesn't already exist.
	auto combined_image_folder = recordingDir;
	combined_image_folder /= nameOfNewImageFolder;
	create_directory(combined_image_folder);
	auto new_file_path = combined_image_folder;

	new_image_file.open(new_file_path /= (std::to_string(image_file_name) + m_image_format), std::ios::binary | std::ios::out);

	new_image_file << "P5" << "\n" << single_image_width * numberOfImageColumns << " " << single_image_height * numberOfImageRows << "\n" << max_val
		<< "\n";

	// Each image row
	for (auto& k : image_matrix)
	{
		for (auto i = 0; i < single_image_height; ++i)
		{
			for (auto& j : k)
			{
				auto& pixel_rows = j.pixel_rows;
				new_image_file.write(pixel_rows[i].get(), single_image_width * data_points);
			}
		}
	}
	new_image_file.close();
	image_matrix.clear();
}

Image checkDimensions(std::vector<std::string>& folders_in_order, const int image_file_name)
{
	std::fstream image_file;
	Image image;

	auto recording_dir = recordingDir;
	const auto image_file_path(recording_dir /=
		folders_in_order[0] + "\\" + std::to_string(image_file_name) + imageFormat);

	image_file.open(image_file_path, std::ios::binary | std::ios::in);
	image.determineFormat(image_file);
	image.determineDimensionsAndMaxValue(image_file);
	image_file.close();

	return image;
}

bool inLastRowWithEmptySpots(const int row_number, const int images_in_last_col)
{
	return row_number == (numberOfImageRows - 1) && images_in_last_col != numberOfImageColumns;
}

void combineImagesFromOneTimestamp(const int image_file_name, std::vector<std::string>& folders_in_order)
{
	const auto images_in_last_col = (numberOfImageColumns - helper());

	auto image_def = checkDimensions(folders_in_order, image_file_name);
	std::string max_val = image_def.max_val;
	std::string image_width = image_def.width;
	std::string image_height = image_def.height;

	auto cam_counter = 0;
	std::vector<std::vector<Image>> image_matrix;

	for (auto row_number = 0; row_number < numberOfImageRows; ++row_number)
	{
		image_matrix.emplace_back();
		if (inLastRowWithEmptySpots(row_number, images_in_last_col))
		{
			for (auto pos_in_last_row = 0; pos_in_last_row < numberOfImageColumns; ++pos_in_last_row)
			{
				if (pos_in_last_row >= images_in_last_col)
				{
					Image empty_image(image_width, image_height, max_val);
					empty_image.fillWithBlack();
					image_matrix[row_number].push_back(empty_image);
				}
				else
				{
					auto recording_dir = recordingDir;
					const auto image_file_path(recording_dir /=
						folders_in_order[cam_counter] + "\\" + std::to_string(image_file_name) + imageFormat);
					cam_counter++;

					readImageAndExtractPixels(image_file_path, image_matrix[row_number]);
				}
			}
		}
		else
		{
			for (auto col_number = 0; col_number < numberOfImageColumns; ++col_number)
			{
				auto recording_dir = recordingDir;
				const auto image_file_path(recording_dir /=
					folders_in_order[cam_counter] + "\\" + std::to_string(image_file_name) + imageFormat);
				cam_counter++;

				readImageAndExtractPixels(image_file_path, image_matrix[row_number]);
			}
		}
	}
	createCombinedImage(image_matrix, image_file_name);
}

void combineImages(std::vector<std::string>& folders_in_order)
{
	for (auto image_file_name = 0; image_file_name < numberOfImagesPerCamera; ++image_file_name)
	{
		combineImagesFromOneTimestamp(image_file_name, folders_in_order);
		std::cout << image_file_name << std::endl;
	}
}

// Extracts the order the images from the cameras shall be put in from a txt file containing the names of the Image folders in that order. The txt file must be named the same as the folder containing the Image folders. One Image folder per camera.
void findFolderOrder(std::filesystem::path file_name, std::vector<std::string>& folders_in_order)
{
	std::fstream txt_file;
	std::string line;
	txt_file.open(file_name /= file_name.filename() += ".txt", std::ios::in);

	if (txt_file.is_open())
	{
		while (std::getline(txt_file, line))
		{
			folders_in_order.push_back(line);
		}
	}
	txt_file.close();
}

void writeToLog(const std::string& message)
{
	const auto current_date_time = currentDateTime();
	std::fstream log_file("error_log.txt", std::ios::out | std::ios::app);
	if (log_file.is_open())
	{
		log_file << current_date_time << ' ' << message << '\n';
	}
	log_file.close();
}

int main(const int arg, char const* argv[])
{
	// Arguments: [1]Path to folder with subfolders for each cam, [2]number of images per cam, [3]number of rows in combined Image, [4]number of columns in combined Image
	if (arg != 5)
	{
		writeToLog("Incorrect number of arguments.");
		exit(EXIT_FAILURE);
	}
	recordingDir = argv[1];
	numberOfImagesPerCamera = std::stoi(argv[2]);
	numberOfImageRows = std::stoi(argv[3]);
	numberOfImageColumns = std::stoi(argv[4]);

	recordingDir.make_preferred();

	std::vector<std::string> folders_in_order;
	findFolderOrder(recordingDir, folders_in_order);

	numberOfCameras = folders_in_order.size();
	if (numberOfCameras > (numberOfImageRows - 1) * numberOfImageColumns && numberOfCameras <= (numberOfImageRows *
		numberOfImageColumns))
		combineImages(folders_in_order);
	else
	{
		writeToLog("Incorrect number of columns or rows.");
		exit(EXIT_FAILURE);
	}
}

