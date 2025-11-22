#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

struct Config{ 
    int value=0; 
};

Config* cfg=nullptr;
std::once_flag flag;

void init(){ 
    cfg=new Config{}; 
    cfg->value=42; 
    std::cout<<"Initialized once\n"; 
}

void worker(int id){ 
    std::call_once(flag, init); 
    std::cout<<"Worker "<<id<<" sees "<<cfg->value<<"\n"; 
}

int main(){ 
    std::vector<std::thread> ts; 
    for(int i=0;i<6;++i) ts.emplace_back(worker,i);
    for(auto& t:ts) t.join(); delete cfg; 
    return 0;
}
