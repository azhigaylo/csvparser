#pragma once

#include <map>
#include <tuple>
#include <string>
#include <vector>
#include <utility>
#include <iostream>

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
        typedef std::pair <std::string, uint32_t> header_item_t;
        typedef std::tuple<std::string, std::string, std::string, std::string> gtw_item_tuple_t;

        CCsvPrserImpl(const CCsvPrserImpl&) = delete;
        CCsvPrserImpl& operator=(const CCsvPrserImpl&) = delete;

        bool prepareHeaderMap(const std::string &csv_file, std::map <std::string, uint32_t>& header_map);
        bool prepareGtwVector(const std::string &csv_file, std::vector<gtw_item_tuple_t>& gtw_vector);

        boost::optional<uint32_t> getColumByName(const std::string &header, const std::string &col_name);
        boost::optional<std::string> getColumValueByNum(uint32_t colum_num, const std::string &data_str);

        std::string m_table_path;
        std::string m_csv_prj_path;
        std::map <std::string, uint32_t> m_header_map;
        std::vector<gtw_item_tuple_t>    m_gtw_vector;

/*
        std::vector<gwt_item_t> m_gwt_vector;
*/
};

} //namespace Parsers

