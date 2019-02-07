#include <chrono>
#include <iostream>
#include <fstream>

// yaml-cpp has to be installed on system
// (installed in Dockerfile)
#include <yaml-cpp/yaml.h>

#include "cpptoml.h"
#include "csv.h"
#include "json.hpp"
#include "tinyxml2.h"

class Person {
    public:
    uint32_t id_;
    std::string name_;
    std::string address_;
    double age_;

    Person(uint32_t id, std::string name,
            std::string address, double age) {
        id_ = id;
        name_ = name;
        address_ = address;
        age_ = age;
    }
};

//=============================================================================
// CSV
//=============================================================================

std::vector<Person> csv_read(std::string input_file) {
    std::vector<Person> people;
    io::CSVReader<4> in(input_file);
    in.read_header(io::ignore_missing_column, "id", "name", "address", "age");
    std::string id, name, address, age;
    while(in.read_row(id, name, address, age)) {
        Person person(std::stoi(id), name, address, std::stod(age));
        people.push_back(person);
    }
    return people;
}

void csv_write(std::vector<Person> people, std::string file_path) {
    std::ofstream file;
    file.open(file_path);
    for(auto person: people) {
        file << person.id_ << "," << person.name_ << "," <<
            person.address_ << "," << person.age_ << std::endl;
    }
    file.close();
}

//=============================================================================
// JSON
//=============================================================================

std::vector<Person> json_read(std::string input_file) {
    std::ifstream i(input_file);
    nlohmann::json json_data;
    i >> json_data;

    std::vector<Person> json_people;
    for(auto json_person: json_data) {
        Person person(json_person["id"],
                json_person["name"],
                json_person["address"],
                json_person["age"]);
        json_people.push_back(person);
    }
    return json_people;
}

void json_write(std::vector<Person> people, std::string file_path) {
    auto json_data = nlohmann::json::array();
    for(auto person: people) {
        auto json_person = nlohmann::json::object();
        json_person["id"] = person.id_;
        json_person["name"] = person.name_;
        json_person["address"] = person.address_;
        json_person["age"] = person.age_;
        json_data.push_back(json_person);
    }

    std::ofstream file(file_path);
    file << json_data << std::endl;
    file.close();
}

//=============================================================================
// TOML
//=============================================================================

std::vector<Person> toml_read(std::string input_file) {
    std::vector<Person> people;
    auto toml_data = cpptoml::parse_file(input_file);
    auto toml_people = toml_data->get_table_array("data");
    for(const auto& toml_person : *toml_people) {
        auto id = toml_person->get_as<int>("id");
        Person person(
            *(toml_person->get_as<uint32_t>("id")),
            *(toml_person->get_as<std::string>("name")),
            *(toml_person->get_as<std::string>("address")),
            *(toml_person->get_as<double>("age")));
        people.push_back(person);
    }
    return people;
}

void toml_write(std::vector<Person> people, std::string file_path) {

    auto root = cpptoml::make_table();
    auto toml_people = cpptoml::make_table_array();
    for(auto person : people) {
        auto toml_person = cpptoml::make_table();
        toml_person->insert("id", person.id_);
        toml_person->insert("name", person.name_);
        toml_person->insert("address", person.address_);
        toml_person->insert("age", person.age_);
        toml_people->push_back(toml_person);
    }

    root->insert("people", toml_people);

    std::ofstream file(file_path);
    file << *root;
    file.close();
}

//=============================================================================
// XML
//=============================================================================

std::vector<Person> xml_read(std::string input_file) {
    std::vector<Person> people;

    tinyxml2::XMLDocument xml_doc;
    xml_doc.LoadFile(input_file.c_str());
    auto xml_people = xml_doc.FirstChildElement("people");

    for(auto xml_person = xml_people->FirstChildElement("person"); 
        xml_person != nullptr; xml_person = xml_person->NextSiblingElement()) {
            Person person(
                std::stoi(xml_person->FirstChildElement("id")->GetText()),
                xml_person->FirstChildElement("name")->GetText(),
                xml_person->FirstChildElement("address")->GetText(),
                std::stof(xml_person->FirstChildElement("age")->GetText()));
            people.push_back(person);
    }

    return people;
}

void xml_write(std::vector<Person> people, std::string file_path) {
    tinyxml2::XMLDocument xml_doc;
    auto xml_people = xml_doc.InsertEndChild(xml_doc.NewElement("people"));
    for(auto person : people) {
        auto xml_person = xml_doc.NewElement("person");

        auto xml_id = xml_doc.NewElement("id");
        xml_id->SetText(std::to_string(person.id_).c_str());
        xml_person->InsertEndChild(xml_id);
        
        auto xml_name = xml_doc.NewElement("name");
        xml_name->SetText(person.name_.c_str());
        xml_person->InsertEndChild(xml_name);
        
        auto xml_address = xml_doc.NewElement("address");
        xml_address->SetText(person.address_.c_str());
        xml_person->InsertEndChild(xml_address);
        
        auto xml_age = xml_doc.NewElement("age");
        xml_age->SetText(std::to_string(person.age_).c_str());
        xml_person->InsertEndChild(xml_age);

        xml_people->InsertEndChild(xml_person);
    }

    xml_doc.SaveFile(file_path.c_str());
}

