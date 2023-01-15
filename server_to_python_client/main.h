#pragma once

#include <filesystem>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <regex>
#include <memory>
#include <fstream>


#include <iostream>

class python_creator
{
private:

	enum request_type : int
	{
		Get,
		Post,
	};

	struct mapped_endpoint
	{
		std::string python_name;
		std::string endpoint;
		request_type rt;

		bool valid = false;
		bool is_host_config = false;
	};

	/// <summary>
	/// Base configuration to create calls
	/// </summary>
	mapped_endpoint host_configuration;

	/// <summary>
	/// End points to call
	/// </summary>
	std::vector<mapped_endpoint> end_points;

	/// <summary>
	/// current working directory
	/// </summary>
	std::string current_directory;

	/// <summary>
	/// file to parse
	/// </summary>
	std::vector<std::string> files_to_analyze;

	std::ofstream output;
public:

	python_creator()
	{
		current_directory = std::filesystem::current_path().string();
		std::cout << "Working directory " << current_directory << std::endl;

		auto w_path = std::filesystem::path(current_directory);
		auto extension_to_analyze = ".cpp";
		for (auto& file : std::filesystem::recursive_directory_iterator(w_path))
		{
			if (file.path().extension() == extension_to_analyze)
				files_to_analyze.push_back(file.path().string());
		}
	}

	python_creator(const char* path)
	{
		current_directory = path;

		std::cout << "Working directory " << current_directory << std::endl;

		auto w_path = std::filesystem::path(current_directory);
		auto extension_to_analyze = ".cpp";
		for (auto& file : std::filesystem::recursive_directory_iterator(w_path))
		{
			if (file.path().extension() == extension_to_analyze)
				files_to_analyze.push_back(file.path().string());
		}

		std::cout << "Files to parse " << files_to_analyze.size() << std::endl;
	}

	void parse_files()
	{
		if (files_to_analyze.size() == 0) return;

		for (auto& file : files_to_analyze)
		{
			std::ifstream input_file(file);
			if (input_file.is_open())
			{
				for (std::string line; std::getline(input_file, line); )
				{
					auto mapped_endpoint = get_mapped_endpoint(line);
					if (!mapped_endpoint.valid) continue;
					if (mapped_endpoint.is_host_config)
						host_configuration = mapped_endpoint;
					else
						end_points.push_back(mapped_endpoint);
				}

				input_file.close();
			}
		}

	}

	void build_python_client()
	{
		if (files_to_analyze.size() == 0 || end_points.size() == 0 || !host_configuration.valid) return;

		std::cout << "Endpoints found " << end_points.size() << std::endl;

		auto file_path = current_directory + "server_client.py";
		output.open(file_path);

		if (!output.is_open()) return;

		add_line("import requests");
		add_line("");
		add_line("class server_client(object):");
		add_line("\tdef __init__(self):");
		add_line("\t\tpass");
		add_line("");

		for (auto& mapped : end_points)
		{
			std::stringstream ss;
			if (mapped.rt == request_type::Get)
				ss << "\tdef " << mapped.python_name << "(self):" << std::endl;
			else if (mapped.rt == request_type::Post)
				ss << "\tdef " << mapped.python_name << "(self, data):" << std::endl;

			ss << "\t\turl = \"http://" << host_configuration.python_name << ":" << host_configuration.endpoint << mapped.endpoint << "\"" << std::endl;

			if (mapped.rt == request_type::Get)
				ss << "\t\tprint (requests.get(url).content)" << std::endl;
			else if (mapped.rt == request_type::Post)
				ss << "\t\tprint (requests.post(url, json = data).content)" << std::endl;

			auto final_python_function = ss.str();
			add_text(final_python_function);
		}

		std::cout << "Saving file server_client.py to " << file_path << std::endl;
		output.close();
		
	}

	void install_python_module()
	{
		std::cout << "Running install commmand... " << std::endl;
		system("python server_client.py build install");
	}

	mapped_endpoint get_mapped_endpoint(std::string line)
	{
		//ignore empty line
		if (line.size() == 0) return mapped_endpoint{};
		//ignore macro definitions
		if (line.find("#define") != std::string::npos) return mapped_endpoint{};

		//get HOST_CONFIG
		auto python_host = line.find("HOST_CONFIG");
		//get method mapped
		auto python_get = line.find("TO_PYTHON_GET");
		//post mehotd mapped
		auto python_post = line.find("TO_PYTHON_POST");

		//well
		if (python_get == std::string::npos && python_post == std::string::npos && python_host == std::string::npos)
			return mapped_endpoint{};

		//actual parser

		//clear '\t'
		line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());

		//clear '\'
		line.erase(std::remove(line.begin(), line.end(), '\\'), line.end());

		auto function_name = std::string{ "" };
		auto end_point = std::string{ "" };

		//maybe improve?
		auto found_parentheses = false;
		auto function_name_done = false;
		for (std::string::size_type i = 0; i < line.size(); ++i)
		{
			if (line[i] == ',')
			{
				function_name_done = true;
			}
			if (found_parentheses)
			{
				if (line[i] == '\\'
					|| line[i] == '"'
					|| line[i] == ','
					|| line[i] == ';'
					|| line[i] == ' '
					|| line[i] == '('
					|| line[i] == ')') continue;

				if (!function_name_done)
					function_name.push_back(line[i]);
				else
					end_point.push_back(line[i]);
			}
			if (line[i] == '(')
				found_parentheses = true;

		}

		if (function_name.size() > 0 && end_point.size() > 0)
		{
			mapped_endpoint me;
			me.valid = true;
			me.python_name = function_name;
			me.endpoint = end_point;
			me.rt = python_get != std::string::npos ? request_type::Get : request_type::Post;
			me.is_host_config = python_host != std::string::npos;
			return me;
		}

		return mapped_endpoint{};
	}

	void add_line(std::string line)
	{
		output << line << std::endl;
	}

	void add_text(std::string text)
	{
		output << text;
	}

	~python_creator()
	{
		files_to_analyze.clear();
	}
};