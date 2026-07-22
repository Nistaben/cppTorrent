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


const int UDP_PORT = 6000;// destination port all Content Discovery processes listen on
const std::string BROADCAST_IP = "192.168.1.255";  // LAN broadcast address; every device on 192.168.1.x receives this
std::string SHARED_DIR ="shared_data";
//fs::path file_path("example.txt");
sockaddr_in my_addr{};
sockaddr_in sender_addr{};
timeval timeout{};
/* struct timeval timeout;
timeout.tv_sec = 5;      
timeout.tv_usec = 0; */
float last_wipe_time = std::time(nullptr);

std::map<std::string, std::string> ip_to_username;
std::map<std::string, std::string> username_to_ip;
std::map<std::string, std::vector<std::string>> content_dict;

void save_dictionaries(){
 // ""Saves all three dictionaries to the shared_data folder so other modules can read them."""

if (!fs::exists(SHARED_DIR)){
fs::create_directory(SHARED_DIR); // folder olusturduk brahahaha
} 
std::ofstream f("shared_data/ip_user_map.txt");
json jsondata = ip_to_username;
f<< jsondata;

    
    f.close();
std::ofstream f1("shared_data/username_to_ip.txt");
json jsondata1 = username_to_ip;
f1<< jsondata1;

    f1.close();
    std::ofstream f2("shared_data/content_dict.txt");
json jsondata2 = content_dict;
f2<< jsondata2;

    f2.close();
    
}


void wipe_content_dict(){
//  """Clears the content dictionary every 60 seconds so only recent content is shown."""


    time_t current_time = std::time(nullptr);
    if(current_time-last_wipe_time>=60){
        content_dict.clear();
        last_wipe_time = current_time ;
        save_dictionaries();
        std::cout<<"[*] Content dictionary wiped (60 seconds passed)"<<std::endl;
        
   
    
    
}

}

void handle_announcement(std::string data,std::string sender_ip){
    
    
    json jdata; //json::parse_error
    try{jdata =json::parse(data);}   
    

    catch(json::parse_error){return;}
    std::string username = jdata.value("username",""); // .value kullanmadan jdata[] yaparsak username veya chunks yokas direkt type::error exception atipyo bozuyo boyle yoksa bile "" donuyor onu da kontrol etcez
    std::vector<std::string> chunks = jdata.value("chunks", std::vector<std::string>{});
    
    if(username ==""){
        return;
    }
    ip_to_username[sender_ip] = username;        //update IP -> username mapping
    username_to_ip[username] = sender_ip;        //update username -> IP mapping
    for (std::string chunk :chunks){
        if(content_dict.find(chunk) != content_dict.end()){
            //burada kaldik 14 07 2026 content dicte ip addressi zate nvar mi falan filan kotnrol etcez sonra bitti bu fonk
           
            content_dict[chunk];
        }
        if (std::find(content_dict[chunk].begin(), content_dict[chunk].end(), sender_ip) == content_dict[chunk].end()) { //.find ile yapilmiyomus
        
        content_dict[chunk].push_back(sender_ip);
        }
    }
    std::cout<<"[*] "<<username<<" : ";
    for(std::string chunk:chunks){
        std::cout<<chunk<<" ,";
    }
    std::cout<<std::endl;
    save_dictionaries();
   
    
}
// tamamen duzelt recvfromu  // duzeltildi recvfrom maine aktarilacak  handle annoucement sadece json parse kismi olarak kaldi 17/07/2026



//void handle_announcement(?? buffer olarak alcaz herhalde data)
void start_discovery(){
//"""Listens for UDP broadcast announcements and processes them."""
my_addr.sin_family =AF_INET; /// 6 KALDIRILDIR
my_addr.sin_port = htons(UDP_PORT);
my_addr.sin_addr.s_addr = htonl(INADDR_ANY); // her ipyi dinle bu porta geldigi surece

timeout.tv_sec =1;
timeout.tv_usec =0; // buralar degisecek
 int sock = socket(AF_INET, SOCK_DGRAM, 0); // udp socketi olusturdum dgram
 int optval = 1;
 setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));//reuseaddr ile tekarra buraya broadcast gelirse seyiengelliyor Address already in use” error messages when you try to restart your server after a crash. beejs
bind(sock,(sockaddr*)&my_addr,sizeof(my_addr)); // porta bindladik ip kismi bos //htonl(INADDR_ANY);
setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(timeval*)&timeout,sizeof(timeout));// 1 degerini daha atmadik ama 1 saniyede 1 timout aticak ki wipecontentdict i calisitircak miyiz calistirmayacak miyiz kontrol edicez

std::cout<<"--- Chunk Discovery Started, listening on port "<<UDP_PORT<<" ---"<<std::endl;

socklen_t sender_addrLen = sizeof(sender_addr);

//try catchli kisim gelecek burada saliyorum
char buffer[65535];
char csenderip[65535];
while(true){
    wipe_content_dict();
     
        ssize_t n = recvfrom(sock,
            buffer,
            sizeof(buffer),
            0,
            (struct sockaddr*)&sender_addr,
            &sender_addrLen);
            if(n !=-1){
            std::string data(buffer,n);
            getnameinfo((struct sockaddr*)&sender_addr,sender_addrLen,csenderip,sizeof(csenderip),nullptr,0,NI_NUMERICHOST);
            std::string sender_ip(csenderip);
            handle_announcement(data,sender_ip);};
            
    
}
close(sock);

}
int main(){
    start_discovery();
}





