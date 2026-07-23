#include <iostream>      // cout, cerr için
#include <string>        // std::string için
#include <filesystem>    // os.path.exists yerine kullanılır (C++17)
#include <cmath>         // math modülünün karşılığı (ceil için)
#include <fstream>
// macOS BSD socket header'ları (sistemin kendi socket API'si):
#include <sys/socket.h>  // socket(), bind(), sendto() vs.
#include <netinet/in.h>  // sockaddr_in, INADDR_BROADCAST gibi yapılar
#include <arpa/inet.h>   // inet_addr(), inet_pton() gibi IP dönüşüm fonksiyonları
#include <unistd.h>         // close() — soketi kapatmak için
#include <vector>
#include <nlohmann/json.hpp>
#include <ctime>
#include <algorithm> // ::find icin
#include <netdb.h>

namespace fs = std::filesystem;
using json = nlohmann::json;

const int TCP_PORT = 6001;
const int p = 31; // Alice's private key = a Bob's priivate key = b // A=g^a mod p //B = g^b mod p then they share A and B Then Key = B^a mod p == A^b mod p inanilmaz
const int g = 7;
std::string SHARED_DIR ="shared_data";
std::string UPLOAD_LOG ="upload_log.txt";
std::string IP_USER_FILE = SHARED_DIR + "ip_user_map.txt";


json load_ip_user_map(){
///"""Reads the IP→username map from disk and returns it as a dict."""
    if(!fs::exists(IP_USER_FILE)){
        return{};
    }
    std::ifstream f(IP_USER_FILE);
    if(!f.is_open()){
        return {};
    }
    json data = json::parse(f);
    
    return data; /// bu boyle calisiyormus cok ilginc json olarak return edebiliyoz hashmapi

    
}
std::string resolve_username(std::string ip){
    //  """Returns the username for a given IP, falling back to the raw IP if not found."""
    std::string username = load_ip_user_map().value(ip,ip); //This is equivalent to Python's dict.get(key, default). "" https://json.nlohmann.me/api/basic_json/value/
    return load_ip_user_map();// buarada kalindi yapilacak belki hashmap degil direkt  json olarak dondurur .value() ile hallederiz
}
// std::string get_des_key(int shared_secret_int){} buna gerek yok cpp stringi zaten byte direkt
void log_upload(std::string chunk_name,std::string recipient_ip){
    // """Appends a SENT entry to the upload log file for the given chunk."""

    std::time_t now = std::time(nullptr);
    std::string timeanddate = std::asctime(std::localtime(&now));
    std::cout<<timeanddate;
    return;
}




int main(){
    log_upload("asda","12312");
    
}
