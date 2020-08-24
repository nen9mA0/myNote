#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

class TextQuery
{
public:
    typedef std::vector<std::string>::size_type line_no;
    void read_file(std::ifstream &is);
    std::set<line_no>* run_query(const std::string &) const;
    std::string text_line(line_no) const;
    void print_map();

private:
    void store_file(std::ifstream &);
    void build_map();
    std::vector<std::string> lines_of_text;
    std::map<std::string, std::set<line_no>> word_map;
};

void TextQuery::print_map()
{
    for (std::map<std::string, std::set<line_no>>::iterator i = word_map.begin(); i != word_map.end(); i++)
    {
        std::cout << i->first << " :" << std::endl;
        for (std::set<line_no>::iterator j = i->second.begin(); j != i->second.end(); j++)
        {
            std::cout << "\t" << *j << std::endl;
        }
    }
}

void TextQuery::read_file(std::ifstream &is)
{
    store_file(is);
    build_map();
}

void TextQuery::store_file(std::ifstream &is)
{
    std::string tmp;
    while (std::getline(is, tmp))
    {
        lines_of_text.push_back(tmp);
    }
}

void TextQuery::build_map()
{
    line_no num = 1;
    for (std::vector<std::string>::iterator i = lines_of_text.begin(); i != lines_of_text.end(); i++)
    {
        std::stringstream tmp(*i);
        std::string key;
        while (tmp >> key)
        {
            word_map[key].insert(num);
        }
        num += 1;
    }
}

std::set<TextQuery::line_no>* TextQuery::run_query(const std::string &key) const
{
    std::set<TextQuery::line_no>* ret = new std::set<TextQuery::line_no>;
    if (word_map.count(key) > 0)
    {
        *ret = word_map.find(key)->second;
        return ret;
    }
    else
    {
        return nullptr;
    }
}

std::string TextQuery::text_line(TextQuery::line_no num) const
{
    return lines_of_text[num];
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "input filename" << std::endl;
        exit(0);
    }

    std::ifstream filein(argv[1]);
    if (!filein)
    {
        std::cout << "Open file failed" << std::endl;
        exit(0);
    }

    TextQuery tq;
    std::string key;

    tq.read_file(filein);
    filein.close();

    //tq.print_map();
    while (1)
    {
        std::cout << "Input Word To Query:" << std::endl;
        std::cin >> key;

        std::set<TextQuery::line_no>* query = tq.run_query(key);

        if (query != nullptr)
        {
            std::cout << key << " occurs " << (*query).size() << std::endl;
            for (std::set<TextQuery::line_no>::iterator i = query->begin(); i != query->end(); i++)
            {
                std::cout << *i << std::endl;
                //std::cout << "(line " << *i << ")  \t" << tq.text_line(*i) << std::endl;
            }
        }
        delete query;
    }
}
