#include "../include/parser/CsvParserImpl.hpp"

#include <memory>
#include <fstream>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace Parsers
{

namespace GtwTbl
{
    const std::string c_gtw_table_main_cfg           = "TableConfig";
    const std::string c_gtw_table_d_routing          = "DPointRouting";
    const std::string c_gtw_table_a_routing          = "APointRouting";
    const std::string c_gtw_item_d_number            = "dpoint_number";
    const std::string c_gtw_item_topic               = "mqtt_topic";
    const std::string c_gtw_item_subscription        = "mqtt_subscribe";
    const std::string c_gtw_item_mapping             = "value_mapping";
    const std::string c_gtw_item_mapping_value_int   = "value";
    const std::string c_gtw_item_mapping_value_str   = "mapp_to";
    const std::string c_gtw_item_a_number            = "apoint_number";
}

namespace CsvTbl
{
    const std::string c_csv_dev_number               = "dev num";
    const std::string c_csv_dev_type                 = "dev type";
    const std::string c_csv_dev_interfcae            = "port / address";
    const std::string c_csv_dev_channels             = "channels";
    const std::string c_csv_dev_channels_description = "channels description";
    const std::string c_csv_plc_point_mapping        = "plc point";
    const std::string c_csv_plc_point_operation      = "operation";
    const std::string c_csv_plc_point_description    = "description";
    const std::string c_csv_plc_point_mqtt_mapping   = "mqtt topic";
    const std::string c_csv_plc_point_mqtt_values    = "mqtt values";
}

CCsvPrserImpl::CCsvPrserImpl(const std::string& csv_prj_path, const std::string& table_path)
    : m_table_path  (table_path)
    , m_csv_prj_path(csv_prj_path)
{
    if (!boost::filesystem::exists(csv_prj_path))
    {
        throw std::runtime_error(std::string("csv file is not exist: ") + m_csv_prj_path );
    }
}

CCsvPrserImpl::~CCsvPrserImpl()
{
    std::cout << "CCsvParserImpl::" << __func__ << ": removed." << std::endl;
}

void CCsvPrserImpl::parseCsvProject()
{
    std::cout << "CCsvParserImpl::" << __func__ << ": parsing result" << std::endl;

    if (true == prepareHeaderMap(m_csv_prj_path, m_header_map))
    {
        if (true == prepareGtwVector(m_csv_prj_path, m_gtw_vector))
        {
            for(gtw_item_tuple_t const& item: m_gtw_vector)
            {
                std::cout << std::setw(5) << std::left << ""
                          << std::get<0>(item) << " : " << std::get<1>(item) << " : "
                          << std::get<2>(item) << " : " << std::get<3>(item) << std::endl;
            }
        }
    }
}

bool CCsvPrserImpl::prepareHeaderMap(const std::string &csv_file, std::map <std::string, uint32_t>& header_map)
{
    bool result = false;
    std::ifstream fs_csv_file(csv_file, std::ifstream::in);

    if (true == fs_csv_file.is_open())
    {
        // read first line from file
        std::string header_str;
        std::getline(fs_csv_file, header_str);

        // parse it
        std::stringstream ss_header(header_str);

        uint32_t col_counter = 0;
        while (ss_header.good())
        {
            std::string col_value;
            std::getline (ss_header, col_value, ',');
            header_map.emplace(col_value, col_counter);

            std::cout << "CCsvParserImpl::" << __func__ << ": pos = " << col_counter << ", col_value = '" << col_value << "'" << std::endl;

            col_counter++;
        }

        fs_csv_file.close();
        result = true;
    }
    return result;
}

bool CCsvPrserImpl::prepareGtwVector(const std::string &csv_file, std::vector<gtw_item_tuple_t>& gtw_vector)
{
    bool result = false;
    std::ifstream fs_csv_file(csv_file, std::ifstream::in);

    if (true == fs_csv_file.is_open())
    {
        // skip header string
        std::string data_str;
        std::getline(fs_csv_file, data_str);

        while (fs_csv_file.good())
        {
            std::getline(fs_csv_file, data_str);

            boost::optional<std::string> plc_point_value   = getColumValueByNum(m_header_map.find(CsvTbl::c_csv_plc_point_mapping)->second, data_str);
            boost::optional<std::string> operation_value   = getColumValueByNum(m_header_map.find(CsvTbl::c_csv_plc_point_operation)->second, data_str);
            boost::optional<std::string> mqtt_topic_value  = getColumValueByNum(m_header_map.find(CsvTbl::c_csv_plc_point_mqtt_mapping)->second, data_str);
            boost::optional<std::string> mqtt_values_value = getColumValueByNum(m_header_map.find(CsvTbl::c_csv_plc_point_mqtt_values)->second, data_str);

            //std::cout << "CCsvParserImpl::" << __func__ << ": " << plc_point_value.get() << " | "
            //                                                    << operation_value.get() << " | "
            //                                                    << mqtt_topic_value.get() << " | "
            //                                                    << mqtt_values_value.get() << std::endl;

            if (plc_point_value && operation_value && mqtt_topic_value && mqtt_values_value)
            {
                gtw_item_tuple_t gtw_item_tuple = std::make_tuple(operation_value.get(),  plc_point_value.get(),
                                                                  mqtt_topic_value.get(), mqtt_values_value.get());
                gtw_vector.push_back(gtw_item_tuple);
            }
            else
            {
                throw std::runtime_error(std::string("csv file parsing error!!!"));
            }
        }
        result = true;
    }
    return result;
}

boost::optional<std::string> CCsvPrserImpl::getColumValueByNum(uint32_t colum_num, const std::string &data_str)
{
    uint32_t col_counter = 0;
    std::stringstream ss_header(data_str);

    while (ss_header.good())
    {
        std::string col_value;
        std::getline (ss_header, col_value, ',');

        if (colum_num == col_counter++)
        {
            //std::cout << "CCsvParserImpl::" << __func__ << ": colum_num = "   << colum_num
            //                                            << " | col_value = '" <<  col_value << "'" << std::endl;
            return col_value;
        }
    }
    return boost::optional<std::string>{};
}

boost::optional<uint32_t> CCsvPrserImpl::getColumByName(const std::string &header, const std::string &col_name)
{
    std::stringstream ss_header(header);

    uint32_t col_counter = 0;
    while (ss_header.good())
    {
        std::string col_value;
        std::getline (ss_header, col_value, ',');

        std::cout << "CCsvParserImpl::" << __func__ << ": col_value = '" << col_value << "'" << std::endl;

        if (col_value == col_name)
        {
            return col_counter;
        }
        col_counter++;
    }
    return boost::optional<uint32_t>{};
}

/*
CGtwTableParser::CGtwTableParser(const std::string& table_path)
    : m_digital_max_number (0)
    , m_analog_max_number  (0)
{
    if (!boost::filesystem::exists(table_path))
    {
        throw std::runtime_error(std::string("configuration unavailable: file is not exist: ") + table_path );
    }
    else
    {
        try
        {
            namespace pt = boost::property_tree;
            pt::ptree table_ptree;
            pt::read_json(table_path, table_ptree);

            // gtw options
            // pt::ptree &main_cfg_node = table_ptree.get_child(Tbl::c_gtw_table_main_cfg);

            // create discret point rouring table
            for (pt::ptree::value_type &d_gtw_item : table_ptree.get_child(Tbl::c_gtw_table_d_routing))
            {
                router_item_t router_item;
                router_item.number = static_cast<uint32_t>(d_gtw_item.second.get<unsigned>(Tbl::c_gtw_item_d_number));
                router_item.mqtt_topic = d_gtw_item.second.get<std::string>(Tbl::c_gtw_item_topic);
                router_item.topic_sub = d_gtw_item.second.get<bool>(Tbl::c_gtw_item_subscription);

                //printDebug("CGtwTableParser/%s: d_num = %i / s = %i / topic = %s", __FUNCTION__, router_item.number,
                //                                                                               router_item.topic_sub,
                //                                                                               router_item.mqtt_topic.c_str());
                // create discret point rouring table
                for (pt::ptree::value_type &d_item_mapping : d_gtw_item.second.get_child(Tbl::c_gtw_item_mapping))
                {
                   uint16_t value_int = static_cast<uint32_t>(d_item_mapping.second.get<unsigned>(Tbl::c_gtw_item_mapping_value_int));
                   std::string value_str = d_item_mapping.second.get<std::string>(Tbl::c_gtw_item_mapping_value_str);

                   router_item.mapping.push_back(std::make_pair(value_int, value_str));

                   //printDebug("CGtwTableParser/%s: int = %i <-> str = %s", __FUNCTION__, value_int, value_str.c_str());
                }
                digitalCheckForMax(router_item.number);
                m_gwt_vector.push_back(std::make_pair(Tbl::c_gtw_table_d_routing, router_item));
            }

            // create analog point rouring table
            for (pt::ptree::value_type &a_gtw_item : table_ptree.get_child(Tbl::c_gtw_table_a_routing))
            {
                router_item_t router_item;

                router_item.number = static_cast<uint32_t>(a_gtw_item.second.get<unsigned>(Tbl::c_gtw_item_a_number));
                router_item.mqtt_topic = a_gtw_item.second.get<std::string>(Tbl::c_gtw_item_topic);
                router_item.topic_sub  = a_gtw_item.second.get<bool>(Tbl::c_gtw_item_subscription);

                //printDebug("CGtwTableParser/%s: a_num = %i / s = %i / topic = %s", __FUNCTION__, router_item.number,
                //                                                                               router_item.topic_sub,
                //                                                                               router_item.mqtt_topic.c_str());
                analogCheckForMax(router_item.number);
                m_gwt_vector.push_back(std::make_pair(Tbl::c_gtw_table_a_routing, router_item));
            }
        }
        catch (const std::exception& e)
        {
            //printError("CGtwTableParser/%s: Error parsing config: %s", __FUNCTION__, table_path.c_str());
            //printError("CGtwTableParser/%s: Error description: %s", __FUNCTION__, e.what());
            throw;
        }
    }
}

CCsvPrserImpl::~CCsvPrserImpl()
{
    std::cout << __func__ << ": CCsvParserImpl removed." << std::endl;
}
*/
} //namespase Parsers