//=============================================================================
// YAML
//=============================================================================

std::vector<Person> yaml_read(std::string input_file) {
    std::vector<Person> people;
    std::cout << "yaml" << std::endl;

    YAML::Node yaml_people = YAML::LoadFile(input_file);
    for (std::size_t i = 0; i<yaml_people.size(); i++) {
        auto yaml_person = yaml_people[i];
        Person person(
            yaml_person["id"].as<int>(),
            yaml_person["name"].as<std::string>(),
            yaml_person["address"].as<std::string>(),
            yaml_person["age"].as<double>()
        );
        people.push_back(person);
    }
    return people;
}

void yaml_write(std::vector<Person> people, std::string file_path) {
    YAML::Node yaml_people;
    for(auto person : people) {
        YAML::Node yaml_person;
        yaml_person["id"] = person.id_;
        yaml_person["name"] = person.name_;
        yaml_person["address"] = person.address_;
        yaml_person["age"] = person.age_;
        yaml_people.push_back(yaml_person);
    }
    std::ofstream file(file_path);
    file << yaml_people;
    file.close();
}


double calc_duration_ms(std::chrono::high_resolution_clock::time_point t1,
    std::chrono::high_resolution_clock::time_point t2) {

    auto duration_us = std::chrono::duration_cast<
        std::chrono::microseconds>(t2 - t1).count();
    return duration_us/1000.0;
}

std::tuple<double, std::vector<Person>> measure_and_repeat_read(
    std::function<std::vector<Person>(std::string)> func,
    std::string file_path) {
    std::vector<double> read_durations_ms;
    auto people = std::vector<Person>();
    for(uint32_t i = 0; i < 3; i++) {
        auto t1 = std::chrono::high_resolution_clock::now();
        people = func(file_path);
        auto t2 = std::chrono::high_resolution_clock::now();
        read_durations_ms.push_back(calc_duration_ms(t1, t2));
    }
    auto min_duration_ms = std::min_element(std::begin(read_durations_ms),
        std::end(read_durations_ms));
    return std::make_tuple(min_duration_ms[0], people);
}

double measure_and_repeat_write(
    std::function<void(std::vector<Person>, std::string)> func,
    std::vector<Person> people,
    std::string file_path) {
    std::vector<double> durations_ms;
    for(uint32_t i = 0; i < 3; i++) {
        auto t1 = std::chrono::high_resolution_clock::now();
        func(people, file_path);
        auto t2 = std::chrono::high_resolution_clock::now();
        durations_ms.push_back(calc_duration_ms(t1, t2));
    }
    auto min_duration_ms = std::min_element(std::begin(durations_ms),
        std::end(durations_ms));
    return min_duration_ms[0];
}

int main(){
    
    auto input_file_dir = std::string("./temp/input_files/");
    auto output_file_dir = std::string("./temp/output_cpp/");
    auto stats_file_dir = std::string("./temp/stats/");

    auto read_funcs = 
            std::vector<std::function<std::vector<Person>(std::string)>>{
        csv_read,
        json_read,
        toml_read,
        xml_read,
        yaml_read,
    };
    
    auto write_funcs =
        std::vector<std::function<void(std::vector<Person>, std::string)>>{
            csv_write,
            json_write,
            toml_write,
            xml_write,
            yaml_write,
        };

    auto extensions = std::vector<std::string>{
        "csv",
        "json",
        "toml",
        "xml",
        "yaml",
    };
    

    std::chrono::high_resolution_clock::time_point t1;
    std::chrono::high_resolution_clock::time_point t2;

    std::vector<double> read_durations_ms;
    std::vector<double> write_durations_ms;
    for(int i = 0; i < extensions.size(); i++) {
        std::cout << "Extension = " << extensions[i] << std::endl;
        auto tuple = measure_and_repeat_read(read_funcs[i],
            input_file_dir + "data." + extensions[i]);
        read_durations_ms.push_back(std::get<0>(tuple));
        std::cout << "Read duration (ms) = " << read_durations_ms[i] << std::endl;

        auto duration_ms = measure_and_repeat_write(
            write_funcs[i],
            std::get<1>(tuple),
            output_file_dir + "data." + extensions[i]);
        write_durations_ms.push_back(duration_ms);
        std::cout << "Write duration (ms) = " << write_durations_ms[i] << std::endl;
    }

    // Write out stats

    std::string stats_file_path = stats_file_dir + "cpp_stats.csv";
    std::cout << "Writing stats to " << stats_file_path << std::endl;
    std::ofstream stats_file;
    stats_file.open(stats_file_dir + "cpp_stats.csv");
    stats_file << "Format, Read (ms), Write (ms)" << std::endl;
    for(uint32_t i = 0; i < extensions.size(); i++) {
        stats_file << extensions[i] << "," << read_durations_ms[i] << "," <<
            write_durations_ms[i] << std::endl;
    }
    stats_file.close();

}