/*
 * Created by Anton Zhigaylo <antoooon@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the MIT License
 */

#include "parser/CsvParserImpl.hpp"

#include <memory>
#include <fstream>
#include <stdexcept>
#include <boost/filesystem.hpp>

namespace Parsers
{

namespace GtwTbl
{
    const std::string c_gtw_table_main_cfg           = "TableConfig";
    const std::string c_gtw_table_data_time          = "table_data_time";
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
    if (true == prepareHeaderMap(m_csv_prj_path, m_header_map))
    {
        if (true == prepareGtwVector(m_csv_prj_path, m_gtw_vector))
        {
           createGtwFile(m_table_path, m_gtw_vector);
        }
    }
}

bool CCsvPrserImpl::createGtwFile(const std::string& gtw_file, const std::vector<gtw_item_tuple_t>& gtw_vector)
{
    bool result = false;
    namespace pt = boost::property_tree;

    pt::ptree gtw_ptree;
    std::ofstream ss_gtw(gtw_file);

    try
    {
        // create table header
        pt::ptree table_header;
        table_header.put(GtwTbl::c_gtw_table_data_time,  getDate_str());
        gtw_ptree.add_child(GtwTbl::c_gtw_table_main_cfg, table_header);

        // create digital items in gtw table
        pt::ptree d_items;
        for(gtw_item_tuple_t const& item: gtw_vector)
        {
            if ("DI" == std::get<0>(item) || "DO" == std::get<0>(item))
            {
                pt::basic_ptree<std::string, std::string> item_node;
                // set point number and topic
                item_node.put(GtwTbl::c_gtw_item_d_number,  std::get<1>(item));
                item_node.put(GtwTbl::c_gtw_item_topic,  std::get<2>(item));
                // set sub
                std::string sub_value = ("DI" == std::get<0>(item)) ? "false" : "true";
                item_node.put(GtwTbl::c_gtw_item_subscription,  sub_value);
                // parse values
                pt::ptree value_items;
                if (true == createValueNode(std::get<3>(item), value_items))
                {
                    item_node.add_child(GtwTbl::c_gtw_item_mapping, value_items);
                }
                else
                {
                    item_node.put(GtwTbl::c_gtw_item_mapping,  std::get<3>(item));
                }
                d_items.push_back(std::make_pair("", item_node));
            }
        }
        gtw_ptree.add_child(GtwTbl::c_gtw_table_d_routing, d_items);


        // create analog items in gtw table
        pt::ptree a_items;
        for(gtw_item_tuple_t const& item: gtw_vector)
        {
            if ("AI" == std::get<0>(item) || "AO" == std::get<0>(item))
            {
                pt::basic_ptree<std::string, std::string> item_node;
                // set point number and topic
                item_node.put(GtwTbl::c_gtw_item_a_number,  std::get<1>(item));
                item_node.put(GtwTbl::c_gtw_item_topic,  std::get<2>(item));
                // set sub
                std::string sub_value = ("AI" == std::get<0>(item)) ? "false" : "true";
                item_node.put(GtwTbl::c_gtw_item_subscription,  sub_value);
                // parse values
                item_node.put(GtwTbl::c_gtw_item_mapping,  std::get<3>(item));

                a_items.push_back(std::make_pair("", item_node));
            }
        }
        gtw_ptree.add_child(GtwTbl::c_gtw_table_a_routing, a_items);

        pt::write_json(ss_gtw, gtw_ptree);
        ss_gtw.close();

    }
    catch (const std::exception& e)
    {
        std::cerr << "CCsvParserImpl::" << __func__ << ": Error in status calculation, description: " << e.what() << std::endl;
    }

    for(gtw_item_tuple_t const& item: gtw_vector)
    {
       std::cout << std::setw(5) << std::left << ""
                  << std::get<0>(item) << " : " << std::get<1>(item) << " : "
                  << std::get<2>(item) << " : " << std::get<3>(item) << std::endl;
    }
    return result;
}

bool CCsvPrserImpl::createValueNode(const std::string& value_str, boost::property_tree::ptree& value_node)
{
    bool result = false;
    uint32_t value_counter = 0;
    std::stringstream ss_values(value_str);

    if (false == value_str.empty())
    {
        while (ss_values.good())
        {
            std::string value;
            std::getline (ss_values, value, '/');

            typedef boost::property_tree::basic_ptree<std::string, std::string> item_value_t;
            item_value_t item_value;
            item_value.put(GtwTbl::c_gtw_item_mapping_value_int, std::to_string(value_counter));
            item_value.put(GtwTbl::c_gtw_item_mapping_value_str, value);
            value_node.push_back(std::make_pair("", item_value));
            value_counter++;
        }
        result = true;
    }

    return result;
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

std::string CCsvPrserImpl::getDate_str()
{
    // ascii date string
    boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
    return std::string(to_simple_string(t1));
}

} //namespase Parsers
