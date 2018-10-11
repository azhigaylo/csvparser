#pragma once

#include <string>
#include <vector>
#include <utility>

#include <boost/optional.hpp>

namespace Parsers
{

class CCsvPrserImpl final
{
    public:
        struct router_item_t
        {
            typedef std::pair <std::uint16_t, std::string> mapp_item_t;

            bool        topic_sub;
            uint32_t    number;
            std::string mqtt_topic;
            std::vector<mapp_item_t> mapping;
        };
        typedef std::pair <std::string, router_item_t> gwt_item_t;


        explicit CCsvPrserImpl(const std::string& csv_prj_path, const std::string& table_path);
        ~CCsvPrserImpl();

        void parseCsvProject();
/*
        const std::vector<gwt_item_t>& getGwtTable()const {return m_gwt_vector; }

        static const std::string& getDigitalKey();
        static const std::string& getAnalogKey();

        uint32_t getDigitalPointAmount()const {return m_digital_max_number;}
        uint32_t getAnalogPointAmount()const {return m_analog_max_number;}
*/
    private:

        CCsvPrserImpl(const CCsvPrserImpl&) = delete;
        CCsvPrserImpl& operator=(const CCsvPrserImpl&) = delete;

        boost::optional<uint32_t> getColumByName(const std::string &header, const std::string &col_name);

        std::string m_table_path;
        std::string m_csv_prj_path;

/*
        std::vector<gwt_item_t> m_gwt_vector;
*/
};

} //namespace Parsers

