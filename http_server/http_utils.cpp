#include "http_utils.h"
using namespace http_server;

status_code_t::status_code_t()
{
	this->operator[]("200") = OK;
	this->operator[]("400") = BAD_REQUEST;
	this->operator[]("404") = NOT_FOUND;
	this->operator[]("405") = METHOD_NOT_ALLOWED;
	this->operator[]("500") = INTERNAL_SERVER_ERROR;
	this->operator[]("501") = NOT_IMPLEMENTED;
}

status_code_t::~status_code_t()
{}

status_code_string_t::status_code_string_t()
{
	this->operator[](OK) = "OK";
	this->operator[](BAD_REQUEST) = "Bad request_t";
	this->operator[](NOT_FOUND) = "Not Found";
	this->operator[](METHOD_NOT_ALLOWED) = "405";
	this->operator[](INTERNAL_SERVER_ERROR) = "500";
	this->operator[](NOT_IMPLEMENTED) = "501";
}

status_code_string_t::~status_code_string_t()
{}