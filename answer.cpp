#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <string>
#include <fstream>
#include <map>
#include <utility>

// Object Definitions
struct HrmlLine {
    std::string tag_name{""};
    std::vector<std::pair<std::string, std::string>> attributes{};
    bool closing_tag{false};
};

// Declarations
HrmlLine parse_input_line(std::string);
std::array<int, 2> GetLinesAndQueries(std::ifstream&);
void IngestHrmlandQueries(std::ifstream&, std::vector<std::string>&, std::vector<HrmlLine>&,std::vector<std::string>&, std::array<int,2>);
void ProcessHrml(std::vector<HrmlLine>&, std::vector<std::string>&, std::map<std::string, std::string>&);
std::string GenKey(std::vector<std::string>, std::string) ;
void SearchMap(std::map<std::string, std::string> &, std::vector<std::string>&);

int main() {
    std::ifstream input_file("test_input2");        // input file with HRML
    std::array<int, 2> lines_and_queries{0,0};      // array to hold number of HRML lines and Query Lines
    std::vector<std::string> input_code{};          // vector for holding each line of HRML to then process
    std::vector<HrmlLine> hrml_code{};              // each line of HRML gets parsed into an object
    std::vector<std::string> key_vec{};             // used for creating key strings for map to query later
    std::map<std::string, std::string> query_map{}; // map holding possible query keys, and the associated attribute value
    std::vector<std::string> queries{};             // vector holding input queries from input file

    lines_and_queries = GetLinesAndQueries(input_file);

    IngestHrmlandQueries(input_file,input_code, hrml_code,queries, lines_and_queries);

    ProcessHrml(hrml_code, key_vec, query_map);

    SearchMap(query_map, queries);
    
    input_file.close();

    return 0;
}

// Function Implementations
HrmlLine parse_input_line(std::string singl_line) {
    HrmlLine new_hrml{};
    size_t line_len = singl_line.length();

    //check for closing tag case, and return new hrml object
    if (singl_line[1] == '/') {
        new_hrml.closing_tag = true;
        size_t num_tag_chars = singl_line.length()-3;
        new_hrml.tag_name = singl_line.substr(2, num_tag_chars);
        return new_hrml;
    }

    //it's not a closing tag, parse for starting tags, and possibly attributes/values
    std::string delim{" "};
    std::string new_attr_name{};
    std::string new_attr_val{};
    size_t delim_pos1 = singl_line.find(delim);
    size_t delim_pos2{};
    

    if (delim_pos1 == std::string::npos) { // just a tag, no attributes
        new_hrml.tag_name = singl_line.substr(1, (singl_line.length() -1 ) ) ;
        return new_hrml;
    }
    else { //singl_line contains a tag, and at least one attribute/value pair
        new_hrml.tag_name = singl_line.substr(1,(delim_pos1-1));
        delim_pos2 = singl_line.find(delim, (delim_pos1+1));
        while (delim_pos2 != std::string::npos) { // continue processing attr/value pairs
            size_t num_attr_name_chars = delim_pos2 - delim_pos1 - 1;
            new_attr_name = singl_line.substr((delim_pos1+1), num_attr_name_chars);
            delim_pos1 = delim_pos2;
            delim_pos2 = singl_line.find(delim, (delim_pos1+1));
            delim_pos1 = delim_pos2;
            delim_pos2 = singl_line.find(delim, (delim_pos1+1));
            
            if (delim_pos2 == std::string::npos) { // then last attr/value pair in line
                size_t num_attr_val_chars = singl_line.length() - delim_pos1 - 4;
                new_attr_val = singl_line.substr((delim_pos1+2), num_attr_val_chars);
            }
            else {
                size_t num_attr_val_chars = delim_pos2 - delim_pos1 - 3;
                new_attr_val = singl_line.substr((delim_pos1+2), num_attr_val_chars);
                delim_pos1 = delim_pos2;
                delim_pos2 = singl_line.find(delim, (delim_pos1+1));
            }
            
            
            new_hrml.attributes.push_back(std::make_pair(new_attr_name, new_attr_val));
        }

    }


    return new_hrml;
}

std::array<int, 2> GetLinesAndQueries(std::ifstream &handle) {
    std::string current_line{};
    std::getline(handle, current_line);
    std::string delimiter{" "};
    int delim_pos = current_line.find(delimiter);
    std::string str_n = current_line.substr(0, delim_pos);
    std::string str_q = current_line.substr((delim_pos+1));
    int num_lines = std::stoi(str_n);
    int num_queries = std::stoi(str_q);
    
    std::array<int, 2> result {num_lines, num_queries};
    return result;
}

/*  ProcessHrml accepts the input file handle, a reference the the
*   vector input_code, a reference to the vector hrml_code, a ref to the 
*   vector queries, and the array with number of hrml lines and queries.
*  
*   After calling, input_code will contain one entry
*   per line of the input file, hrml_code will contain one object 
*   HrmlLine per entry, and queries will contain one entry per query. 
*/
void IngestHrmlandQueries(   std::ifstream &handle, 
                    std::vector<std::string> &input_lines, 
                    std::vector<HrmlLine> &hrml,
                    std::vector<std::string> &query_lines, 
                    std::array<int,2> ls_and_qs
                    ) { 
    std::string current_line{};

    for (int i = 0; i < ls_and_qs[0]; i++) {
        std::getline(handle, current_line);
        input_lines.push_back(current_line);
    }

    for (int i = 0; i < ls_and_qs[1]; i++) {
        std::getline(handle, current_line);
        query_lines.push_back(current_line);
    }

    for (const auto& elem : input_lines) {
        hrml.push_back(parse_input_line(elem));
    }

}

void ProcessHrml(   std::vector<HrmlLine> &r_hrml, 
                    std::vector<std::string> &h_key_vec, 
                    std::map<std::string, std::string> &h_query_map) 
                    {
    std::string curr_key{};
    for (const auto& elem : r_hrml) {
        if (elem.closing_tag == true && !h_key_vec.empty()) {
            h_key_vec.pop_back();
        }
        else {
            h_key_vec.push_back(elem.tag_name);
            for (const auto& att_elem : elem.attributes) {
                curr_key = GenKey(h_key_vec, att_elem.first );
                h_query_map.insert(std::make_pair(curr_key, att_elem.second));

            }
        }
    }

}

std::string GenKey(std::vector<std::string> keyvec, std::string curr_attr) {
    std::string ret_string{};
    for (const auto& elem: keyvec) {
        ret_string += elem;
        ret_string += '.';
    }
    ret_string[ret_string.length()-1] = '~';
    ret_string += curr_attr;
    return ret_string;
}

void SearchMap(std::map<std::string, std::string> &r_hrml_map, std::vector<std::string>&r_queries) {
    std::map<std::string, std::string>::iterator it{};
    for (const auto& single_query:r_queries) {
        it = r_hrml_map.find(single_query); 
        if (it == r_hrml_map.end()) {
            std::cout << "Not Found!" << std::endl;
        }
        else {
            std::cout << it->second << std::endl;
        }
    }
}