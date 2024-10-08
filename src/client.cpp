#include "client.h"

Client::Client(std::string id, const Server& server):id(std::move(id)),server(&server)
{
    crypto::generate_key(public_key,private_key);
}

std::string Client::get_id()
{
    return id;
}

std::string Client::get_publickey() const
{
    return public_key;
}

double Client::get_wallet()
{
    return  server->get_wallet(id);
}

std::string Client::sign(std::string txt) const
{
    return crypto::signMessage(private_key, txt);
}

bool Client::transfer_money(std::string receiver, double value)
{
    if(	server->get_client(receiver)==nullptr)
        return false;
    std::string trx;
    trx=id+"-"+receiver+"-"+std::to_string(value);
    if(!server->add_pending_trx(trx, sign(id)))
        return false;
    
    return true;
}

size_t Client::generate_nonce()
{
    std::random_device rd;
    std::default_random_engine e(rd());
    std::uniform_int_distribution<int> distr(10,999);
    return distr(e);
}