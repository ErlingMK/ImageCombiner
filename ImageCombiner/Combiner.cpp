#include "pch.h"
#include "Combiner.h"

Combiner::Combiner(const std::string path, std::vector<std::string> camera_order, const int number_of_columns, const int number_of_rows, const int number_of_cams, const int images_per_cam):
	path(path), camera_order(std::move(camera_order)), number_of_columns_(number_of_columns),
	number_of_rows_(number_of_rows), number_of_cams_(number_of_cams), images_per_cam_(images_per_cam)
{
}

void Combiner::initialize()
{
	image_prototype_ = checkDimensions();

	combined_folder_path_ = path;
	combined_folder_path_ /= "combined";
	create_directory(combined_folder_path_);

	combineImages();
}

void Combiner::combineImages()
{
	for (auto image_file_name = 0; image_file_name < images_per_cam_; ++image_file_name)
	{
		combineImagesFromOneTimestamp(image_file_name);
	}
}

void Combiner::combineImagesFromOneTimestamp(const int image_file_name)
{
	const auto images_in_last_col = (number_of_columns_ - helper());

	auto cam_counter = 0;

	std::vector<std::vector<Image>> image_matrix;

	for (auto row_number = 0; row_number < number_of_rows_; ++row_number)
	{
		image_matrix.emplace_back();

		if (inLastRowWithEmptySpots(row_number, images_in_last_col))
		{
			for (auto pos_in_last_row = 0; pos_in_last_row < number_of_columns_; ++pos_in_last_row)
			{
				if (pos_in_last_row >= images_in_last_col)
				{
					Image empty_image(image_prototype_.width, image_prototype_.height, image_prototype_.max_val);
					empty_image.fillWithBlack();
					image_matrix[row_number].push_back(empty_image);
				}
				else
				{
					auto recording_dir = path;
					const auto image_file_path(recording_dir /=
						camera_order[cam_counter] + "\\" + std::to_string(image_file_name) + "." + image_format_);

					cam_counter++;

					readImageAndExtractPixels(image_file_path, image_matrix[row_number]);
				}
			}
		}
		else
		{
			for (auto col_number = 0; col_number < number_of_columns_; ++col_number)
			{
				auto recording_dir = path;
				const auto image_file_path(recording_dir /=
					camera_order[cam_counter] + "\\" + std::to_string(image_file_name) + image_format_);

				cam_counter++;

				readImageAndExtractPixels(image_file_path, image_matrix[row_number]);
			}
		}
	}
	createCombinedImage(image_matrix, image_file_name);
}


void Combiner::createCombinedImage(std::vector<std::vector<Image>>& image_matrix, const int image_file_name) const
{
	std::fstream new_image_file;
	const auto s = image_prototype_.m_data_points_per_pixel * image_prototype_.width;
	auto new_file_path = combined_folder_path_;

	new_image_file.open(new_file_path /= (std::to_string(image_file_name) + "." + image_prototype_.image_format), std::ios::binary | std::ios::out);
	new_image_file << image_prototype_.magic_number << "\n" << image_prototype_.width * number_of_columns_ << " " << image_prototype_.height * number_of_rows_ << "\n" << image_prototype_.max_val
		<< "\n";

	for (auto& k : image_matrix)
	{
		for (auto i = 0; i < image_prototype_.height; ++i)
		{
			for (auto& j : k)
			{
				auto& pixel_rows = j.pixel_rows;
				new_image_file.write(pixel_rows[i].get(), s);
			}
		}
	}
	new_image_file.close();
	image_matrix.clear();
}
Image Combiner::checkDimensions() const
{
	std::fstream image_file;
	Image image;

	auto recording_dir = path;
	const auto image_file_path(recording_dir /=
		camera_order[0] + "\\0." + image_format_);

	image_file.open(image_file_path, std::ios::binary | std::ios::in);

	image.determineFormat(image_file);
	image.determineDimensionsAndMaxValue(image_file);

	image_file.close();

	return image;
}

void Combiner::readImageAndExtractPixels(const std::filesystem::path& image_file_path, std::vector<Image>& image_row)
{
	std::fstream image_file;
	Image image;

	image_file.open(image_file_path, std::ios::binary | std::ios::in);
	image.extractImageRows(image_file);
	image_file.close();

	image_row.push_back(image);
}

int Combiner::helper() const
{
	return (number_of_columns_ * number_of_rows_) - number_of_cams_;

}

bool Combiner::inLastRowWithEmptySpots(const int row_number, const int images_in_last_col) const
{
	return row_number == (number_of_rows_ - 1) && images_in_last_col != number_of_columns_;
}
