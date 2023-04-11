#include <fstream>
#include <sstream>

#include "Config.h"
#include "String.h"

using namespace std;

namespace utility {
Config::Config(const string& filePath)
    : m_key_values{}, file_path {filePath} {
    if (!file_path.empty()) {
        load(file_path);
    }

}

bool Config::load(const string& filePath) {
    if (filePath.empty()) {
        return false;
    }

    ifstream f(widen(filePath));

    if (!f) {
        return false;
    }

    for (string line{}; getline(f, line);) {
        istringstream ss{line};
        string key{};
        string value{};

        getline(ss, key, '=');
        getline(ss, value);

        set(key, value);
    }

    return true;
}

bool Config::save(const string& filePath) {
    ofstream f(widen(filePath));

    if (!f) {
        return false;
    }

    for (auto& keyValue : m_key_values) {
        f << keyValue.first << "=" << keyValue.second << endl;
    }

    return true;
}

optional<string> Config::get(const string& key) const {
    auto search = m_key_values.find(key);

    if (search == m_key_values.end()) {
        return {};
    }

    return search->second;
}

void Config::set(const string& key, const string& value) {
    if (!key.empty() && !value.empty()) {
        m_key_values[key] = value;
    }
}
} // namespace utility