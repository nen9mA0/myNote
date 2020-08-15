#include <iostream>
#include <set>
#include <string>
#include <cstdio>
#include <sstream>

class Message;

class Folder
{
public:
    Folder(std::string folder_name): name(folder_name), father(nullptr) {  }
    Folder(Folder* folder_father, std::string folder_name);
    ~Folder();

    void Tree(int);
    Folder* Find(const std::string&);
    Folder* GetFather();

    std::string name;
    std::set<Message*> msg_set;
    bool insert_success;
private:
    std::set<Folder*> sub_folder;
    Folder* father;
};

class Message
{
public:
    Message(const std::string str = ""): contents(str) {  }
    Message(const Message&);
    Message& operator=(const Message&);
    ~Message();

    void save(Folder&);
    void remove(Folder&);
    void PntFolder();

private:
    friend void Folder::Tree(int);
    std::string contents;
    std::set<Folder*> folders;
    void put_Msg_in_Folders(const std::set<Folder*>&);
    void remove_Msg_from_Folders();
};



Folder::Folder(Folder* folder_father, std::string folder_name): father(folder_father), name(folder_name), insert_success(true)
{
    bool have_same_name = false;
    for(std::set<Folder*>::iterator i=folder_father->sub_folder.begin(); i!=folder_father->sub_folder.end(); i++)
    {
        if((*i)->name == folder_name)
        {
            have_same_name = true;
            insert_success = false;
        }
    }

    if(!have_same_name)
    {
        std::pair<std::set<Folder*>::iterator, bool> ret;
        ret = folder_father->sub_folder.insert(this);
        if(ret.second != true)
            insert_success = false;
    }
}

Folder::~Folder()
{
    std::set<Folder*>::iterator iter = father->sub_folder.find(this);
    if(iter != father->sub_folder.end())
    {
        father->sub_folder.erase(this);
    }
}

void Folder::Tree(int depth=0)
{
    std::cout << name << std::endl;
    for(std::set<Folder*>::iterator i=sub_folder.begin(); i!=sub_folder.end(); i++)
    {
        Folder* tmp = *i;
        for(int j=0; j<depth; j++)
        {
            std::cout << "|  ";
        }
        std::cout << "|-";
        (*tmp).Tree(depth+1);
    }
    for(std::set<Message*>::iterator i=msg_set.begin(); i!=msg_set.end(); i++)
    {
        for(int j=0; j<depth; j++)
        {
            std::cout << "|  ";
        }
        std::cout << "|--" << *i << "--" << (*i)->contents << std::endl;
    }
}

Folder* Folder::Find(const std::string& find_name)
{
    Folder* ret = nullptr;
    if(name == find_name)
        ret = this;
    else
    {
        for(std::set<Folder*>::iterator i=sub_folder.begin(); i!=sub_folder.end(); i++)
        {
            Folder* tmp = *i;
            ret = (*tmp).Find(find_name);
            if(ret != nullptr)
                break;
        }
    }
    return ret;
}

void Message::PntFolder()
{
    for(std::set<Folder*>::iterator i=folders.begin(); i!=folders.end(); i++)
        std::cout << (*i)->name << std::endl;
}

Folder* Folder::GetFather()
{
    return father;
}

Message::Message(const Message& rhs): contents(rhs.contents), folders(rhs.folders)
{
    //==== It's better to use Initialize list ====
    //contents = rhs.contents;
    //folders.insert(rhs.folders.begin(), rhs.folders.end());
    put_Msg_in_Folders(folders);
}

Message& Message::operator=(const Message& rhs)
{
    if(&rhs != this)    //It's important to determine whether rhs==this
    {
        remove_Msg_from_Folders();
        contents = rhs.contents;
        //==== We can simply use operator= to copy a std::set ====
        //folders.clear();
        //folders.insert(rhs.folders.begin(), rhs.folders.end());
        folders = rhs.folders;
        put_Msg_in_Folders(folders);
    }
    return *this;
}

Message::~Message()
{
    remove_Msg_from_Folders();
}

void Message::put_Msg_in_Folders(const std::set<Folder*>& myfolders)
{
    for(std::set<Folder*>::iterator i = myfolders.begin(); i!=myfolders.end(); i++)
    {
        Folder* tmp = *i;
        tmp->msg_set.insert(this);
    }
}

void Message::save(Folder& dst_folder)
{
    folders.insert(&dst_folder);
    dst_folder.msg_set.insert(this);
}

void Message::remove(Folder& dst_folder)
{
    folders.erase(&dst_folder);
    dst_folder.msg_set.erase(this);
}

void Message::remove_Msg_from_Folders()
{
    for(std::set<Folder*>::iterator i = folders.begin(); i!=folders.end(); i++)
    {
        Folder* tmp = *i;
        tmp->msg_set.erase(this);
    }
}

