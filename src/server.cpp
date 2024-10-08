#include "server.h"
#include <regex>
 void  show_wallets(const  Server& server)
 {
 	std::cout << std::string(20, '*') << std::endl;
 	for(const auto& client: server.clients)
 		std::cout << client.first->get_id() <<  " : "  << client.second << std::endl;
 	std::cout << std::string(20, '*') << std::endl;
 }
Server::Server()
{
    clients = std::map<std::shared_ptr<Client>, double> {};
}

std::shared_ptr<Client> Server::add_client(std::string id)
{
    if(get_client(id)!=nullptr)
    {
        std::random_device rd; //作种
        std::default_random_engine e(rd());//初始化随机引擎
        std::uniform_int_distribution<int> distr(1000, 9999);//范围
        id+=std::to_string(distr(e));
    }
    std::shared_ptr addedClient = std::make_shared<Client>(id, *this);
    clients.insert(make_pair(addedClient, 5));
    return addedClient;
}

std::shared_ptr<Client> Server::get_client(std::string id) const
{
    for (const auto pack : clients)
    {
        if(pack.first->get_id()== id)
        return pack.first;
    }
    return nullptr;
}

double Server::get_wallet(std::string id) const
{
    for(const auto pack : clients)
    {
        if(pack.first->get_id()==id)
        return pack.second;
    }
    return 0;
}

bool Server::parse_trx(std::string trx, std::string &sender, std::string &receiver, double &value)
{
    std::regex pattern("([a-zA-Z\\d]+)-([a-zA-Z\\d]+)-(\\d+(\\.\\d+)?)");
    std::smatch matches;
    if(!std::regex_match(trx,matches,pattern))
    throw std::runtime_error("trx error");
    sender=matches[1];
    receiver=matches[2];
    value = std::stod(matches[3]);
    return true;
}

bool Server::add_pending_trx(std::string trx, std::string signature) const
{
    std::string sender,receiver;
    double value;
    if(!parse_trx( trx, sender, receiver, value))
        return false;
    if(get_client(sender)==nullptr|get_client(receiver)==nullptr)
        return false;
    bool authentic = crypto::verifySignature(get_client(sender)->get_publickey(), sender, signature);
    if(!authentic)
        return false;

    if(get_wallet(sender)<value)
        return false;
    pending_trxs.push_back(trx);
    return true;
}
size_t Server::mine()
{   
    std::string mempool;
    for (const auto& trx : pending_trxs) {
        mempool += trx; 
    }
    for (int count = 0; count < 1000; count++) {
    for ( auto& client : clients) {
        size_t nonce = client.first->generate_nonce(); 
        std::string hash = crypto::sha256(mempool + std::to_string(nonce));
        if (hash.substr(0, 10).find("000")!= std::string::npos) { 
            std::cout << "Miner: " << client.first->get_id() << " successfully mined with nonce: " << nonce << '\n';
            double& wallet = client.second;
            wallet += 6.25;
            // 清空待处理交易
            pending_trxs.clear();
            return nonce; // 返回 nonce
        }
    }
    }
    std::cout <<  " failed " << '\n';
    return 0; // 挖矿未成功

}