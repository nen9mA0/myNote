#include <cstdlib>
#include <cstring>
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
    std::set<line_no>& run_query(const std::string &) const;
    std::string text_line(line_no) const;
    void print_map();
    size_t get_length() { return word_map.size(); }

private:
    void store_file(std::ifstream &);
    void build_map();
    std::vector<std::string> lines_of_text;
    std::map<std::string, std::set<line_no>> word_map;

    const char *delim = ":,./\\ \"'()";
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
        char *str = new char[(*i).size()+1];
        strcpy(str, (*i).c_str());
        char *p = strtok(str, delim);
        while (p)
        {
            std::string key(p);
            word_map[key].insert(num);
            p = strtok(NULL, delim);
        }
        delete [] str;
        num += 1;
    }
}

std::set<TextQuery::line_no>& TextQuery::run_query(const std::string &key) const
{
    std::set<TextQuery::line_no>* ret = new std::set<TextQuery::line_no>;
    if (word_map.count(key) > 0)
    {
        *ret = word_map.find(key)->second;
    }
    return *ret;
}

std::string TextQuery::text_line(TextQuery::line_no num) const
{
    return lines_of_text[num];
}



class Query;

class Query_base
{
    friend class Query;
protected:
    virtual ~Query_base() { }
    std::set<TextQuery::line_no> lines;
private:
    virtual std::set<TextQuery::line_no>& eval(const TextQuery &tq) const = 0;
    virtual std::ostream& display(std::ostream&) const = 0;
};

class Query
{
    friend Query operator~(const Query& rhs);
    friend Query operator|(const Query& lhs, const Query& rhs);
    friend Query operator&(const Query& lhs, const Query& rhs);
public:
    Query(const std::string&);
    Query(const Query &rhs): q(rhs.q), use(rhs.use) { *use++; }
    ~Query() { decr_use(); }
    Query operator=(Query&);
    std::set<TextQuery::line_no> eval(const TextQuery& tq) const { return q->eval(tq); }
    std::ostream& display(std::ostream &os) const { return q->display(os); }
private:
    Query(Query_base* qb): q(qb), use(new std::size_t(1)) {  }
    Query_base* q;
    std::size_t* use;
    void decr_use() { *use--; if(*use == 0) { delete q; delete use;} };
};

Query Query::operator=(Query& oper)
{
    *(oper.use)++;
    decr_use();
    use = oper.use;
    q = oper.q;
    return *this;
}

std::ostream& operator<<(std::ostream& os, const Query& q)
{
    return q.display(os);
}


class Word_Query: public Query_base
{
    friend class Query;
public:
    Word_Query(const std::string& str): query_word(str) {  }
    std::set<TextQuery::line_no>& eval(const TextQuery& tq) const { return tq.run_query(query_word); }
    std::ostream& display(std::ostream& os) const { return os << query_word; }
private:
    std::string query_word;
};

Query::Query(const std::string& str): q(new Word_Query(str)), use(new std::size_t(1)) {  }

class NotQuery: public Query_base
{
    friend class Query;
public:
    NotQuery(const Query& oper): q(oper) {  }
    std::set<TextQuery::line_no>& eval(const TextQuery&) const;
    std::ostream& display(std::ostream& os) const { return os << "Not(" << q << ")"; }
private:
    Query q;
};

std::set<TextQuery::line_no>& NotQuery::eval(const TextQuery&) const
{
    
}

class BinaryQuery: public Query_base
{
public:
    BinaryQuery(const Query& lhs, const Query& rhs, char oper): qbl(lhs), qbr(rhs), myoper(oper) {  }
    std::ostream& display(std::ostream& os) const { return os << qbr << myoper << qbl; }
private:
    Query qbr;
    Query qbl;
    char myoper;
};

class AndQuery: public BinaryQuery
{
public:
    AndQuery(const Query& lhs, const Query& rhs): BinaryQuery(lhs, rhs, '&') {  }
    std::set<TextQuery::line_no>& eval(const TextQuery&) const;
};

class OrQuery: public BinaryQuery
{
public:
    OrQuery(const Query& lhs, const Query& rhs): BinaryQuery(lhs, rhs, '|') {  }
    std::set<TextQuery::line_no>& eval(const TextQuery&) const;
};

Query operator~(const Query& rhs)
{
    return new NotQuery(rhs);
}

Query operator&(const Query& lhs, const Query& rhs)
{
    return new OrQuery(lhs, rhs);
}

Query operator|(const Query& lhs, const Query& rhs)
{
    return new AndQuery(lhs, rhs);
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
