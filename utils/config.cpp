#include "config.hpp"
using namespace config;

Configuration::Configuration(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        ERROR_LOG("Error opening config file: " << filename);
    }

    rapidjson::IStreamWrapper isw(file);
    m_document.ParseStream(isw);
}