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
#include "Combiner.h"
#pragma warning(disable : 4996)


std::string currentDateTime()
{
	auto now = time(nullptr);
	struct tm tstruct{};
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%d_%m_%y_%H_%M_%S", &tstruct);

	return buf;
}

// Finds the placements of the images in the new combined image. You have to provide a txt file containing the names of the image directories for the current recording, the order of these dictates the placement in the new combined image. Use the serial numbers of the cameras, as those are used by the recording software to name the directories the images are put in. New line for each number. The txt file must be named the same as the current recording's directory and placed next to the directories with the images. 
// Example:
// 13_01_2019.txt, should contain something like this:
// 12385618 <-- Should match the name of the directory with this camera's images.
// 12395298
// 12312392

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
		log_file << '\n' << current_date_time << ' ' << message;
	}
	log_file.close();
}

int main(const int arg, char const* argv[])
{
	// Arguments:
	// [1] Path to folder with subfolders for each cam. 
	// Example: C:/Recordings/13_01_2018 <-- Folder containing folders with each the cameras' images.
	// [2] Number of images per camera.
	// [3] Number of rows in the new combined Image.
	// [4] Number of columns in the new combined Image.
	// Full example: C:/Recordings/13_01_2019 1000 3 3
	if (arg != 5)
	{
		writeToLog("Incorrect number of arguments.");
		exit(EXIT_FAILURE);
	}
	
	//recordingDir.make_preferred();

	std::vector<std::string> folders_in_order;
	findFolderOrder(argv[1], folders_in_order);

	const auto number_of_cams = folders_in_order.size();

	auto combiner = Combiner{ argv[1], folders_in_order, std::stoi(argv[4]), std::stoi(argv[3]), number_of_cams, std::stoi(argv[2]) };

	// Starts the combination process.
	combiner.initialize();
}

