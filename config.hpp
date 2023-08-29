#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <fstream>
#include <map>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace config
{

    class Configuration
    {
    public:
        Configuration(const std::string &filename)
        {
            std::ifstream file(filename);
            if (!file.is_open())
            {
                std::cerr << "Error opening config file: " << filename << std::endl;
            }

            rapidjson::IStreamWrapper isw(file);
            m_document.ParseStream(isw);
        }

        std::map<std::string, std::string>
        getMainConfig() const
        {
            std::map<std::string, std::string> config;
            for (const auto &member : m_document.GetObject())
            {
                if (member.value.IsString())
                {
                    config[member.name.GetString()] = member.value.GetString();
                }
            }

            return config;
        }

        std::vector<std::pair<std::string, int>>
        getPeers() const
        {
            std::vector<std::pair<std::string, int>> peers;
            auto it = m_document.FindMember("Peers");
            if (it != m_document.MemberEnd() && it->value.IsArray())
            {
                for (const auto &element : it->value.GetArray())
                {
                    std::pair<std::string, int> host;
                    auto it = element.FindMember("addr");
                    if (it != m_document.MemberEnd() && it->value.IsString()) {
                       host.first = it->value.GetString(); 
                    } else {
                        throw std::invalid_argument("addr must be a string");
                    }
                    it = element.FindMember("port");
                    if (it != m_document.MemberEnd() && it->value.IsInt()) {
                       host.second = it->value.GetInt(); 
                    } else {
                        throw std::invalid_argument("port must be a int");
                    }
                    peers.emplace_back(host);
                }
            }
            return peers;
        }

    private:
        rapidjson::Document m_document;
    };

} // namespace config

#endif // __CONFIG_HPP__