int main()
{
    Folder* root = new Folder("/");
    
    for(int i=0; i<5; i++)
    {
        char name[5] = {0};
        name[0] = '0'+ i; 
        std::string name_str(name);
        Folder* tmp = new Folder(root, name_str);

        for(int j=0; j<3; j++)
        {
            name[1] = '0' + j;
            name_str = std::string(name);
            Folder* sub = new Folder(tmp, name_str);
        }
    }


    root->Tree();
    
    std::string input;
    Folder* current_folder(root);
    while(1)
    {
        std::cout << "cmd: ";
        std::getline(std::cin, input);

        if(input == "q")
        {
            break;
        }
        else if(input == "tree")
        {
            current_folder->Tree();
        }
        else if(input.compare(0, 2, "cd") == 0)
        {
            Folder* tmp;
            std::string new_folder = input.substr(3);
            if(new_folder.compare("..") == 0)
            {
                tmp = current_folder->GetFather();
            }
            else
            {
                tmp = current_folder->Find(new_folder);
            }
            if(tmp != nullptr)
                current_folder = tmp;
        }
        else if(input.compare(0, 5, "mkdir") == 0)
        {
            Folder* tmp = new Folder(current_folder, input.substr(6));
            if(!tmp->insert_success)
                delete tmp;
        }
        else if(input.compare(0, 5, "mkmsg") == 0)
        {
            Message* msg = new Message(input.substr(6));
            msg->save(*current_folder);
        }
        else if(input.compare(0, 5, "cpmsg") == 0)
        {
            Message* pmsg;
            size_t tmp;
            std::istringstream is(input.substr(6));

            is >> std::hex >> tmp;
            pmsg = (Message*)tmp;
            for(std::set<Message*>::iterator i=current_folder->msg_set.begin(); i!=current_folder->msg_set.end(); i++)
            {
                if(pmsg == *i)
                {
                    Message *msg_cp = new Message(*pmsg);
                    break;
                }
            }
        }
        else if(input.compare(0, 6, "chgmsg") == 0)
        {
            Message *pmsg1(nullptr), *pmsg2(nullptr);
            size_t src1, src2;
            std::istringstream is(input.substr(7));

            is >> std::hex >> src1 >> src2;
            for(std::set<Message*>::iterator i=current_folder->msg_set.begin(); i!=current_folder->msg_set.end(); i++)
            {
                if((Message*)src1 == *i || (Message*)src2 == *i)
                {
                    if((Message*)src1 == *i)
                        pmsg1 = (Message*)src1;
                    else
                        pmsg2 = (Message*)src2;
                }
            }
            if(pmsg1!=nullptr && pmsg2!=nullptr)
                *pmsg1 = *pmsg2;
        }
        else if(input.compare(0, 6, "pntmsg") == 0)
        {
            Message* pmsg;
            size_t tmp;
            std::istringstream is(input.substr(6));

            is >> std::hex >> tmp;
            pmsg = (Message*)tmp;
            for(std::set<Message*>::iterator i=current_folder->msg_set.begin(); i!=current_folder->msg_set.end(); i++)
            {
                if(pmsg == *i)
                {
                    (*pmsg).PntFolder();
                    break;
                }
            }
        }
        else if(input.compare(0, 5, "mvmsg") == 0)
        {
            Message *pmsg(nullptr);
            size_t src;
            std::string dst;
            Folder* dst_folder;
            std::istringstream is(input.substr(6));

            is >> std::hex >> src >> dst;

            pmsg = (Message*)src;
            for(std::set<Message*>::iterator i=current_folder->msg_set.begin(); i!=current_folder->msg_set.end(); i++)
            {
                if(pmsg == *i)
                {
                    dst_folder = current_folder->Find(dst);
                    if(dst_folder != nullptr)
                    {
                        pmsg->remove(*current_folder);
                        pmsg->save(*dst_folder);
                    }
                    break;
                }
            }
        }
        else if(input.compare(0, 2, "ln") == 0)
        {
            Message *pmsg(nullptr);
            size_t src;
            std::string dst;
            Folder* dst_folder;
            std::istringstream is(input.substr(3));

            is >> std::hex >> src >> dst;

            pmsg = (Message*)src;
            for(std::set<Message*>::iterator i=current_folder->msg_set.begin(); i!=current_folder->msg_set.end(); i++)
            {
                if(pmsg == *i)
                {
                    dst_folder = current_folder->Find(dst);
                    if(dst_folder != nullptr)
                    {
                        pmsg->save(*dst_folder);
                    }
                    break;
                }
            }
        }
        else
        {
            std::cout << "Invaild Command" << std::endl;
        }
        
    }
}